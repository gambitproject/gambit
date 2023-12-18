import pytest

import pygambit as gbt

from . import games

@pytest.mark.parametrize(
    "game,label",
    [(games.read_from_file("complicated_extensive_game.efg"), "random label")]
)
def test_set_action_label(game: gbt.Game, label: str):
    game.root.infoset.actions[0].label = label
    assert game.root.infoset.actions[0].label == label


@pytest.mark.parametrize(
    "game,inprobs,outprobs",
    [(games.read_from_file("complicated_extensive_game.efg"),
      [0.75, 0.25], [0.75, 0.25]),
     (games.read_from_file("complicated_extensive_game.efg"),
      ["16/17", "1/17"], [gbt.Rational("16/17"), gbt.Rational("1/17")])]
)
def test_set_chance_valid_probability(game: gbt.Game, inprobs: list, outprobs: list):
    game.set_chance_probs(game.root.infoset, inprobs)
    for (action, prob) in zip(game.root.infoset.actions, outprobs):
        assert action.prob == prob


@pytest.mark.parametrize(
    "game,inprobs",
    [(games.read_from_file("complicated_extensive_game.efg"), [0.75, -0.10]),
     (games.read_from_file("complicated_extensive_game.efg"), [0.75, 0.40]),
     (games.read_from_file("complicated_extensive_game.efg"), ["foo", "bar"])]
)
def test_set_chance_improper_probability(game: gbt.Game, inprobs: list):
    with pytest.raises(ValueError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game,inprobs",
    [(games.read_from_file("complicated_extensive_game.efg"), [0.25, 0.75, 0.25]),
     (games.read_from_file("complicated_extensive_game.efg"), [1.00])]
)
def test_set_chance_bad_dimension(game: gbt.Game, inprobs: list):
    with pytest.raises(IndexError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("complicated_extensive_game.efg")]
)
def test_set_chance_personal(game: gbt.Game):
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_chance_probs(game.players[0].infosets[0], [0.75, 0.25])


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("complicated_extensive_game.efg")]
)
def test_action_precedes(game: gbt.Game):
    child = game.root.children[0]
    assert game.root.infoset.actions[0].precedes(child)
    assert not game.root.infoset.actions[1].precedes(child)


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("complicated_extensive_game.efg")]
)
def test_action_precedes_nonnode(game: gbt.Game):
    with pytest.raises(TypeError):
        game.root.infoset.actions[0].precedes(game)


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("complicated_extensive_game.efg")]
)
def test_action_delete_personal(game: gbt.Game):
    node = game.players[0].infosets[0].members[0]
    action_count = len(node.infoset.actions)
    game.delete_action(node.infoset.actions[0])
    assert len(node.infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


@pytest.mark.parametrize(
    "game",
    [games.read_from_file("complicated_extensive_game.efg")]
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
     games.read_from_file("complicated_extensive_game.efg"),
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
