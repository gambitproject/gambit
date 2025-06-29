import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize(
    "game,label",
    [(games.create_myerson_2_card_poker_efg(), "random label")]
)
def test_set_action_label(game: gbt.Game, label: str):
    game.root.infoset.actions[0].label = label
    assert game.root.infoset.actions[0].label == label


@pytest.mark.parametrize(
    "game,inprobs,outprobs",
    [(games.create_myerson_2_card_poker_efg(),
      [0.75, 0.25], [0.75, 0.25]),
     (games.create_myerson_2_card_poker_efg(),
      ["16/17", "1/17"], [gbt.Rational("16/17"), gbt.Rational("1/17")])]
)
def test_set_chance_valid_probability(game: gbt.Game, inprobs: list, outprobs: list):
    game.set_chance_probs(game.root.infoset, inprobs)
    for (action, prob) in zip(game.root.infoset.actions, outprobs):
        assert action.prob == prob


@pytest.mark.parametrize(
    "game,inprobs",
    [(games.create_myerson_2_card_poker_efg(), [0.75, -0.10]),
     (games.create_myerson_2_card_poker_efg(), [0.75, 0.40]),
     (games.create_myerson_2_card_poker_efg(), ["foo", "bar"])]
)
def test_set_chance_improper_probability(game: gbt.Game, inprobs: list):
    with pytest.raises(ValueError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game,inprobs",
    [(games.create_myerson_2_card_poker_efg(), [0.25, 0.75, 0.25]),
     (games.create_myerson_2_card_poker_efg(), [1.00])]
)
def test_set_chance_bad_dimension(game: gbt.Game, inprobs: list):
    with pytest.raises(IndexError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game",
    [games.create_myerson_2_card_poker_efg()]
)
def test_set_chance_personal(game: gbt.Game):
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_chance_probs(game.players[0].infosets[0], [0.75, 0.25])


@pytest.mark.parametrize(
    "game",
    [games.create_myerson_2_card_poker_efg()]
)
def test_action_precedes(game: gbt.Game):
    child = game.root.children[0]
    assert game.root.infoset.actions[0].precedes(child)
    assert not game.root.infoset.actions[1].precedes(child)


@pytest.mark.parametrize(
    "game",
    [games.create_myerson_2_card_poker_efg()]
)
def test_action_precedes_nonnode(game: gbt.Game):
    with pytest.raises(TypeError):
        game.root.infoset.actions[0].precedes(game)


@pytest.mark.parametrize(
    "game",
    [games.create_myerson_2_card_poker_efg()]
)
def test_action_delete_personal(game: gbt.Game):
    node = game.players[0].infosets[0].members[0]
    action_count = len(node.infoset.actions)
    game.delete_action(node.infoset.actions[0])
    assert len(node.infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


@pytest.mark.parametrize(
    "game",
    [games.create_myerson_2_card_poker_efg()]
)
def test_action_delete_last(game: gbt.Game):
    node = game.players[0].infosets[0].members[0]
    while len(node.infoset.actions) > 1:
        game.delete_action(node.infoset.actions[0])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_action(node.infoset.actions[0])


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("chance_root_3_moves_only_one_nonzero_prob.efg"),
     games.create_myerson_2_card_poker_efg(),
     games.read_from_file("chance_root_5_moves_no_nonterm_player_nodes.efg")]
)
def test_action_delete_chance(game: gbt.Game):
    """Test the renormalization of action probabilities when an action is deleted at a chance
    node
    """
    chance_iset = game.players.chance.infosets[0]
    while len(chance_iset.actions) > 1:
        old_probs = [a.prob for a in chance_iset.actions]
        game.delete_action(chance_iset.actions[0])
        new_probs = [a.prob for a in chance_iset.actions]
        assert sum(new_probs) == 1
        if sum(old_probs[1:]) == 0:
            for p in new_probs:
                assert p == 1/len(new_probs)
        else:
            for p1, p2 in zip(old_probs[1:], new_probs):
                if p1 == 0:
                    assert p2 == 0
                else:
                    assert p2 == p1 / (1-old_probs[0])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_action(chance_iset.actions[0])


def test_action_plays():
    """Verify `action.plays` returns plays reachable from a given action.
    """
    game = games.read_from_file("e01.efg")
    list_nodes = list(game.nodes)
    list_infosets = list(game.infosets)

    test_action = list_infosets[2].actions[0]  # members' paths=[0, 1, 0], [0, 1]

    expected_set_of_plays = {
        list_nodes[4], list_nodes[7]           # paths=[0, 1, 0], [0, 1]
    }

    assert set(test_action.plays) == expected_set_of_plays


@pytest.mark.parametrize(
    "game, player_ind, str_ind, infoset_ind, expected_action_ind",
    [
        (games.read_from_file("e01.efg"), 0, 0, 0, 0),
        (games.read_from_file("e01.efg"), 0, 1, 0, 1),
        (games.read_from_file("e01.efg"), 1, 0, 1, 0),
        (games.read_from_file("e01.efg"), 1, 1, 1, 1),
        (games.read_from_file("e01.efg"), 2, 0, 2, 0),
        (games.read_from_file("e01.efg"), 2, 1, 2, 1),
        (games.read_from_file("e02.efg"), 0, 0, 0, 0),
        (games.read_from_file("e02.efg"), 0, 1, 0, 1),
        (games.read_from_file("e02.efg"), 1, 0, 2, 0),
        (games.read_from_file("e02.efg"), 1, 1, 2, 1),
        (games.read_from_file("basic_extensive_game.efg"), 0, 0, 0, 0),
        (games.read_from_file("basic_extensive_game.efg"), 0, 1, 0, 1),
        (games.read_from_file("basic_extensive_game.efg"), 1, 0, 1, 0),
        (games.read_from_file("basic_extensive_game.efg"), 1, 1, 1, 1),
        (games.read_from_file("basic_extensive_game.efg"), 2, 0, 2, 0),
        (games.read_from_file("basic_extensive_game.efg"), 2, 1, 2, 1),
    ],
)
def test_strategy_action_defined(game, player_ind, str_ind, infoset_ind, expected_action_ind):
    """Verify `Strategy.action` retrieves the correct action for defined actions.
    """
    player = game.players[player_ind]
    strategy = player.strategies[str_ind]
    infoset = game.infosets[infoset_ind]
    expected_action = infoset.actions[expected_action_ind]

    prescribed_action = strategy.action(infoset)

    assert prescribed_action == expected_action


@pytest.mark.parametrize(
    "game, player_ind, str_ind, infoset_ind",
    [
        (games.read_from_file("e02.efg"), 0, 0, 1),
        (games.read_from_file("cent3.efg"), 0, 0, 1),
        (games.read_from_file("cent3.efg"), 0, 0, 2),
        (games.read_from_file("cent3.efg"), 0, 1, 2),
        (games.read_from_file("cent3.efg"), 1, 0, 7),
        (games.read_from_file("cent3.efg"), 1, 0, 7),
        (games.read_from_file("cent3.efg"), 1, 1, 8),
    ],
)
def test_strategy_action_undefined_returns_none(game, player_ind, str_ind, infoset_ind):
    """Verify `Strategy.action` returns None when called on an unreached player's infoset
    """
    player = game.players[player_ind]
    strategy = player.strategies[str_ind]
    infoset = game.infosets[infoset_ind]

    prescribed_action = strategy.action(infoset)

    assert prescribed_action is None


@pytest.mark.parametrize(
    "game, player_ind, infoset_ind",
    [
        (games.read_from_file("e01.efg"), 0, 1),
        (games.read_from_file("e01.efg"), 1, 0),
        (games.read_from_file("e02.efg"), 0, 2),
        (games.read_from_file("e02.efg"), 1, 0),
        (games.read_from_file("basic_extensive_game.efg"), 0, 1),
        (games.read_from_file("basic_extensive_game.efg"), 1, 2),
        (games.read_from_file("basic_extensive_game.efg"), 2, 0),
    ],
)
def test_strategy_action_raises_value_error_for_wrong_player(game, player_ind, infoset_ind):
    """
    Verify `Strategy.action` raises ValueError when the infoset belongs
    to a different player than the strategy.
    """
    player = game.players[player_ind]
    strategy = player.strategies[0]
    other_players_infoset = game.infosets[infoset_ind]

    with pytest.raises(ValueError):
        strategy.action(other_players_infoset)


def test_strategy_action_raises_error_for_strategic_game():
    """Verify `Strategy.action` retrieves the action prescribed by the strategy
    """
    game_efg = games.read_from_file("e02.efg")
    game_nfg = game_efg.from_arrays(game_efg.to_arrays()[0], game_efg.to_arrays()[1])
    alice = game_nfg.players[0]
    strategy = alice.strategies[0]
    test_infoset = game_efg.infosets[0]

    with pytest.raises(gbt.UndefinedOperationError):
        strategy.action(test_infoset)
