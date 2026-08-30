import pytest

import pygambit as gbt

from . import games


def _asymmetric_2x2() -> gbt.Game:
    return gbt.Game.from_arrays([[1, 2], [3, 4]], [[4, 3], [2, 1]])


def _asymmetric_poker_behavior_data() -> gbt.MixedBehaviorProfile:
    game = games.create_stripped_down_poker_efg()
    data = game.mixed_behavior_profile(rational=False)
    for player in game.players:
        for infoset in games.player_infosets(player):
            node = next(iter(infoset.members))
            data[node] = {a: float(i + 2) for i, a in enumerate(infoset.actions)}
    return data


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
                rational_result.profile[player.label][strategy]
                == pytest.approx(float_result.profile[player.label][strategy])
            )


@pytest.mark.parametrize("use_empirical", [False, True])
@pytest.mark.parametrize("local_max", [False, True])
def test_logit_estimate_behavior_completes(use_empirical: bool, local_max: bool):
    """logit_estimate() on a MixedBehaviorProfile returns a well-formed result instead of
    crashing or raising.

    Regression coverage for three bugs found while implementing rational-profile support
    for `liap_solve`/`logit_estimate` (gambitproject/gambit#721, #458):
    - `_estimate_behavior_fixedpoint()` segfaulted, because `EquationSystem` (in both
      efglogit.cc and nfglogit.cc) stored its `Game` by dangling reference rather than
      by value; `LogitBehaviorEstimate()` bound that reference straight to a temporary.
    - `LogitQREMixedBehaviorProfile.profile`/`.game` (nash.pxi) constructed their return
      value by calling the blocked `MixedBehaviorProfileDouble()`/`Game()` constructors
      directly instead of via their `.wrap()` factories, raising `ValueError`.
    - `_estimate_behavior_empirical()` assigned a `list` into a `MixedBehaviorProfile`,
      which requires a `Mapping` from action label to weight, raising `TypeError`.
    """
    data = _asymmetric_poker_behavior_data()
    result = gbt.qre.logit_estimate(data, use_empirical=use_empirical, local_max=local_max)
    assert isinstance(result.profile, gbt.MixedBehaviorProfileDouble)
    for player in data.game.players:
        for infoset in games.player_infosets(player):
            node = next(iter(infoset.members))
            probs = dict(result.profile[node])
            assert probs.keys() == set(infoset.actions)
            assert sum(probs.values()) == pytest.approx(1.0)
