import pytest

import pygambit as gbt

from . import games


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


def test_make_outcome_accepts_selector():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    outcome = game.make_outcome(gbt.H.path("U"), {"Alice": 1, "Bob": -1}, "shared")
    assert up.outcome == outcome
    assert not middle.outcome
    assert not down.outcome


def test_make_outcome_accepts_selector_matching_several_nodes():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    outcome = game.make_outcome(gbt.H.plays, {"Alice": 1, "Bob": -1}, "shared")
    assert up.outcome == outcome
    assert middle.outcome == outcome
    assert down.outcome == outcome


def test_make_outcome_accepts_history_tuple():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    outcome = game.make_outcome(("U",), {"Alice": 1, "Bob": -1}, "shared")
    assert up.outcome == outcome
    assert not middle.outcome
    assert not down.outcome


def test_make_outcome_accepts_iterable_of_history_tuples():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    outcome = game.make_outcome([("U",), ("M",)], {"Alice": 1, "Bob": -1}, "shared")
    assert up.outcome == outcome
    assert middle.outcome == outcome
    assert not down.outcome


def test_make_outcome_attaches_at_contingencies():
    game = gbt.Game.new_table([2, 2])
    outcome = game.make_outcome(
        [{"1": "1", "2": "1"}, {"1": "2", "2": "2"}], {"1": 2, "2": -2}, "diagonal"
    )
    assert game.get_outcome({"1": "1", "2": "1"}) == outcome
    assert game.get_outcome({"1": "2", "2": "2"}) == outcome
    assert not game.get_outcome({"1": "1", "2": "2"})
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


@pytest.mark.parametrize("bad_label", ["", "win"])
def test_make_outcome_bad_label_raises_and_leaves_game_unchanged(bad_label: str):
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(game.root, "A", ["win", "lose"])
    win_node, lose_node = game.root.children
    game.make_outcome(win_node, {"A": 1, "B": 2}, "win")
    with pytest.raises(ValueError):
        game.make_outcome(lose_node, {"A": 3, "B": 4}, bad_label)
    assert [o.label for o in game.outcomes] == ["win"]


def test_make_outcome_incomplete_payoffs_raises():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "D"])
    with pytest.raises(ValueError):
        game.make_outcome(next(iter(game.root.children)), {"Alice": 1}, "w")


class _RepeatedEntryPayoffs:
    """A Mapping-like object whose `.items()` may repeat a key.

    Used to exercise `make_outcome`'s "named twice" check, which a plain
    ``dict`` literal cannot: duplicate string keys collapse before the
    dict is ever constructed.
    """

    def __init__(self, entries):
        self._entries = entries

    def items(self):
        return self._entries


def test_make_outcome_payoffs_naming_player_twice_raises():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "D"])
    payoffs = _RepeatedEntryPayoffs([("Alice", 1), ("Alice", 2), ("Bob", 0)])
    with pytest.raises(ValueError):
        game.make_outcome(next(iter(game.root.children)), payoffs, "w")


def test_make_outcome_null_resets_given_nodes_to_null():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    game.make_outcome([up, middle], {"Alice": 1, "Bob": -1}, "shared")
    game.make_outcome_null(up)
    assert not up.outcome
    assert middle.outcome
    assert not down.outcome


def test_make_outcome_null_accepts_selector():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    game.make_outcome([up, middle], {"Alice": 1, "Bob": -1}, "shared")
    game.make_outcome_null(gbt.H.path("U"))
    assert not up.outcome
    assert middle.outcome
    assert not down.outcome


def test_make_outcome_null_accepts_history_tuple():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    game.make_outcome([up, middle], {"Alice": 1, "Bob": -1}, "shared")
    game.make_outcome_null(("U",))
    assert not up.outcome
    assert middle.outcome
    assert not down.outcome


def test_make_outcome_null_resets_given_contingencies_to_null():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome(
        [{"1": "1", "2": "1"}, {"1": "2", "2": "2"}], {"1": 2, "2": -2}, "diagonal"
    )
    game.make_outcome_null({"1": "1", "2": "1"})
    assert not game.get_outcome({"1": "1", "2": "1"})
    assert game.get_outcome({"1": "2", "2": "2"})


def test_make_outcome_null_removes_fully_orphaned_outcome():
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome_count = len(game.outcomes)
    p1, p2 = game.players
    s1 = next(iter(game.get_strategies(p1)))
    s2 = next(iter(game.get_strategies(p2)))
    game.make_outcome_null({p1: s1, p2: s2})
    assert len(game.outcomes) == outcome_count - 1


def test_make_outcome_null_keeps_partially_referenced_outcome():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["U", "M", "D"])
    up, middle, down = game.root.children
    game.make_outcome([up, middle], {"Alice": 1}, "shared")
    outcome_count = len(game.outcomes)
    game.make_outcome_null(up)
    assert len(game.outcomes) == outcome_count
    assert middle.outcome


def test_make_outcome_null_on_already_null_node_is_a_no_op():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["U", "D"])
    up, _ = game.root.children
    outcome_count = len(game.outcomes)
    game.make_outcome_null(up)
    assert outcome_count == len(game.outcomes)
    assert not up.outcome


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


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_invalid_type(game: gbt.Game):
    with pytest.raises(TypeError):
        _ = game.outcomes[1.3]


def test_outcome_payoff_by_player_label():
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    pl1, pl2 = list(game.players)
    game.relabel_players({pl1: "joe", pl2: "dan"})
    out1, out2, *_ = list(game.outcomes)
    out1["joe"] = 1
    out1["dan"] = 2
    out2["joe"] = 3
    out2["dan"] = 4
    assert out1["joe"] == 1
    assert out1["dan"] == 2
    assert out2["joe"] == 3
    assert out2["dan"] == 4


def test_outcome_relabel_duplicate_rejected_and_label_unchanged():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(game.root, "A", ["win", "lose"])
    win_node, lose_node = game.root.children
    game.make_outcome(win_node, {"A": 1, "B": 2}, "win")
    outcome = game.make_outcome(lose_node, {"A": 0, "B": 0}, "lose")
    with pytest.raises(ValueError):
        outcome.label = "win"
    assert outcome.label == "lose"
