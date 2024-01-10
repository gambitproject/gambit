import pytest

import pygambit as gbt


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2]), gbt.Game.new_tree()]
)
def test_outcome_add(game: gbt.Game):
    outcome_count = len(game.outcomes)
    game.add_outcome()
    assert len(game.outcomes) == outcome_count + 1


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_delete(game: gbt.Game):
    outcome_count = len(game.outcomes)
    game.delete_outcome(game.outcomes[0])
    assert len(game.outcomes) == outcome_count - 1


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.new_table([2, 2]), "outcome label")]
)
def test_outcome_label(game: gbt.Game, label: str):
    game.outcomes[0].label = label
    assert game.outcomes[0].label == label


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.new_table([2, 2]), "outcome label")]
)
def test_outcome_index_label(game: gbt.Game, label: str):
    game.outcomes[0].label = label
    assert game.outcomes[0] == game.outcomes[label]
    assert game.outcomes[label].label == label


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_int_range(game: gbt.Game):
    with pytest.raises(IndexError):
        _ = game.outcomes[2 * len(game.outcomes)]


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_label_range(game: gbt.Game):
    with pytest.raises(KeyError):
        _ = game.outcomes["not an outcome"]


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_invalid_type(game: gbt.Game):
    with pytest.raises(TypeError):
        _ = game.outcomes[1.3]


def test_outcome_payoff_by_player_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "joe"
    game.players[1].label = "dan"
    game.outcomes[0]["joe"] = 1
    game.outcomes[0]["dan"] = 2
    game.outcomes[1]["joe"] = 3
    game.outcomes[1]["dan"] = 4
    assert game.outcomes[0]['joe'] == 1
    assert game.outcomes[0]['dan'] == 2
    assert game.outcomes[1]['joe'] == 3
    assert game.outcomes[1]['dan'] == 4


def test_outcome_payoff_by_player():
    game = gbt.Game.new_table([2, 2])
    game.players[0].label = "joe"
    game.players[1].label = "dan"
    game.outcomes[0]["joe"] = 1
    game.outcomes[0]["dan"] = 2
    game.outcomes[1]["joe"] = 3
    game.outcomes[1]["dan"] = 4
    player1 = game.players[0]
    player2 = game.players[1]
    assert game.outcomes[0][player1] == 1
    assert game.outcomes[0][player2] == 2
    assert game.outcomes[1][player1] == 3
    assert game.outcomes[1][player2] == 4
