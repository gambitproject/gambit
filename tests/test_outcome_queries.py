import pytest

import pygambit as gbt

from . import games


def _labeled_table_game() -> gbt.Game:
    """A 2x2 table game whose two outcomes are each given a distinct, nonempty
    label at creation, so they can be addressed by `relabel_outcomes`/
    `get_outcome_payoffs`/`set_outcome_payoffs`."""
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 0, "2": 0}, "o1")
    game.make_outcome([{"1": "1", "2": "2"}, {"1": "2", "2": "1"}, {"1": "2", "2": "2"}],
                      {"1": 0, "2": 0}, "o2")
    return game


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_outcome_relabel(label: str):
    game = _labeled_table_game()
    game.relabel_outcomes({"o1": label})
    assert label in game.get_outcomes()


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_outcome_relabel_invalid_raises_valueerror(label: str):
    game = _labeled_table_game()
    with pytest.raises(ValueError):
        game.relabel_outcomes({"o1": label})


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_outcome_relabel_unicode_accepted(label: str):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = _labeled_table_game()
    game.relabel_outcomes({"o1": label})
    assert label in game.get_outcomes()


@pytest.mark.parametrize(
    "game", [gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])]
)
def test_outcome_payoffs_unmatched_label_raises_keyerror(game: gbt.Game):
    with pytest.raises(KeyError):
        _ = game.get_outcome_payoffs("not an outcome")


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_payoffs_invalid_label_type_raises_typeerror(game: gbt.Game):
    with pytest.raises(TypeError):
        _ = game.get_outcome_payoffs(1.3)


def test_outcome_payoff_by_player_label():
    game = _labeled_table_game()
    game.relabel_players({"1": "joe", "2": "dan"})
    game.set_outcome_payoffs("o1", {"joe": 1, "dan": 2})
    game.set_outcome_payoffs("o2", {"joe": 3, "dan": 4})
    assert game.get_outcome_payoffs("o1")["joe"] == 1
    assert game.get_outcome_payoffs("o1")["dan"] == 2
    assert game.get_outcome_payoffs("o2")["joe"] == 3
    assert game.get_outcome_payoffs("o2")["dan"] == 4
