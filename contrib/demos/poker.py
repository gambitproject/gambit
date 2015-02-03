import gambit


g = gambit.Game.new_tree()
g.players.add("Alice")
g.players.add("Bob")
g.title = "A simple poker example"

iset = g.root.append_move(g.players.chance, 2)
iset.actions[0].label = "A"
iset.actions[1].label = "K"

iset = g.root.children[0].append_move(g.players["Alice"], 2)
iset.label = "a"
iset.actions[0].label = "R"
iset.actions[1].label = "F"

iset = g.root.children[1].append_move(g.players["Alice"], 2)
iset.label ="k"
iset.actions[0].label = "R"
iset.actions[1].label = "F"

iset = g.root.children[0].children[0].append_move(g.players["Bob"], 2)
iset.label = "b"
iset.actions[0].label = "M"
iset.actions[1].label = "P"

g.root.children[1].children[0].append_move(iset)

alice_big = g.outcomes.add("Alice wins big")
alice_big[0] = 2
alice_big[1] = -2

alice_small = g.outcomes.add("Alice wins")
alice_small[0] = 1
alice_small[1] = -1

bob_big = g.outcomes.add("Bob wins big")
bob_big[0] = -2
bob_big[1] = 2

bob_small = g.outcomes.add("Bob wins")
bob_small[0] = -1
bob_small[1] = 1

g.root.children[0].children[0].children[0].outcome = alice_big
g.root.children[0].children[0].children[1].outcome = alice_small
g.root.children[0].children[1].outcome = bob_small

g.root.children[1].children[0].children[0].outcome = bob_big
g.root.children[1].children[0].children[1].outcome = alice_small
g.root.children[1].children[1].outcome = bob_small

print g.write()

print gambit.gte.write_game(g)

file("poker.efg", "w").write(g.write()+"\n")
file("poker.xml", "w").write(gambit.gte.write_game(g)+"\n")
