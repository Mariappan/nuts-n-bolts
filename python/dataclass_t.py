from dataclasses import dataclass
from typing import Optional, Any, Type, TypeVar, Tuple
import json
import dataclasses

import asyncio
import dataclasses
import inspect
import random
import time
from contextlib import AbstractContextManager
from enum import Enum
from functools import wraps
from typing import Any, Callable, Tuple, Type, TypeVar, Union

T = TypeVar("T")

def _get_types_to_check(field_type: Type) -> Tuple[Type, ...]:
    """Helper function to get ordered tuple of types to check for as_dataclass()
       field conversion.
    """
    # NOTE: __origin__ is the best way to get the original type for now:
    # https://github.com/python/typing/issues/136#issuecomment-138392956
    if hasattr(field_type, "__origin__"):  # field_type is a template
        if field_type.__origin__ == Union:
            # Return all union types (flattened with sum())
            return sum((_get_types_to_check(t) for t in field_type.__args__), ())

        # field_type is some other generic, incl. List, Dict, etc.
        return (field_type.__origin__,)

    return (field_type,)

def _cast_dataclass_field_value(obj: Any, field_name: str, field_type: Type[T], **kwargs) -> T:
    """Helper function to convert an object's field with some given name to the given type.

    Notes on conversion:
    - If field_type is a Dataclass, conversion uses duck typing by recursing on the field.
    - If field_type is an enum, conversion succeeds if the enum object can be initialised using
      the field value.
    - If field_type is a Union, conversion is attempted for each of the type parameters,
      in the same order as the Union definition (nested Unions are flattened). The first
      successful conversion is returned.
    - If field_type is some other Generic (e.g. List[T], Dict[T, U]), type parameters are not
      checked, and the field is returned if it matches the origin type (e.g. List, Dict
      respectively) using isinstance().
    - For other field_types, conversion succeeds if isinstance(value, field_type) is True.
    - kwarg overrides are type checked against field_type.

    :param Any obj: Object to cast
    :param str field_name: Field name
    :param Type[T] field_type: Field type
    :param kwargs: Fields => override values; will not override fields of nested dataclasses
    :raises ValueError: Failed to convert field to given type
    :return T: Casted object
    """
    types_to_check = _get_types_to_check(field_type)
    field_value = obj.get(field_name, getattr(obj, field_name, None))
    field_value = kwargs.get(field_name, field_value)


    # print(f" == MAARI == => Fieldname:{field_name} type:{types_to_check} attrs:{obj}")


    for possible_type in types_to_check:
        # print(f" == MAARI == Possible type Loop: {possible_type} {obj[field_name]} {dataclasses.is_dataclass(possible_type)} ")

        if dataclasses.is_dataclass(possible_type):
            try:
                return as_dataclass(field_value, possible_type)
            except ValueError:
                continue
        # elif dataclasses.is_dataclass(possible_type) and field_name in obj:
        #     try:
        #         return as_dataclass(field_value, possible_type)
        #     except ValueError:
        #         continue
        elif inspect.isclass(possible_type) and issubclass(possible_type, Enum):
            try:
                return possible_type(field_value)  # type: ignore
            except ValueError:
                continue
        elif isinstance(field_value, possible_type):
            return field_value
    else:
        value_type = type(field_value)
        dict_repr = None
        if hasattr(obj, "__dict__"):
            dict_repr = obj.__dict__

        raise ValueError(
            f"as_dataclass: Could not match any types for from_object=({obj}, {dict_repr}). "
            f"field={field_name} with value={field_value} of type={value_type} could not "
            f"be converted to target_type={field_type} which can be one of "
            f"types_to_check={types_to_check}"
        )

def as_dataclass(obj: Any, dataclass_: Type[T], **kwargs) -> T:
    """Cast object to a dataclass

    :param Any obj: Object to cast
    :param Type[T] dataclass_: Target dataclass
    :param kwargs: Fields => override values; will not override fields of nested dataclasses
    :raises ValueError: Failed to convert object to given type
    :return T: Casted object

    FIXME: [BDRK-326] Convert to an iterative solution to avoid stack overflow.
    FIXME: [BDRK-327] mypy complains that `cast_type` is being fed too many arguments
    """

    if not dataclasses.is_dataclass(dataclass_):
        raise ValueError(
            f"as_dataclass: Attempted to convert object {obj} to non-dataclass type {dataclass_}"
        )

    try:
        return dataclass_(  # type: ignore
            **{
                field.name: _cast_dataclass_field_value(
                    obj=obj, field_name=field.name, field_type=field.type, **kwargs
                )
                for field in dataclasses.fields(dataclass_)
            }
        )
    except ValueError as ex:
        raise ValueError(
            f"as_dataclass: Could not convert object={obj} to type={dataclass_} "
            f"with kwargs={kwargs}"
        ) from ex

json_string = """
{
  "name": "broker-test",
  "enabled": true,
  "consul_address": "consul",
  "consul_port": 8500,
  "consul_prefix": "apps/span/api/dev",
  "poll_interval": 10,
  "heartbeat": {
    "enabled": true,
    "prometheus_url": "https://prom.amoy.ai/api/v2/query"
   }
}
"""

@dataclass(frozen=True)
class HeartbeatConfig:
    enabled: bool
    prometheus_url: str


@dataclass(frozen=True)
class LeaderConfig:
    name: str
    enabled: bool
    consul_address: str
    consul_port: int
    consul_prefix: str
    poll_interval: int
    heartbeat: Optional[HeartbeatConfig] = None

def print_empty_line():
    print("")

def main():
    print('Entering main app...(Dummy app)')

    json_config_file = "./dataclass_t.json"
    with open(json_config_file, "r") as f:
        json_config = json.load(f)

    json_config = json.loads(json_string)

    if True:
        config = as_dataclass(json_config, LeaderConfig)
    else:
        temp_hb = HeartbeatConfig(**(json_config['heartbeat']))
        json_config['heartbeat'] = temp_hb
        config: LeaderConfig = LeaderConfig(**json_config)

    print_empty_line()
    print(json_config)
    print_empty_line()
    print(config)
    print_empty_line()


if __name__ == "__main__":
    print("Starting mod:", __name__)
    main()
else:
    print("Starting other mod:", __name__)


