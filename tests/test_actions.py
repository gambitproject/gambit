import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_action_label(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.actions))
    game.relabel_actions(game.root, {action: label})
    assert label in game.root.actions


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_action_label_invalid_raises_valueerror(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.actions))
    with pytest.raises(ValueError):
        game.relabel_actions(game.root, {action: label})


def test_relabel_action_empty_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.actions))
    with pytest.raises(ValueError):
        game.relabel_actions(game.root, {action: ""})


def test_relabel_actions_duplicate_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root, {"King": "Queen"})


def test_relabel_actions_error_mismatch():
    """Test to ensure `infoset` is from the same game."""
    game1 = gbt.Game.new_tree()
    game2 = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.MismatchError):
        game1.relabel_actions(game2.root, {"King": "Queen"})


def test_relabel_actions_simultaneous_swap():
    """Reassignment is simultaneous, so a swap is well-defined; applying the entries one
    at a time would collide on the intermediate state.
    """
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(game.root, {"King": "Queen", "Queen": "King"})
    assert list(game.root.event.actions) == ["Queen", "King"]


def test_relabel_actions_duplicate_targets_raises_valueerror():
    """Both replacements are free of the actions left untouched but collide with each
    other, so checking each against the untouched actions alone would let this through.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root, {"King": "Ace", "Queen": "Ace"})


def test_relabel_actions_unknown_label_raises_keyerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(KeyError):
        game.relabel_actions(game.root, {"Jack": "Ace"})


def test_relabel_actions_unknown_label_not_strict_is_ignored():
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(game.root, {"Jack": "Ace", "King": "Ace"}, strict=False)
    assert list(game.root.event.actions) == ["Ace", "Queen"]


def test_relabel_actions_failure_leaves_game_unchanged():
    """The whole mapping is validated before any label is written, so a mapping that
    fails part way through leaves no partial reassignment behind.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root, {"King": "Ace", "Queen": ""})
    assert list(game.root.event.actions) == ["King", "Queen"]


def test_relabel_actions_scope_is_the_information_set():
    """Action labels are unique within an information set, not within a player: Alice's
    two information sets both offer "Bet", and relabelling one leaves the other untouched
    and free to take the same new label.
    """
    game = games.create_stripped_down_poker_efg()
    king = games.find_infoset(game, "Alice", "Alice has King")
    queen = games.find_infoset(game, "Alice", "Alice has Queen")
    game.relabel_actions(next(iter(king.members)), {"Bet": "Raise"})
    assert list(king.actions) == ["Raise", "Fold"]
    assert list(queen.actions) == ["Bet", "Fold"]
    game.relabel_actions(next(iter(queen.members)), {"Bet": "Raise"})
    assert list(queen.actions) == ["Raise", "Fold"]


def test_relabel_actions_not_a_mapping_raises_typeerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(game.root, [("King", "Queen")])


@pytest.mark.parametrize("labels", [{1: "Queen"}, {"King": 1}])
def test_relabel_actions_non_str_label_raises_typeerror(labels: dict):
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(game.root, labels)


def test_set_move_actions_drop_shrinks_actions_and_children():
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    node = next(iter(infoset.members))
    action_count = len(infoset.actions)
    remaining = list(infoset.actions)[1:]
    game.set_move_actions(node, remaining, drop=True)
    assert len(infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


def test_set_move_actions_cannot_remove_the_only_action():
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    node = next(iter(infoset.members))
    last = next(iter(infoset.actions))
    game.set_move_actions(node, [last], drop=True)
    assert list(infoset.actions) == [last]
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_move_actions(node, [], drop=True)


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
    infoset = game.root.children["x"].infoset
    members = list(infoset.members)
    children_before = [{label: member.children[label] for label in ("a", "b", "c")}
                       for member in members]
    game.set_move_actions(game.root.children["x"], ["c", "a", "b"])
    assert list(infoset.actions) == ["c", "a", "b"]
    for member, children in zip(members, children_before, strict=True):
        assert list(member.children) == [children["c"], children["a"], children["b"]]


def test_set_move_actions_add_drop_and_reorder_together():
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    node = next(iter(infoset.members))
    nodes_before = len(game.nodes)
    game.set_move_actions(node, ["Raise", "Fold"], drop=True)
    assert list(infoset.actions) == ["Raise", "Fold"]
    # "Bet" and its subtree (Bob's node and its two terminals) go; "Raise" adds one.
    assert len(game.nodes) == nodes_before - 3 + 1
    assert len(games.find_infoset(game, "Bob", "Bob's response").members) == 1


def test_set_move_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    node = next(iter(infoset.members))
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(node, ["Bet"])
    with pytest.raises(ValueError):
        game.set_move_actions(node, ["Bet", "Fold", "Raise"], add=False)
    assert game.to_efg() == before


def test_set_move_actions_raises_at_an_event():
    """`set_move_actions` is only for a personal player's move; `set_event_actions` is the
    corresponding operation for an event."""
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(game.root, ["King", "Queen"])


@pytest.mark.parametrize("bad_labels", [["Bet", "Bet"], ["Bet", ""], ["Bet", " x"]])
def test_set_move_actions_bad_labels_raise_and_leave_game_unchanged(bad_labels):
    """Duplicate, empty, and invalid labels in `actions` are rejected in C++,
    after the Python guards pass; the game must be unmodified by the failure."""
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    node = next(iter(infoset.members))
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(node, bad_labels, drop=True)
    assert game.to_efg() == before


def test_set_move_actions_absent_minded_drop_and_add():
    """Dropping an action whose subtree contains another member of the same information
    set deletes that member with the subtree."""
    game = gbt.Game.new_tree(players=["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["a", "b"])
    game.append_infoset(gbt.H.path("a"), gbt.H.path())
    game.set_move_actions(game.root, ["b", "c"], drop=True)
    assert list(game.root.infoset.actions) == ["b", "c"]
    assert len(game.root.infoset.members) == 1
    assert len(game.nodes) == 3


def test_set_event_actions_error_mismatch():
    """Test to ensure `event` is from the same game."""
    game1 = gbt.Game.new_tree()
    game2 = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.MismatchError):
        game1.set_event_actions(game2.root, {"King": "1/2", "Queen": "1/2"})


def test_set_event_actions_reorder_carries_probabilities():
    game = games.create_stripped_down_poker_efg()
    game.set_event_actions(game.root, {"King": "3/4", "Queen": "1/4"})
    game.set_event_actions(game.root, {"Queen": "1/4", "King": "3/4"})
    assert list(game.root.actions) == ["Queen", "King"]
    assert game.root.action_probs == {"Queen": gbt.Rational(1, 4), "King": gbt.Rational(3, 4)}


def test_set_event_actions_add_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    nodes_before = len(game.nodes)
    game.set_event_actions(game.root, {"Jack": "1/2", "King": "1/4", "Queen": "1/4"})
    assert list(game.root.actions) == ["Jack", "King", "Queen"]
    assert game.root.action_probs == {
        "Jack": gbt.Rational(1, 2), "King": gbt.Rational(1, 4), "Queen": gbt.Rational(1, 4)
    }
    assert len(game.nodes) == nodes_before + 1


def test_set_event_actions_drop_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    game.set_event_actions(game.root, {"King": 1}, drop=True)
    assert list(game.root.actions) == ["King"]
    assert game.root.action_probs == {"King": 1}


def test_set_event_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    _ = game.root.event
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(game.root, {"King": 1})
    with pytest.raises(ValueError):
        game.set_event_actions(
            game.root, {"King": "1/2", "Queen": "1/4", "Jack": "1/4"}, add=False
        )
    assert game.to_efg() == before


def test_set_event_actions_raises_at_a_move():
    """`set_event_actions` is only for an event; `set_move_actions` is the corresponding
    operation for a personal player's move."""
    game = games.create_stripped_down_poker_efg()
    infoset = games.find_infoset(game, "Alice", "Alice has King")
    with pytest.raises(ValueError):
        game.set_event_actions(next(iter(infoset.members)), {"Bet": 1})


def test_set_event_actions_rejects_non_mapping_probs():
    """`probs` must be a mapping: with no separate list of actions, there's nothing for a
    plain sequence of probabilities to be paired with positionally."""
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(TypeError):
        game.set_event_actions(game.root, ["3/4", "1/4"])
    assert game.to_efg() == before


def test_set_event_actions_bad_distribution_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(game.root, {"King": "3/4", "Queen": "3/4"})
    assert game.to_efg() == before


@pytest.mark.parametrize(
    "game, player_label, strategy_label, infoset_path, expected_action_label",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", "1", [], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", "2", [], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", "1", ["R"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", "2", ["R"], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 3", "1", ["R", "L"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 3", "2", ["R", "L"], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "1", [], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "2", [], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", "1", ["L"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", "2", ["L"], "L"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", "1", [], "U1"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", "2", [], "D1"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", "1", ["U1"], "U2"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", "2", ["U1"], "D2"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", "1", ["U1", "U2"], "U3"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", "2", ["U1", "U2"], "D3"),
    ],
)
def test_get_behavior_prescribed_action_defined(
    game, player_label, strategy_label, infoset_path, expected_action_label
):
    """Verify `Game.get_behavior` retrieves the correct action for defined actions."""
    node = game.root
    for action_label in infoset_path:
        node = node.children[action_label]
    infoset = node.infoset

    prescribed_action = game.get_behavior(player_label, strategy_label).get(infoset)

    assert prescribed_action == expected_action_label


@pytest.mark.parametrize(
    "game, player_label, strategy_label, infoset_label, infoset_path",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "1", None, ["L", "L"]),
        (games.read_from_file("cent3.efg"), "Player 1", "1", "(1,3)", None),
        (games.read_from_file("cent3.efg"), "Player 1", "1", "(1,5)", None),
        (games.read_from_file("cent3.efg"), "Player 1", "2", "(1,5)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "1", "(2,4)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "1", "(2,4)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "2", "(2,5)", None),
    ],
)
def test_get_behavior_prescribed_action_undefined_returns_none(
    game, player_label, strategy_label, infoset_label, infoset_path
):
    """Verify `Game.get_behavior` returns None when called on an unreached player's infoset"""
    if infoset_label is not None:
        infoset = games.find_infoset_in_game(game, infoset_label)
    else:
        node = game.root
        for action_label in infoset_path:
            node = node.children[action_label]
        infoset = node.infoset

    prescribed_action = game.get_behavior(player_label, strategy_label).get(infoset)

    assert prescribed_action is None


@pytest.mark.parametrize(
    "game, player_label, other_infoset_path",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", ["R"]),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", []),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", ["L"]),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", []),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", ["U1"]),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", ["U1", "U2"]),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", []),
    ],
)
def test_get_behavior_raises_value_error_for_wrong_player(
    game, player_label, other_infoset_path
):
    """
    Verify `Game.get_behavior`'s result raises ValueError when the infoset belongs
    to a different player than the strategy.
    """
    behavior = game.get_behavior(player_label, next(iter(game.get_strategies(player_label))))
    node = game.root
    for action_label in other_infoset_path:
        node = node.children[action_label]
    other_players_infoset = node.infoset

    with pytest.raises(ValueError):
        behavior.get(other_players_infoset)
