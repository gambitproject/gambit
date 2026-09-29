import io

import numpy as np
import pytest

import pygambit as gbt

from . import games


def _asymmetric_2x2() -> gbt.Game:
    return gbt.StrategicGame.from_arrays([[1, 2], [3, 4]], [[4, 3], [2, 1]])


def _asymmetric_poker_behavior_data() -> gbt.MixedBehaviorProfile:
    game = games.create_stripped_down_poker_efg()
    data = game.mixed_behavior_profile(rational=False)
    for player in game.players:
        for history in games.player_infosets(game, player):
            selector = gbt.H.path(*history.actions)
            data[selector] = {
                a: float(i + 2) for i, a in enumerate(game.get_actions(selector))
            }
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
        for strategy in game.get_strategies(player):
            assert (
                rational_result.profile[player][strategy]
                == pytest.approx(float_result.profile[player][strategy])
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
        for history in games.player_infosets(data.game, player):
            selector = gbt.H.path(*history.actions)
            probs = dict(result.profile[selector])
            assert probs.keys() == set(data.game.get_actions(selector))
            assert sum(probs.values()) == pytest.approx(1.0)


def _logit_strategy_path(game: gbt.Game, lam: float) -> tuple[np.ndarray, list[tuple[int, int]]]:
    events = []
    _, stats = gbt.gambit._logit_strategy_lambda(game, [lam], 0.03, 1.1, events.append,
                                                 _with_stats=True)
    points = np.array([
        [event.qre.lam] + [event.qre[i] for i in range(len(event.qre))]
        for event in events if isinstance(event, gbt.LogitPathEvent)
    ])
    return points, [(s.accepted_steps, s.corrector_iterations) for s in stats]


@pytest.mark.parametrize(
    "game",
    [
        pytest.param(games.create_stripped_down_poker_efg(), id="stripped_down_poker"),
        pytest.param(games.create_stripped_down_poker_efg(nonterm_outcomes=True),
                     id="stripped_down_poker_nonterm_outcomes"),
        pytest.param(games.read_from_file("3_player_with_nonterm_outcomes.efg"),
                     id="3_player_with_nonterm_outcomes"),
        pytest.param(games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
                     id="chance_in_middle_with_nonterm_outcomes"),
        pytest.param(games.read_from_file("nature_leaves_generic.efg"),
                     id="nature_leaves_generic"),
    ],
)
def test_logit_strategy_tree_matches_table_form(game: gbt.Game):
    """Strategic-form logit on a tree traces the same path as on the table game written from
    its reduced strategic form.  A fixed lambda is used so the comparison does not depend on
    the payoff range, which is defined differently for the two representations."""
    table = gbt.read_nfg(io.BytesIO(game.to_nfg().encode()))
    tree_points, tree_counts = _logit_strategy_path(game, 5.0)
    table_points, table_counts = _logit_strategy_path(table, 5.0)
    assert tree_counts == table_counts
    assert tree_points.shape == table_points.shape
    assert np.max(np.abs(tree_points - table_points)) <= 1.0e-12
