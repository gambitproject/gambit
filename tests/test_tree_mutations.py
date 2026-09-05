import pytest

import pygambit as gbt

from . import games


def test_append_move_error_player_actions():
    """Test to ensure there are actions when appending with a player"""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_move(gbt.H.path(), "Player 1", [])


def test_append_move_error_empty_label():
    """Test that an empty label in `actions` is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.append_move(gbt.H.path(), "Player 1", ["a", ""])


def test_append_move_error_duplicate_label():
    """Test that duplicated labels in `actions` are rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.append_move(gbt.H.path(), "Player 1", ["a", "a"])


def test_insert_move_error_player_actions():
    """Test to ensure there are actions when inserting with a player"""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.insert_move(gbt.H.path(), "Player 1", [])


def test_insert_move_error_empty_label():
    """Test that an empty label in `actions` is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.insert_move(gbt.H.path(), "Player 1", ["a", ""])


def test_insert_move_error_duplicate_label():
    """Test that duplicated labels in `actions` are rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.insert_move(gbt.H.path(), "Player 1", ["a", "a"])


def test_node_actions_becomes_undefined_when_truncated():
    """A node's actions become undefined after it is truncated to a leaf."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = games.node_at_history(game, ("U1",))
    assert node.actions
    game.delete_tree(gbt.H.path("U1"))
    with pytest.raises(AttributeError):
        _ = node.actions


def test_node_delete_parent():
    """Test to ensure deleting a parent node works"""
    game = games.read_from_file("basic_extensive_game.efg")
    node = games.node_at_history(game, ("U1",))
    game.delete_parent(gbt.H.path("U1"))
    assert games.node_at_history(game, ()) == node


def test_node_delete_tree():
    """Test to ensure deleting every child of a node works"""
    game = games.read_from_file("basic_extensive_game.efg")
    game.delete_tree(gbt.H.path("U1"))
    assert not game.get_actions(gbt.H.path("U1"))


def test_node_copy_nonterminal():
    """Test on copying to a nonterminal node."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.copy_tree(gbt.H.path(), gbt.H.path())


def _subtrees_equal(
        n1: gbt.Node,
        n2: gbt.Node,
        recursion_stop_node: gbt.Node | None = None
) -> bool:
    if n1 == recursion_stop_node:
        return not n2._children()
    if not n1._children() and not n2._children():
        if not n1.outcome and not n2.outcome:
            return True
        return n1.outcome == n2.outcome
    if bool(n1._children()) != bool(n2._children()):
        return False
    # now, both n1 and n2 are non-terminal
    # check that they are in the same infosets
    if n1 not in n2.members:
        return False
    # check that they have the same number of children
    if len(n1._children()) != len(n2._children()):
        return False

    return all(
        _subtrees_equal(c1, c2, recursion_stop_node) for (c1, c2) in zip(
            n1._children(), n2._children(), strict=True
            )
    )


def test_copy_tree_onto_nondescendent_terminal_node():
    """Test copying a subtree to a non-descendent node."""
    g = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    src_node = games.node_at_history(g, ("R", "L"))
    dest_node = games.node_at_history(g, ("R", "R"))

    g.copy_tree(gbt.H.path("R", "L"), gbt.H.path("R", "R"))

    assert _subtrees_equal(src_node, dest_node)


def test_copy_tree_onto_descendent_terminal_node():
    """Test copying a subtree to a node that's a descendent of the original."""
    g = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    src_node = games.node_at_history(g, ("R",))
    dest_node = games.node_at_history(g, ("R", "L", "R"))

    g.copy_tree(gbt.H.path("R"), gbt.H.path("R", "L", "R"))

    assert _subtrees_equal(src_node, dest_node, dest_node)


def test_node_move_nonterminal():
    """Test on moving to a nonterminal node."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.move_tree(gbt.H.path(), gbt.H.path())


def test_node_move_successor():
    """Test on moving a node to one of its successors."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.move_tree(gbt.H.path(), gbt.H.path("U1", "U2", "U3"))


def test_append_move_creates_single_infoset_list_of_nodes():
    """Test that appending a Selector matching several nodes creates a single
    infoset."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    matches = (("2", "1"), ("1", "1"), ("1", "2"))
    game.append_move(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in matches),
        "Player 3", ["B", "F"]
    )
    assert len(game.get_infosets("Player 3")) == 1


def test_append_move_same_infoset_list_of_nodes():
    """Test that nodes matched by a Selector are resolved in the same infoset."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    matches = (("2", "1"), ("1", "1"))
    game.append_move(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in matches), "Player 3", ["B", "F"]
    )
    assert ("2", "1") in game.get_members(gbt.H.path("1", "1"))


def test_append_move_actions_list_of_nodes():
    """Test that nodes matched by a Selector that resolved in the same infoset
    have the same actions.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    matches = (("2", "1"), ("1", "1"))
    game.append_move(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in matches),
        "Player 3", ["B", "F", "S"]
    )
    assert game.get_actions(gbt.H.path("2", "1")) == game.get_actions(gbt.H.path("1", "1"))


def test_append_move_labels_list_of_nodes():
    """Test that nodes matched by a Selector that resolved in the same infoset
    have the same labels per action.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    matches = (("2", "1"), ("1", "1"))
    game.append_move(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in matches),
        "Player 3", ["B", "F", "S"]
    )

    assert game.get_actions(gbt.H.path("2", "1")) == game.get_actions(gbt.H.path("1", "1"))


def test_append_move_node_list_with_non_terminal_node():
    """Test that we get an UndefinedOperationError when a Selector passed to
    append_move matches a non-terminal node.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_move(gbt.H.path(...), "Player 3", ["B", "F"])


def test_append_move_node_list_is_empty():
    """Test that we get a ValueError when a Selector passed to append_move
    matches no nodes.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    with pytest.raises(ValueError):
        game.append_move(gbt.H.path(...).filter(lambda h: False), "Player 3", ["B", "F"])


def test_append_infoset_node_list_with_non_terminal_node():
    """Test that we get an UndefinedOperationError when a Selector passed to
    append_infoset matches a non-terminal node.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    game.append_move(gbt.H.path("1", "1"), "Player 3", ["B", "F"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_infoset(gbt.H.path(...), gbt.H.path("1", "1"))


def test_append_infoset_node_list_is_empty():
    """Test that we get a ValueError when a Selector passed to append_infoset
    matches no nodes.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    game.append_move(gbt.H.path("1", "1"), "Player 3", ["B", "F"])
    with pytest.raises(ValueError):
        game.append_infoset(gbt.H.path(...).filter(lambda h: False), gbt.H.path("1", "1"))


def test_append_infoset_error_infoset_not_a_selector():
    """Test that we get a TypeError when `infoset` is not a Selector."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    game.append_move(gbt.H.path("1", "1"), "Player 3", ["B", "F"])
    with pytest.raises(TypeError):
        game.append_infoset(gbt.H.path("1", "2"), 42)


def test_append_infoset_error_infoset_terminal():
    """Test that we get an UndefinedOperationError when `infoset` resolves to a
    terminal node."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.set_players(list(game.players) + ["Player 3"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_infoset(gbt.H.path("1", "2"), gbt.H.path("1", "1"))


def test_append_infoset_error_infoset_chance():
    """Test that we get an UndefinedOperationError when `infoset` resolves to a
    chance node."""
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_infoset(gbt.H.path("King", "Bet"), gbt.H.path())


def test_append_event_creates_single_event_list_of_nodes():
    """Test that appending a Selector matching several nodes creates a single
    chance event."""
    game = games.read_from_file("sample_extensive_game.efg")
    matches = (("2", "1"), ("1", "1"))
    game.append_event(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in matches),
        {"a": gbt.Rational(1, 2), "b": gbt.Rational(1, 2)}
    )
    assert ("2", "1") in game.get_members(gbt.H.path("1", "1"))
    assert game.get_actions(gbt.H.path("2", "1"))


def test_append_event_sets_distribution():
    """Test that the new event's actions carry the given probabilities."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.append_event(gbt.H.path("1", "1"), {"a": gbt.Rational(1, 4), "b": gbt.Rational(3, 4)})
    assert list(game.get_action_probs(gbt.H.path("1", "1")).values()) == [
        gbt.Rational(1, 4), gbt.Rational(3, 4)
    ]


def test_append_event_error_actions_empty():
    """Test to ensure there are actions when appending an event."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_event(gbt.H.path("U1", "U2", "U3"), {})


def test_append_event_error_empty_label():
    """Test that an empty label in `actions` is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.append_event(
            gbt.H.path("U1", "U2", "U3"), {"a": gbt.Rational(1, 2), "": gbt.Rational(1, 2)}
        )


def test_append_event_error_node_list_with_non_terminal_node():
    """Test that we get an UndefinedOperationError when a Selector passed to
    append_event matches a non-terminal node.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_event(gbt.H.path(...), {"a": gbt.Rational(1, 2), "b": gbt.Rational(1, 2)})


def test_append_event_error_node_list_is_empty():
    """Test that we get a ValueError when a Selector passed to append_event
    matches no nodes.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    with pytest.raises(ValueError):
        game.append_event(
            gbt.H.path(...).filter(lambda h: False),
            {"a": gbt.Rational(1, 2), "b": gbt.Rational(1, 2)}
        )


def test_append_event_error_invalid_distribution():
    """Test that a distribution which does not sum to one is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.append_event(
            gbt.H.path("U1", "U2", "U3"), {"a": gbt.Rational(1, 2), "b": gbt.Rational(1, 3)}
        )


def test_insert_event_actions_labeled():
    """Test that the inserted event's actions are labeled according to `actions`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    game.insert_event(gbt.H.path("L", "R"), {"Up": gbt.Rational(1, 2), "Down": gbt.Rational(1, 2)})
    assert game.get_actions(gbt.H.path("L", "R")) == ["Up", "Down"]
    assert game.get_player(gbt.H.path("L", "R")) == "Chance"


def test_insert_event_sets_distribution():
    """Test that the inserted event's actions carry the given probabilities."""
    game = games.read_from_file("basic_extensive_game.efg")
    game.insert_event(gbt.H.path(), {"a": gbt.Rational(1, 4), "b": gbt.Rational(3, 4)})
    assert list(game.get_action_probs(gbt.H.path()).values()) == [
        gbt.Rational(1, 4), gbt.Rational(3, 4)
    ]


def test_insert_event_error_actions_empty():
    """Test to ensure there are actions when inserting an event."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.insert_event(gbt.H.path(), {})


def test_insert_event_error_empty_label():
    """Test that an empty label in `actions` is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.insert_event(gbt.H.path(), {"a": gbt.Rational(1, 2), "": gbt.Rational(1, 2)})


def test_insert_event_error_invalid_distribution():
    """Test that a distribution which does not sum to one is rejected."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.insert_event(gbt.H.path(), {"a": gbt.Rational(1, 2), "b": gbt.Rational(1, 3)})


def _count_subtree_nodes(game: gbt.Game, history: tuple, count_terminal: bool) -> int:
    """Counts nodes in the subtree rooted at the node with the given History
    (including that node itself).

    Parameters
    ----------
    game: Game
    history: tuple
        The History of the root of the subtree
    count_terminal: bool
        Include or exclude terminal nodes from count
    """
    children = games.children_histories(game, history)
    count = 1 if count_terminal or children else 0

    for child in children:
        count += _count_subtree_nodes(game, child, count_terminal)
    return count


def _n_nodes(game: gbt.Game) -> int:
    """The number of nodes in the game -- a stand-in for `len(Game.nodes)`,
    removed since `Game.nodes` was."""
    return len(game.get_histories(gbt.H.after()))


def test_len_matches_expected_node_count():
    """Verify the node count matches expectations."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    expected_node_count = 9

    direct_len = _n_nodes(game)
    assert direct_len == expected_node_count

    assert direct_len == _count_subtree_nodes(game, (), True)


def test_len_after_delete_tree():
    """Verify the node count is correct after `delete_tree`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)

    number_of_deleted_nodes = _count_subtree_nodes(game, ("R", "L"), True) - 1

    game.delete_tree(gbt.H.path("R", "L"))

    assert _n_nodes(game) == initial_number_of_nodes - number_of_deleted_nodes


def test_len_after_delete_parent():
    """Verify the node count is correct after `delete_parent`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = _n_nodes(game)

    node_history = ("L", "L")

    number_of_node_ancestors = _count_subtree_nodes(game, node_history, True)
    number_of_parent_ancestors = _count_subtree_nodes(game, node_history[:-1], True)
    diff = number_of_parent_ancestors - number_of_node_ancestors

    game.delete_parent(gbt.H.path("L", "L"))

    assert _n_nodes(game) == initial_number_of_nodes - diff


def test_len_after_append_move():
    """Verify the node count is correct after `append_move`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)

    player = "Player 1"
    actions_to_add = ["T", "M", "B"]

    game.append_move(gbt.H.path("R", "L", "L"), player, actions_to_add)  # the [1,1,0] terminal

    assert _n_nodes(game) == initial_number_of_nodes + len(actions_to_add)


def test_len_after_append_infoset():
    """Verify the node count is correct after `append_infoset`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = _n_nodes(game)

    number_of_infoset_actions = len(game.get_actions(gbt.H.path("L")))

    game.append_infoset(gbt.H.path("L", "L", "l"), gbt.H.path("L"))

    assert _n_nodes(game) == initial_number_of_nodes + number_of_infoset_actions


def test_len_after_set_move_actions_add():
    """Verify the node count is correct after `set_move_actions` creates an action."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)
    # "L" is Player 2's infoset
    num_nodes_in_infoset = len(game.get_members(gbt.H.path("L")))
    labels = list(game.get_actions(gbt.H.path("L")))
    game.set_move_actions(gbt.H.path("L"), labels + ["new"])
    assert _n_nodes(game) == initial_number_of_nodes + num_nodes_in_infoset


def test_len_after_set_move_actions_drop():
    """Verify the node count is correct after `set_move_actions` deletes an action."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = _n_nodes(game)
    action_to_drop = "L"
    nodes_to_delete = sum(
        _count_subtree_nodes(game, (*member, action_to_drop), True)
        for member in game.get_members(gbt.H.path())
    )
    remaining = [a for a in game.get_actions(gbt.H.path()) if a != "L"]
    game.set_move_actions(gbt.H.path(), remaining, drop=True)
    assert _n_nodes(game) == initial_number_of_nodes - nodes_to_delete


def test_len_after_insert_move():
    """Verify the node count is correct after `insert_move`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)

    player = "Player 2"
    actions_to_add = ["a", "b", "c"]

    game.insert_move(gbt.H.path("L", "R"), player, actions_to_add)  # the [1, 0] node

    assert _n_nodes(game) == initial_number_of_nodes + len(actions_to_add)


def test_insert_move_actions_labeled():
    """Test that the inserted move's actions are labeled according to `actions`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    game.insert_move(gbt.H.path("L", "R"), "Player 2", ["Up", "Down"])
    assert game.get_actions(gbt.H.path("L", "R")) == ["Up", "Down"]


def test_len_after_insert_infoset():
    """Verify the node count is correct after `insert_infoset`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)

    number_of_infoset_actions = len(game.get_actions(gbt.H.path("L")))

    game.insert_infoset(gbt.H.path("L", "R"), gbt.H.path("L"))

    assert _n_nodes(game) == initial_number_of_nodes + number_of_infoset_actions


def test_len_after_copy_tree():
    """Verify the node count is correct after `copy_tree`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = _n_nodes(game)
    number_of_src_ancestors = _count_subtree_nodes(game, ("R", "L"), True)

    game.copy_tree(gbt.H.path("R", "L"), gbt.H.path("R", "R"))

    assert _n_nodes(game) == initial_number_of_nodes + number_of_src_ancestors - 1


def test_make_infoset_change_player_keeps_membership():
    """Re-forming an information set under a different player retains its
    membership."""
    game = games.read_from_file("basic_extensive_game.efg")
    _, p2, *_ = game.players
    members = game.get_members(gbt.H.path())
    game.make_infoset(games.selector_for_histories(members), p2)
    assert game.get_player(gbt.H.path()) == p2
    assert game.get_members(gbt.H.path()) == members


def test_make_infoset_terminal_node_raises():
    """All nodes must be decision nodes."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path("U1", "U2", "U3"), game.get_player(gbt.H.path()))


def test_make_infoset_converts_chance_node():
    """A chance node becomes a personal decision node, discarding its probabilities."""
    game = games.read_from_file("stripped_down_poker.efg")   # the deal is a chance move
    personal_history = next(
        h for h in game.get_histories(gbt.H.after())
        if game.get_actions(gbt.H.path(*h)) and game.get_player(gbt.H.path(*h)) != "Chance"
    )
    personal_player = game.get_player(gbt.H.path(*personal_history))
    game.make_infoset(gbt.H.path(), personal_player)
    assert game.get_player(gbt.H.path()) != "Chance"
    assert game.get_player(gbt.H.path()) == personal_player


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
        game.make_infoset(
            gbt.H.path(...).filter(lambda h: False), game.get_player(gbt.H.path())
        )


def test_make_infoset_strategic_game_raises():
    """`make_infoset` is only defined for games with a tree representation."""
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path(), "1")


def test_set_move_actions_add_preserves_existing_action_order():
    """New actions may be declared at any position; the existing actions' relative
    order is preserved."""
    game = games.read_from_file("basic_extensive_game.efg")
    labels = list(game.get_actions(gbt.H.path()))
    game.set_move_actions(gbt.H.path(), labels + ["end"])
    assert game.get_actions(gbt.H.path())[:-1] == labels
    game.set_move_actions(gbt.H.path(), ["front"] + labels + ["end"])
    assert game.get_actions(gbt.H.path())[1:-1] == labels


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
    probs = game.get_action_probs(gbt.H.path())
    for action, prob in zip(game.get_actions(gbt.H.path()), outprobs, strict=True):
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
    king, queen = ("King",), ("Queen",)
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert king in game.get_members(gbt.H.path(*queen))
    assert list(game.get_action_probs(gbt.H.path(*king)).values()) == [
        gbt.Rational("1/4"), gbt.Rational("3/4")
    ]
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
    game.make_event(gbt.H.path("King"), {"Bet": "1/4", "Fold": "3/4"})
    assert game.get_player(gbt.H.path("King")) == "Chance"
    assert list(game.get_action_probs(gbt.H.path("King")).values()) == [
        gbt.Rational("1/4"), gbt.Rational("3/4")
    ]


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
    king, queen = ("King",), ("Queen",)
    game.make_event(gbt.H.path(...), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert king in game.get_members(gbt.H.path(*queen))
    assert list(game.get_action_probs(gbt.H.path(*king)).values()) == [
        gbt.Rational("1/4"), gbt.Rational("3/4")
    ]


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


def _bagwell_p2_histories(game: gbt.Game) -> tuple[tuple, tuple, tuple, tuple]:
    """Player 2's four decision node Histories in Bagwell (1995).

    Player 2 has two information sets -- one for each signal the chance move
    can produce -- each with two members and actions ("S", "C").  Returns
    (A, B, C, D) with {A, B} the members of one and {C, D} of the other.
    """
    return (("S", "s"), ("C", "s"), ("S", "c"), ("C", "c"))


def test_make_infoset_cherry_pick_leaves_rumps():
    """Partial consumption leaves the remainders behind."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_histories(game)
    game.make_infoset(games.selector_for_histories([B, C]), "Player 2")
    assert B in game.get_members(gbt.H.path(*C))
    assert game.get_members(gbt.H.path(*A)) == [A]
    assert game.get_members(gbt.H.path(*D)) == [D]


def test_make_infoset_label_held_by_rump_raises():
    """Reusing a label whose infoset is only partly consumed is rejected."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_histories(game)
    game.make_infoset(games.selector_for_histories([A]), "Player 2", "X")
    # A remains in "X"
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_histories([B, C]), "Player 2", "X")


def test_make_infoset_failure_leaves_game_unchanged():
    """A rejected call must not modify the partition."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_histories(game)
    game.make_infoset(games.selector_for_histories([A, B]), "Player 2", "X")
    game.make_infoset(games.selector_for_histories([C, D]), "Player 2", "Y")
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_histories([B, C]), "Player 2", "X")
    assert A in game.get_members(gbt.H.path(*B))
    assert C in game.get_members(gbt.H.path(*D))


def test_make_infoset_idempotent():
    """Repeating a call is a no-op: label reuse permits equality of membership."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_histories(game)
    game.make_infoset(games.selector_for_histories([B, C]), "Player 2", "Z")
    game.make_infoset(games.selector_for_histories([B, C]), "Player 2", "Z")
    assert B in game.get_members(gbt.H.path(*C))


def test_make_infoset_split_creates_new_infoset():
    """A node split out of an infoset lands in a fresh infoset of its own; the rump
    keeps the rest of the original membership."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_histories(game)
    game.make_infoset(gbt.H.path("S", "s"), "Player 2")
    assert game.get_members(gbt.H.path(*A)) == [A]
    assert B not in game.get_members(gbt.H.path(*A))


def test_make_infoset_across_different_source_players():
    """Nodes drawn from different players all land under the target player."""
    game = gbt.Game.new_tree(players=["1", "2", "3"])
    game.append_move(gbt.H.path(), "1", ["a", "b"])
    game.append_move(gbt.H.path("a"), "2", ["a", "b"])   # player 2
    game.append_move(gbt.H.path("b"), "3", ["a", "b"])   # player 3
    n2, n3 = ("a",), ("b",)
    assert game.get_player(gbt.H.path(*n2)) == "2"
    assert game.get_player(gbt.H.path(*n3)) == "3"
    game.make_infoset(gbt.H.path(...), "1")
    assert n2 in game.get_members(gbt.H.path(*n3))
    assert game.get_player(gbt.H.path(*n2)) == "1"
    assert game.get_player(gbt.H.path(*n3)) == "1"


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_action_label(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.get_actions(gbt.H.path())))
    game.relabel_actions(gbt.H.path(), {action: label})
    assert label in game.get_actions(gbt.H.path())


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_action_label_invalid_raises_valueerror(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.get_actions(gbt.H.path())))
    with pytest.raises(ValueError):
        game.relabel_actions(gbt.H.path(), {action: label})


def test_relabel_action_empty_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.get_actions(gbt.H.path())))
    with pytest.raises(ValueError):
        game.relabel_actions(gbt.H.path(), {action: ""})


def test_relabel_actions_duplicate_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(gbt.H.path(), {"King": "Queen"})


def test_relabel_actions_simultaneous_swap():
    """Reassignment is simultaneous, so a swap is well-defined; applying the entries one
    at a time would collide on the intermediate state.
    """
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(gbt.H.path(), {"King": "Queen", "Queen": "King"})
    assert game.get_actions(gbt.H.path()) == ["Queen", "King"]


def test_relabel_actions_duplicate_targets_raises_valueerror():
    """Both replacements are free of the actions left untouched but collide with each
    other, so checking each against the untouched actions alone would let this through.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(gbt.H.path(), {"King": "Ace", "Queen": "Ace"})


def test_relabel_actions_unknown_label_raises_keyerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(KeyError):
        game.relabel_actions(gbt.H.path(), {"Jack": "Ace"})


def test_relabel_actions_unknown_label_not_strict_is_ignored():
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(gbt.H.path(), {"Jack": "Ace", "King": "Ace"}, strict=False)
    assert game.get_actions(gbt.H.path()) == ["Ace", "Queen"]


def test_relabel_actions_failure_leaves_game_unchanged():
    """The whole mapping is validated before any label is written, so a mapping that
    fails part way through leaves no partial reassignment behind.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(gbt.H.path(), {"King": "Ace", "Queen": ""})
    assert game.get_actions(gbt.H.path()) == ["King", "Queen"]


def _infoset_history(game: gbt.Game, player: str, label: str) -> tuple:
    """The History of the representative member of `player`'s information set
    historically identified by `label`, matching the removed `Player.infosets`
    by-label lookup."""
    history = games._INFOSET_LABEL_HISTORIES[(game.title, label)]
    assert game.get_player(gbt.H.path(*history)) == player
    return history


def test_relabel_actions_scope_is_the_information_set():
    """Action labels are unique within an information set, not within a player: Alice's
    two information sets both offer "Bet", and relabelling one leaves the other untouched
    and free to take the same new label.
    """
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    queen = _infoset_history(game, "Alice", "Alice has Queen")
    game.relabel_actions(gbt.H.path(*king), {"Bet": "Raise"})
    assert game.get_actions(gbt.H.path(*king)) == ["Raise", "Fold"]
    assert game.get_actions(gbt.H.path(*queen)) == ["Bet", "Fold"]
    game.relabel_actions(gbt.H.path(*queen), {"Bet": "Raise"})
    assert game.get_actions(gbt.H.path(*queen)) == ["Raise", "Fold"]


def test_relabel_actions_not_a_mapping_raises_typeerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(gbt.H.path(), [("King", "Queen")])


@pytest.mark.parametrize("labels", [{1: "Queen"}, {"King": 1}])
def test_relabel_actions_non_str_label_raises_typeerror(labels: dict):
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(gbt.H.path(), labels)


def test_set_move_actions_drop_shrinks_actions_and_children():
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    action_count = len(game.get_actions(gbt.H.path(*king)))
    remaining = game.get_actions(gbt.H.path(*king))[1:]
    game.set_move_actions(gbt.H.path(*king), remaining, drop=True)
    assert len(game.get_actions(gbt.H.path(*king))) == action_count - 1
    assert len(games.children_histories(game, king)) == action_count - 1


def test_set_move_actions_cannot_remove_the_only_action():
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    last = next(iter(game.get_actions(gbt.H.path(*king))))
    selector = gbt.H.path(*king)
    game.set_move_actions(selector, [last], drop=True)
    assert game.get_actions(gbt.H.path(*king)) == [last]
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_move_actions(selector, [], drop=True)


def test_set_move_actions_reorder_carries_subtrees():
    """Reordering three actions as a cycle moves every action to a new position.
    Each action carries its whole subtree with it, at every member of the information set."""
    game = gbt.Game.new_tree(players=["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Bob", ["x", "y"])
    game.append_move(gbt.H.path(...), "Alice", ["a", "b", "c"])
    game.append_move(
        gbt.H.path(..., ...).filter(lambda h: (h[0], h[1]) in (("x", "a"), ("y", "b"))),
        "Bob", ["l", "r"]
    )
    members = game.get_members(gbt.H.path("x"))
    children_before = [{label: (*member, label) for label in ("a", "b", "c")}
                       for member in members]
    game.set_move_actions(gbt.H.path("x"), ["c", "a", "b"])
    assert game.get_actions(gbt.H.path("x")) == ["c", "a", "b"]
    for member, children in zip(members, children_before, strict=True):
        assert games.children_histories(game, member) == [
            children["c"], children["a"], children["b"]
        ]


def test_set_move_actions_add_drop_and_reorder_together():
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    nodes_before = _n_nodes(game)
    game.set_move_actions(gbt.H.path(*king), ["Raise", "Fold"], drop=True)
    assert game.get_actions(gbt.H.path(*king)) == ["Raise", "Fold"]
    # "Bet" and its subtree (Bob's node and its two terminals) go; "Raise" adds one.
    assert _n_nodes(game) == nodes_before - 3 + 1
    # Bob's response infoset survives, now with only its Queen-side member: the
    # King-side member (found via the removed "Bet" action) is gone with the subtree.
    assert len(game.get_members(gbt.H.path("Queen", "Bet"))) == 1


def test_set_move_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    before = game.to_efg()
    selector = gbt.H.path(*king)
    with pytest.raises(ValueError):
        game.set_move_actions(selector, ["Bet"])
    with pytest.raises(ValueError):
        game.set_move_actions(selector, ["Bet", "Fold", "Raise"], add=False)
    assert game.to_efg() == before


def test_set_move_actions_raises_at_an_event():
    """`set_move_actions` is only for a personal player's move; `set_event_actions` is the
    corresponding operation for an event."""
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(gbt.H.path(), ["King", "Queen"])


@pytest.mark.parametrize("bad_labels", [["Bet", "Bet"], ["Bet", ""], ["Bet", " x"]])
def test_set_move_actions_bad_labels_raise_and_leave_game_unchanged(bad_labels):
    """Duplicate, empty, and invalid labels in `actions` are rejected in C++,
    after the Python guards pass; the game must be unmodified by the failure."""
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(gbt.H.path(*king), bad_labels, drop=True)
    assert game.to_efg() == before


def test_set_move_actions_absent_minded_drop_and_add():
    """Dropping an action whose subtree contains another member of the same information
    set deletes that member with the subtree."""
    game = gbt.Game.new_tree(players=["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["a", "b"])
    game.append_infoset(gbt.H.path("a"), gbt.H.path())
    game.set_move_actions(gbt.H.path(), ["b", "c"], drop=True)
    assert game.get_actions(gbt.H.path()) == ["b", "c"]
    assert len(game.get_members(gbt.H.path())) == 1
    assert _n_nodes(game) == 3


def test_set_event_actions_reorder_carries_probabilities():
    game = games.create_stripped_down_poker_efg()
    game.set_event_actions(gbt.H.path(), {"King": "3/4", "Queen": "1/4"})
    game.set_event_actions(gbt.H.path(), {"Queen": "1/4", "King": "3/4"})
    assert game.get_actions(gbt.H.path()) == ["Queen", "King"]
    assert game.get_action_probs(gbt.H.path()) == {
        "Queen": gbt.Rational(1, 4), "King": gbt.Rational(3, 4)
    }


def test_set_event_actions_add_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    nodes_before = _n_nodes(game)
    game.set_event_actions(gbt.H.path(), {"Jack": "1/2", "King": "1/4", "Queen": "1/4"})
    assert game.get_actions(gbt.H.path()) == ["Jack", "King", "Queen"]
    assert game.get_action_probs(gbt.H.path()) == {
        "Jack": gbt.Rational(1, 2), "King": gbt.Rational(1, 4), "Queen": gbt.Rational(1, 4)
    }
    assert _n_nodes(game) == nodes_before + 1


def test_set_event_actions_drop_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    game.set_event_actions(gbt.H.path(), {"King": 1}, drop=True)
    assert game.get_actions(gbt.H.path()) == ["King"]
    assert game.get_action_probs(gbt.H.path()) == {"King": 1}


def test_set_event_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(gbt.H.path(), {"King": 1})
    with pytest.raises(ValueError):
        game.set_event_actions(
            gbt.H.path(), {"King": "1/2", "Queen": "1/4", "Jack": "1/4"}, add=False
        )
    assert game.to_efg() == before


def test_set_event_actions_raises_at_a_move():
    """`set_event_actions` is only for an event; `set_move_actions` is the corresponding
    operation for a personal player's move."""
    game = games.create_stripped_down_poker_efg()
    king = _infoset_history(game, "Alice", "Alice has King")
    with pytest.raises(ValueError):
        game.set_event_actions(gbt.H.path(*king), {"Bet": 1})


def test_set_event_actions_rejects_non_mapping_probs():
    """`probs` must be a mapping: with no separate list of actions, there's nothing for a
    plain sequence of probabilities to be paired with positionally."""
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(TypeError):
        game.set_event_actions(gbt.H.path(), ["3/4", "1/4"])
    assert game.to_efg() == before


def test_set_event_actions_bad_distribution_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(gbt.H.path(), {"King": "3/4", "Queen": "3/4"})
    assert game.to_efg() == before
