import pytest

import pygambit as gbt

from . import games


def test_make_outcome_attaches_to_all_given_nodes():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    outcome = game.make_outcome(
        gbt.H.path(...).filter(lambda h: h[0] in ("U", "M")), {"Alice": 1, "Bob": -1}, "shared"
    )
    assert game.get_outcome(gbt.H.path("U")) == outcome.label
    assert game.get_outcome(gbt.H.path("M")) == outcome.label
    assert game.get_outcome(gbt.H.path("D")) is None
    assert outcome["Alice"] == 1
    assert outcome["Bob"] == -1


def test_make_outcome_accepts_selector():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    outcome = game.make_outcome(gbt.H.path("U"), {"Alice": 1, "Bob": -1}, "shared")
    assert game.get_outcome(gbt.H.path("U")) == outcome.label
    assert game.get_outcome(gbt.H.path("M")) is None
    assert game.get_outcome(gbt.H.path("D")) is None


def test_make_outcome_accepts_selector_matching_several_nodes():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    outcome = game.make_outcome(gbt.H.plays, {"Alice": 1, "Bob": -1}, "shared")
    assert game.get_outcome(gbt.H.path("U")) == outcome.label
    assert game.get_outcome(gbt.H.path("M")) == outcome.label
    assert game.get_outcome(gbt.H.path("D")) == outcome.label


def test_make_outcome_accepts_grouped_selector_pooled():
    """A `GroupedSelector`'s groups are pooled together: every matched node
    receives the same outcome, regardless of grouping."""
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    outcome = game.make_outcome(
        gbt.H.path(...).by(lambda h: h[0]), {"Alice": 1, "Bob": -1}, "shared"
    )
    assert game.get_outcome(gbt.H.path("U")) == outcome.label
    assert game.get_outcome(gbt.H.path("M")) == outcome.label
    assert game.get_outcome(gbt.H.path("D")) == outcome.label


def test_make_outcome_error_location_not_a_selector():
    """A bare `Node` or `History` tuple is no longer accepted for an extensive game."""
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    with pytest.raises(TypeError):
        game.make_outcome(games.node_at_history(game, ("U",)), {"Alice": 1}, "w")
    with pytest.raises(TypeError):
        game.make_outcome(("U",), {"Alice": 1}, "w")


def test_make_outcome_attaches_at_contingencies():
    game = gbt.Game.new_table([2, 2])
    outcome = game.make_outcome(
        [{"1": "1", "2": "1"}, {"1": "2", "2": "2"}], {"1": 2, "2": -2}, "diagonal"
    )
    assert game.get_outcome({"1": "1", "2": "1"}) == outcome.label
    assert game.get_outcome({"1": "2", "2": "2"}) == outcome.label
    assert not game.get_outcome({"1": "1", "2": "2"})
    assert outcome["1"] == 2


def test_make_outcome_absorbs_fully_covered_outcome_and_reuses_label():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    game.make_outcome(gbt.H.path("U"), {"Alice": 1}, "w")
    game.make_outcome(gbt.H.path(...), {"Alice": 2}, "w")
    assert [(o.label, o["Alice"]) for o in game.outcomes] == [("w", 2)]


def test_make_outcome_label_of_partially_covered_outcome_refused():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    game.make_outcome(gbt.H.path(...).filter(lambda h: h[0] in ("U", "M")), {"Alice": 1}, "w")
    with pytest.raises(ValueError):
        game.make_outcome(gbt.H.path("D"), {"Alice": 2}, "w")
    assert len(game.outcomes) == 1


@pytest.mark.parametrize("bad_label", ["", "win"])
def test_make_outcome_bad_label_raises_and_leaves_game_unchanged(bad_label: str):
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["win", "lose"])
    game.make_outcome(gbt.H.path("win"), {"A": 1, "B": 2}, "win")
    with pytest.raises(ValueError):
        game.make_outcome(gbt.H.path("lose"), {"A": 3, "B": 4}, bad_label)
    assert [o.label for o in game.outcomes] == ["win"]


def test_make_outcome_incomplete_payoffs_raises():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    with pytest.raises(ValueError):
        game.make_outcome(gbt.H.path("U"), {"Alice": 1}, "w")


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
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    payoffs = _RepeatedEntryPayoffs([("Alice", 1), ("Alice", 2), ("Bob", 0)])
    with pytest.raises(ValueError):
        game.make_outcome(gbt.H.path("U"), payoffs, "w")


def test_make_outcome_null_accepts_selector():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    game.make_outcome(
        gbt.H.path(...).filter(lambda h: h[0] in ("U", "M")), {"Alice": 1, "Bob": -1}, "shared"
    )
    game.make_outcome_null(gbt.H.path("U"))
    assert game.get_outcome(gbt.H.path("U")) is None
    assert game.get_outcome(gbt.H.path("M")) is not None
    assert game.get_outcome(gbt.H.path("D")) is None


def test_make_outcome_null_error_location_not_a_selector():
    """A bare `Node` or `History` tuple is no longer accepted for an extensive game."""
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    with pytest.raises(TypeError):
        game.make_outcome_null(games.node_at_history(game, ("U",)))
    with pytest.raises(TypeError):
        game.make_outcome_null(("U",))


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
    game.append_move(gbt.H.path(), "Alice", ["U", "M", "D"])
    game.make_outcome(gbt.H.path(...).filter(lambda h: h[0] in ("U", "M")), {"Alice": 1}, "shared")
    outcome_count = len(game.outcomes)
    game.make_outcome_null(gbt.H.path("U"))
    assert len(game.outcomes) == outcome_count
    assert game.get_outcome(gbt.H.path("M")) is not None


def test_make_outcome_null_on_already_null_node_is_a_no_op():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(gbt.H.path(), "Alice", ["U", "D"])
    outcome_count = len(game.outcomes)
    game.make_outcome_null(gbt.H.path("U"))
    assert outcome_count == len(game.outcomes)
    assert game.get_outcome(gbt.H.path("U")) is None


def test_outcome_relabel_duplicate_rejected_and_label_unchanged():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["win", "lose"])
    game.make_outcome(gbt.H.path("win"), {"A": 1, "B": 2}, "win")
    outcome = game.make_outcome(gbt.H.path("lose"), {"A": 0, "B": 0}, "lose")
    with pytest.raises(ValueError):
        outcome.label = "win"
    assert outcome.label == "lose"
