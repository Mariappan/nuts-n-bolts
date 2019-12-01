import asyncio
import inspect
from typing import TypeVar, Generic
from abc import ABC
from functools import wraps

# Library Code

T = TypeVar('T')
S = TypeVar('S')

class NotAuthorised(Exception):
    pass

class ScopedService(Generic[T, S]):
    def __init__(self, service: T, engine: S, user: str):
        self.service = service
        self.engine = engine
        self.user = user


class PolicyEngine(ABC):
    def assert_role(self, entity_a, entity_b, role) -> bool:
        raise NotImplementedError


def policy(target_entity_arg, role):
    def decorated(func):
        @wraps(func)
        def wrapped(*args, **kwargs):
            _self = args[0]

            # there is probably a better way to get the value of the target argument
            argspec = inspect.getfullargspec(func)
            argument_index = argspec.args.index(target_entity_arg)

            if _self.engine.assert_role(_self.user, args[argument_index], role):
                return func(*args, **kwargs)
            raise NotAuthorised
        return wrapped
    return decorated

# Application code

class MyPolicy(PolicyEngine):
    def assert_role(self, entity_a, entity_b, role) -> bool:
        roles = {
            'entity_a': {'entity_b': 'owner'}
        }

        try:
            return roles[entity_a][entity_b] == role
        except KeyError:
            return False


class HelloService:
    def say_message(self, target) -> str:
        return f'messaging {target}'


class ScopedHelloService(ScopedService[HelloService, MyPolicy]):
    @policy('target', 'owner')
    def say_message(self, target: str):
        return self.service.say_message(target)


# Run applications
async def main() -> None:
    service = HelloService()
    policy_engine = MyPolicy()
    scoped_service = ScopedHelloService(service=service, engine=policy_engine, user='entity_a')

    # calling the underlying service directly
    print(scoped_service.service.say_message('anything'))

    # user-scoped call
    print(scoped_service.say_message('entity_b'))

    try:
        print(scoped_service.say_message('entity_c'))
    except NotAuthorised:
        print('Not able to call entity_c')


if __name__ == "__main__":
    asyncio.run(main())