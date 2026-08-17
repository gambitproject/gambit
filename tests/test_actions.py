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
    infoset = game.root.infoset
    assert any(child.is_successor_of(m.children["King"]) for m in infoset.members)
    assert not any(child.is_successor_of(m.children["Queen"]) for m in infoset.members)


@pytest.mark.parametrize("game", [games.create_stripped_down_poker_efg()])
def test_action_delete_personal(game: gbt.Game):
    node = next(iter(game.players["Alice"].infosets["Alice has King"].members))
    action_count = len(node.infoset.actions)
    game.delete_action(next(iter(node.infoset.actions)))
    assert len(node.infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


@pytest.mark.parametrize("game", [games.create_stripped_down_poker_efg()])
def test_action_delete_last(game: gbt.Game):
    infoset = game.players["Alice"].infosets["Alice has King"]
    while len(infoset.actions) > 1:
        game.delete_action(next(iter(infoset.actions)))
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_action(next(iter(infoset.actions)))


@pytest.mark.parametrize(
    "game",
    [
        games.read_from_file("chance_root_3_moves_only_one_nonzero_prob.efg"),
        games.create_stripped_down_poker_efg(),
        games.read_from_file("chance_root_5_moves_no_nonterm_player_nodes.efg"),
    ],
)
def test_action_delete_chance(game: gbt.Game):
    """Test the renormalization of action probabilities when an action is deleted at a chance
    node
    """
    chance_infoset = next(iter(game.players.chance.infosets))
    node = next(iter(chance_infoset.members))
    while len(chance_infoset.actions) > 1:
        old_probs = list(node.action_probs.values())
        game.delete_action(next(iter(chance_infoset.actions)))
        new_probs = list(node.action_probs.values())
        assert sum(new_probs) == 1
        if sum(old_probs[1:]) == 0:
            for p in new_probs:
                assert p == 1 / len(new_probs)
        else:
            for p1, p2 in zip(old_probs[1:], new_probs, strict=True):
                if p1 == 0:
                    assert p2 == 0
                else:
                    assert p2 == p1 / (1 - old_probs[0])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_action(next(iter(chance_infoset.actions)))


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
