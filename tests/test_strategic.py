import pytest

import pygambit as gbt

from . import games


def test_strategic_game_actions():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.actions


def test_strategic_game_player_actions():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        _ = player.actions


def test_strategic_game_infosets():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.infosets


def test_strategic_game_player_infosets():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        _ = player.infosets


def test_strategic_game_root():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.root


def test_strategic_game_nodes():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.nodes


def test_game_behav_profile_error():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_behavior_profile()


def test_game_is_const_sum():
    game = games.read_from_file("const_sum_game.nfg")
    assert game.is_const_sum


def test_game_is_not_const_sum():
    game = games.read_from_file("non_const_sum_game.nfg")
    assert not game.is_const_sum


def test_game_get_min_payoff():
    game = games.read_from_file("mixed_strategy.nfg")
    assert game.min_payoff == 0


def test_game_get_max_payoff():
    game = games.read_from_file("mixed_strategy.nfg")
    assert game.max_payoff == 3


def test_relabel_strategies_swap():
    """Swap is well-defined; strategies keep their positions."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = (strategy.label for strategy in player.strategies)
    game.relabel_strategies(player, {a: b, b: a})
    assert [strategy.label for strategy in player.strategies] == [b, a]


def test_relabel_strategies_duplicate_raises_valueerror():
    """A replacement colliding with an untouched strategy, and two replacements
    colliding with each other, are both rejected; checking each against the
    untouched strategies alone would let the second through."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = (strategy.label for strategy in player.strategies)
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: b})
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: "X", b: "X"})


@pytest.mark.parametrize("bad", ["", " x"])
def test_relabel_strategies_bad_label_raises_and_leaves_game_unchanged(bad: str):
    """The whole mapping is validated before any label is written."""
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a, b = (strategy.label for strategy in player.strategies)
    with pytest.raises(ValueError):
        game.relabel_strategies(player, {a: "X", b: bad})
    assert [strategy.label for strategy in player.strategies] == [a, b]


def test_relabel_strategies_unknown_label_strictness():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    a = next(iter(player.strategies)).label
    with pytest.raises(KeyError):
        game.relabel_strategies(player, {"no-such-strategy": "X"})
    game.relabel_strategies(player, {"no-such-strategy": "X", a: "Y"}, strict=False)
    assert next(iter(player.strategies)).label == "Y"


def test_relabel_strategies_scope_is_the_player():
    """Strategy labels are unique within a player, not within the game."""
    game = gbt.Game.new_table([2, 2])
    one, two = game.players
    game.relabel_strategies(one, {next(iter(one.strategies)).label: "X"})
    game.relabel_strategies(two, {next(iter(two.strategies)).label: "X"})
    assert [next(iter(p.strategies)).label for p in game.players] == ["X", "X"]


def test_relabel_strategies_tree_game_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.relabel_strategies(game.players["Alice"], {"11": "XY"})
