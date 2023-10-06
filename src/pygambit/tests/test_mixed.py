import pytest

import pygambit
import pygambit as gbt


def _create_strategic_game() -> gbt.Game:
    game = pygambit.Game.new_table([2, 2])
    game.players[0].label = "Joe"
    game.players["Joe"].strategies[0].label = "cooperate"
    game.players[1].label = "Dan"
    game.players["Dan"].strategies[1].label = "defect"
    return game


def test_get_probabilities_strategy():
    game = _create_strategic_game()
    strategy = game.players[0].strategies[0]
    assert game.mixed_strategy_profile(rational=False)[strategy] == 0.5
    assert game.mixed_strategy_profile(rational=True)[strategy] == gbt.Rational("1/2")


def test_get_probabilities_player():
    game = _create_strategic_game()
    assert game.mixed_strategy_profile(rational=False)[game.players[0]] == [0.5, 0.5]
    assert (
        game.mixed_strategy_profile(rational=True)[game.players[0]] ==
        [gbt.Rational("1/2"), gbt.Rational("1/2")]
    )


def test_set_probability_strategy():
    game = _create_strategic_game()
    profile = game.mixed_strategy_profile(rational=False)
    profile[game.players[0].strategies[0]] = 0.72
    assert profile[game.players[0].strategies[0]] == 0.72

    profile = game.mixed_strategy_profile(rational=True)
    profile[game.players[0].strategies[0]] = gbt.Rational("2/9")
    assert profile[game.players[0].strategies[0]] == gbt.Rational("2/9")


def test_set_probability_player():
    game = _create_strategic_game()
    profile = game.mixed_strategy_profile(rational=False)
    profile[game.players[0]] = [0.72, 0.28]
    assert profile[game.players[0]] == [0.72, 0.28]

    profile = game.mixed_strategy_profile(rational=True)
    profile[game.players[0]] = [gbt.Rational("7/9"), gbt.Rational("2/9")]
    assert profile[game.players[0]] == [gbt.Rational("7/9"), gbt.Rational("2/9")]


def test_payoffs():
    game = _create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).payoff(game.players[0]) == 0
    assert game.mixed_strategy_profile(rational=True).payoff(game.players[0]) == 0


def test_payoffs_by_label():
    game = _create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).payoff("Joe") == 0
    assert game.mixed_strategy_profile(rational=True).payoff("Joe") == 0


def test_strategy_value():
    game = _create_strategic_game()
    strategy = game.players[0].strategies[1]
    assert game.mixed_strategy_profile(rational=False).strategy_value(strategy) == 0
    assert game.mixed_strategy_profile(rational=True).strategy_value(strategy) == 0


def test_strategy_value_by_label():
    game = _create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).strategy_value("defect") == 0
    assert game.mixed_strategy_profile(rational=True).strategy_value("defect") == 0


def test_liap_value():
    game = _create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).liap_value() == 0
    assert game.mixed_strategy_profile(rational=True).liap_value() == 0


def test_as_behavior_roundtrip():
    game = gbt.Game.read_game("test_games/mixed_behavior_game.efg")
    assert (
        game.mixed_strategy_profile(rational=False).as_behavior().as_strategy() ==
        game.mixed_strategy_profile(rational=False)
    )
    assert (
        game.mixed_strategy_profile(rational=True).as_behavior().as_strategy() ==
        game.mixed_strategy_profile(rational=True)
    )


def test_as_behavior_error():
    game = _create_strategic_game()
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=False).as_behavior()
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=True).as_behavior()
