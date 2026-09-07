import pytest

import pygambit as gbt

from . import games


def test_relabel_strategies_swap():
    """Swap is well-defined; strategies keep their positions."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = game.get_strategies(player)
    game.relabel_strategies(player, {a: b, b: a})
    assert list(game.get_strategies(player)) == [b, a]


def test_relabel_strategies_duplicate_raises_valueerror():
    """A replacement colliding with an untouched strategy, and two replacements
    colliding with each other, are both rejected; checking each against the
    untouched strategies alone would let the second through."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = game.get_strategies(player)
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: b})
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: "X", b: "X"})


@pytest.mark.parametrize("bad", ["", " x"])
def test_relabel_strategies_bad_label_raises_and_leaves_game_unchanged(bad: str):
    """The whole mapping is validated before any label is written."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = game.get_strategies(player)
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: "X", b: bad})
    assert list(game.get_strategies(player)) == [a, b]


def test_relabel_strategies_unknown_label_strictness():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a = next(iter(game.get_strategies(player)))
    with pytest.raises(KeyError):
        game.relabel_strategies(player, {"no-such-strategy": "X"})
    game.relabel_strategies(player, {"no-such-strategy": "X", a: "Y"}, strict=False)
    assert next(iter(game.get_strategies(player))) == "Y"


def test_relabel_strategies_scope_is_the_player():
    """Strategy labels are unique within a player, not within the game."""
    game = gbt.Game.new_table([2, 2])
    one, two = game.players
    game.relabel_strategies(one, {next(iter(game.get_strategies(one))): "X"})
    game.relabel_strategies(two, {next(iter(game.get_strategies(two))): "X"})
    assert [next(iter(game.get_strategies(p))) for p in game.players] == ["X", "X"]


def test_relabel_strategies_tree_game_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.relabel_strategies("Alice", {"11": "XY"})


def _payoffs_by_label(game: gbt.Game) -> dict:
    one, two = game.players
    result = {}
    for s in game.get_strategies(one):
        for t in game.get_strategies(two):
            payoffs = game.get_payoffs({one: s, two: t})
            result[s, t] = (payoffs[one], payoffs[two])
    return result


def test_set_strategies_reorder_carries_outcomes():
    """Reordering permutes the payoff table: each contingency keeps the outcome
    it had, identified by the labels of its strategies."""
    game = gbt.Game.from_arrays([[1, 2], [3, 4]], [[5, 6], [7, 8]])
    player, _ = game.players
    a, b = game.get_strategies(player)
    kept = list(game.get_strategies(player))
    before = _payoffs_by_label(game)
    game.set_strategies(player, [b, a])
    assert list(game.get_strategies(player)) == [b, a]
    assert list(game.get_strategies(player)) == list(reversed(kept))
    assert _payoffs_by_label(game) == before


def test_set_strategies_add_drop_and_reorder_together():
    """A single call can create, delete, and reorder; the surviving strategy keeps
    the outcomes at its contingencies."""
    game = gbt.Game.from_arrays([[1, 2], [3, 4]], [[5, 6], [7, 8]])
    player, other = game.players
    a, b = game.get_strategies(player)
    kept = {
        t: game.get_payoffs({player: a, other: t})[player]
        for t in game.get_strategies(other)
    }
    game.set_strategies(player, ["X", a], drop=True)
    assert list(game.get_strategies(player)) == ["X", a]
    assert {
        t: game.get_payoffs({player: a, other: t})[player]
        for t in game.get_strategies(other)
    } == kept


def test_set_strategies_unconfirmed_drop_and_disabled_add_raise():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = game.get_strategies(player)
    with pytest.raises(ValueError):
        game.set_strategies(player, [a])
    with pytest.raises(ValueError):
        game.set_strategies(player, [a, b, "X"], add=False)
    assert list(game.get_strategies(player)) == [a, b]
