import pytest
import pygambit as gbt

@pytest.fixture
def small_efg():
    """Fixture providing a minimal 2-player perfect-info sequential game."""
    g = gbt.Game.new_tree(players=["Alice", "Bob"], title="Small Sequential")
    g.append_move(g.root, "Alice", ["L", "R"])
    # Note: append_move to individual node gives Bob two distinct information sets
    g.append_move(g.root.children[0], "Bob", ["u", "d"])
    g.append_move(g.root.children[1], "Bob", ["u", "d"])
    
    g.set_outcome(g.root.children["L"].children["u"], g.add_outcome([2, 1]))
    g.set_outcome(g.root.children["L"].children["d"], g.add_outcome([0, 0]))
    g.set_outcome(g.root.children["R"].children["u"], g.add_outcome([-1, 3]))
    g.set_outcome(g.root.children["R"].children["d"], g.add_outcome([1, 1]))
    return g

def test_as_strategy_payoff_consistency(small_efg):
    """
    Test that the expected payoffs match between computing with the original 
    behavior profile and the converted mixed strategy profile.
    """
    g = small_efg
    profile = g.mixed_behavior_profile(rational=True)
    
    # Assign some arbitrary probabilities
    profile["L"] = gbt.Rational(1, 4)
    profile["R"] = gbt.Rational(3, 4)
    profile[g.players["Bob"].infosets[0]] = [gbt.Rational(1, 2), gbt.Rational(1, 2)]
    profile[g.players["Bob"].infosets[1]] = [gbt.Rational(1, 4), gbt.Rational(3, 4)]
    
    strat_profile = profile.as_strategy()
    
    # Check consistency of payoffs across both profile structures
    assert profile.payoff("Alice") == strat_profile.payoff("Alice")
    assert profile.payoff("Bob") == strat_profile.payoff("Bob")

def test_as_strategy_kuhn_manual_reference(small_efg):
    """
    Test that the mixed strategy representation calculated by as_strategy() 
    matches manual calculation using Kuhn's Theorem (multiplying 
    action probabilities structurally along each pure strategy's execution path).
    """
    g = small_efg
    profile = g.mixed_behavior_profile(rational=True)
    
    # Let Pr(L) = 1/4, Pr(R) = 3/4
    profile["L"] = gbt.Rational(1, 4)
    profile["R"] = gbt.Rational(3, 4)
    
    # Let Bob play u with prob=1/2 after L; Bob play u with prob=1/4 after R
    profile[g.players["Bob"].infosets[0]] = [gbt.Rational(1, 2), gbt.Rational(1, 2)]
    profile[g.players["Bob"].infosets[1]] = [gbt.Rational(1, 4), gbt.Rational(3, 4)]
    
    strat_profile = profile.as_strategy()
    
    # Alice has 1 information set with 2 actions -> 2 pure strategies in the NFG: L, R
    assert strat_profile[g.players["Alice"].strategies[0]] == gbt.Rational(1, 4) # Pr(L)
    assert strat_profile[g.players["Alice"].strategies[1]] == gbt.Rational(3, 4) # Pr(R)
    
    # Bob has 2 information sets, each taking 2 actions -> 4 pure strategies
    # For Bob, strategy order conceptually maps to combinations of the two infoset distributions.
    # We verify the correct probabilities using the mathematical product:
    # 0 = u at 0, u at 1 -> (1/2) * (1/4) = 1/8
    # 1 = u at 0, d at 1 -> (1/2) * (3/4) = 3/8
    # 2 = d at 0, u at 1 -> (1/2) * (1/4) = 1/8
    # 3 = d at 0, d at 1 -> (1/2) * (3/4) = 3/8
    assert strat_profile[g.players["Bob"].strategies[0]] == gbt.Rational(1, 8)
    assert strat_profile[g.players["Bob"].strategies[1]] == gbt.Rational(3, 8)
    assert strat_profile[g.players["Bob"].strategies[2]] == gbt.Rational(1, 8)
    assert strat_profile[g.players["Bob"].strategies[3]] == gbt.Rational(3, 8)
