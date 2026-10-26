"""Tests of the strategy payoffs of mixed strategy profiles on game trees.

The regret and Liapunov value of a profile are computed from all of the players' strategy payoffs
at once.  On trees with perfect recall these are computed from sequences; they are compared with
the payoffs of the strategies computed one at a time.
"""

import random
import time

import pytest

import pygambit as gbt

from . import games


def _interior_profile(game: gbt.Game, seed: int, *, rational: bool = False):
    rng = random.Random(seed)
    data = []
    for player in game.players:
        weights = [rng.randint(1, 9) for _ in game.get_strategies(player)]
        total = sum(weights)
        data.append([gbt.Rational(w, total) if rational else w / total for w in weights])
    return game.mixed_strategy_profile(data=data, rational=rational)


def _reference(profile):
    """Max regret and Liapunov value from the payoffs of the strategies one at a time."""
    payoffs = profile.payoffs
    values = profile.strategy_values
    regrets = {p: max(v for _, v in values[p]) - payoffs[p] for p in profile.game.players}
    liap = sum(max(v - payoffs[p], 0) ** 2 for p in profile.game.players for _, v in values[p])
    return max(regrets.values()), liap, regrets


def _n_types_game(n: int) -> gbt.Game:
    game = gbt.ExtensiveGame(players=["1", "2"])
    game.append_event(gbt.H.path(), {f"t{i}": gbt.Rational(1, n) for i in range(n)})
    for i in range(n):
        game.append_move(gbt.H.path(f"t{i}"), "1", ["a", "b"])
        game.make_outcome(gbt.H.path(f"t{i}", "a"), {"1": 1, "2": 0}, f"w{i}")
    return game


PERFECT_RECALL = [
    pytest.param(games.create_stripped_down_poker_efg(), id="poker"),
    pytest.param(
        games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
        id="chance-in-middle-nonterminal-outcomes",
    ),
    pytest.param(games.read_from_file("basic_extensive_game.efg"), id="basic"),
]


@pytest.mark.parametrize("game", PERFECT_RECALL)
def test_double_matches_per_strategy_path(game: gbt.Game):
    profile = _interior_profile(game, seed=1)
    max_regret, liap, _ = _reference(profile)
    assert profile.max_regret() == pytest.approx(max_regret, abs=1e-12)
    assert profile.liap_value() == pytest.approx(liap, abs=1e-12)


@pytest.mark.parametrize("game", PERFECT_RECALL)
def test_rational_matches_per_strategy_path_exactly(game: gbt.Game):
    profile = _interior_profile(game, seed=2, rational=True)
    max_regret, liap, _ = _reference(profile)
    assert profile.max_regret() == max_regret
    assert profile.liap_value() == liap


@pytest.mark.parametrize("game", PERFECT_RECALL)
def test_pure_profile_matches_per_strategy_path(game: gbt.Game):
    profile = game.mixed_strategy_profile(rational=True)
    max_regret, liap, _ = _reference(profile)
    assert profile.max_regret() == max_regret
    assert profile.liap_value() == liap


def test_imperfect_recall_falls_back_via_conversion_constructor():
    game = games.read_from_file("gilboa_two_am_agents.efg")
    assert not game.is_perfect_recall
    behavior = game.mixed_behavior_profile(rational=True)
    profile = behavior.as_strategy()
    max_regret, liap, _ = _reference(profile)
    assert profile.max_regret() == max_regret
    assert profile.liap_value() == liap


def test_n_types_regret_is_fast():
    game = _n_types_game(12)
    profile = game.mixed_strategy_profile()
    start = time.perf_counter()
    profile.max_regret()
    assert time.perf_counter() - start < 1.0
