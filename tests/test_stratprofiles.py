import pytest

import pygambit as gbt

from . import games


def test_remove_strategy():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.strategy_support_profile()
    strategy = game.players["Player 1"].strategies["1"]
    new_profile = support_profile.remove(strategy)
    assert len(support_profile) == len(new_profile) + 1
    assert strategy not in new_profile


def test_difference():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.strategy_support_profile()
    strategies = [p.strategies["1"] for p in game.players]
    dif_profile = game.strategy_support_profile(lambda x: x in strategies)
    new_profile = support_profile - dif_profile
    assert len(new_profile) == 3
    for strategy in strategies:
        assert strategy not in new_profile


def test_difference_error():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.strategy_support_profile()
    with pytest.raises(ValueError):
        game.strategy_support_profile(lambda x: x.label == "1") - support_profile


def test_intersection():
    game = games.read_from_file("mixed_strategy.nfg")
    first_profile = game.strategy_support_profile(lambda x: x.label not in ["1", "3"])
    second_profile = game.strategy_support_profile(lambda x: x.label != "3")
    intersect_profile = first_profile & second_profile
    assert len(intersect_profile) == 2
    assert intersect_profile <= first_profile
    assert intersect_profile <= second_profile


def test_intersection_error():
    game = games.read_from_file("mixed_strategy.nfg")
    first_profile = game.strategy_support_profile(lambda x: x.label not in ["1", "3"])
    second_profile = game.strategy_support_profile(
        lambda x: x.player.label == "Player 1" or x.label == "1"
    )
    with pytest.raises(ValueError):
        _ = first_profile & second_profile


def test_union():
    game = games.read_from_file("mixed_strategy.nfg")
    first_profile = game.strategy_support_profile(lambda x: x.label not in ["1", "3"])
    second_profile = game.strategy_support_profile(
        lambda x: x.player.label == "Player 1" or x.label == "1"
    )
    assert (first_profile | second_profile) == game.strategy_support_profile()


def test_undominated():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = gbt.supports.undominated_strategies_solve(game)
    while True:
        new_profile = gbt.supports.undominated_strategies_solve(profile)
        if new_profile == profile:
            break
        profile = new_profile
    assert profile == game.strategy_support_profile(lambda x: x.label == "1")


def test_remove_error():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.strategy_support_profile()
    profile = support_profile.remove(game.players["Player 2"].strategies["1"])
    with pytest.raises(gbt.UndefinedOperationError):
        profile.remove(game.players["Player 2"].strategies["2"])
