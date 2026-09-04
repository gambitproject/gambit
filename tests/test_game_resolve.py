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
    _test_valid_resolutions(game.nodes,
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
    """`_resolve_infoset` resolves a Node to the Infoset it belongs to, or a node's
    label to the same; any member node of an infoset resolves to an equal Infoset."""
    for player in game.players:
        for node in game.get_infosets(player):
            resolved = game._resolve_infoset(node, "test")
            assert resolved == node.infoset
            if node.label:
                assert game._resolve_infoset(node.label, "test") == node.infoset
            for member in node.infoset.members:
                assert game._resolve_infoset(member, "test") == node.infoset


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
