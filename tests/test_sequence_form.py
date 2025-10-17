import pygambit as gbt


def check_subtensor(game, payoffs, seq_dict, player_index, sequence_form):
    if player_index == len(game.players):
        for player in game.players:
            assert sequence_form.get_payoff(seq_dict, player) == payoffs[player.label]
    else:
        player = game.players[player_index]
        seq_dict[player] = None
        sub_payoffs = payoffs[None]
        check_subtensor(game, sub_payoffs, seq_dict, player_index + 1, sequence_form)
        for action in player.actions:
            seq_dict[player] = action
            sub_payoffs = payoffs[action.label]
            check_subtensor(game, sub_payoffs, seq_dict, player_index + 1, sequence_form)

def test_sequence_form(game, payoffs):
    sequence_form = gbt.GameSequenceForm(game)
    seq_dict = {}
    check_subtensor(game, payoffs, seq_dict, 0, sequence_form)


g = gbt.Game.new_tree(
    players=["Buyer", "Seller"],
    title="One-shot trust game, after Kreps (1990)"
)

g.append_move(
    g.root,  # This is the node to append the move to
    player="Buyer",
    actions=["A", "B"]
)

g.set_outcome(
    g.root.children[0],
    outcome=g.add_outcome(
        payoffs=[4, 1],
        label="10"
    )
)

g.set_outcome(
    g.root.children[1],
    outcome=g.add_outcome(
        payoffs=[4, 1],
        label="1"
    )
)

p1 = g.players[0]
p2 = g.players[1]


names = {p1 : ["A", "B"], p2 : []}
pffs = {None: {None : {"Buyer" : 0, "Seller" : 0}}, "A" : {None : {"Buyer" : 4, "Seller" : 1}}, "B" : {None : {"Buyer" : 4, "Seller" : 1}}}

test_sequence_form(g, pffs)
