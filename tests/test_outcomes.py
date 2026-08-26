import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2]), gbt.Game.new_tree()]
)
def test_outcome_add(game: gbt.Game):
    outcome_count = len(game.outcomes)
    game.add_outcome(label="new outcome")
    assert len(game.outcomes) == outcome_count + 1


def test_make_outcome_attaches_to_all_given_nodes():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    outcome = game.make_outcome([up, middle], {"Alice": 1, "Bob": -1}, "shared")
    assert up.outcome == outcome
    assert middle.outcome == outcome
    assert not down.outcome
    assert outcome["Alice"] == 1
    assert outcome["Bob"] == -1


def test_make_outcome_attaches_at_contingencies():
    game = gbt.Game.new_table([2, 2])
    outcome = game.make_outcome([(0, 0), (1, 1)], {"1": 2, "2": -2}, "diagonal")
    assert game[0, 0] == outcome
    assert game[1, 1] == outcome
    assert not game[0, 1]
    assert outcome["1"] == 2


def test_make_outcome_absorbs_fully_covered_outcome_and_reuses_label():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["U", "D"])
    up, down = game.root.children
    game.make_outcome(up, {"Alice": 1}, "w")
    game.make_outcome([up, down], {"Alice": 2}, "w")
    assert [(o.label, o["Alice"]) for o in game.outcomes] == [("w", 2)]


def test_make_outcome_label_of_partially_covered_outcome_refused():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    game.make_outcome([up, middle], {"Alice": 1}, "w")
    with pytest.raises(ValueError):
        game.make_outcome(down, {"Alice": 2}, "w")
    assert len(game.outcomes) == 1


def test_make_outcome_incomplete_payoffs_raises():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "D"])
    with pytest.raises(ValueError):
        game.make_outcome(next(iter(game.root.children)), {"Alice": 1}, "w")


def test_make_outcome_payoffs_naming_player_twice_raises():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "D"])
    alice = game.players["Alice"]
    with pytest.raises(ValueError):
        game.make_outcome(next(iter(game.root.children)),
                          {"Alice": 1, alice: 2, "Bob": 0}, "w")


@pytest.mark.parametrize(
    "game", [gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])]
)
def test_outcome_delete(game: gbt.Game):
    outcome_count = len(game.outcomes)
    game.delete_outcome(next(iter(game.outcomes)))
    assert len(game.outcomes) == outcome_count - 1


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_outcome_label(label: str):
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_outcome_label_invalid_raises_valueerror(label: str):
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    with pytest.raises(ValueError):
        outcome.label = label


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_outcome_label_unicode_accepted(label: str):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome.label == label


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]]), "outcome label")]
)
def test_outcome_index_label(game: gbt.Game, label: str):
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome == game.outcomes[label]
    assert game.outcomes[label].label == label


@pytest.mark.parametrize(
    "game", [gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])]
)
def test_outcome_index_unmatched_label(game: gbt.Game):
    with pytest.raises(KeyError):
        _ = game.outcomes["not an outcome"]


def test_add_outcome_requires_label():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        game.add_outcome([0, 0])


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_invalid_type(game: gbt.Game):
    with pytest.raises(TypeError):
        _ = game.outcomes[1.3]


def test_outcome_payoff_by_player_label():
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    pl1, pl2 = list(game.players)
    game.relabel_players({pl1.label: "joe", pl2.label: "dan"})
    out1, out2, *_ = list(game.outcomes)
    out1["joe"] = 1
    out1["dan"] = 2
    out2["joe"] = 3
    out2["dan"] = 4
    assert out1["joe"] == 1
    assert out1["dan"] == 2
    assert out2["joe"] == 3
    assert out2["dan"] == 4


@pytest.mark.parametrize("bad_label", ["", "win"])
def test_add_outcome_bad_label_raises_and_leaves_game_unchanged(bad_label: str):
    game = gbt.Game.new_tree(players=["A", "B"])
    game.add_outcome("win", [1, 2])
    with pytest.raises(ValueError):
        game.add_outcome(bad_label, [3, 4])
    assert [o.label for o in game.outcomes] == ["win"]


def test_outcome_relabel_duplicate_rejected_and_label_unchanged():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.add_outcome("win", [1, 2])
    outcome = game.add_outcome("lose", [0, 0])
    with pytest.raises(ValueError):
        outcome.label = "win"
    assert outcome.label == "lose"
