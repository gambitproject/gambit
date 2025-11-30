import pygambit as gbt

solver = gbt.nash.lp_solve

g = gbt.Game.new_tree(players=["1", "2"], title="2 player perfect info win lose")
g.append_move(g.root, "2", ["a", "b"])
g.append_move(g.root.children[0], "1", ["L", "R"])
g.append_move(g.root.children[1], "1", ["L", "R"])
g.append_move(g.root.children[0].children[0], "2", ["l", "r"])
g.set_outcome(
    g.root.children[0].children[0].children[0], g.add_outcome([1, -1], label="aLl")
)
g.set_outcome(
    g.root.children[0].children[0].children[1], g.add_outcome([-1, 1], label="aLr")
)
g.set_outcome(g.root.children[0].children[1], g.add_outcome([1, -1], label="aR"))
g.set_outcome(g.root.children[1].children[0], g.add_outcome([1, -1], label="bL"))
g.set_outcome(g.root.children[1].children[1], g.add_outcome([-1, 1], label="bR"))

print("blah")

equilibria = solver(g).equilibria
eqm = equilibria[0]
print(eqm["1"])
print(eqm["2"])


g = gbt.Game.new_tree(players=["1", "2"], title="2 player perfect info win lose")
g.append_move(g.root, "2", ["a", "b"])
g.append_move(g.root.children[0], "1", ["L", "R"])
g.append_move(g.root.children[1], "1", ["L", "R"])
g.append_move(g.root.children[0].children[0], "2", ["l", "r"])
g.set_outcome(g.root.children[0], g.add_outcome([-10, 10], label="a"))
g.set_outcome(
    g.root.children[0].children[0].children[0], g.add_outcome([11, -11], label="aLl")
)
g.set_outcome(
    g.root.children[0].children[0].children[1], g.add_outcome([9, -9], label="aLr")
)
g.set_outcome(g.root.children[0].children[1], g.add_outcome([11, -11], label="aR"))
g.set_outcome(g.root.children[1].children[0], g.add_outcome([1, -1], label="bL"))
g.set_outcome(g.root.children[1].children[1], g.add_outcome([-1, 1], label="bR"))

equilibria = solver(g).equilibria
eqm = equilibria[0]
print(eqm["1"])
print(eqm["2"])
