import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_action_label(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.infoset.actions))
    game.relabel_actions(game.root.infoset, {action.label: label})
    assert action.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_action_label_invalid_raises_valueerror(label: str):
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.infoset.actions))
    with pytest.raises(ValueError):
        game.relabel_actions(game.root.infoset, {action.label: label})


def test_relabel_action_empty_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    action = next(iter(game.root.infoset.actions))
    with pytest.raises(ValueError):
        game.relabel_actions(game.root.infoset, {action.label: ""})


def test_relabel_actions_duplicate_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root.infoset, {"King": "Queen"})


def test_relabel_actions_simultaneous_swap():
    """Reassignment is simultaneous, so a swap is well-defined; applying the entries one
    at a time would collide on the intermediate state.
    """
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(game.root.infoset, {"King": "Queen", "Queen": "King"})
    assert [action.label for action in game.root.infoset.actions] == ["Queen", "King"]


def test_relabel_actions_duplicate_targets_raises_valueerror():
    """Both replacements are free of the actions left untouched but collide with each
    other, so checking each against the untouched actions alone would let this through.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root.infoset, {"King": "Ace", "Queen": "Ace"})


def test_relabel_actions_unknown_label_raises_keyerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(KeyError):
        game.relabel_actions(game.root.infoset, {"Jack": "Ace"})


def test_relabel_actions_unknown_label_not_strict_is_ignored():
    game = games.create_stripped_down_poker_efg()
    game.relabel_actions(game.root.infoset, {"Jack": "Ace", "King": "Ace"}, strict=False)
    assert [action.label for action in game.root.infoset.actions] == ["Ace", "Queen"]


def test_relabel_actions_failure_leaves_game_unchanged():
    """The whole mapping is validated before any label is written, so a mapping that
    fails part way through leaves no partial reassignment behind.
    """
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(ValueError):
        game.relabel_actions(game.root.infoset, {"King": "Ace", "Queen": ""})
    assert [action.label for action in game.root.infoset.actions] == ["King", "Queen"]


def test_relabel_actions_scope_is_the_information_set():
    """Action labels are unique within an information set, not within a player: Alice's
    two information sets both offer "Bet", and relabelling one leaves the other untouched
    and free to take the same new label.
    """
    game = games.create_stripped_down_poker_efg()
    king = game.players["Alice"].infosets["Alice has King"]
    queen = game.players["Alice"].infosets["Alice has Queen"]
    game.relabel_actions(king, {"Bet": "Raise"})
    assert [action.label for action in king.actions] == ["Raise", "Fold"]
    assert [action.label for action in queen.actions] == ["Bet", "Fold"]
    game.relabel_actions(queen, {"Bet": "Raise"})
    assert [action.label for action in queen.actions] == ["Raise", "Fold"]


def test_relabel_actions_not_a_mapping_raises_typeerror():
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(game.root.infoset, [("King", "Queen")])


@pytest.mark.parametrize("labels", [{1: "Queen"}, {"King": 1}])
def test_relabel_actions_non_str_label_raises_typeerror(labels: dict):
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(TypeError):
        game.relabel_actions(game.root.infoset, labels)


@pytest.mark.parametrize("game", [games.create_stripped_down_poker_efg()])
def test_action_precedes(game: gbt.Game):
    child = game.root.children["King"]
    assert game.root.infoset.actions["King"].precedes(child)
    assert not game.root.infoset.actions["Queen"].precedes(child)


@pytest.mark.parametrize("game", [games.create_stripped_down_poker_efg()])
def test_action_precedes_nonnode(game: gbt.Game):
    action = next(iter(game.root.infoset.actions))
    with pytest.raises(TypeError):
        action.precedes(game)


def test_set_move_actions_drop_shrinks_actions_and_children():
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    node = next(iter(infoset.members))
    action_count = len(infoset.actions)
    remaining = [action.label for action in infoset.actions][1:]
    game.set_move_actions(infoset, remaining, drop=True)
    assert len(infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


def test_set_move_actions_cannot_remove_the_only_action():
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    last = next(iter(infoset.actions)).label
    game.set_move_actions(infoset, [last], drop=True)
    assert [action.label for action in infoset.actions] == [last]
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_move_actions(infoset, [], drop=True)


def test_set_move_actions_reorder_carries_subtrees():
    """Reordering three actions as a cycle moves every action to a new position.
    Each action carries its whole subtree with it, at every member of the information set."""
    game = gbt.Game.new_tree(players=["Alice", "Bob"])
    game.append_move(game.root, "Bob", ["x", "y"])
    game.append_move(list(game.root.children), "Alice", ["a", "b", "c"])
    game.append_move([game.root.children["x"].children["a"],
                      game.root.children["y"].children["b"]], "Bob", ["l", "r"])
    infoset = game.root.children["x"].infoset
    members = list(infoset.members)
    children_before = [{label: member.children[label] for label in ("a", "b", "c")}
                       for member in members]
    plays_before = {action.label: set(action.plays) for action in infoset.actions}
    game.set_move_actions(infoset, ["c", "a", "b"])
    assert [action.label for action in infoset.actions] == ["c", "a", "b"]
    for member, children in zip(members, children_before, strict=True):
        assert list(member.children) == [children["c"], children["a"], children["b"]]
    assert {action.label: set(action.plays) for action in infoset.actions} == plays_before


def test_set_move_actions_add_drop_and_reorder_together():
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    nodes_before = len(game.nodes)
    game.set_move_actions(infoset, ["Raise", "Fold"], drop=True)
    assert [action.label for action in infoset.actions] == ["Raise", "Fold"]
    # "Bet" and its subtree (Bob's node and its two terminals) go; "Raise" adds one.
    assert len(game.nodes) == nodes_before - 3 + 1
    assert len(game.players["Bob"].infosets["Bob's response"].members) == 1


def test_set_move_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(infoset, ["Bet"])
    with pytest.raises(ValueError):
        game.set_move_actions(infoset, ["Bet", "Fold", "Raise"], add=False)
    assert game.to_efg() == before


def test_set_move_actions_raises_at_an_event():
    """`set_move_actions` is only for a personal player's move; `set_event_actions` is the
    corresponding operation for an event."""
    game = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_move_actions(game.root.infoset, ["King", "Queen"])


@pytest.mark.parametrize("bad_labels", [["Bet", "Bet"], ["Bet", ""], ["Bet", " x"]])
def test_set_move_actions_bad_labels_raise_and_leave_game_unchanged(bad_labels):
    """Duplicate, empty, and invalid labels in `actions` are rejected in C++,
    after the Python guards pass; the game must be unmodified by the failure."""
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_move_actions(infoset, bad_labels, drop=True)
    assert game.to_efg() == before


def test_set_move_actions_absent_minded_drop_and_add():
    """Dropping an action whose subtree contains another member of the same information
    set deletes that member with the subtree."""
    game = gbt.Game.new_tree(players=["Alice"])
    game.append_move(game.root, "Alice", ["a", "b"])
    game.append_infoset(game.root.children["a"], game.root.infoset)
    game.set_move_actions(game.root.infoset, ["b", "c"], drop=True)
    assert [action.label for action in game.root.infoset.actions] == ["b", "c"]
    assert len(game.root.infoset.members) == 1
    assert len(game.nodes) == 3


def test_set_event_actions_reorder_carries_probabilities():
    game = games.create_stripped_down_poker_efg()
    event = game.root.infoset
    game.set_event_actions(event, {"King": "3/4", "Queen": "1/4"})
    game.set_event_actions(event, {"Queen": "1/4", "King": "3/4"})
    assert [(a.label, a.prob) for a in event.actions] == [("Queen", gbt.Rational(1, 4)),
                                                          ("King", gbt.Rational(3, 4))]


def test_set_event_actions_add_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    event = game.root.infoset
    nodes_before = len(game.nodes)
    game.set_event_actions(event, {"Jack": "1/2", "King": "1/4", "Queen": "1/4"})
    assert [(a.label, a.prob) for a in event.actions] == [("Jack", gbt.Rational(1, 2)),
                                                          ("King", gbt.Rational(1, 4)),
                                                          ("Queen", gbt.Rational(1, 4))]
    assert len(game.nodes) == nodes_before + 1


def test_set_event_actions_drop_with_probs_mapping():
    game = games.create_stripped_down_poker_efg()
    event = game.root.infoset
    game.set_event_actions(event, {"King": 1}, drop=True)
    assert [(a.label, a.prob) for a in event.actions] == [("King", 1)]


def test_set_event_actions_unconfirmed_drop_and_disabled_add_raise():
    game = games.create_stripped_down_poker_efg()
    event = game.root.infoset
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(event, {"King": 1})
    with pytest.raises(ValueError):
        game.set_event_actions(event, {"King": "1/2", "Queen": "1/4", "Jack": "1/4"}, add=False)
    assert game.to_efg() == before


def test_set_event_actions_raises_at_a_move():
    """`set_event_actions` is only for an event; `set_move_actions` is the corresponding
    operation for a personal player's move."""
    game = games.create_stripped_down_poker_efg()
    infoset = game.players["Alice"].infosets["Alice has King"]
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_event_actions(infoset, {"Bet": 1})


def test_set_event_actions_rejects_non_mapping_probs():
    """`probs` must be a mapping: with no separate list of actions, there's nothing for a
    plain sequence of probabilities to be paired with positionally."""
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(TypeError):
        game.set_event_actions(game.root.infoset, ["3/4", "1/4"])
    assert game.to_efg() == before


def test_set_event_actions_bad_distribution_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.set_event_actions(game.root.infoset, {"King": "3/4", "Queen": "3/4"})
    assert game.to_efg() == before


def test_action_plays():
    """Verify `action.plays` returns plays reachable from a given action."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")

    def node_at(path: list[str]) -> gbt.Node:
        node = game.root
        for action_label in path:
            node = node.children[action_label]
        return node

    test_action = node_at(["L"]).infoset.actions["R"]

    expected_set_of_plays = {node_at(["R", "L", "R"]), node_at(["L", "R"])}

    assert set(test_action.plays) == expected_set_of_plays


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
def test_strategy_action_defined(
    game, player_label, strategy_label, infoset_path, expected_action_label
):
    """Verify `Strategy.action` retrieves the correct action for defined actions."""
    player = game.players[player_label]
    strategy = player.strategies[strategy_label]
    node = game.root
    for action_label in infoset_path:
        node = node.children[action_label]
    infoset = node.infoset
    expected_action = infoset.actions[expected_action_label]

    prescribed_action = strategy.action(infoset)

    assert prescribed_action == expected_action


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
def test_strategy_action_undefined_returns_none(
    game, player_label, strategy_label, infoset_label, infoset_path
):
    """Verify `Strategy.action` returns None when called on an unreached player's infoset"""
    player = game.players[player_label]
    strategy = player.strategies[strategy_label]
    if infoset_label is not None:
        infoset = game.infosets[infoset_label]
    else:
        node = game.root
        for action_label in infoset_path:
            node = node.children[action_label]
        infoset = node.infoset

    prescribed_action = strategy.action(infoset)

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
def test_strategy_action_raises_value_error_for_wrong_player(
    game, player_label, other_infoset_path
):
    """
    Verify `Strategy.action` raises ValueError when the infoset belongs
    to a different player than the strategy.
    """
    player = game.players[player_label]
    strategy = next(iter(player.strategies))
    node = game.root
    for action_label in other_infoset_path:
        node = node.children[action_label]
    other_players_infoset = node.infoset

    with pytest.raises(ValueError):
        strategy.action(other_players_infoset)


def test_strategy_action_raises_error_for_strategic_game():
    """Verify `Strategy.action` retrieves the action prescribed by the strategy"""
    game_efg = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    game_nfg = game_efg.from_arrays(game_efg.to_arrays()[0], game_efg.to_arrays()[1])
    alice = next(iter(game_nfg.players))
    strategy = next(iter(alice.strategies))
    test_infoset = next(iter(game_efg.infosets))

    with pytest.raises(gbt.UndefinedOperationError):
        strategy.action(test_infoset)


def test_player_actions_len():
    game = games.create_stripped_down_poker_efg()
    for player in game.players:
        assert len(player.actions) == len(list(player.actions))
