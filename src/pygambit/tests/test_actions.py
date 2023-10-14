import pytest

import pygambit as gbt


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.read_game("test_games/complicated_extensive_game.efg"), "random label")]
)
def test_set_action_label(game: gbt.Game, label: str):
    game.root.infoset.actions[0].label = label
    assert game.root.infoset.actions[0].label == label


@pytest.mark.parametrize(
    "game,inprobs,outprobs",
    [(gbt.Game.read_game("test_games/complicated_extensive_game.efg"),
      [0.75, 0.25], [0.75, 0.25]),
     (gbt.Game.read_game("test_games/complicated_extensive_game.efg"),
      ["16/17", "1/17"], [gbt.Rational("16/17"), gbt.Rational("1/17")])]
)
def test_set_chance_valid_probability(game: gbt.Game, inprobs: list, outprobs: list):
    game.set_chance_probs(game.root.infoset, inprobs)
    for (action, prob) in zip(game.root.infoset.actions, outprobs):
        assert action.prob == prob


@pytest.mark.parametrize(
    "game,inprobs",
    [(gbt.Game.read_game("test_games/complicated_extensive_game.efg"), [0.75, -0.10]),
     (gbt.Game.read_game("test_games/complicated_extensive_game.efg"), [0.75, 0.40]),
     (gbt.Game.read_game("test_games/complicated_extensive_game.efg"), ["foo", "bar"])]
)
def test_set_chance_improper_probability(game: gbt.Game, inprobs: list):
    with pytest.raises(ValueError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game,inprobs",
    [(gbt.Game.read_game("test_games/complicated_extensive_game.efg"), [0.25, 0.75, 0.25]),
     (gbt.Game.read_game("test_games/complicated_extensive_game.efg"), [1.00])]
)
def test_set_chance_bad_dimension(game: gbt.Game, inprobs: list):
    with pytest.raises(IndexError):
        game.set_chance_probs(game.root.infoset, inprobs)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_set_chance_personal(game: gbt.Game):
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_chance_probs(game.players[0].infosets[0], [0.75, 0.25])


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_action_precedes(game: gbt.Game):
    child = game.root.children[0]
    assert game.root.infoset.actions[0].precedes(child)
    assert not game.root.infoset.actions[1].precedes(child)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_action_precedes_nonnode(game: gbt.Game):
    with pytest.raises(TypeError):
        game.root.infoset.actions[0].precedes(game)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_action_delete_personal(game: gbt.Game):
    node = game.players[0].infosets[0].members[0]
    action_count = len(node.infoset.actions)
    game.delete_action(node.infoset.actions[0])
    assert len(node.infoset.actions) == action_count - 1
    assert len(node.children) == action_count - 1


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_action_delete_last(game: gbt.Game):
    node = game.players[0].infosets[0].members[0]
    while len(node.infoset.actions) > 1:
        game.delete_action(node.infoset.actions[0])
    with pytest.raises(gbt.UndefinedOperationError):
        game.delete_action(node.infoset.actions[0])
