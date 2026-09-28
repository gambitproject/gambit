"""Tests of the derivatives of action values with respect to log action probabilities.

The derivatives are computed by MixedBehaviorProfile::DiffActionValues, exposed privately to
Python for testing; they are checked against central finite differences of the profile's own
action values.
"""
import math
import random

import numpy as np
import pytest

import pygambit as gbt

from . import games

STEP = 1.0e-6
TOL = 1.0e-7


def _interior_profile(game: gbt.Game, seed: int):
    """A random totally mixed profile, with the information sets and their action labels in
    the order of the positions of the profile."""
    rng = random.Random(seed)
    profile = game.mixed_behavior_profile(rational=False)
    infosets = []
    for player in game.players:
        for history, mixed_action in profile[player]:
            infosets.append((gbt.H.path(*history.actions), [label for label, _ in mixed_action]))
    weights = {}
    for selector, labels in infosets:
        weights[selector] = {label: rng.uniform(0.2, 1.0) for label in labels}
        profile[selector] = weights[selector]
    return profile, infosets, weights


def _action_values(profile, entries) -> np.ndarray:
    values = profile.action_values
    return np.array([values[selector][label] for selector, label in entries], dtype=float)


@pytest.mark.parametrize(
    "game",
    [
        pytest.param(games.create_stripped_down_poker_efg(), id="stripped_down_poker"),
        pytest.param(games.create_stripped_down_poker_efg(nonterm_outcomes=True),
                     id="stripped_down_poker_nonterm_outcomes"),
        pytest.param(games.create_kuhn_poker_efg(), id="kuhn_poker"),
        pytest.param(games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
                     id="chance_in_middle_nonterm_outcomes"),
        pytest.param(games.read_from_file("nature_leaves_generic.efg"),
                     id="nature_leaves_generic"),
        pytest.param(games.read_from_file("3_player_with_nonterm_outcomes.efg"),
                     id="3_player_nonterm_outcomes"),
        pytest.param(games.create_allpay_correlated_types_efg(gbt.Rational(3, 5), True, 3),
                     id="allpay_cv_p60_b3"),
    ],
)
def test_diff_action_values_matches_finite_differences(game: gbt.Game):
    profile, infosets, weights = _interior_profile(game, seed=1)
    entries = [(selector, label) for selector, labels in infosets for label in labels]
    analytic = np.array(profile._diff_action_values())
    assert analytic.shape == (len(entries), len(entries))

    numeric = np.zeros_like(analytic)
    for j, (selector, label) in enumerate(entries):
        columns = []
        for sign in (+1, -1):
            perturbed = game.mixed_behavior_profile(rational=False)
            for other, w in weights.items():
                perturbed[other] = dict(w)
            shifted = dict(weights[selector])
            shifted[label] *= math.exp(sign * STEP)
            perturbed[selector] = shifted
            columns.append(_action_values(perturbed, entries))
        numeric[:, j] = (columns[0] - columns[1]) / (2 * STEP)

    # Derivatives with respect to actions at the same information set are defined as zero
    start = 0
    for _, labels in infosets:
        block = slice(start, start + len(labels))
        numeric[block, block] = 0.0
        assert np.all(analytic[block, block] == 0.0)
        start += len(labels)

    np.testing.assert_allclose(analytic, numeric, atol=TOL, rtol=0)
