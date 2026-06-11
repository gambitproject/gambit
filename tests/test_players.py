import pytest

import pygambit as gbt

from . import games


def test_player_count():
    game = gbt.Game.new_table([2, 2])
    assert len(game.players) == 2


def test_player_label():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    pl1.label = "Alphonse"
    pl2.label = "Gaston"
    assert pl1.label == "Alphonse"
    assert pl2.label == "Gaston"


def test_player_index_by_string():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    pl1.label = "Alphonse"
    pl2.label = "Gaston"
    assert game.players["Alphonse"].label == "Alphonse"
    assert game.players["Gaston"].label == "Gaston"


def test_player_index_rejects_int():
    game = gbt.Game.new_table([2, 2])
    assert len(game.players) == 2
    # Integer indexing was removed in 16.7.0; collections are label-keyed.
    for bad_index in (0, 2, 3, -1):
        with pytest.raises(TypeError):
            _ = game.players[bad_index]


def test_player_index_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.players[1.3]


def test_player_label_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.players["Not a player"]


def test_set_empty_player_futurewarning():
    game = games.create_stripped_down_poker_efg()
    player = next(iter(game.players))
    with pytest.warns(FutureWarning):
        player.label = ""


def test_set_duplicate_player_futurewarning():
    game = games.create_stripped_down_poker_efg()
    pl1, pl2, *_ = game.players
    with pytest.warns(FutureWarning):
        pl1.label = pl2.label


def test_strategic_game_add_player():
    game = gbt.Game.new_table([2, 2])
    new_player = game.add_player()
    assert len(game.players) == 3
    assert len(new_player.strategies) == 1


def test_extensive_game_add_player():
    game = gbt.Game.new_tree()
    game.add_player()
    pl1 = next(iter(game.players))
    assert len(game.players) == 1
    assert len(pl1.infosets) == 0
    assert len(pl1.strategies) == 1


def test_strategic_game_add_strategy():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.add_strategy(pl1, "new strategy")
    assert len(pl1.strategies) == 3
    # This second add also ensures that we are testing the case where there
    # are null outcomes in the table
    game.add_strategy(pl2, "new strategy")
    assert len(pl2.strategies) == 3


def test_extensive_game_add_strategy():
    game = gbt.Game.new_tree(["Alice"])
    assert len(game.players["Alice"].strategies) == 1
    with pytest.raises(gbt.UndefinedOperationError):
        game.add_strategy(game.players["Alice"], "new strategy")


def test_strategic_game_delete_strategy():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    game.delete_strategy(next(iter(pl1.strategies)))
    assert len(pl1.strategies) == 1


def test_strategic_game_delete_last_strategy():
    game = gbt.Game.new_table([1, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_strategy(next(iter(pl1.strategies)))


def test_extensive_game_delete_strategy():
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_strategy(next(iter(game.players["Alice"].strategies)))


def test_player_strategy_by_label():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    next(iter(pl1.strategies)).label = "Cooperate"
    assert pl1.strategies["Cooperate"].label == "Cooperate"


def test_player_strategy_rejects_int():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    # Integer indexing was removed in 16.7.0; strategies are label-keyed.
    for bad_index in (0, 42):
        with pytest.raises(TypeError):
            _ = pl1.strategies[bad_index]


def test_player_strategy_bad_label():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(KeyError):
        _ = pl1.strategies["Cooperate"]


def test_player_strategy_bad_type():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(TypeError):
        _ = pl1.strategies[1.3]


@pytest.mark.parametrize(
    "game,exp_min_payoffs,exp_max_payoffs",
    [
        # NFGs
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [-1, 0, -1],
            [2, 4, 2]
        ),
        (games.read_from_file("mixed_strategy.nfg"), [0, 0], [2, 3]),
        # EFGs only terminal outcomes
        (games.create_kuhn_poker_efg(), [-2, -2], [2, 2]),
        (games.create_stripped_down_poker_efg(), [-2, -2], [2, 2]),
        # with non-terminal outcomes
        (games.create_kuhn_poker_efg(nonterm_outcomes=True), [-2, -2], [2, 2]),
        (games.create_stripped_down_poker_efg(nonterm_outcomes=True), [-2, -2], [2, 2]),
    ],
)
def test_player_get_min_max_payoff(game: gbt.Game, exp_min_payoffs: list, exp_max_payoffs: list):
    for player, exp_min, exp_max in zip(
        game.players, exp_min_payoffs, exp_max_payoffs, strict=True
    ):
        assert player.min_payoff == exp_min
        assert player.max_payoff == exp_max


def test_player_get_min_payoff_nonterminal_outcomes():
    """Test whether `min_payoff` correctly reports minimum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.players["Alice"].min_payoff == -2
    assert game.players["Bob"].min_payoff == -2
    game.set_outcome(game.root, game.add_outcome([-1, -1]))
    assert game.players["Alice"].min_payoff == -3
    assert game.players["Bob"].min_payoff == -3


def test_player_get_min_payoff_null_outcome():
    """Test whether `min_payoff` correctly reports minimum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[1, 1], [1, 1]], [[2, 2], [2, 2]])
    pl1, pl2 = game.players
    assert pl1.min_payoff == 1
    assert pl2.min_payoff == 2
    game.add_strategy(pl1)
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert player.min_payoff == 0


def test_player_get_max_payoff_nonterminal_outcomes():
    """Test whether `max_payoff` correctly reports maximum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.players["Alice"].max_payoff == 2
    assert game.players["Bob"].max_payoff == 2
    game.set_outcome(game.root, game.add_outcome([-1, -1]))
    assert game.players["Alice"].max_payoff == 1
    assert game.players["Bob"].max_payoff == 1


def test_player_get_max_payoff_null_outcome():
    """Test whether `max_payoff` correctly reports maximum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[-1, -1], [-1, -1]], [[-2, -2], [-2, -2]])
    pl1, pl2 = game.players
    assert pl1.max_payoff == -1
    assert pl2.max_payoff == -2
    game.add_strategy(pl1)
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert player.max_payoff == 0
