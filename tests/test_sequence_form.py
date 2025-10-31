import pygambit as gbt
import numpy as np

def test_sequence_form(game, correspondence, payoffs):
    test_object = TestSequenceForm(game, correspondence, payoffs)
    test_object.check_subtensor(0)

class TestSequenceForm:

    def __init__(self, game, correspondence, payoffs):
        self.correspondence = correspondence
        self.payoffs = payoffs
        self.sequence_form = gbt.GameSequenceForm(game)
        self.players = game.players
        self.seq_dict = {}
        self.seq_idx_list = [0]*len(self.players)

    def test(self):
        self.check_subtensor(0)

    def get_seq_idx(self, player_idx, action):
        for i in range(len(self.correspondence[player_idx])):
            if action.label == self.correspondence[player_idx][i]:
                return i + 1
        return None

    def check_component(self):
        for player_idx in range(len(self.players)):
            player = self.players[player_idx]
            payoff_tensor = self.payoffs[player_idx]
            if float(self.sequence_form.get_payoff(self.seq_dict, player)) != payoff_tensor[tuple(self.seq_idx_list)]:
                print(player_idx)
                print(self.seq_idx_list)
                print(self.sequence_form.get_payoff(self.seq_dict, player))
                print(payoff_tensor[tuple(self.seq_idx_list)])
            assert float(self.sequence_form.get_payoff(self.seq_dict, player)) == payoff_tensor[tuple(self.seq_idx_list)]

    def check_subtensor(self, player_idx):
        if player_idx == len(self.players):
            self.check_component()
        else:
            self.check_empty_sequence_subtensor(player_idx)
            self.check_non_empty_sequence_subtensors(player_idx)

    def check_empty_sequence_subtensor(self, player_idx):
        player = self.players[player_idx]
        self.seq_dict[player] = None
        self.seq_idx_list[player_idx] = 0
        self.check_subtensor(player_idx + 1)

    def check_non_empty_sequence_subtensors(self,player_idx):
        player = self.players[player_idx]
        for action in player.actions:
            self.seq_dict[player] = action
            seq_idx = self.get_seq_idx(player_idx, action)
            self.seq_idx_list[player_idx] = seq_idx
            self.check_subtensor(player_idx + 1)

def test_constraints(game, infosets, actions, constraints):
    test_object = TestConstraints(game, infosets, actions, constraints)
    test_object.test()


class TestConstraints:

    def __init__(self, game, infosets, actions, constraints):
        self.sequence_form = gbt.GameSequenceForm(game)
        self.game = game
        self.infosets = infosets
        self.actions = actions
        self.constraints = constraints

    def get_seq_idx(self, action):
        for i in range(len(self.actions)):
            if action.label == self.actions[i]:
                return i + 1
        return None

    def get_infoset_idx(self, infoset):
        for i in range(len(self.infosets)):
            if infoset.label == self.infosets[i]:
                return i
        return None

    def test(self):
        for infoset_player in self.game.players:
            for infoset in infoset_player.infosets:
                self.check_null_sequence_entry(infoset)
                for player in self.game.players:
                    for action in player.actions:
                        self.check_entry(infoset, action)

    def check_null_sequence_entry(self, infoset):
        infoset_idx = self.get_infoset_idx(infoset)
        actual = self.sequence_form.get_constraint_entry(infoset, None)
        expected = self.constraints[infoset_idx][0]
        if actual != expected:
            print(infoset_idx, 0)
            print(actual, expected)
        assert actual == expected

    def check_entry(self, infoset, action):
        infoset_idx = self.get_infoset_idx(infoset)
        action_idx = self.get_seq_idx(action)
        actual = self.sequence_form.get_constraint_entry(infoset, action)
        expected = self.constraints[infoset_idx][action_idx]
        if actual != expected:
            print(infoset_idx, action_idx)
            print(actual, expected)
        assert actual == expected


# Simple constraint test

tree = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Test tree"
)

tree.append_move(
    tree.root,
    player="player1",
    actions=["A", "B"]
)

tree.append_move(
    tree.root.children[0],
    player="player2",
    actions=["a", "b"]
)

tree.append_move(
    tree.root.children[0].children[0],
    player="player1",
    actions=["C", "D"]
)

tree.root.infoset.label = "root"
tree.root.children[0].infoset.label = "A"
tree.root.children[0].children[0].infoset.label = "a"


infosets = ["root", "A", "a"]
actions = ["A", "B", "C", "D", "a", "b"]

constraints = [[1, -1, -1, 0, 0, 0, 0], [1, 0, 0, 0, 0, -1, -1], [0, 1, 0, -1, -1, 0, 0]]

test_constraints(tree, infosets, actions, constraints)



# Test tree

tree = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Test tree"
)

tree.append_move(
    tree.root,
    player="player1",
    actions=["A", "B", "C", "D"]
)

tree.append_move(
    tree.root.children[0],
    player="player2",
    actions=["a", "b"]
)

tree.append_move(
    tree.root.children[1],
    player="player2",
    actions=["c", "d"]
)

tree.append_move(
    tree.root.children[2],
    player="player2",
    actions=["e", "f"]
)

tree.append_move(
    tree.root.children[0].children[1],
    player="player2",
    actions=["g", "h"]
)

tree.append_move(
    tree.root.children[2].children[0],
    player="player1",
    actions=["E", "F"]
)

tree.append_infoset(tree.root.children[2].children[1],
                    tree.root.children[2].children[0].infoset)

tree.set_outcome(
    tree.root.children[0].children[0],
    outcome=tree.add_outcome(
        payoffs=[2, 8],
        label="Aa"
    )
)

tree.set_outcome(
    tree.root.children[0].children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[0, 1],
        label="Abg"
    )
)

tree.set_outcome(
    tree.root.children[0].children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[5, 2],
        label="Abh"
    )
)

tree.set_outcome(
    tree.root.children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[7, 6],
        label="Bc"
    )
)

tree.set_outcome(
    tree.root.children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[4, 2],
        label="Bd"
    )
)

tree.set_outcome(
    tree.root.children[2].children[0].children[0],
    outcome=tree.add_outcome(
        payoffs=[3, 7],
        label="CeE"
    )
)

tree.set_outcome(
    tree.root.children[2].children[0].children[1],
    outcome=tree.add_outcome(
        payoffs=[8, 3],
        label="CeF"
    )
)

tree.set_outcome(
    tree.root.children[2].children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[7, 8],
        label="CfE"
    )
)

tree.set_outcome(
    tree.root.children[2].children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[2, 2],
        label="CfF"
    )
)

tree.set_outcome(
    tree.root.children[3],
    outcome=tree.add_outcome(
        payoffs=[6, 4],
        label="D"
    )
)

tree.root.infoset.label = "root"
tree.root.children[0].infoset.label = "0"
tree.root.children[1].infoset.label = "1"
tree.root.children[2].infoset.label = "2"
tree.root.children[0].children[1].infoset.label = "01"
tree.root.children[2].children[0].infoset.label = "20"

actions = ["A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f", "g", "h"]
infosets = ["root", "0", "1", "2", "01", "20"]

c_root = [1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
c_0 = [1, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 0, 0]
c_1 = [1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0]
c_2 = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0]
c_01 = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, -1]
c_20 = [0, 0, 0, 1, 0, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0]

constraints = [c_root, c_0, c_1, c_2, c_01, c_20]

test_constraints(tree, infosets, actions, constraints)




player1_correspondence = ["A", "B", "C", "D", "E", "F"]
player2_correspondence = ["a", "b", "c", "d", "e", "f", "g", "h"]
correspondence = [player1_correspondence, player2_correspondence]

n1 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
n2 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
A1 = [0, 2, 0, 0, 0, 0, 0, 0, 5]
A2 = [0, 8, 0, 0, 0, 0, 0, 1, 2]
B1 = [0, 0, 0, 7, 4, 0, 0, 0, 0]
B2 = [0, 0, 0, 6, 2, 0, 0, 0, 0]
C1 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
C2 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
D1 = [6, 0, 0, 0, 0, 0, 0, 0, 0]
D2 = [4, 0, 0, 0, 0, 0, 0, 0, 0]
E1 = [0, 0, 0, 0, 0, 3, 7, 0, 0]
E2 = [0, 0, 0, 0, 0, 7, 8, 0, 0]
F1 = [0, 0, 0, 0, 0, 8, 2, 0, 0]
F2 = [0, 0, 0, 0, 0, 3, 2, 0, 0]

player1_payoffs = [n1, A1, B1, C1, D1, E1, F1]
player2_payoffs = [n2, A2, B2, C2, D2, E2, F2]
payoffs = np.array([player1_payoffs, player2_payoffs])

test_sequence_form(tree, correspondence, payoffs)


# Simple Three player game

three_player = gbt.Game.new_tree(
    players=["player1", "player2", "player3"],
    title="Simple three player game"
)

three_player.append_move(
    three_player.root,
    player="player1",
    actions=["A", "B"]
)

three_player.append_move(
    three_player.root.children[0],
    player="player2",
    actions=["a", "b"]
)

three_player.append_move(
    three_player.root.children[1],
    player="player3",
    actions=["alpha", "beta"]
)

three_player.append_move(
    three_player.root.children[0].children[0],
    player="player3",
    actions=["gamma", "delta"]
)

three_player.append_infoset(three_player.root.children[0].children[1],
                    three_player.root.children[0].children[0].infoset)

three_player.set_outcome(
    three_player.root.children[0].children[0],
    outcome=three_player.add_outcome(
        payoffs=[1, 2, 3],
        label="Aa"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1],
    outcome=three_player.add_outcome(
        payoffs=[4, 5, 6],
        label="Ab"
    )
)

three_player.set_outcome(
    three_player.root.children[1].children[0],
    outcome=three_player.add_outcome(
        payoffs=[7, 8, 9],
        label="Balpha"
    )
)

three_player.set_outcome(
    three_player.root.children[1].children[1],
    outcome=three_player.add_outcome(
        payoffs=[10, 11, 12],
        label="Bbeta"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[0].children[0],
    outcome=three_player.add_outcome(
        payoffs=[13, 14, 15],
        label="Aagamma"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[0].children[1],
    outcome=three_player.add_outcome(
        payoffs=[16, 17, 18],
        label="Aadelta"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1].children[0],
    outcome=three_player.add_outcome(
        payoffs=[19, 20, 21],
        label="Abgamma"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1].children[1],
    outcome=three_player.add_outcome(
        payoffs=[22, 23, 24],
        label="Abdelta"
    )
)

correspondence = [["A", "B"], ["a", "b"], ["alpha", "beta", "gamma", "delta"]]

An1 = [0, 0, 0, 0, 0]
An2 = [0, 0, 0, 0, 0]
An3 = [0, 0, 0, 0, 0]
Aa1 = [1, 0, 0, 13, 16]
Aa2 = [2, 0, 0, 14, 17]
Aa3 = [3, 0, 0, 15, 18]
Ab1 = [4, 0, 0, 19, 22]
Ab2 = [5, 0, 0, 20, 23]
Ab3 = [6, 0, 0, 21, 24]
Bn1 = [0, 7, 10, 0, 0]
Bn2 = [0, 8, 11, 0, 0]
Bn3 = [0, 9, 12, 0, 0]
Ba1 = [0, 0, 0, 0, 0]
Ba2 = [0, 0, 0, 0, 0]
Ba3 = [0, 0, 0, 0, 0]
Bb1 = [0, 0, 0, 0, 0]
Bb2 = [0, 0, 0, 0, 0]
Bb3 = [0, 0, 0, 0, 0]

null = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
A1 = [An1, Aa1, Ab1]
A2 = [An2, Aa2, Ab2]
A3 = [An3, Aa3, Ab3]
B1 = [Bn1, Ba1, Bb1]
B2 = [Bn2, Ba2, Bb2]
B3 = [Bn3, Ba3, Bb3]

player1_payoffs = [null, A1, B1]
player2_payoffs = [null, A2, B2]
player3_payoffs = [null, A3, B3]

payoffs = np.array([player1_payoffs, player2_payoffs, player3_payoffs])

test_sequence_form(three_player, correspondence, payoffs)


# Five player no action game

just_root = gbt.Game.new_tree(
    players=["player1", "player2", "player3", "player4", "player5"],
    title="Just root"
)

just_root.set_outcome(
    just_root.root,
    outcome=just_root.add_outcome(
        payoffs=[1, 2, 3, 4, 5],
        label="root"
    )
)

correspondence = [[],[],[],[],[]]
payoffs1 = [[[[[1]]]]]
payoffs2 = [[[[[2]]]]]
payoffs3 = [[[[[3]]]]]
payoffs4 = [[[[[4]]]]]
payoffs5 = [[[[[5]]]]]
payoffs = np.array([payoffs1, payoffs2, payoffs3, payoffs4, payoffs5])
test_sequence_form(just_root, correspondence, payoffs)


# Player 2 at root

p2_at_root = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Just root"
)

p2_at_root.append_move(
    p2_at_root.root,
    player="player2",
    actions=["a", "b"]
)

p2_at_root.append_move(
    p2_at_root.root.children[0],
    player="player1",
    actions=["A", "B"]
)

p2_at_root.set_outcome(
    p2_at_root.root.children[0].children[0],
    outcome=p2_at_root.add_outcome(
        payoffs=[1, 2],
        label="aA"
    )
)

p2_at_root.set_outcome(
    p2_at_root.root.children[0].children[1],
    outcome=p2_at_root.add_outcome(
        payoffs=[3, 4],
        label="aB"
    )
)

p2_at_root.set_outcome(
    p2_at_root.root.children[1],
    outcome=p2_at_root.add_outcome(
        payoffs=[5, 6],
        label="b"
    )
)

correspondence = [["A","B"],["a","b"]]

n1 = [0, 0, 5]
n2 = [0, 0, 6]
A1 = [0, 1, 0]
A2 = [0, 2, 0]
B1 = [0, 3, 0]
B2 = [0, 4, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])
test_sequence_form(p2_at_root, correspondence, payoffs)

# Chance at root

chance_at_root = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Chance at root"
)

chance_at_root.append_move(
    chance_at_root.root,
    player=chance_at_root.players.chance,
    actions=["l", "r"]
)

chance_at_root.set_chance_probs(chance_at_root.root.infoset, [0.2,0.8])

chance_at_root.append_move(
    chance_at_root.root.children[0],
    player="player1",
    actions=["A", "B"]
)

chance_at_root.append_infoset(chance_at_root.root.children[1], chance_at_root.root.children[0].infoset)

chance_at_root.append_move(
    chance_at_root.root.children[0].children[0],
    player="player2",
    actions=["a", "b"]
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[0].children[0],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 1],
        label="lAa"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[0].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[0, 1],
        label="lAb"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 1],
        label="lB"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[1].children[0],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 0],
        label="rA"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[1].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[0, 1],
        label="rB"
    )
)

correspondence = [["A", "B"], ["a", "b"]]

n1 = [0, 0, 0]
n2 = [0, 0, 0]
A1 = [0.8, 0.2, 0]
A2 = [0, 0.2, 0.2]
B1 = [0.2, 0, 0]
B2 = [1, 0, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])
test_sequence_form(chance_at_root, correspondence, payoffs)


# Chance in middle

chance_in_middle = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Chance in middle"
)

chance_in_middle.append_move(
    chance_in_middle.root,
    player="player1",
    actions=["A", "B"]
)

chance_in_middle.append_move(
    chance_in_middle.root.children[0],
    player=chance_in_middle.players.chance,
    actions=["l", "r"]
)

chance_in_middle.append_move(
    chance_in_middle.root.children[0].children[0],
    player="player2",
    actions=["a", "b"]
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[0].children[0],
    outcome=chance_in_middle.add_outcome(
        payoffs=[1, 2],
        label="Ala"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[0].children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[3, 4],
        label="Alb"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[5, 6],
        label="Ar"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[7, 8],
        label="B"
    )
)

correspondence = [["A", "B"], ["a", "b"]]

n1 = [0, 0, 0]
n2 = [0, 0, 0]
A1 = [2.5, 0.5, 1.5]
A2 = [3, 1, 2]
B1 = [7, 0, 0]
B2 = [8, 0, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])

test_sequence_form(chance_in_middle, correspondence, payoffs)


# Another chance tree

chance_tree = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Chance tree"
)

chance_tree.append_move(
    chance_tree.root,
    player=chance_tree.players.chance,
    actions=["first", "second", "third", "forth"]
)

chance_tree.set_chance_probs(chance_tree.root.infoset, [0.2, 0.2, 0.2, 0.4])

chance_tree.append_move(
    chance_tree.root.children[0],
    player="player1",
    actions=["l", "r"]
)

chance_tree.append_move(
    chance_tree.root.children[1],
    player="player1",
    actions=["c", "d"]
)

chance_tree.append_infoset(chance_tree.root.children[2],
                            chance_tree.root.children[1].infoset)

chance_tree.append_move(
    chance_tree.root.children[0].children[1],
    player="player2",
    actions=["p", "q"]
)

chance_tree.append_move(
    chance_tree.root.children[0].children[1].children[0],
    player="player1",
    actions=["L", "R"]
)

chance_tree.append_infoset(chance_tree.root.children[0].children[1].children[1],
                           chance_tree.root.children[0].children[1].children[0].infoset)

chance_tree.append_move(
    chance_tree.root.children[2].children[0],
    player="player2",
    actions=["s", "t"]
)

chance_tree.append_infoset(chance_tree.root.children[2].children[1],
                           chance_tree.root.children[2].children[0].infoset)

chance_tree.set_outcome(
    chance_tree.root.children[0].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[5, -5],
        label="l"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[0].children[1].children[0].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[10, -10],
        label="rpL"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[0].children[1].children[0].children[1],
    outcome=chance_tree.add_outcome(
        payoffs=[15, -15],
        label="rpR"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[0].children[1].children[1].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[20, -20],
        label="rqL"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[0].children[1].children[1].children[1],
    outcome=chance_tree.add_outcome(
        payoffs=[-5, 5],
        label="rqR"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[1].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[10, -10],
        label="c"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[1].children[1],
    outcome=chance_tree.add_outcome(
        payoffs=[20, -20],
        label="d"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[2].children[0].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[20, -20],
        label="cs"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[2].children[0].children[1],
    outcome=chance_tree.add_outcome(
        payoffs=[50, -50],
        label="ct"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[2].children[1].children[0],
    outcome=chance_tree.add_outcome(
        payoffs=[30, -30],
        label="ds"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[2].children[1].children[1],
    outcome=chance_tree.add_outcome(
        payoffs=[15, -15],
        label="dt"
    )
)

chance_tree.set_outcome(
    chance_tree.root.children[3],
    outcome=chance_tree.add_outcome(
        payoffs=[5, -5],
        label="nothing"
    )
)

chance_tree.root.children[0].infoset.label = "0"
chance_tree.root.children[1].infoset.label = "1"
chance_tree.root.children[0].children[1].infoset.label = "01"
chance_tree.root.children[2].children[0].infoset.label = "20"
chance_tree.root.children[0].children[1].children[0].infoset.label = "010"

actions = ["l", "r", "c", "d", "L", "R", "p", "q", "s", "t"]
infosets = ["0", "1", "01", "20", "010"]

c_0 = [1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0]
c_1 = [1, 0, 0, -1, -1, 0, 0, 0, 0, 0, 0]
c_01 = [1, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0]
c_20 = [1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1]
c_010 = [0, 0, 1, 0, 0, -1, -1, 0, 0, 0, 0]
constraints = [c_0, c_1, c_01, c_20, c_010]

test_constraints(chance_tree, infosets, actions, constraints)


player1_actions = ["l", "r", "L", "R", "c", "d"]
player2_actions = ["p", "q", "s", "t"]
correspondence = [player1_actions, player2_actions]

null = [2, 0, 0, 0, 0]
l = [1, 0, 0, 0, 0]
r = [0, 0, 0, 0, 0]
L = [0, 2, 4, 0, 0]
R = [0, 3, -1, 0, 0]
c = [2, 0, 0, 4, 10]
d = [4, 0, 0, 6, 3]

player1_payoffs = np.array([null, l, r, L, R, c, d])
payoffs = [player1_payoffs, -player1_payoffs]

test_sequence_form(chance_tree, correspondence, payoffs)

# Big three player tree

btp = gbt.Game.new_tree(
    players=["player1", "player2", "player3"],
    title="Chance tree"
)

btp.append_move(
    btp.root,
    player="player2",
    actions=["a", "b"]
)

btp.append_move(
    btp.root.children[0],
    player="player1",
    actions=["A"]
)

btp.append_infoset(btp.root.children[1], btp.root.children[0].infoset)

btp.append_move(
    btp.root.children[0].children[0],
    player="player3",
    actions=["al", "be", "ga"]
)

btp.append_move(
    btp.root.children[0].children[0].children[0],
    player="player2",
    actions=["c", "d"]
)

btp.append_infoset(btp.root.children[0].children[0].children[1], btp.root.children[0].children[0].children[0].infoset)
btp.append_infoset(btp.root.children[0].children[0].children[2], btp.root.children[0].children[0].children[0].infoset)

btp.append_move(
    btp.root.children[1].children[0],
    player=btp.players.chance,
    actions=["left", "right"]
)

btp.set_chance_probs(btp.root.children[1].children[0].infoset, [0.2, 0.8])

btp.append_move(
    btp.root.children[1].children[0].children[0],
    player="player1",
    actions=["B", "C"]
)

btp.append_move(
    btp.root.children[1].children[0].children[1],
    player="player3",
    actions=["de", "ep"]
)

btp.set_outcome(
    btp.root.children[0].children[0].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[2, 1, 3],
        label="aAalc"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[7, 2, 4],
        label="aAald"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[8, 6, 0],
        label="aAbec"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[1, 3, 5],
        label="aAbed"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[2].children[0],
    outcome=btp.add_outcome(
        payoffs=[1, 7, 2],
        label="aAgac"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[2].children[1],
    outcome=btp.add_outcome(
        payoffs=[3, 5, 4],
        label="aAgad"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[5, 10, 15],
        label="bAlB"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[20, 15, 5],
        label="bAlC"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[10, 15, 5],
        label="bArde"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[5, 15, 20],
        label="bArep"
    )
)

btp.root.infoset.label = "root"
btp.root.children[0].infoset.label = "0"
btp.root.children[0].children[0].infoset.label = "00"
btp.root.children[0].children[0].children[0].infoset.label = "000"
btp.root.children[1].children[0].children[0].infoset.label = "100"
btp.root.children[1].children[0].children[1].infoset.label = "101"

actions = ["A", "B", "C", "a", "b", "c", "d", "al", "be", "ga", "de", "ep"]
infosets = ["root", "0", "00", "000", "100", "101"]

c_root = [1, 0, 0, 0, -1, -1, 0, 0, 0, 0, 0, 0, 0]
c_0 = [1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
c_00 = [1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0]
c_000 = [0, 0, 0, 0, 1, 0, -1, -1, 0, 0, 0, 0, 0]
c_100 = [0, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
c_101 = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1]

constraints = [c_root, c_0, c_00, c_000, c_100, c_101]
test_constraints(btp, infosets, actions, constraints)

player1_actions = ["A", "B", "C"]
player2_actions = ["a", "b", "c", "d"]
player3_actions = ["al", "be", "ga", "de", "ep"]
correspondence = [player1_actions, player2_actions, player3_actions]

zrs = [0, 0, 0, 0, 0, 0]
null = [zrs, zrs, zrs, zrs, zrs]
Aa1 = zrs
Aa2 = zrs
Aa3 = zrs
Ab1 = [0, 0, 0, 0, 8, 4]
Ab2 = [0, 0, 0, 0, 12, 12]
Ab3 = [0, 0, 0, 0, 4, 16]
Ac1 = [0, 2, 8, 1, 0, 0]
Ac2 = [0, 1, 6, 7, 0, 0]
Ac3 = [0, 3, 0, 2, 0, 0]
Ad1 = [0, 7, 1, 3, 0, 0]
Ad2 = [0, 2, 3, 5, 0, 0]
Ad3 = [0, 4, 5, 4, 0, 0]
Ba1 = zrs
Ba2 = zrs
Ba3 = zrs
Bb1 = [1, 0, 0, 0, 0, 0]
Bb2 = [2, 0, 0, 0, 0, 0]
Bb3 = [3, 0, 0, 0, 0, 0]
Bc1 = zrs
Bc2 = zrs
Bc3 = zrs
Bd1 = zrs
Bd2 = zrs
Bd3 = zrs
Ca1 = zrs
Ca2 = zrs
Ca3 = zrs
Cb1 = [4, 0, 0, 0, 0, 0]
Cb2 = [3, 0, 0, 0, 0, 0]
Cb3 = [1, 0, 0, 0, 0, 0]
Cc1 = zrs
Cc2 = zrs
Cc3 = zrs
Cd1 = zrs
Cd2 = zrs
Cd3 = zrs

A1 = [zrs, Aa1, Ab1, Ac1, Ad1]
A2 = [zrs, Aa2, Ab2, Ac2, Ad2]
A3 = [zrs, Aa3, Ab3, Ac3, Ad3]
B1 = [zrs, Ba1, Bb1, Bc1, Bd1]
B2 = [zrs, Ba2, Bb2, Bc2, Bd2]
B3 = [zrs, Ba3, Bb3, Bc3, Bd3]
C1 = [zrs, Ca1, Cb1, Cc1, Cd1]
C2 = [zrs, Ca2, Cb2, Cc2, Cd2]
C3 = [zrs, Ca3, Cb3, Cc3, Cd3]

payoffs = np.array([[null, A1, B1, C1], [null, A2, B2, C2], [null, A3, B3, C3]])

test_sequence_form(btp, correspondence, payoffs)


# Maximal Information Sets

btp = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="MIS"
)

btp.append_move(
    btp.root,
    player="player1",
    actions=["A", "B"]
)

btp.append_move(
    btp.root.children[0],
    player="player2",
    actions=["a", "b"]
)

btp.append_infoset(btp.root.children[1], btp.root.children[0].infoset)

btp.append_move(
    btp.root.children[0].children[0],
    player="player1",
    actions=["C", "D"]
)

btp.append_infoset(btp.root.children[0].children[1], btp.root.children[0].children[0].infoset)

btp.append_move(
    btp.root.children[1].children[0],
    player="player1",
    actions=["E", "F"]
)

btp.append_infoset(btp.root.children[1].children[1], btp.root.children[1].children[0].infoset)

btp.append_move(
    btp.root.children[0].children[0].children[0],
    player="player2",
    actions=["c", "d"]
)

btp.append_infoset(btp.root.children[0].children[0].children[1], btp.root.children[0].children[0].children[0].infoset)
btp.append_infoset(btp.root.children[1].children[0].children[0], btp.root.children[0].children[0].children[0].infoset)
btp.append_infoset(btp.root.children[1].children[0].children[1], btp.root.children[0].children[0].children[0].infoset)

btp.append_move(
    btp.root.children[0].children[1].children[0],
    player="player2",
    actions=["e", "f"]
)

btp.append_infoset(btp.root.children[0].children[1].children[1], btp.root.children[0].children[1].children[0].infoset)
btp.append_infoset(btp.root.children[1].children[1].children[0], btp.root.children[0].children[1].children[0].infoset)
btp.append_infoset(btp.root.children[1].children[1].children[1], btp.root.children[0].children[1].children[0].infoset)

btp.set_outcome(
    btp.root.children[0].children[0].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[2, 4],
        label="0000"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[3, 6],
        label="0001"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[1, 2],
        label="0010"
    )
)

btp.set_outcome(
    btp.root.children[0].children[0].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[5, 3],
        label="0011"
    )
)

btp.set_outcome(
    btp.root.children[0].children[1].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[4, 0],
        label="0100"
    )
)

btp.set_outcome(
    btp.root.children[0].children[1].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[1, 2],
        label="0101"
    )
)

btp.set_outcome(
    btp.root.children[0].children[1].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[5, 6],
        label="0110"
    )
)

btp.set_outcome(
    btp.root.children[0].children[1].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[2, 1],
        label="0111"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[1, 7],
        label="1000"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[1, 8],
        label="1001"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[5, 2],
        label="1010"
    )
)

btp.set_outcome(
    btp.root.children[1].children[0].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[3, 4],
        label="1011"
    )
)

btp.set_outcome(
    btp.root.children[1].children[1].children[0].children[0],
    outcome=btp.add_outcome(
        payoffs=[4, 3],
        label="1100"
    )
)

btp.set_outcome(
    btp.root.children[1].children[1].children[0].children[1],
    outcome=btp.add_outcome(
        payoffs=[7, 0],
        label="1101"
    )
)

btp.set_outcome(
    btp.root.children[1].children[1].children[1].children[0],
    outcome=btp.add_outcome(
        payoffs=[9, 1],
        label="1110"
    )
)

btp.set_outcome(
    btp.root.children[1].children[1].children[1].children[1],
    outcome=btp.add_outcome(
        payoffs=[2, 4],
        label="1111"
    )
)

btp.root.infoset.label = "root"
btp.root.children[0].infoset.label = "A"
btp.root.children[0].children[0].infoset.label = "Aa"
btp.root.children[1].children[0].infoset.label = "Ba"
btp.root.children[0].children[0].children[0].infoset.label = "AaC"
btp.root.children[0].children[1].children[0].infoset.label = "AbC"

actions = ["A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f"]
infosets = ["root", "A", "Aa", "Ba", "AaC", "AbC"]

root = [1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
A = [1, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0]
Aa = [0, 1, 0, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0]
Ba = [0, 0, 1, 0, 0, -1, -1, 0, 0, 0, 0, 0, 0]
AaC = [0, 0, 0, 0, 0, 0, 0, 1, 0, -1, -1, 0, 0]
AbC = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, -1, -1]

constraints = [root, A, Aa, Ba, AaC, AbC]

test_constraints(btp, infosets, actions, constraints)

correspondence = [["A", "B", "C", "D", "E", "F"], ["a", "b", "c", "d", "e", "f"]]

zrs = [0, 0, 0, 0, 0, 0, 0]
n1 = zrs
n2 = zrs
A1 = zrs
A2 = zrs
B1 = zrs
B2 = zrs
C1 = [0, 0, 0, 2, 3, 4, 1]
C2 = [0, 0, 0, 4, 6, 0, 2]
D1 = [0, 0, 0, 1, 5, 5, 2]
D2 = [0, 0, 0, 2, 3, 6, 1]
E1 = [0, 0, 0, 1, 1, 4, 7]
E2 = [0, 0, 0, 7, 8, 3, 0]
F1 = [0, 0, 0, 5, 3, 9, 2]
F2 = [0, 0, 0, 2, 4, 1, 4]

payoffs = np.array([[n1, A1, B1, C1, D1, E1, F1], [n2, A2, B2, C2, D2, E2, F2]])
test_sequence_form(btp, correspondence, payoffs)
































