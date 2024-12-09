import itertools
import typing

import pytest

import pygambit as gbt

from . import games


def _test_valid_resolutions(collection: list, resolver: typing.Callable) -> None:
    """Generic function to exercise resolving objects as themselves or via existing labels."""
    for label, objects in itertools.groupby(
            sorted(collection, key=lambda x: x.label), lambda x: x.label
    ):
        objects = list(objects)
        # Objects resolve to themselves
        for obj in objects:
            assert obj == resolver(obj, "test")
        # Ambiguous labels raise ValueError
        if len(objects) > 1:
            with pytest.raises(ValueError):
                _ = resolver(label, "test")
        else:
            assert objects[0] == resolver(label, "test")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_player(game: gbt.Game) -> None:
    _test_valid_resolutions(game.players,
                            lambda label, fn: game._resolve_player(label, fn))


@pytest.mark.parametrize(
    "game,player,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "random", KeyError),
    ]
)
def test_resolve_player_invalid(game: gbt.Game, player: str, exception: BaseException) -> None:
    with pytest.raises(exception):
        game._resolve_player(player, "test_resolve_player_invalid")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_outcome(game: gbt.Game) -> None:
    _test_valid_resolutions(game.outcomes,
                            lambda label, fn: game._resolve_outcome(label, fn))


@pytest.mark.parametrize(
    "game,outcome,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "nosuchoutcome", KeyError),
    ]
)
def test_resolve_outcome_invalid(game: gbt.Game, outcome: str, exception: BaseException) -> None:
    with pytest.raises(exception):
        game._resolve_outcome(outcome, "test_resolve_outcome_invalid")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_strategy(game: gbt.Game) -> None:
    _test_valid_resolutions(game.strategies,
                            lambda label, fn: game._resolve_strategy(label, fn))


@pytest.mark.parametrize(
    "game,strategy,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "doesntexist", KeyError),
    ]
)
def test_resolve_strategy_invalid(
        game: gbt.Game, strategy: str, exception: BaseException
) -> None:
    with pytest.raises(exception):
        game._resolve_strategy(strategy, "test_resolve_strategy_invalid")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_node(game: gbt.Game) -> None:
    _test_valid_resolutions(game.nodes(),
                            lambda label, fn: game._resolve_node(label, fn))


@pytest.mark.parametrize(
    "game,node,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "fictitious", KeyError),
    ]
)
def test_resolve_node_invalid(game: gbt.Game, node: str, exception: BaseException) -> None:
    with pytest.raises(exception):
        game._resolve_node(node, "test_resolve_node_invalid")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_infoset(game: gbt.Game) -> None:
    _test_valid_resolutions(game.infosets,
                            lambda label, fn: game._resolve_infoset(label, fn))


@pytest.mark.parametrize(
    "game,infoset,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "neverhappens", KeyError),
    ]
)
def test_resolve_infoset_invalid(game: gbt.Game, infoset: str, exception: BaseException) -> None:
    with pytest.raises(exception):
        game._resolve_infoset(infoset, "test_resolve_infoset_invalid")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_action(game: gbt.Game) -> None:
    _test_valid_resolutions(game.actions,
                            lambda label, fn: game._resolve_action(label, fn))


@pytest.mark.parametrize(
    "game,action,exception",
    [
        (games.read_from_file("sample_extensive_game.efg"), "", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), " ", ValueError),
        (games.read_from_file("sample_extensive_game.efg"), "inaction", KeyError),
    ]
)
def test_resolve_action_invalid(game: gbt.Game, action: str, exception: BaseException) -> None:
    with pytest.raises(exception):
        game._resolve_action(action, "test_resolve_action_invalid")
