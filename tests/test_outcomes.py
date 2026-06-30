import pytest

import pygambit as gbt

from . import games


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
    game.delete_outcome(next(iter(game.outcomes)))
    assert len(game.outcomes) == outcome_count - 1


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_outcome_label(label: str):
    game = gbt.Game.new_table([2, 2])
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_outcome_label_invalid_raises_valueerror(label: str):
    game = gbt.Game.new_table([2, 2])
    outcome = next(iter(game.outcomes))
    with pytest.raises(ValueError):
        outcome.label = label


@pytest.mark.parametrize("label", games.NON_ASCII_LABELS)
def test_outcome_label_non_ascii_rejected(label: str):
    """ASCII-only for 16.7 (#944); Unicode deferred to #862 (17.0)."""
    game = gbt.Game.new_table([2, 2])
    outcome = next(iter(game.outcomes))
    with pytest.raises(UnicodeEncodeError):
        outcome.label = label


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.new_table([2, 2]), "outcome label")]
)
def test_outcome_index_label(game: gbt.Game, label: str):
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome == game.outcomes[label]
    assert game.outcomes[label].label == label


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_unmatched_label(game: gbt.Game):
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
    pl1, pl2 = list(game.players)
    pl1.label = "joe"
    pl2.label = "dan"
    out1, out2, *_ = list(game.outcomes)
    out1["joe"] = 1
    out1["dan"] = 2
    out2["joe"] = 3
    out2["dan"] = 4
    assert out1["joe"] == 1
    assert out1["dan"] == 2
    assert out2["joe"] == 3
    assert out2["dan"] == 4
