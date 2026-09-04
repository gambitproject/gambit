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
def test_resolve_node(game: gbt.Game) -> None:
    _test_valid_resolutions(games.all_nodes(game),
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


def test_resolve_node_mismatch():
    """A `Node` from a different game raises `MismatchError`."""
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("sample_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1._resolve_node(game2.root, "test_resolve_node_mismatch")


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("sample_extensive_game.efg"),
    ]
)
def test_resolve_infoset(game: gbt.Game) -> None:
    """`_resolve_infoset` resolves a Selector to the node it identifies, or a
    node's label to that node, validating that it currently belongs to a
    personal player's information set; this holds for every member node, not
    just the representative `Game.get_infosets` returns."""
    for player in game.players:
        for history in game.get_infosets(player):
            for member in game.get_members(gbt.H.path(*history)):
                resolved = game._resolve_infoset(gbt.H.path(*member), "test")
                assert games._node_history(resolved) == member
                if resolved.label:
                    resolved_by_label = game._resolve_infoset(resolved.label, "test")
                    assert games._node_history(resolved_by_label) == member


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
