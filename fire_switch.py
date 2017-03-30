################################################################
#
#                       F I R E W A L L
#
################################################################

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller import dpset
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3, ether
from ryu.lib import ofctl_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ipv4
from ryu.lib.packet import in_proto
from ryu.lib.packet import udp
from ryu.lib.packet import tcp
from ryu.lib.packet import ether_types


class FireSwitch(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    _CONTEXTS = {'dpset': dpset.DPSet}

    def __init__(self, *args, **kwargs):
        super(FireSwitch, self).__init__(*args, **kwargs)
        self.mac_to_port = {}

        self.tcp_seq = {}

        self.cookie = 40890

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']

        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]

        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            return

        dst = eth.dst
        src = eth.src

        dpid = datapath.id
        self.mac_to_port.setdefault(dpid, {})

        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        if eth.ethertype == ether_types.ETH_TYPE_IP:
            ip = pkt.get_protocols(ipv4.ipv4)[0]
            self.logger.info("MARI => IPv4: %s %s %s", ip.proto, ip.total_length, ip.header_length)

            if ip.proto == in_proto.IPPROTO_UDP:
                udp_hdr = pkt.get_protocols(udp.udp)[0]
                # Dropping UDP packet of size greater than 548
                if (udp_hdr.total_length > 548):
                    self.logger.info("Dropping UDP packets size:%s",\
                            udp_hdr.total_length)
                    return

                # Dropping DNS packet of size greater than 548
                if (udp_hdr.src_port == 53 or udp_hdr.dst_port == 53):
                    self.logger.info("Redirected and dropped DNS packets ")
                    return

            if ip.proto == in_proto.IPPROTO_TCP:
                self.logger.info("MARI => TCP packets")
                tcp_hdr = pkt.get_protocols(tcp.tcp)[0]

                if (ip.src, ip.dst, tcp_hdr.src_port, tcp_hdr.dst_port) not in self.tcp_seq:
                    self.tcp_seq[(ip.src, ip.dst, tcp_hdr.src_port, tcp_hdr.dst_port)] = {}
                    self.tcp_seq[(ip.src, ip.dst, tcp_hdr.src_port, tcp_hdr.dst_port)]['seq'] = tcp_hdr.seq
                    self.tcp_seq[(ip.src, ip.dst, tcp_hdr.src_port, tcp_hdr.dst_port)]['next_seq'] = \
                            self.tcp_seq.seq + (ip.total_length - ip.header_length)
                else:
                    pass

        # learn a mac address to avoid FLOOD next time.
        self.mac_to_port[dpid][src] = in_port

        if dst in self.mac_to_port[dpid]:
            out_port = self.mac_to_port[dpid][dst]
        else:
            out_port = ofproto.OFPP_FLOOD

        actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD:
            match = parser.OFPMatch(in_port=in_port, eth_dst=dst)
            # verify if we have a valid buffer_id, if yes avoid to send both
            # flow_mod & packet_out
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow(datapath, 1, match, actions, msg.buffer_id)
                return
            else:
                self.add_flow(datapath, 1, match, actions)
        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)

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

        # add catchall drop rule to datapath
        drop_act = []

        # Drop IPv6 packets
        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IPV6
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 10000, match, drop_act)

        # Drop ICMP packets
        # aclmatch = {}
        # aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        # aclmatch['ip_proto'] = 0x1
        # match = ofctl_v1_3.to_match(dp, aclmatch)
        # self.add_flow(dp, 10000, match, drop_act)

        act_redirect = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER)]

        # Redirect UDP and TCP packets
        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_UDP
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_TCP
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        # Redirect DNS packets
        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_UDP
        aclmatch['udp_src'] = 53
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_UDP
        aclmatch['udp_dst'] = 53
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        # Redirect SNMP packets
        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_UDP
        aclmatch['udp_src'] = 161
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_UDP
        aclmatch['udp_dst'] = 161
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_TCP
        aclmatch['tcp_src'] = 161
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        aclmatch = {}
        aclmatch['eth_type'] = ether_types.ETH_TYPE_IP
        aclmatch['ip_proto'] = in_proto.IPPROTO_TCP
        aclmatch['tcp_dst'] = 161
        match = ofctl_v1_3.to_match(dp, aclmatch)
        self.add_flow(dp, 1000, match, act_redirect)

        self.logger.info("Datapath configured")
