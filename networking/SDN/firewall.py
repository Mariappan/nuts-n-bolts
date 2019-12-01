# Copyright (C) 2013 Nippon Telegraph and Telephone Corporation.
# Copyright (C) 2015 Brad Cowie, Christopher Lorier and Joe Stringer.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys, struct, copy, logging, socket, os, signal

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller import dpset
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3, ether
from ryu.lib import ofctl_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import vlan
from ryu.lib.dpid import str_to_dpid
from ryu.lib import hub
from operator import attrgetter

class Firewall(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    _CONTEXTS = {'dpset': dpset.DPSet}


    def __init__(self, *args, **kwargs):
        super(Firewall, self).__init__(*args, **kwargs)

        # Create dpset object for querying Ryu's DPSet application
        self.dpset = kwargs['dpset']

        self.cookie = 0x40890
        # Initialise datastructures to store state
        self.mac_to_port = {}
        self.dps = {}

    def fix_acls(self, conf):
        # Recursively walk config replacing all acls with ACL objects
        for k, v in conf.items():
            if k == 'acls':
                if isinstance(v, dict):
                    for ip, acls in v.items():
                        conf[k][ip] = [ACL(x['match'], x['action']) for x in acls]
                else:
                    conf[k] = [ACL(x['match'], x['action']) for x in v]
            elif isinstance(v, dict):
                self.fix_acls(v)

    def clear_flows(self, datapath, cookie):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        match = parser.OFPMatch()
        mod = parser.OFPFlowMod(
            datapath=datapath, cookie=cookie, cookie_mask=0xFFFFFFFFFFFFFFFF,
            command=ofproto.OFPFC_DELETE, out_port=ofproto.OFPP_ANY,
            out_group=ofproto.OFPG_ANY, match=match, instructions=[])
        datapath.send_msg(mod)

    def add_flow(self, datapath, match, actions, priority, cookie):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                                                    actions)]
        mod = parser.OFPFlowMod(
            datapath=datapath, cookie=cookie, priority=priority,
            command=ofproto.OFPFC_ADD, match=match, instructions=inst)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        msg = ev.msg
        dp = msg.datapath
        ofproto = dp.ofproto
        parser = dp.ofproto_parser

        pkt = packet.Packet(msg.data)
        ethernet_proto = pkt.get_protocols(ethernet.ethernet)[0]

        src = ethernet_proto.src
        dst = ethernet_proto.dst
        eth_type = ethernet_proto.ethertype

        in_port = msg.match['in_port']
        self.mac_to_port.setdefault(dp.id, {})


        self.logger.info("MARI => Ethproto: %s", ethernet_proto)

        if dp.id not in self.dps:
            self.logger.error("Packet_in on unknown datapath")
            return
        else:
            datapath = self.dps[dp.id]

        if not datapath.running:
            self.logger.error("Packet_in on unconfigured datapath")

        if in_port not in datapath.ports:
            return

        if eth_type == ether.ETH_TYPE_8021Q:
            # tagged packet
            vlan_proto = pkt.get_protocols(vlan.vlan)[0]
            vid = vlan_proto.vid
            if Port(in_port, 'tagged') not in datapath.vlans[vid].tagged:
                self.logger.warn("HAXX:RZ in_port:%d isn't tagged on vid:%s" % \
                    (in_port, vid))
                return
        else:
            # untagged packet
            vid = datapath.get_native_vlan(in_port).vid
            if not vid:
                self.logger.warn("Untagged packet_in on port:%d without native vlan" % \
                        (in_port))
                return

        self.mac_to_port[dp.id].setdefault(vid, {})

        self.logger.info("Packet_in src:%s dst:%s in_port:%d vid:%s",
                         src, dst, in_port, vid)

        # learn a mac address to avoid FLOOD next time.
        self.mac_to_port[dp.id][vid][src] = in_port

        # generate the output actions for broadcast traffic
        tagged_act = self.tagged_output_action(parser, datapath.vlans[vid].tagged)
        untagged_act = self.untagged_output_action(parser, datapath.vlans[vid].untagged)

        matches = []
        action = []
        if datapath.ports[in_port].is_tagged():
            # send rule for matching packets arriving on tagged ports
            strip_act = [parser.OFPActionPopVlan()]
            if tagged_act:
                action += tagged_act
            if untagged_act:
                action += strip_act + untagged_act

            # match multicast/broadcast
            matches.append(parser.OFPMatch(vlan_vid=vid|ofproto_v1_3.OFPVID_PRESENT,
                    in_port=in_port,
                    eth_src=src,
                    eth_dst=('01:00:00:00:00:00',
                             '01:00:00:00:00:00')))
        elif datapath.ports[in_port].is_untagged():
            # send rule for each untagged port
            push_act = [
              parser.OFPActionPushVlan(ether.ETH_TYPE_8021Q),
              parser.OFPActionSetField(vlan_vid=vid|ofproto_v1_3.OFPVID_PRESENT)
              ]
            if untagged_act:
                action += untagged_act
            if tagged_act:
                action += push_act + tagged_act

            # match multicast/broadcast
            matches.append(parser.OFPMatch(in_port=in_port,
                    eth_src=src,
                    eth_dst=('01:00:00:00:00:00',
                             '01:00:00:00:00:00')))

        # install broadcast/multicast rules onto datapath
        if datapath.config_default['smart_broadcast']:
            for match in matches:
                priority = datapath.config_default['high_priority'] + 10
                cookie = datapath.config_default['cookie']
                self.add_flow(dp, match, action, priority, cookie)

        # install unicast flows onto datapath
        if dst in self.mac_to_port[dp.id][vid]:
            # install a flow to avoid packet_in next time
            out_port = self.mac_to_port[dp.id][vid][dst]
            if out_port == in_port:
                self.logger.info("in_port is the same as out_port, skipping unicast flow " \
                        "dl_dst:%s dl_src:%s vid:%d", dst, src, vid)
                return

            self.logger.info("Adding unicast flow dl_dst:%s vid:%d", dst, vid)

            actions = []

            if datapath.ports[in_port].is_tagged():
                match = parser.OFPMatch(
                    in_port=in_port,
                    eth_src=src,
                    eth_dst=dst,
                    vlan_vid=vid|ofproto_v1_3.OFPVID_PRESENT)
                if datapath.ports[out_port].is_untagged():
                    actions.append(parser.OFPActionPopVlan())
            if datapath.ports[in_port].is_untagged():
                match = parser.OFPMatch(
                    in_port=in_port,
                    eth_src=src,
                    eth_dst=dst)
                if datapath.ports[out_port].is_tagged():
                    actions.append(parser.OFPActionPushVlan())
                    actions.append(parser.OFPActionSetField(vlan_vid=vid|ofproto_v1_3.OFPVID_PRESENT))
            actions.append(parser.OFPActionOutput(out_port))

            priority = datapath.config_default['high_priority']
            cookie = datapath.config_default['cookie']
            self.add_flow(dp, match, actions, priority, cookie)

    @set_ev_cls(dpset.EventDP, dpset.DPSET_EV_DISPATCHER)
    def handler_datapath(self, ev):
        dp = ev.dp
        ofproto = dp.ofproto
        parser = dp.ofproto_parser

        if ev.enter:
            self.logger.info("MARI => Switch connected:%s", dp.id)
        else:
            self.logger.info("MARI => Switch disconnected:%s", dp.id)
            return

        self.logger.info("Configuring datapath")

        # clear flow table on datapath
        self.clear_flows(dp, self.cookie)

        # add catchall drop rule to datapath
        drop_act = []

        aclmatch = {}
        aclmatch['eth_type'] = 0x0800
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, match, drop_act, 10000, self.cookie)
        self.logger.info("Configured match")

        self.logger.info("Datapath configured")
