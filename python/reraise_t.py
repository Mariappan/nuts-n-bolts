from contextlib import AbstractContextManager
from typing import Any, Callable, Tuple, Type, TypeVar, Union
from functools import wraps


class FatalError(Exception):
    pass

class BaseError(Exception):
    def __init__(self, debug_message: str = "", *, resp_details: str = ""):
        super().__init__(debug_message)
        self.debug_message = debug_message
        self.resp_details = resp_details

    def __str__(self):
        return f"{self.resp_message} details={self.resp_details}, debug={self.debug_message}"

    resp_message = "Error encountered."
    status_code = 500

class AuthenticationError(BaseError):
    resp_message = "Authentication error."
    status_code = 403


class UnauthorizedError(BaseError):
    resp_message = "Unauthorized error."
    resp_details = "You are not authorized"
    status_code = 404


class InternalError(BaseError):
    resp_message = "Internal error."
    resp_details = "something fucked up"
    status_code = 404

T = TypeVar("T")

# Decorators to preserve function signature using this pattern:
# https://github.com/python/mypy/issues/1927
FuncT = TypeVar("FuncT", bound=Callable[..., Any])

class reraise(AbstractContextManager):
    """
    Context manager to reraise one exception from another
    """

    def __init__(self, exc_a: Type[Exception], exc_b: Type[Exception], **kwargs):
        self.exc_a = exc_a
        self.exc_b = exc_b
        self.kwargs = kwargs

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_value, traceback):
        print(f"== MAARI == => Type:{exc_type} Value:{exc_value} BT:{traceback}")
        if exc_type and issubclass(exc_type, self.exc_a):
            raise self.exc_b(**self.kwargs) from exc_value

    def __call__(self, func: FuncT) -> FuncT:
        @wraps(func)
        def decorated(*args, **kwargs):
            with self:
                return func(*args, **kwargs)

        return decorated  # type: ignore


@reraise(AuthenticationError, InternalError)
def reraise_exp():
    print(f"Calling reraise exception")
    normal_exp()

def normal_exp():
    print(f"Calling normal exception")
    raise AuthenticationError


reraise_exp()