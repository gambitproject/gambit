import pygambit as gbt
import pytest

from . import games


def test_remove_strategy():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strategy = support_profile[0]
    new_profile = support_profile.remove(strategy)
    assert len(support_profile) == len(new_profile) + 1
    assert strategy not in new_profile


def test_difference():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strat_list = [support_profile[0], support_profile[4]]
    dif_profile = gbt.StrategySupportProfile(game, strat_list)
    new_profile = support_profile - dif_profile
    assert len(new_profile) == 3
    for strategy in strat_list:
        assert strategy not in new_profile


def test_difference_error():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strat_list = [support_profile[0], support_profile[4]]
    dif_profile = gbt.StrategySupportProfile(game, strat_list)
    with pytest.raises(ValueError):
        dif_profile - support_profile


def test_intersection():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strat_list = [support_profile[0], support_profile[2],
                  support_profile[4]]
    fir_profile = gbt.StrategySupportProfile(game, strat_list)
    sec_profile = support_profile.remove(support_profile[2])
    new_profile = fir_profile & sec_profile
    assert len(new_profile) == 2
    assert new_profile <= sec_profile
    assert new_profile <= fir_profile


def test_intersection_error():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strat_list = [support_profile[0], support_profile[2],
                  support_profile[4]]
    fir_profile = gbt.StrategySupportProfile(game, strat_list)
    sec_profile = support_profile.remove(support_profile[4])
    with pytest.raises(ValueError):
        fir_profile & sec_profile


def test_union():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    strat_list = [support_profile[0], support_profile[2],
                  support_profile[4]]
    fir_profile = gbt.StrategySupportProfile(game, strat_list)
    sec_profile = support_profile.remove(support_profile[4])
    new_profile = fir_profile | sec_profile
    assert new_profile == support_profile


def test_undominated():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    new_profile = support_profile
    loop_profile = gbt.supports.undominated_strategies_solve(new_profile)
    while loop_profile != new_profile:
        new_profile = loop_profile
        loop_profile = gbt.supports.undominated_strategies_solve(new_profile)
    assert len(loop_profile) == 2
    assert loop_profile == gbt.StrategySupportProfile(
            game, [support_profile[0], support_profile[3]]
    )


def test_remove_error():
    game = games.read_from_file("mixed_strategy.nfg")
    support_profile = game.support_profile()
    profile = support_profile.remove(support_profile[3])
    with pytest.raises(gbt.UndefinedOperationError):
        profile.remove(profile[3])
