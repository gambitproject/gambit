import pytest
import pygambit as gbt

@pytest.fixture
def small_efg():
    g = gbt.Game.new_tree(players=["Alice", "Bob"], title="Small Sequential")
    g.append_move(g.root, "Alice", ["L", "R"])
    g.append_move(g.root.children[0], "Bob", ["u", "d"])
    g.append_move(g.root.children[1], "Bob", ["u", "d"])
    
    g.set_outcome(g.root.children["L"].children["u"], g.add_outcome([2, 1]))
    g.set_outcome(g.root.children["L"].children["d"], g.add_outcome([0, 0]))
    g.set_outcome(g.root.children["R"].children["u"], g.add_outcome([-1, 3]))
    g.set_outcome(g.root.children["R"].children["d"], g.add_outcome([1, 1]))
    return g

def test_as_strategy_payoff_consistency(small_efg):
    g = small_efg
    profile = g.mixed_behavior_profile(rational=True)
    profile["L"] = gbt.Rational(1, 4)
    profile["R"] = gbt.Rational(3, 4)
    profile[g.players["Bob"].infosets[0]] = [gbt.Rational(1, 2), gbt.Rational(1, 2)]
    profile[g.players["Bob"].infosets[1]] = [gbt.Rational(1, 4), gbt.Rational(3, 4)]
    strat_profile = profile.as_strategy()
    assert profile.payoff("Alice") == strat_profile.payoff("Alice")
    assert profile.payoff("Bob") == strat_profile.payoff("Bob")

def test_as_strategy_kuhn_manual_reference(small_efg):
    g = small_efg
    profile = g.mixed_behavior_profile(rational=True)
    profile["L"] = gbt.Rational(1, 4)
    profile["R"] = gbt.Rational(3, 4)
    profile[g.players["Bob"].infosets[0]] = [gbt.Rational(1, 2), gbt.Rational(1, 2)]
    profile[g.players["Bob"].infosets[1]] = [gbt.Rational(1, 4), gbt.Rational(3, 4)]
    strat_profile = profile.as_strategy()
   
    assert strat_profile[g.players["Alice"].strategies[0]] == gbt.Rational(1, 4) # Pr(L)
    assert strat_profile[g.players["Alice"].strategies[1]] == gbt.Rational(3, 4) # Pr(R)
    assert strat_profile[g.players["Bob"].strategies[0]] == gbt.Rational(1, 8)
    assert strat_profile[g.players["Bob"].strategies[1]] == gbt.Rational(3, 8)
    assert strat_profile[g.players["Bob"].strategies[2]] == gbt.Rational(1, 8)
    assert strat_profile[g.players["Bob"].strategies[3]] == gbt.Rational(3, 8)
