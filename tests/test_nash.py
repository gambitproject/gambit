"""Test of calls to Nash equilibrium solvers.

These tests primarily ensure that calling the solvers works and returns
expected results on a very simple game.  This is not intended to be a
rigorous test suite for the algorithms across all games.
"""

import pygambit as gbt
import unittest

from . import games


class TestNash(unittest.TestCase):
    """Test calls to Nash algorithms using Myerson poker - a game to which all algorithms apply."""
    def setUp(self):
        self.poker = games.read_from_file("poker.efg")
        self.mixed_rat = self.poker.mixed_strategy_profile(
            rational=True,
            data=[[gbt.Rational(1, 3), gbt.Rational(2, 3), gbt.Rational(0), gbt.Rational(0)],
                  [gbt.Rational(2, 3), gbt.Rational(1, 3)]]
        )
        self.behav_rat = self.poker.mixed_behavior_profile(rational=True)
        for action, prob in zip(self.poker.actions,
                                [1, 0, gbt.Rational(1, 3), gbt.Rational(2, 3),
                                 gbt.Rational(2, 3), gbt.Rational(1, 3)]):
            self.behav_rat[action] = prob

    def tearDown(self):
        del self.poker

    def test_enumpure_strategy(self):
        """Test calls of enumeration of pure strategies."""
        assert len(gbt.nash.enumpure_solve(self.poker, use_strategic=True).equilibria) == 0

    def test_enumpure_agent(self):
        """Test calls of enumeration of pure agent strategies."""
        assert len(gbt.nash.enumpure_solve(self.poker, use_strategic=False).equilibria) == 0

    def test_enummixed_strategy_double(self):
        """Test calls of enumeration of mixed strategy equilibria, floating-point."""
        result = gbt.nash.enummixed_solve(self.poker, rational=False)
        assert len(result.equilibria) == 1
        # For floating-point results are not exact, so we skip testing exact values for now

    def test_enummixed_strategy_rational(self):
        """Test calls of enumeration of mixed strategy equilibria, rational precision."""
        result = gbt.nash.enummixed_solve(self.poker, rational=True)
        assert len(result.equilibria) == 1
        assert result.equilibria[0] == self.mixed_rat

    def test_lcp_strategy_double(self):
        """Test calls of LCP for mixed strategy equilibria, floating-point."""
        result = gbt.nash.lcp_solve(self.poker, use_strategic=True, rational=False)
        assert len(result.equilibria) == 1
        # For floating-point results are not exact, so we skip testing exact values for now

    def test_lcp_strategy_rational(self):
        """Test calls of LCP for mixed strategy equilibria, rational precision."""
        result = gbt.nash.lcp_solve(self.poker, use_strategic=True, rational=True)
        assert len(result.equilibria) == 1
        assert result.equilibria[0] == self.mixed_rat

    def test_lcp_behavior_double(self):
        """Test calls of LCP for mixed behavior equilibria, floating-point."""
        result = gbt.nash.lcp_solve(self.poker, use_strategic=False, rational=False)
        assert len(result.equilibria) == 1
        # For floating-point results are not exact, so we skip testing exact values for now

    def test_lcp_behavior_rational(self):
        """Test calls of LCP for mixed behavior equilibria, rational precision."""
        result = gbt.nash.lcp_solve(self.poker, use_strategic=False, rational=True)
        assert len(result.equilibria) == 1
        assert result.equilibria[0] == self.behav_rat

    def test_lp_strategy_double(self):
        """Test calls of LP for mixed strategy equilibria, floating-point."""
        result = gbt.nash.lp_solve(self.poker, use_strategic=True, rational=False)
        assert len(result.equilibria) == 1
        # For floating-point results are not exact, so we skip testing exact values for now

    def test_lp_strategy_rational(self):
        """Test calls of LP for mixed strategy equilibria, rational precision."""
        result = gbt.nash.lp_solve(self.poker, use_strategic=True, rational=True)
        assert len(result.equilibria) == 1
        assert result.equilibria[0] == self.mixed_rat

    def test_lp_behavior_double(self):
        """Test calls of LP for mixed behavior equilibria, floating-point."""
        result = gbt.nash.lp_solve(self.poker, use_strategic=False, rational=False)
        assert len(result.equilibria) == 1
        # For floating-point results are not exact, so we skip testing exact values for now

    def test_lp_behavior_rational(self):
        """Test calls of LP for mixed behavior equilibria, rational precision."""
        result = gbt.nash.lp_solve(self.poker, use_strategic=False, rational=True)
        assert len(result.equilibria) == 1
        assert result.equilibria[0] == self.behav_rat

    def test_liap_strategy(self):
        """Test calls of liap for mixed strategy equilibria."""
        result = gbt.nash.liap_solve(self.poker, use_strategic=False)
        # Currently default parameter liap fails to find an equilibrium
        assert len(result.equilibria) == 0

    def test_liap_behavior(self):
        """Test calls of liap for mixed behavior equilibria."""
        result = gbt.nash.liap_solve(self.poker, use_strategic=True)
        # Currently default parameter liap fails to find an equilibrium
        assert len(result.equilibria) == 0

    def test_simpdiv_strategy(self):
        """Test calls of simplicial subdivision for mixed strategy equilibria."""
        result = gbt.nash.simpdiv_solve(self.poker)
        assert len(result.equilibria) == 1

    def test_ipa_strategy(self):
        """Test calls of IPA for mixed strategy equilibria."""
        result = gbt.nash.ipa_solve(self.poker)
        assert len(result.equilibria) == 1

    def test_gnm_strategy(self):
        """Test calls of GNM for mixed strategy equilibria."""
        result = gbt.nash.gnm_solve(self.poker)
        assert len(result.equilibria) == 1

    def test_logit_strategy(self):
        """Test calls of logit for mixed strategy equilibria."""
        result = gbt.nash.logit_solve(self.poker, use_strategic=True)
        assert len(result.equilibria) == 1

    def test_logit_behavior(self):
        """Test calls of logit for mixed behavior equilibria."""
        result = gbt.nash.logit_solve(self.poker, use_strategic=False)
        assert len(result.equilibria) == 1


def test_logit_zerochance():
    """Test handling zero-probability information sets when computing QRE."""
    g = gbt.Game.new_tree(["Alice"])
    g.append_move(g.root, g.players.chance, ["A", "B", "C"])
    g.set_chance_probs(g.players.chance.infosets[0], [0, 0, 1])
    g.append_move(g.root.children[0], "Alice", ["A", "B"])
    g.append_infoset(g.root.children[1], g.root.children[0].infoset)
    win = g.add_outcome([1])
    g.set_outcome(g.root.children[0].children[0], win)
    result = gbt.nash.logit_solve(g, use_strategic=False)
    assert result.equilibria[0][g.players["Alice"].infosets[0].actions[0]] == 1
    assert result.equilibria[0][g.players["Alice"].infosets[0].actions[1]] == 0
