import pytest
import pygambit as gbt
import warnings

def test_use_strategic_warning_lp_solve():
    g = gbt.Game.new_table([2, 2])
    
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        gbt.nash.enumpure_solve(g)
    
    p1 = g.players[0]
    p2 = g.players[1]
    g[0, 0][p1] = 1; g[0, 0][p2] = -1
    g[0, 1][p1] = -1; g[0, 1][p2] = 1
    g[1, 0][p1] = -1; g[1, 0][p2] = 1
    g[1, 1][p1] = 1; g[1, 1][p2] = -1
    
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        gbt.nash.lp_solve(g)

    with pytest.warns(UserWarning, match="Game has no tree representation; using strategic form."):
        gbt.nash.lp_solve(g, use_strategic=False)

def test_use_strategic_warning_lcp_solve():
    g = gbt.Game.new_table([2, 2])
    
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        gbt.nash.lcp_solve(g)
        gbt.nash.lcp_solve(g, use_strategic=True)

    with pytest.warns(UserWarning, match="Game has no tree representation; using strategic form."):
        gbt.nash.lcp_solve(g, use_strategic=False)

def test_use_strategic_warning_logit_solve():
    g = gbt.Game.new_table([2, 2])
    
    with pytest.warns(UserWarning, match="Game has no tree representation; using strategic form."):
        gbt.nash.logit_solve(g, use_strategic=False)
        
def test_use_strategic_warning_qre():
    g = gbt.Game.new_table([2, 2])
    
    with pytest.warns(UserWarning, match="Game has no tree representation; using strategic form."):
        gbt.qre.logit_solve_branch(g, use_strategic=False)

def test_use_strategic_warning_tree():
    g_tree = gbt.Game.new_tree(players=["Alice", "Bob"])
    g_tree.append_move(g_tree.root, g_tree.players[0], ["L", "R"])
    
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        gbt.nash.enumpoly_solve(g_tree, use_strategic=False)
