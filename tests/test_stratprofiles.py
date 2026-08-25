import pygambit as gbt

from . import games


def test_undominated():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = gbt.supports.undominated_strategies_solve(game)
    while True:
        new_profile = gbt.supports.undominated_strategies_solve(profile)
        if new_profile == profile:
            break
        profile = new_profile
    assert profile == game.strategy_support_profile(lambda x: x.label == "1")
