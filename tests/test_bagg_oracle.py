"""Tests of the extensive-game oracle for BAGGs against hand-derived constants.

The test game is ``bayesian_2p2t_shared_node.bagg``.  Writing nodes as n0..n5:

* player 1 type 1 (prior 1/3) chooses between n0 and n1; type 2 (prior 2/3) between n1 and n2;
* player 2 type 1 (prior 1/4) chooses between n3 and n4; type 2 (prior 3/4) between n4 and n5.

Player i's payoff depends only on the opponent's node: u1(n0, n1, n2 | n3, n4, n5) is
(4 0 2 / 1 3 0 / 0 2 5) and u2(n3, n4, n5 | n0, n1, n2) is (1 3 2 / 2 0 1 / 0 1 4).

Each profile below lists the weights by action position per (player, type), the expected
ex-ante payoffs, and the player-level max regret.  For example, uniform play gives player 1
type 1 conditional values 5/4 (n0) and 13/8 (n1) against player 2's node marginal
(n3, n4, n5) = (1/8, 1/2, 3/8), so its ex-ante payoff is 1/3 * 23/16 + 2/3 * 9/4 = 95/48.
"""

import pytest

import pygambit as gbt

from . import bagg_oracle

R = gbt.Rational
GAME = "bayesian_2p2t_shared_node.bagg"

PURE_NASH = {(1, 1): [1, 0], (1, 2): [0, 1], (2, 1): [1, 0], (2, 2): [0, 1]}
FIRST_ACTIONS = {(1, 1): [1, 0], (1, 2): [1, 0], (2, 1): [1, 0], (2, 2): [1, 0]}
SAME_NODES = {(1, 1): [0, 1], (1, 2): [1, 0], (2, 1): [0, 1], (2, 2): [1, 0]}
UNIFORM = {(1, 1): [1, 1], (1, 2): [1, 1], (2, 1): [1, 1], (2, 2): [1, 1]}
MIXED = {(1, 1): [1, 3], (1, 2): [1, 1], (2, 1): [1, 0], (2, 2): [1, 2]}

PROFILES = [
    pytest.param(PURE_NASH, (R(10, 3), R(29, 12)), R(0), id="pure-nash"),
    pytest.param(FIRST_ACTIONS, (R(2), R(13, 12)), R(1, 2), id="first-actions"),
    pytest.param(SAME_NODES, (R(3), R(0)), R(3, 2), id="common-nodes"),
    pytest.param(UNIFORM, (R(95, 48), R(21, 16)), R(31, 48), id="uniform"),
    pytest.param(MIXED, (R(7, 4), R(41, 24)), R(11, 12), id="mixed"),
]


@pytest.fixture(scope="module")
def oracle() -> gbt.Game:
    return bagg_oracle.bagg_to_efg(GAME)


def test_oracle_structure(oracle):
    assert list(oracle.players) == ["1", "2"]
    for player in ("1", "2"):
        assert len(oracle.get_infosets(player)) == 2


@pytest.mark.parametrize("strategies,expected_payoffs,expected_regret", PROFILES)
def test_oracle_ex_ante_payoffs(oracle, strategies, expected_payoffs, expected_regret):
    profile = bagg_oracle.behavior_profile(oracle, strategies)
    assert profile.payoffs["1"] == expected_payoffs[0]
    assert profile.payoffs["2"] == expected_payoffs[1]


@pytest.mark.parametrize("strategies,expected_payoffs,expected_regret", PROFILES)
def test_oracle_player_level_max_regret(oracle, strategies, expected_payoffs, expected_regret):
    profile = bagg_oracle.behavior_profile(oracle, strategies)
    assert profile.max_regret() == expected_regret


def test_oracle_pure_nash_is_equilibrium(oracle):
    profile = bagg_oracle.behavior_profile(oracle, PURE_NASH)
    assert profile.max_regret() == 0
    assert profile.agent_max_regret() == 0


def test_oracle_equilibrium_values_by_construction(oracle):
    profile = bagg_oracle.behavior_profile(oracle, PURE_NASH)
    assert profile.payoffs["1"] == R(1, 3) * R(5, 2) + R(2, 3) * R(15, 4)
    assert profile.payoffs["2"] == R(1, 4) * R(5, 3) + R(3, 4) * R(8, 3)
