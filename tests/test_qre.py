import pytest

import pygambit as gbt


def _asymmetric_2x2() -> gbt.Game:
    return gbt.Game.from_arrays([[1, 2], [3, 4]], [[4, 3], [2, 1]])


@pytest.mark.parametrize("use_empirical", [False, True])
def test_logit_estimate_strategy_accepts_rational_profile(use_empirical: bool):
    """logit_estimate() accepts a MixedStrategyProfileRational, converting it to
    floating-point precision internally (gambitproject/gambit#458).
    """
    game = _asymmetric_2x2()
    data = game.mixed_strategy_profile(rational=True)
    data["1"] = {"1": "3", "2": "5"}
    data["2"] = {"1": "4", "2": "4"}
    result = gbt.qre.logit_estimate(data, use_empirical=use_empirical)
    assert isinstance(result.profile, gbt.MixedStrategyProfileDouble)
    assert result.data is data


def test_logit_estimate_strategy_rational_and_float_data_agree():
    game = _asymmetric_2x2()
    rational_data = game.mixed_strategy_profile(rational=True)
    rational_data["1"] = {"1": "3", "2": "5"}
    rational_data["2"] = {"1": "4", "2": "4"}
    float_data = rational_data.as_float()

    rational_result = gbt.qre.logit_estimate(rational_data)
    float_result = gbt.qre.logit_estimate(float_data)

    assert rational_result.lam == pytest.approx(float_result.lam)
    for player in game.players:
        for strategy in player.strategies:
            assert (
                rational_result.profile[player.label][strategy.label]
                == pytest.approx(float_result.profile[player.label][strategy.label])
            )
