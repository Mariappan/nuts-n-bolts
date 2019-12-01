import asyncio
import consul
from logging import getLogger

CONSUL_HOST = '192.168.0.2'


class Heartbeat:
    def __init__(self, loop=None):
        self.loop = loop or asyncio.get_running_loop()
        self.logger = getLogger("TestConsul")
        self.con = None
        self.sid = None
        self.session_task = self.loop.create_task(self._renew_session())
        self.leader_task = self.loop.create_task(self._elect_leader())

    async def _renew_session(self):
        while True:
            if self.con:
                if self.sid is None:
                    self.sid = self.con.session.create(name="FirstCli", behavior='release', lock_delay=0, ttl=10)
                else:
                    self.con.session.renew(self.sid)

            await asyncio.sleep(10)

    async def _elect_leader(self):
        while True:
            # print("Enter the leader election")
            if not self.con:
                await asyncio.sleep(10)
                continue
            try:
                if self.con.kv.put(key="db/canary/leader", value=None, acquire=self.sid):
                    print("Acquired the key")
                else:
                    print("Not able to acquire the key")
            except Exception as err:
                print("Thrown exception as ", err)

            await asyncio.sleep(10)

    def start(self):
        self.con = consul.Consul(host=CONSUL_HOST)

    def __str__(self):
        return f"TestConsul: inited"


async def main():
    print('Entering main app...(Dummy app)')
    while True:
        await asyncio.sleep(36000)


def async_main():
    loop = asyncio.get_event_loop()
    print("Starting Event loop")
    hb = Heartbeat(loop)
    hb.start()
    loop.run_until_complete(main())


if __name__ == "__main__":
    print("Starting mod:", __name__)
    async_main()
else:
    print("Starting other mod:", __name__)