from dataclasses import dataclass, asdict, field, InitVar
import msgpack
import datetime

@dataclass()
class HeartbeatMessage:
    A: str = ""
    B: bool = False
    environment: InitVar[str]  = field(repr=False, default="")
    prometheus: InitVar[bool]  = field(repr=False, default=False)

    def __post_init__(self, environment, prometheus):
        self.A = environment
        self.B = prometheus

    @property
    def environment(self):
        self.A

m = HeartbeatMessage(environment="dev-canary", prometheus=True)
print (m)
print (f"M as dict {asdict(m)} and env is {m.environment} {m.A}")

packed = msgpack.packb(asdict(m), use_bin_type=True)
print(f"== MAARI ==> packed {packed}")

unpacked = msgpack.unpackb(packed, raw=False)
print(f"== MAARI ==> unpacked {unpacked}")
print("=========================================")

try:
    print(msgpack.unpackb("heartbeat", raw=False))
except Exception:
    print("Unpacking failed")



@dataclass(frozen=True)
class HeartbeatMessage1:
    environment: str
    prometheus: str

    @classmethod
    def deserialize(cls, packed_data: bytes):
        unpacked_data = msgpack.unpackb(packed_data, raw=False)
        return HeartbeatMessage(**unpacked_data)

    def serialize(self) -> bytes:
        packed_data = msgpack.packb(asdict(self), use_bin_type=True)
        return packed_data

hb = HeartbeatMessage1("dev-canary".encode().decode(), '{"status":"failure"}')
prom = json.loads(hb.prometheus)
print(prom, prom['status'])


HeartbeatMessage1.deserialize("dev-canary".encode())

# from datetime import date
# from marshmallow import Schema, fields, pprint
#
# class HeartbeatSchema(Schema):
#     environment = fields.Str()
#     prom_health = fields.Bool(default=True)
#
# schema = HeartbeatSchema()
# result = schema.dump(m)
# pprint(result, indent=2)


# useful_dict = {
#     "id": 1,
#     "created": datetime.datetime.now(),
# }
#
# def decode_datetime(obj):
#     if b'__datetime__' in obj:
#         obj = datetime.datetime.strptime(obj["as_str"], "%Y%m%dT%H:%M:%S.%f")
#     return obj
#
# def encode_datetime(obj):
#     if isinstance(obj, datetime.datetime):
#         return {'__datetime__': True, 'as_str': obj.strftime("%Y%m%dT%H:%M:%S.%f")}
#     return obj
#

# packed_dict = msgpack.packb(useful_dict, default=encode_datetime, use_bin_type=True)
# this_dict_again = msgpack.unpackb(packed_dict, object_hook=decode_datetime, raw=False)
# print(this_dict_again)
#
# def decode_hb(obj):
#     print(f"== MAARI ==> decodehb {obj}, {type(obj)}")
#     return obj
#
# def encode_hb(obj):
#     print(f"== MAARI ==> encodehb {obj}, {type(obj)}")
#     return obj.environment + str(obj.prom_health)
#
# packed = msgpack.packb(asdict(m), use_bin_type=True)
# print(f"== MAARI ==> packed {packed}")
#
# unpacked = msgpack.unpackb(packed, raw=False)
# print(f"== MAARI ==> unpacked {unpacked}")
