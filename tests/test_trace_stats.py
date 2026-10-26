"""Tests of the private path-tracing statistics reported by the logit and HP solvers.

These statistics are deliberately undocumented and not part of the public API; the tests
check that they are populated, internally consistent, and do not leak into public
behaviour.
"""
import dataclasses

import pytest

import pygambit as gbt

from . import games

TIME_TOL = 1.0e-3

TIMING_FIELDS = [
    "function_seconds", "jacobian_seconds", "factorization_seconds", "newton_seconds",
    "terminate_seconds", "callback_seconds",
]


def _check_trace_invariants(stats) -> None:
    """Relationships that must hold for any trace that ends by its termination or
    criterion function, rather than by exceeding the corrector's iteration limit."""
    assert stats.kind == "trace"
    # One Jacobian evaluation and factorisation per predictor attempt, plus one for the
    # tangent at the starting point.
    assert stats.jacobian_evals == stats.predictor_attempts + 1
    assert stats.factorizations == stats.jacobian_evals
    # Each corrector iteration evaluates the system of equations exactly once.
    assert stats.function_evals == stats.corrector_iterations
    # Every predictor attempt is either accepted or rejected for exactly one reason.
    assert stats.predictor_attempts == (
        stats.accepted_steps + stats.rejected_distance + stats.rejected_contraction
        + stats.rejected_orientation
    )
    _check_timings(stats)


def _check_timings(stats) -> None:
    timings = [getattr(stats, f) for f in TIMING_FIELDS]
    assert all(t >= 0.0 for t in timings)
    assert sum(timings) <= stats.total_seconds + TIME_TOL


@pytest.mark.parametrize(
    "game,branch",
    [
        pytest.param(games.create_stripped_down_poker_efg(), gbt.gambit._logit_behavior_branch,
                     id="behavior-poker"),
        pytest.param(games.create_stripped_down_poker_efg(), gbt.gambit._logit_strategy_branch,
                     id="strategy-poker"),
        pytest.param(games.create_2x2_symmetric_coordination_nfg(),
                     gbt.gambit._logit_strategy_branch, id="strategy-2x2"),
        pytest.param(games.read_from_file("2x2_fraction_types.bagg"),
                     gbt.gambit._logit_strategy_branch, id="strategy-bagg"),
    ],
)
def test_logit_branch_trace_stats(game: gbt.Game, branch):
    profiles, stats = branch(game, 0.01, 0.1, 1.1, _with_stats=True)
    assert len(stats) == 1
    # The tracer reports the starting point and then each accepted step.
    assert stats[0].accepted_steps == len(profiles) - 1
    _check_trace_invariants(stats[0])


def test_logit_branch_without_stats_returns_profiles_only():
    game = games.create_stripped_down_poker_efg()
    profiles = gbt.gambit._logit_behavior_branch(game, 0.01, 0.1, 1.1)
    assert isinstance(profiles, list)
    assert all(isinstance(p, gbt.LogitQREMixedBehaviorProfile) for p in profiles)


@pytest.mark.parametrize(
    "lam_solve", [gbt.gambit._logit_behavior_lambda, gbt.gambit._logit_strategy_lambda]
)
def test_logit_lambda_trace_stats_one_entry_per_target(lam_solve):
    game = games.create_stripped_down_poker_efg()
    targets = [0.5, 1.0, 2.0]
    profiles, stats = lam_solve(game, targets, 0.1, 1.1, _with_stats=True)
    assert [p.lam for p in profiles] == pytest.approx(targets)
    assert len(stats) == len(targets)
    for entry in stats:
        _check_trace_invariants(entry)


def test_logit_lambda_records_orientation_rejection_at_bifurcation():
    """The 2x2 symmetric coordination game's principal branch has a bifurcation
    (see test_logit_solve_lambda_reports_bifurcation_and_perturbation_events);
    detecting it rejects at least one step for a flip in tangent orientation."""
    game = games.create_2x2_symmetric_coordination_nfg()
    _, stats = gbt.gambit._logit_strategy_lambda(game, [20.0], 0.03, 1.1, _with_stats=True)
    assert stats[0].rejected_orientation >= 1
    _check_trace_invariants(stats[0])


@pytest.mark.parametrize("use_strategic", [False, True])
def test_logit_solve_result_has_trace_stats(use_strategic: bool):
    game = games.create_stripped_down_poker_efg()
    result = gbt.nash.logit_solve(game, use_strategic=use_strategic)
    assert len(result._trace_stats) == 1
    _check_trace_invariants(result._trace_stats[0])


def test_trace_stats_hidden_from_repr_and_equality():
    game = games.create_stripped_down_poker_efg()
    result = gbt.nash.logit_solve(game)
    assert "_trace_stats" not in repr(result)
    assert dataclasses.replace(result, _trace_stats=[]) == result


def test_hp_solve_result_has_trace_and_polish_stats():
    game = games.create_hs1988_base_game()
    prior = game.mixed_strategy_profile(
        data=[[0.5, 0.5], [2.0 / 3.0, 1.0 / 3.0]], rational=False
    )
    result = gbt.nash.hp_solve(prior)
    assert result.success
    assert [s.kind for s in result._trace_stats] == ["trace", "polish"]
    trace, polish = result._trace_stats
    _check_trace_invariants(trace)
    # Polishing is Newton iteration: each iteration evaluates the system and its
    # Jacobian once, and factorises once, with no predictor steps.
    assert polish.predictor_attempts == 0
    assert polish.function_evals == polish.corrector_iterations
    assert polish.jacobian_evals == polish.corrector_iterations
    assert polish.factorizations == polish.corrector_iterations
    _check_timings(polish)
