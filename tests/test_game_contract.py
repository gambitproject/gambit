"""Conformance tests for the game classes.

A game class provides a set of capabilities, each a named list of members with fixed
failure modes and number types.  `CAPABILITIES` and `PROVIDES` below are the specification;
the tests check each class against it.

* A class has every member of its capabilities and no other public member.
* Accessors that compute a value are methods; only `title` and `description`, which are
  stored and settable, are properties.
* A member the class does not provide is absent, not present and raising.
* An unknown label raises `KeyError`, an argument of the wrong type `TypeError` and a
  malformed composite argument `ValueError`.
* Every payoff-valued member returns `Rational`.

`DEPARTURES` lists where the classes do not yet conform.  Each entry is an expected failure
that names the step that removes it, and is `xfail(strict=True)` so that the step which fixes
a departure must delete its entry.

A new representation, or a member added to a capability, is added to the tables here.
Membership is checked on the class (`dir` and `inspect.getattr_static`), never with `hasattr`
on an instance: on a Bayesian game that calls the raising properties.
"""

import inspect

import pytest

import pygambit as gbt

from . import games

# ---------------------------------------------------------------------------------------
# The specification
# ---------------------------------------------------------------------------------------

# Member name -> kind.  "method" is an accessor or an operation, "property" a stored and
# settable attribute, "classmethod" a constructor and "any" a member whose shape is owned by
# another piece of work (`players`).
#
# `get_min_payoff` and `get_max_payoff` are listed under Core, which gives the game-wide
# bound.  The strategic-form capability adds the `player` form under the same name.
CAPABILITIES: dict[str, dict[str, str]] = {
    "core": {
        "title": "property",
        "description": "property",
        "players": "any",
        "get_min_payoff": "method",
        "get_max_payoff": "method",
        "is_perfect_recall": "method",
        "has_perfect_recall": "method",
    },
    "strategic": {
        "get_strategies": "method",
        "get_payoffs": "method",
        "get_contingencies": "method",
        "is_const_sum": "method",
        "mixed_strategy_profile": "method",
        "random_strategy_profile": "method",
        "strategy_support_profile": "method",
        "to_arrays": "method",
        "to_nfg": "method",
        "to_html": "method",
        "to_latex": "method",
        "draw": "method",
        "relabel_players": "method",
    },
    "behavior": {
        "get_infosets": "method",
        "mixed_behavior_profile": "method",
        "random_behavior_profile": "method",
    },
    # The shape of these members belongs to the extensive-game rework; the list is the
    # present set of members that only a tree has.
    "tree": {
        "get_histories": "method",
        "get_player": "method",
        "get_actions": "method",
        "get_action_probs": "method",
        "get_members": "method",
        "get_subgame_roots": "method",
        "get_minimal_subgame": "method",
        "get_events": "method",
        "get_strategy_unreachable": "method",
        "get_behavior": "method",
        "behavior_support_profile": "method",
        "to_efg": "method",
        "to_gtdraw": "method",
        "append_move": "method",
        "append_infoset": "method",
        "append_event": "method",
        "insert_move": "method",
        "insert_infoset": "method",
        "insert_event": "method",
        "copy_tree": "method",
        "move_tree": "method",
        "delete_parent": "method",
        "delete_tree": "method",
        "set_move_actions": "method",
        "set_event_actions": "method",
        "make_event": "method",
        "make_infoset": "method",
        "relabel_actions": "method",
        "set_players": "method",
    },
    "outcomes": {
        "get_outcomes": "method",
        "get_outcome_payoffs": "method",
        "get_outcome": "method",
        "set_outcome_payoffs": "method",
        "make_outcome": "method",
        "make_outcome_null": "method",
        "relabel_outcomes": "method",
    },
    "table": {
        "from_arrays": "classmethod",
        "from_dict": "classmethod",
        "set_players": "method",
        "set_strategies": "method",
        "relabel_strategies": "method",
    },
}

PROVIDES: dict[type, tuple[str, ...]] = {
    gbt.ExtensiveGame: ("core", "strategic", "behavior", "tree", "outcomes"),
    gbt.StrategicGame: ("core", "strategic", "outcomes", "table"),
    gbt.ActionGraphGame: ("core", "strategic"),
    gbt.BayesianActionGraphGame: ("core", "behavior"),
}

SAMPLE_GAMES: dict[type, str] = {
    gbt.ExtensiveGame: "2_player_chance.efg",
    gbt.StrategicGame: "const_sum_game.nfg",
    gbt.ActionGraphGame: "2x2.agg",
    gbt.BayesianActionGraphGame: "bayesian_2p2t_shared_node.bagg",
}

CLASSES = list(PROVIDES)
ALL_NAMES = sorted({name for members in CAPABILITIES.values() for name in members})


def members_of(cls: type) -> dict[str, str]:
    """The members `cls` should have, from the capability table."""
    return {name: kind for cap in PROVIDES[cls] for name, kind in CAPABILITIES[cap].items()}


def public_names(cls: type) -> set[str]:
    return {name for name in dir(cls) if not name.startswith("_")}


def load(cls: type) -> gbt.Game:
    game = games.read_from_file(SAMPLE_GAMES[cls])
    assert type(game) is cls
    return game


# ---------------------------------------------------------------------------------------
# Known departures from the specification
# ---------------------------------------------------------------------------------------

# (check, class name, member or "member:case") -> the step that removes the departure.
DEPARTURES: dict[tuple[str, str, str], str] = {}

_BAGG = gbt.BayesianActionGraphGame.__name__
_AGG = gbt.ActionGraphGame.__name__
_SG = gbt.StrategicGame.__name__
_EFG = gbt.ExtensiveGame.__name__

_USED_DEPARTURES: set[tuple[str, str, str]] = set()


def departure(check: str, cls: type, member: str = "") -> list:
    """The strict xfail marks for a known departure, if this check has one."""
    key = (check, cls.__name__, member)
    reason = DEPARTURES.get(key)
    if reason is None:
        return []
    _USED_DEPARTURES.add(key)
    return [pytest.mark.xfail(strict=True, reason=reason)]


def cases(check: str, items):
    """Parametrisation of `(class, member)` pairs, each with its departure marks."""
    return [
        pytest.param(
            cls, member, marks=departure(check, cls, member), id=f"{cls.__name__}-{member}"
        )
        for cls, member in items
    ]


def test_every_departure_names_a_check():
    """A departure whose key matches no parametrised case would never be consumed."""
    assert set(DEPARTURES) == _USED_DEPARTURES


# ---------------------------------------------------------------------------------------
# Membership: presence, absence and kind
# ---------------------------------------------------------------------------------------


@pytest.mark.parametrize(
    "cls,member",
    cases("membership", [(cls, name) for cls in CLASSES for name in ALL_NAMES]),
)
def test_member_is_present_exactly_where_provided(cls, member):
    assert (member in dir(cls)) == (member in members_of(cls))


@pytest.mark.parametrize(
    "cls",
    [pytest.param(c, id=c.__name__, marks=departure("no-unlisted-members", c)) for c in CLASSES],
)
def test_class_has_no_public_member_that_no_capability_lists(cls):
    assert public_names(cls) - set(members_of(cls)) == set()


def _kind_of(cls: type, name: str) -> str:
    attribute = inspect.getattr_static(cls, name)
    if isinstance(attribute, classmethod):
        return "classmethod"
    if inspect.isdatadescriptor(attribute):
        return "property"
    return "method" if callable(attribute) else "other"


@pytest.mark.parametrize(
    "cls,member",
    cases(
        "member-kind",
        [
            (cls, name)
            for cls in CLASSES
            for name, kind in members_of(cls).items()
            if kind != "any" and name in dir(cls)
        ],
    ),
)
def test_member_has_the_specified_kind(cls, member):
    assert _kind_of(cls, member) == members_of(cls)[member]


@pytest.mark.parametrize("cls", CLASSES, ids=lambda c: c.__name__)
@pytest.mark.parametrize("name", ["title", "description"])
def test_stored_attributes_are_settable(cls, name):
    game = load(cls)
    setattr(game, name, "a new value")
    assert getattr(game, name) == "a new value"


def _is_raising_stub(cls: type, name: str) -> bool:
    """Whether `name`, as `cls` defines it, raises `UndefinedOperationError` however it is
    called.  A real member called with no arguments raises `TypeError` (no `self`)."""
    try:
        attribute = inspect.getattr_static(cls, name)
    except AttributeError:
        return False
    function = attribute.fget if isinstance(attribute, property) else attribute
    if not callable(function):
        return False
    try:
        function()
    except gbt.UndefinedOperationError:
        return True
    except Exception:
        return False
    return False


@pytest.mark.parametrize(
    "cls,member",
    cases(
        "no-raising-override",
        [(cls, name) for cls in CLASSES for name in ALL_NAMES],
    ),
)
def test_no_class_overrides_a_member_to_raise(cls, member):
    assert not _is_raising_stub(cls, member)


# ---------------------------------------------------------------------------------------
# Core queries and number types
# ---------------------------------------------------------------------------------------


@pytest.mark.parametrize(
    "cls", [pytest.param(c, id=c.__name__, marks=departure("core-queries", c)) for c in CLASSES]
)
def test_core_queries(cls):
    game = load(cls)
    assert isinstance(game.get_min_payoff(), gbt.Rational)
    assert isinstance(game.get_max_payoff(), gbt.Rational)
    assert game.get_min_payoff() <= game.get_max_payoff()
    assert isinstance(game.is_perfect_recall(), bool)
    for player in game.players:
        assert isinstance(game.has_perfect_recall(player), bool)


def _efg_with_decimal_payoffs() -> gbt.Game:
    game = games.read_from_file(SAMPLE_GAMES[gbt.ExtensiveGame])
    for outcome in game.get_outcomes():
        game.set_outcome_payoffs(outcome, {p: gbt.Decimal("1.5") for p in game.players})
    return game


def _sg_with_decimal_payoffs() -> gbt.Game:
    return gbt.StrategicGame.from_arrays([[1.5, 0], [0, 1]], [[0, 1.5], [1, 0]])


def _agg_with_decimal_payoffs() -> gbt.Game:
    return games.read_from_file("2x2_fraction_payoffs.agg")


DECIMAL_GAMES = {
    gbt.ExtensiveGame: _efg_with_decimal_payoffs,
    gbt.StrategicGame: _sg_with_decimal_payoffs,
    gbt.ActionGraphGame: _agg_with_decimal_payoffs,
}

PAYOFF_MEMBERS = ("get_payoffs", "get_min_payoff", "get_max_payoff", "get_outcome_payoffs")


def _payoff_values(game: gbt.Game, member: str) -> list:
    players = list(game.players)
    if member == "get_payoffs":
        contingency = {p: game.get_strategies(p)[0] for p in players}
        payoffs = game.get_payoffs(contingency)
        return [payoffs[p] for p in players]
    if member == "get_outcome_payoffs":
        return [
            game.get_outcome_payoffs(label)[p] for label in game.get_outcomes() for p in players
        ]
    return [getattr(game, member)(p) for p in players]


@pytest.mark.parametrize(
    "cls,member",
    cases(
        "payoff-type",
        [
            (cls, member)
            for cls in DECIMAL_GAMES
            for member in PAYOFF_MEMBERS
            if member in members_of(cls)
            and (member != "get_min_payoff" or "strategic" in PROVIDES[cls])
        ],
    ),
)
def test_payoff_valued_members_return_rational(cls, member):
    game = DECIMAL_GAMES[cls]()
    values = _payoff_values(game, member)
    assert values
    assert all(isinstance(value, gbt.Rational) for value in values), [type(v) for v in values]


# ---------------------------------------------------------------------------------------
# Failure rules F1-F3 for the members that take a label or a composite argument
# ---------------------------------------------------------------------------------------


def _first(game: gbt.Game) -> str:
    return list(game.players)[0]


def _contingency(game: gbt.Game) -> dict:
    return {p: game.get_strategies(p)[0] for p in game.players}


def _unknown_strategy(game: gbt.Game) -> dict:
    return {**_contingency(game), _first(game): "no such strategy"}


def _unknown_player(game: gbt.Game) -> dict:
    return {**_contingency(game), "no such player": "1"}


def _wrong_type_strategy(game: gbt.Game) -> dict:
    return {**_contingency(game), _first(game): 5}


def _incomplete(game: gbt.Game) -> dict:
    return {_first(game): game.get_strategies(_first(game))[0]}


def _set_outcome(game: gbt.Game, label, payoffs) -> None:
    game.set_outcome_payoffs(label, payoffs)


# (capability, member, case, call on a game, exception it must raise)
PROBES = [
    (
        "core",
        "has_perfect_recall",
        "unknown player",
        lambda g: g.has_perfect_recall("none"),
        KeyError,
    ),
    ("core", "has_perfect_recall", "wrong type", lambda g: g.has_perfect_recall(1), TypeError),
    (
        "strategic",
        "get_strategies",
        "unknown player",
        lambda g: g.get_strategies("none"),
        KeyError,
    ),
    ("strategic", "get_strategies", "wrong type", lambda g: g.get_strategies(1), TypeError),
    (
        "strategic",
        "get_min_payoff",
        "unknown player",
        lambda g: g.get_min_payoff("none"),
        KeyError,
    ),
    ("strategic", "get_min_payoff", "wrong type", lambda g: g.get_min_payoff(1), TypeError),
    (
        "strategic",
        "get_max_payoff",
        "unknown player",
        lambda g: g.get_max_payoff("none"),
        KeyError,
    ),
    ("strategic", "get_max_payoff", "wrong type", lambda g: g.get_max_payoff(1), TypeError),
    (
        "strategic",
        "get_payoffs",
        "unknown strategy",
        lambda g: g.get_payoffs(_unknown_strategy(g)),
        KeyError,
    ),
    (
        "strategic",
        "get_payoffs",
        "unknown player",
        lambda g: g.get_payoffs(_unknown_player(g)),
        KeyError,
    ),
    ("strategic", "get_payoffs", "wrong type contingency", lambda g: g.get_payoffs(5), TypeError),
    (
        "strategic",
        "get_payoffs",
        "wrong type strategy",
        lambda g: g.get_payoffs(_wrong_type_strategy(g)),
        TypeError,
    ),
    (
        "strategic",
        "get_payoffs",
        "incomplete contingency",
        lambda g: g.get_payoffs(_incomplete(g)),
        ValueError,
    ),
    (
        "strategic",
        "mixed_strategy_profile",
        "wrong shape data",
        lambda g: g.mixed_strategy_profile(data=[[1.0]]),
        ValueError,
    ),
    (
        "strategic",
        "mixed_strategy_profile",
        "wrong type data",
        lambda g: g.mixed_strategy_profile(data=5),
        TypeError,
    ),
    (
        "strategic",
        "strategy_support_profile",
        "wrong type",
        lambda g: g.strategy_support_profile(5),
        TypeError,
    ),
    (
        "strategic",
        "relabel_players",
        "unknown player",
        lambda g: g.relabel_players({"none": "x"}),
        KeyError,
    ),
    ("strategic", "relabel_players", "wrong type", lambda g: g.relabel_players(5), TypeError),
    ("behavior", "get_infosets", "unknown player", lambda g: g.get_infosets("none"), KeyError),
    ("behavior", "get_infosets", "wrong type", lambda g: g.get_infosets(1), TypeError),
    (
        "behavior",
        "mixed_behavior_profile",
        "wrong shape data",
        lambda g: g.mixed_behavior_profile(data=[[1.0]]),
        ValueError,
    ),
    (
        "behavior",
        "mixed_behavior_profile",
        "wrong type data",
        lambda g: g.mixed_behavior_profile(data=5),
        TypeError,
    ),
    (
        "outcomes",
        "get_outcome_payoffs",
        "unknown outcome",
        lambda g: g.get_outcome_payoffs("none"),
        KeyError,
    ),
    (
        "outcomes",
        "get_outcome_payoffs",
        "wrong type",
        lambda g: g.get_outcome_payoffs(1),
        TypeError,
    ),
    (
        "outcomes",
        "set_outcome_payoffs",
        "unknown outcome",
        lambda g: _set_outcome(g, "none", {_first(g): 1}),
        KeyError,
    ),
    (
        "outcomes",
        "set_outcome_payoffs",
        "wrong type outcome",
        lambda g: _set_outcome(g, 1, {_first(g): 1}),
        TypeError,
    ),
    (
        "outcomes",
        "set_outcome_payoffs",
        "unknown player",
        lambda g: _set_outcome(g, g.get_outcomes()[0], {"none": 1}),
        KeyError,
    ),
    (
        "outcomes",
        "set_outcome_payoffs",
        "wrong type payoffs",
        lambda g: _set_outcome(g, g.get_outcomes()[0], 5),
        TypeError,
    ),
    (
        "outcomes",
        "relabel_outcomes",
        "unknown outcome",
        lambda g: g.relabel_outcomes({"none": "x"}),
        KeyError,
    ),
    ("outcomes", "relabel_outcomes", "wrong type", lambda g: g.relabel_outcomes(5), TypeError),
]


def _probe_cases():
    return [
        pytest.param(
            cls,
            capability,
            member,
            case,
            call,
            exception,
            marks=departure("failure-rules", cls, f"{member}:{case}"),
            id=f"{cls.__name__}-{member}-{case}".replace(" ", "_"),
        )
        for cls in CLASSES
        for capability, member, case, call, exception in PROBES
        if capability in PROVIDES[cls]
    ]


@pytest.mark.parametrize("cls,capability,member,case,call,exception", _probe_cases())
def test_failure_rules(cls, capability, member, case, call, exception):
    game = load(cls)
    with pytest.raises(exception):
        call(game)
