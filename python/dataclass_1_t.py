import dataclasses
from dataclasses import dataclass
from typing import Optional


@dataclass(frozen=True)
class Team():
    public_id: str
    name: str

@dataclass(frozen=True)
class User():
    public_id: str
    email: str

@dataclass(frozen=True)
class Collaborator():
    role: int
    user: Optional[User] = None
    team: Optional[Team] = None

    def __post_init__(self):
        if self.user and self.team:
            raise ValueError
        if not self.user and not self.team:
            raise ValueError


user = User(public_id="maari-4089", email="maari@basis.com")
print(user)

team = Team(public_id="basis-10089", name="Basis")
print(team)

coll = Collaborator(role=1, user=user)
print(coll)

coll = Collaborator(role=1, team=team)
print(coll)

coll = Collaborator(role=1, user=user, team=team)
print(coll)