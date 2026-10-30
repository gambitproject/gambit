"""Tests of the Nash solvers on a Bayesian action-graph game (BAGG).

`enumpure_solve` and `enumpoly_solve` solve the agent form and return BAGG mixed behavior
profiles; the golden values below are the unchanged agent-form equilibria regrouped by
(player, type).  The solvers that cannot apply raise `UnsupportedForBayesianGameError`.
"""

import numpy as np
import pytest

import pygambit as gbt
from pygambit import levelk

from . import bagg_oracle, games

R = gbt.Rational
SHARED = "bayesian_2p2t_shared_node.bagg"
TWO_BY_TWO = "2x2.bagg"
COFFEE = "Bayesian-Coffee-3-2-2-3.bagg"


def _grouped(profile) -> dict:
    return {
        (player, infoset): [probability for _, probability in profile[player, infoset]]
        for player in profile.game.players
        for infoset in profile.game.get_infosets(player)
    }


def _pure(num_actions: int, action: int) -> list:
    return [R(1) if k == action else R(0) for k in range(num_actions)]


COFFEE_PURE = [
    {("1", "1"): 1, ("1", "2"): 0, ("2", "1"): 0, ("2", "2"): 0, ("3", "1"): 0, ("3", "2"): 0},
    {("1", "1"): 0, ("1", "2"): 0, ("2", "1"): 1, ("2", "2"): 0, ("3", "1"): 0, ("3", "2"): 0},
    {("1", "1"): 0, ("1", "2"): 0, ("2", "1"): 0, ("2", "2"): 0, ("3", "1"): 1, ("3", "2"): 0},
]


def test_enumpure_two_by_two_golden():
    result = gbt.nash.enumpure_solve(games.read_from_file(TWO_BY_TWO))
    assert [_grouped(eq) for eq in result.equilibria] == [
        {("1", "1"): [R(1), R(0)], ("2", "1"): [R(1), R(0)]},
        {("1", "1"): [R(0), R(1)], ("2", "1"): [R(0), R(1)]},
    ]


def test_enumpure_coffee_golden():
    result = gbt.nash.enumpure_solve(games.read_from_file(COFFEE))
    assert [_grouped(eq) for eq in result.equilibria] == [
        {key: _pure(7, action) for key, action in expected.items()} for expected in COFFEE_PURE
    ]


def test_enumpoly_two_by_two_golden():
    result = gbt.nash.enumpoly_solve(games.read_from_file(TWO_BY_TWO))
    expected = [
        {("1", "1"): [1, 0], ("2", "1"): [1, 0]},
        {("1", "1"): [0, 1], ("2", "1"): [0, 1]},
        {("1", "1"): [10 / 11, 1 / 11], ("2", "1"): [10 / 11, 1 / 11]},
    ]
    assert len(result.equilibria) == len(expected)
    for eq, exp in zip(result.equilibria, expected, strict=True):
        grouped = _grouped(eq)
        for key, probs in exp.items():
            assert grouped[key] == pytest.approx(probs, abs=1e-8)


def test_enumpoly_coffee_golden():
    result = gbt.nash.enumpoly_solve(games.read_from_file(COFFEE), stop_after=1)
    assert len(result.equilibria) == 1
    grouped = _grouped(result.equilibria[0])
    for key, action in COFFEE_PURE[2].items():
        assert grouped[key] == pytest.approx([float(p) for p in _pure(7, action)], abs=1e-8)


@pytest.mark.parametrize("solver", [gbt.nash.enumpure_solve, gbt.nash.enumpoly_solve])
@pytest.mark.parametrize("path", [SHARED, TWO_BY_TWO, COFFEE])
def test_equilibria_are_bagg_profiles_with_zero_regret(solver, path):
    kwargs = {"stop_after": 1} if solver is gbt.nash.enumpoly_solve and path == COFFEE else {}
    result = solver(games.read_from_file(path), **kwargs)
    assert result.equilibria
    for eq in result.equilibria:
        assert isinstance(eq, gbt.BAGGMixedBehaviorProfile)
        assert eq.max_regret() == pytest.approx(0, abs=1e-8)


@pytest.mark.parametrize("solver", [gbt.nash.enumpure_solve, gbt.nash.enumpoly_solve])
def test_equilibria_match_the_oracle(solver):
    oracle = bagg_oracle.bagg_to_efg(SHARED)
    result = solver(games.read_from_file(SHARED))
    assert result.equilibria
    for eq in result.equilibria:
        strategies = {
            (int(player), int(infoset)): [float(p) for p in probs]
            for (player, infoset), probs in _grouped(eq).items()
        }
        reference = bagg_oracle.behavior_profile(oracle, strategies, rational=False)
        assert reference.max_regret() == pytest.approx(0, abs=1e-8)
        for player in ("1", "2"):
            assert float(eq.payoffs[player]) == pytest.approx(
                float(reference.payoffs[player]), abs=1e-8
            )


@pytest.mark.parametrize("solver", [gbt.nash.enumpure_solve, gbt.nash.enumpoly_solve])
def test_nash_callback_receives_bagg_profiles(solver):
    seen = []
    result = solver(games.read_from_file(TWO_BY_TWO), nash_callback=seen.append)
    assert len(seen) == len(result.equilibria) > 0
    assert all(isinstance(eq, gbt.BAGGMixedBehaviorProfile) for eq in seen)


RAISING = [
    pytest.param(lambda g: gbt.nash.lcp_solve(g), "lcp_solve", id="lcp"),
    pytest.param(lambda g: gbt.nash.lcp_solve(g, rational=False), "lcp_solve", id="lcp-float"),
    pytest.param(lambda g: gbt.nash.lp_solve(g), "lp_solve", id="lp"),
    pytest.param(lambda g: gbt.nash.enummixed_solve(g), "enummixed_solve", id="enummixed"),
    pytest.param(
        lambda g: gbt.nash.enummixed_solve(g, lrsnash_path="/nonexistent/lrsnash"),
        "enummixed_solve",
        id="enummixed-lrs",
    ),
    pytest.param(
        lambda g: gbt.nash.enumpure_agent_solve(g),
        "enumpure_agent_solve",
        id="enumpure-agent",
    ),
    pytest.param(
        lambda g: gbt.nash.liap_agent_solve(g.mixed_behavior_profile()),
        "liap_agent_solve",
        id="liap-agent",
    ),
    pytest.param(
        lambda g: gbt.nash.enumpoly_solve(g, phcpack_path="/nonexistent/phc"),
        "enumpoly_solve",
        id="enumpoly-phcpack",
    ),
]


@pytest.mark.parametrize("path", [TWO_BY_TWO, SHARED, COFFEE])
@pytest.mark.parametrize("call,name", RAISING)
def test_inapplicable_solvers_raise_the_dedicated_error(call, name, path):
    game = games.read_from_file(path)
    with pytest.raises(gbt.UnsupportedForBayesianGameError) as excinfo:
        call(game)
    assert str(excinfo.value) == f"{name}(): not defined for Bayesian games"


def test_python_level_external_solvers_fail_clearly():
    game = games.read_from_file(TWO_BY_TWO)
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="lrsnash_solve"):
        gbt.nashlrs.lrsnash_solve(game, lrsnash_path="/nonexistent/lrsnash")
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="phcpack_solve"):
        gbt.nashphc.phcpack_solve(game, "/nonexistent/phc", 1e-8)


def _start_near_a_pure_equilibrium(game, offset=0, rational=False):
    """A starting profile at a pure equilibrium of `game` (`offset` 0) or mixed into the uniform
    profile by `offset`.

    Lyapunov minimization approaches a boundary equilibrium slowly and can stall elsewhere, so
    its tests start at or very near an equilibrium, where convergence does not depend on the
    path the minimizer takes.
    """
    pure = gbt.nash.enumpure_solve(game).equilibria[0]
    start = game.mixed_behavior_profile(rational=rational)
    offset = gbt.Rational(offset) if rational else float(offset)
    for player in game.players:
        for infoset in game.get_infosets(player):
            weights = [(action, float(prob)) for action, prob in pure[player, infoset]]
            start[player, infoset] = {
                action: (1 - offset) * (gbt.Rational(round(prob)) if rational else prob)
                + offset / len(weights)
                for action, prob in weights
            }
    return start


START_SOLVERS = {
    "liap": lambda g, rng, **kw: gbt.nash.liap_solve(
        _start_near_a_pure_equilibrium(g), maxregret=1e-6, maxiter=10000, **kw
    ),
    "simpdiv": lambda g, rng, **kw: gbt.nash.simpdiv_solve(
        g.random_behavior_profile(denom=4, gen=rng), maxregret=gbt.Rational(1, 100), **kw
    ),
    "hp": lambda g, rng, **kw: gbt.nash.hp_solve(g.random_behavior_profile(gen=rng), **kw),
    "ipa": lambda g, rng, **kw: gbt.nash.ipa_solve(g.random_behavior_profile(gen=rng), **kw),
    "gnm": lambda g, rng, **kw: gbt.nash.gnm_solve(g.random_behavior_profile(gen=rng), **kw),
}
START_GAMES = {
    "liap": [SHARED, TWO_BY_TWO],
    "simpdiv": [SHARED, TWO_BY_TWO, COFFEE],
    "hp": [SHARED, TWO_BY_TWO, COFFEE],
    "ipa": [SHARED, TWO_BY_TWO, COFFEE],
    "gnm": [SHARED, TWO_BY_TWO, COFFEE],
}
START_CASES = [
    pytest.param(name, path, id=f"{name}-{path}")
    for name, paths in START_GAMES.items()
    for path in paths
]


def _rng() -> np.random.Generator:
    return np.random.default_rng(7)


def _equilibria(result) -> list:
    return result.equilibria if hasattr(result, "equilibria") else [result.equilibrium]


@pytest.mark.parametrize("solver,path", START_CASES)
def test_start_solvers_return_bagg_profiles_with_zero_regret(solver, path):
    game = games.read_from_file(path)
    result = START_SOLVERS[solver](game, _rng())
    equilibria = _equilibria(result)
    assert equilibria
    for eq in equilibria:
        assert isinstance(eq, gbt.BAGGMixedBehaviorProfile)
        assert eq.max_regret() == pytest.approx(0, abs=1e-3)


def test_simpdiv_with_a_rational_start_stays_exact():
    game = games.read_from_file(TWO_BY_TWO)
    start = game.random_behavior_profile(denom=4, gen=np.random.default_rng(1))
    result = gbt.nash.simpdiv_solve(start, maxregret=R(1, 100))
    assert isinstance(result.equilibrium, gbt.BAGGMixedBehaviorProfileRational)
    assert isinstance(result.start, gbt.BAGGMixedBehaviorProfileRational)


def test_simpdiv_rejects_a_double_start_as_the_tree_does():
    game = games.read_from_file(TWO_BY_TWO)
    start = game.random_behavior_profile(gen=np.random.default_rng(1))
    assert isinstance(start, gbt.BAGGMixedBehaviorProfileDouble)
    with pytest.raises(TypeError, match="BAGGMixedBehaviorProfileRational"):
        gbt.nash.simpdiv_solve(start)


@pytest.mark.parametrize("rational", [False, True])
def test_liap_accepts_either_precision_of_start(rational):
    game = games.read_from_file(TWO_BY_TWO)
    start = _start_near_a_pure_equilibrium(game, offset=gbt.Rational(1, 100), rational=rational)
    result = gbt.nash.liap_solve(start, maxregret=1e-6, maxiter=10000)
    assert result.success
    assert isinstance(result.equilibrium, gbt.BAGGMixedBehaviorProfileDouble)


def test_hp_rejects_a_rational_start_as_on_a_tree():
    game = games.read_from_file(TWO_BY_TWO)
    with pytest.raises(TypeError, match="BAGGMixedBehaviorProfileDouble"):
        gbt.nash.hp_solve(game.random_behavior_profile(denom=4))


@pytest.mark.parametrize("solver", ["liap", "simpdiv", "hp", "ipa", "gnm"])
def test_equilibria_match_the_oracle_payoffs(solver):
    oracle = bagg_oracle.bagg_to_efg(SHARED)
    result = START_SOLVERS[solver](games.read_from_file(SHARED), _rng())
    for eq in _equilibria(result):
        strategies = {
            (int(player), int(infoset)): [float(p) for p in probs]
            for (player, infoset), probs in _grouped(eq).items()
        }
        reference = bagg_oracle.behavior_profile(oracle, strategies, rational=False)
        assert reference.max_regret() == pytest.approx(0, abs=1e-3)
        for player in ("1", "2"):
            assert float(eq.payoffs[player]) == pytest.approx(
                float(reference.payoffs[player]), abs=1e-3
            )


def test_events_carry_bagg_profiles():
    game = games.read_from_file(TWO_BY_TWO)
    expectations = {
        "liap": (gbt.LiapStartEvent,),
        "simpdiv": (gbt.SimpdivStartEvent,),
        "hp": (gbt.HPStepEvent,),
        "gnm": (gbt.GNMPerturbationEvent, gbt.GNMStartEvent, gbt.GNMStepEvent),
        "ipa": (gbt.IPAStepEvent,),
    }
    for name, event_types in expectations.items():
        events = []
        START_SOLVERS[name](game, _rng(), event_callback=events.append)
        for event_type in event_types:
            carried = [e for e in events if isinstance(e, event_type)]
            assert carried, (name, event_type)
            assert all(isinstance(e.profile, gbt.BAGGMixedBehaviorProfile) for e in carried)


def test_liap_runs_on_coffee_from_near_a_pure_equilibrium():
    """The acceptance criterion is on the agent form, so the players' ex-ante `max_regret` of
    the result is looser than `maxregret` suggests."""
    game = games.read_from_file(COFFEE)
    start = _start_near_a_pure_equilibrium(game, offset=0.001)
    result = gbt.nash.liap_solve(start, maxregret=1e-3, maxiter=10000)
    assert result.success
    assert isinstance(result.equilibrium, gbt.BAGGMixedBehaviorProfileDouble)
    assert result.equilibrium.max_regret() < 0.02


@pytest.mark.parametrize("name", ["liap", "simpdiv", "ipa", "gnm"])
def test_start_solvers_nash_callback_receives_bagg_profiles(name):
    game = games.read_from_file(TWO_BY_TWO)
    seen = []
    START_SOLVERS[name](game, _rng(), nash_callback=seen.append)
    assert seen
    assert all(isinstance(eq, gbt.BAGGMixedBehaviorProfile) for eq in seen)


WRONG_INPUTS = [
    pytest.param(lambda g: gbt.nash.liap_solve(g), "liap_solve", id="liap-game"),
    pytest.param(lambda g: gbt.nash.simpdiv_solve(g), "simpdiv_solve", id="simpdiv-game"),
    pytest.param(lambda g: gbt.nash.hp_solve(g), "hp_solve", id="hp-game"),
    pytest.param(lambda g: gbt.nash.ipa_solve(g), "ipa_solve", id="ipa-game"),
    pytest.param(lambda g: gbt.nash.gnm_solve(g), "gnm_solve", id="gnm-game"),
]


@pytest.mark.parametrize("call,name", WRONG_INPUTS)
def test_a_game_is_not_a_start_on_a_bagg(call, name):
    with pytest.raises(TypeError, match=f"{name}.*BAGGMixedBehaviorProfile"):
        call(games.read_from_file(TWO_BY_TWO))


def test_liap_end_event_carries_a_bagg_profile():
    game = games.read_from_file(SHARED)
    events = []
    gbt.nash.liap_solve(
        game.random_behavior_profile(gen=_rng()),
        maxregret=1e-6,
        maxiter=1,
        event_callback=events.append,
    )
    ends = [e for e in events if isinstance(e, gbt.LiapEndEvent)]
    assert len(ends) == 1
    assert isinstance(ends[0].profile, gbt.BAGGMixedBehaviorProfileDouble)


@pytest.mark.parametrize("path", [TWO_BY_TWO, SHARED])
def test_level_k_modelling_refuses_a_bagg(path):
    game = games.read_from_file(path)
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="compute_coghier"):
        levelk.compute_coghier(game, tau=1.0, lam=1.0)
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="fit_coghier"):
        levelk.fit_coghier(game, [1, 1], 0.5, 1.5, 0.5, 1.5)
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="logit_br"):
        levelk.logit_br(game, None, 1.0)


@pytest.mark.parametrize("path", [SHARED, TWO_BY_TWO, COFFEE])
def test_logit_returns_a_bagg_profile_with_small_regret(path):
    result = gbt.nash.logit_solve(games.read_from_file(path), maxregret=1.0e-6)
    assert result.success
    assert isinstance(result.equilibrium, gbt.BAGGMixedBehaviorProfile)
    assert result.equilibrium.max_regret() < 1.0e-3


def _logit_events(path: str) -> list:
    events = []
    gbt.qre.logit_solve_lambda(
        games.read_from_file(path), [1.0, 20.0], event_callback=events.append
    )
    return events


@pytest.mark.parametrize("path", [SHARED, TWO_BY_TWO, COFFEE])
def test_logit_path_events_carry_bagg_qre_profiles(path):
    paths = [e for e in _logit_events(path) if isinstance(e, gbt.LogitPathEvent)]
    assert paths
    for event in paths:
        qre = event.qre
        assert isinstance(qre, gbt.BAGGLogitQREMixedBehaviorProfile)
        assert isinstance(qre.profile, gbt.BAGGMixedBehaviorProfile)
        assert qre.game is not None
        assert qre.lam >= 0.0
        assert isinstance(qre.log_like, float)
    assert paths[0].qre.lam == 0.0
    uniform = paths[0].qre.profile
    for player in uniform.game.players:
        for infoset in uniform.game.get_infosets(player):
            probs = [float(p) for _, p in uniform[player, infoset]]
            assert probs == pytest.approx([1 / len(probs)] * len(probs))


def test_logit_lambda_returns_bagg_qre_profiles():
    qres = gbt.qre.logit_solve_lambda(games.read_from_file(COFFEE), [1.0, 5.0])
    assert [q.lam for q in qres] == pytest.approx([1.0, 5.0], abs=1e-6)
    assert all(isinstance(q, gbt.BAGGLogitQREMixedBehaviorProfile) for q in qres)


def test_logit_branch_returns_bagg_qre_profiles():
    branch = gbt.qre.logit_solve_branch(games.read_from_file(TWO_BY_TWO), maxregret=1.0e-2)
    assert all(isinstance(q, gbt.BAGGLogitQREMixedBehaviorProfile) for q in branch)
    assert [q.lam for q in branch] == sorted(q.lam for q in branch)


def test_logit_qre_profiles_cannot_be_created_directly():
    with pytest.raises(ValueError):
        gbt.BAGGLogitQREMixedBehaviorProfile()


def test_logit_bifurcations_carry_bagg_qre_profiles():
    result = gbt.nash.logit_solve(games.read_from_file(TWO_BY_TWO), maxregret=1.0e-4)
    for bifurcation in result.bifurcations:
        assert isinstance(bifurcation.before, gbt.BAGGLogitQREMixedBehaviorProfile)
        assert isinstance(bifurcation.after, gbt.BAGGLogitQREMixedBehaviorProfile)


@pytest.mark.parametrize("use_empirical", [False, True])
@pytest.mark.parametrize("path", [TWO_BY_TWO, COFFEE])
def test_qre_fitting_refuses_a_bagg(path, use_empirical):
    data = games.read_from_file(path).mixed_behavior_profile()
    with pytest.raises(gbt.UnsupportedForBayesianGameError):
        gbt.qre.logit_estimate(data, use_empirical=use_empirical)


def test_logit_qre_repr_and_docs_state_the_type_level_meaning():
    assert "type" in gbt.BAGGLogitQREMixedBehaviorProfile.__doc__
    assert "precision" in gbt.BAGGLogitQREMixedBehaviorProfile.__doc__
    assert "type-level" in gbt.nash.logit_solve.__doc__
