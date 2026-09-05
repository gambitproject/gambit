import pytest

import pygambit as gbt

from . import games


def test_strategic_game_get_infosets():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.get_infosets(player)


def test_strategic_game_get_histories_root_raises():
    """A bare `H.path()` (the root) still resolves through the same
    tree-only guard as `H.after()`, just via a different internal path."""
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.get_histories(gbt.H.path())


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


def test_game_get_outcome():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 0, "2": 0}, "top left")
    assert game.get_outcome({"1": "1", "2": "1"}) == next(iter(game.outcomes))


def test_game_get_outcome_by_relabeled_strategies():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_strategies(pl1, {next(iter(game.get_strategies(pl1))): "defect"})
    game.relabel_strategies(pl2, {next(iter(game.get_strategies(pl2))): "cooperate"})
    game.make_outcome({pl1: "defect", pl2: "cooperate"}, {"1": 0, "2": 0}, "corner")
    assert game.get_outcome({pl1: "defect", pl2: "cooperate"}) == \
        next(iter(game.outcomes))


def test_game_get_outcome_incomplete_contingency_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(ValueError):
        _ = game.get_outcome({"1": "1"})


def test_game_get_outcome_unknown_player_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.get_outcome({"1": "1", "2": "1", "3": "1"})


def test_game_get_outcome_non_mapping_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.get_outcome(42)


def test_game_get_outcome_non_str_value_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.get_outcome({"1": 1.23, "2": "1"})


def test_game_get_outcome_unknown_strategy_label_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.get_outcome({"1": "1", "2": "99"})


def test_game_get_outcome_unmatched_label_after_relabel_raises():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_strategies(pl1, {next(iter(game.get_strategies(pl1))): "defect"})
    game.relabel_strategies(pl2, {next(iter(game.get_strategies(pl2))): "cooperate"})
    with pytest.raises(KeyError):
        _ = game.get_outcome({pl1: "defect", pl2: "defect"})


def test_game_get_outcome_tree_rejects_contingency():
    """A pure-strategy contingency (a Mapping) is only meaningful for a
    strategic game; for a tree game, `location` must be a `Selector`."""
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["a", "b"])
    with pytest.raises(TypeError):
        _ = game.get_outcome({"Alice": "a"})


def test_game_get_payoffs():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 3, "2": -3}, "top left")
    payoffs = game.get_payoffs({"1": "1", "2": "1"})
    assert payoffs["1"] == 3
    assert payoffs["2"] == -3


def test_game_get_payoffs_tree():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["a", "b"])
    selector = gbt.H.path()
    strategy = next(
        s for s in game.get_strategies("Alice")
        if game.get_behavior("Alice", s).get(selector) == "a"
    )
    game.make_outcome(gbt.H.path("a"), {"Alice": 1}, "a-outcome")
    payoffs = game.get_payoffs({"Alice": strategy})
    assert payoffs["Alice"] == 1
