import pytest

import pygambit as gbt

from . import games


def test_infoset_player_retrieval():
    game = games.read_from_file("basic_extensive_game.efg")
    p1, *_ = game.players
    assert p1 == game.root.player


def test_make_infoset_change_player_keeps_membership():
    """Re-forming an information set under a different player retains its
    membership."""
    game = games.read_from_file("basic_extensive_game.efg")
    _, p2, *_ = game.players
    members = list(game.root.members)
    game.make_infoset(games.selector_for_nodes(members), p2)
    assert game.root.player == p2
    assert list(game.root.members) == members


def test_make_infoset_terminal_node_raises():
    """All nodes must be decision nodes."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path("U1", "U2", "U3"), game.root.player)


def test_make_infoset_converts_chance_node():
    """A chance node becomes a personal decision node, discarding its probabilities."""
    game = games.read_from_file("stripped_down_poker.efg")
    chance_node = game.root            # the deal is a chance move
    personal = next(n for n in game.nodes if not n.is_terminal and n.player != "Chance")
    game.make_infoset(gbt.H.path(), personal.player)
    assert chance_node.player != "Chance"
    assert chance_node.player == personal.player


@pytest.mark.parametrize("node_actions", [["c", "d"], ["b", "a"]])
def test_make_infoset_requires_matching_action_labels(node_actions):
    """Nodes must have the same actions, with the same labels in the same order;
    a matching count is not sufficient."""
    game = gbt.Game.new_tree(players=["1"])
    game.append_move(gbt.H.path(), "1", ["a", "b"])
    game.append_move(gbt.H.path("a"), "1", node_actions)
    with pytest.raises(ValueError):
        game.make_infoset(gbt.H.after().filter(lambda h: h[:] in ((), ("a",))), "1")


def test_make_infoset_empty_nodes_raises():
    """`nodes` must be nonempty."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.make_infoset(gbt.H.path(...).filter(lambda h: False), game.root.player)


def test_make_infoset_strategic_game_raises():
    """`make_infoset` is only defined for games with a tree representation."""
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path(), "1")


def test_set_move_actions_add_preserves_existing_action_order():
    """New actions may be declared at any position; the existing actions' relative
    order is preserved."""
    game = games.read_from_file("basic_extensive_game.efg")
    labels = list(game.root.actions)
    game.set_move_actions(gbt.H.path(), labels + ["end"])
    assert list(game.root.actions)[:-1] == labels
    game.set_move_actions(gbt.H.path(), ["front"] + labels + ["end"])
    assert list(game.root.actions)[1:-1] == labels


@pytest.mark.parametrize(
    "inprobs,outprobs",
    [
        ({"King": "1/4", "Queen": "3/4"}, [gbt.Rational("1/4"), gbt.Rational("3/4")]),
        ({"King": 0.75, "Queen": 0.25}, [0.75, 0.25]),
        ({"King": 1}, [1, 0]),
    ],
)
def test_make_event_sets_probabilities(inprobs, outprobs):
    """Probabilities are given as a mapping from action label to probability,
    which may be sparse: an omitted action is assigned probability zero.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    game.make_event(gbt.H.path(), inprobs, "Deal")
    probs = game.root.action_probs
    for action, prob in zip(game.root.actions, outprobs, strict=True):
        assert probs[action] == prob


@pytest.mark.parametrize("probs", [["1/4", "3/4"], [0.75, 0.25]])
def test_make_event_probs_not_a_mapping_raises_typeerror(probs):
    """A positional sequence of probabilities is no longer accepted."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(TypeError):
        game.make_event(gbt.H.path(), probs, "Deal")


def test_make_event_pools_nodes_from_different_infosets():
    """Nodes in distinct information sets are formed into a single event."""
    game = games.read_from_file("stripped_down_poker.efg")
    nodes = [game.root.children["King"], game.root.children["Queen"]]
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert nodes[0] in nodes[1].members
    assert list(nodes[0].action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]
    assert not game.get_infosets("Alice")


def test_make_event_requires_matching_action_labels():
    """Nodes must have the same actions, with the same labels in the same order."""
    game = games.read_from_file("stripped_down_poker.efg")
    # King node has actions Bet, Fold; its own Bet-child has actions Call, Fold.
    with pytest.raises(ValueError):
        game.make_event(
            gbt.H.after().filter(lambda h: h[:] in (("King",), ("King", "Bet"))),
            {"Bet": "1/2", "Fold": "1/2"}
        )


def test_make_event_converts_personal_node():
    """A personal decision node becomes a chance node carrying the probabilities given."""
    game = games.read_from_file("stripped_down_poker.efg")
    node = game.root.children["King"]
    game.make_event(gbt.H.path("King"), {"Bet": "1/4", "Fold": "3/4"})
    assert node.player == "Chance"
    assert list(node.action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]


def test_make_event_terminal_node_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_event(gbt.H.path("King", "Fold"), {"a": "1/2", "b": "1/2"})


def test_make_event_strategic_game_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_event(gbt.H.path(), {"a": 1})


def test_make_event_empty_nodes_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path(...).filter(lambda h: False), {"a": "1/2", "b": "1/2"})


def test_make_event_label_held_by_rump_raises():
    """A label may be reused only if all members of the event holding it are absorbed."""
    game = games.read_from_file("stripped_down_poker.efg")
    game.make_event(gbt.H.path(...), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path("King"), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    assert game.to_efg() == before


def test_make_event_label_reused_when_fully_absorbed():
    """A label held by an existing event may be reused once all of that
    event's members are absorbed into the new one; the old event is not left behind.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    nodes = [game.root.children["King"], game.root.children["Queen"]]
    game.make_event(gbt.H.path(...), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert nodes[0] in nodes[1].members
    assert list(nodes[0].action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]


@pytest.mark.parametrize(
    "probs", [{"King": "3/4", "Queen": "-1/2"}, {"King": 0.75, "Queen": 0.40},
              {"King": "foo", "Queen": "bar"}]
)
def test_make_event_invalid_probs_raises(probs):
    """Values must be numbers, non-negative, and sum to exactly one."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path(), probs)


def test_make_event_malformed_probs_raises():
    """An unknown action label as a mapping key raises KeyError."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(KeyError):
        game.make_event(gbt.H.path(), {"Jack": 1})


def _bagwell_p2_nodes(game: gbt.Game) -> tuple[gbt.Node, gbt.Node, gbt.Node, gbt.Node]:
    """Player 2's four decision nodes in Bagwell (1995).

    Player 2 has two information sets -- one for each signal the chance move
    can produce -- each with two members and actions ("S", "C").  Returns
    (A, B, C, D) with {A, B} the members of one and {C, D} of the other.
    """
    return (game.root.children["S"].children["s"],
            game.root.children["C"].children["s"],
            game.root.children["S"].children["c"],
            game.root.children["C"].children["c"])


def test_make_infoset_cherry_pick_leaves_rumps():
    """Partial consumption leaves the remainders behind."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2")
    assert B in C.members
    assert list(A.members) == [A]
    assert list(D.members) == [D]


def test_make_infoset_label_held_by_rump_raises():
    """Reusing a label whose infoset is only partly consumed is rejected."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(games.selector_for_nodes([A]), "Player 2", "X")
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "X")  # A remains in "X"


def test_make_infoset_failure_leaves_game_unchanged():
    """A rejected call must not modify the partition."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(games.selector_for_nodes([A, B]), "Player 2", "X")
    game.make_infoset(games.selector_for_nodes([C, D]), "Player 2", "Y")
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "X")
    assert A in B.members
    assert C in D.members


def test_make_infoset_idempotent():
    """Repeating a call is a no-op: label reuse permits equality of membership."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "Z")
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "Z")
    assert B in C.members


def test_make_infoset_split_creates_new_infoset():
    """A node split out of an infoset lands in a fresh infoset of its own; the rump
    keeps the rest of the original membership."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(gbt.H.path("S", "s"), "Player 2")
    assert list(A.members) == [A]
    assert B not in A.members


def test_make_infoset_across_different_source_players():
    """Nodes drawn from different players all land under the target player."""
    game = gbt.Game.new_tree(players=["1", "2", "3"])
    game.append_move(gbt.H.path(), "1", ["a", "b"])
    game.append_move(gbt.H.path("a"), "2", ["a", "b"])   # player 2
    game.append_move(gbt.H.path("b"), "3", ["a", "b"])   # player 3
    n2 = game.root.children["a"]
    n3 = game.root.children["b"]
    assert n2.player == "2"
    assert n3.player == "3"
    game.make_infoset(gbt.H.path(...), "1")
    assert n2 in n3.members
    assert n2.player == "1"
    assert n3.player == "1"


def test_node_members_is_a_plain_snapshot_list():
    """`members` returns a plain `list`, not a lazily-resolved view: it supports
    integer indexing, and a list obtained before a mutation keeps reflecting the
    information set as it was at the time, rather than tracking its owner."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    members = node.members
    assert isinstance(members, list)
    assert node in (members[0], members[1])
    game.make_infoset(gbt.H.path("U1"), node.player)
    assert len(members) == 2
    assert list(node.members) == [node]
