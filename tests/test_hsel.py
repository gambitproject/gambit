import pytest

import pygambit as gbt


def test_history_view_members_on_shared_infoset():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    captured = {}

    def key(h):
        captured[h[:]] = h.members
        return frozenset(h.members)

    groups = game._get_groups(gbt.H.path(...).by(key))
    assert captured == {
        ("U",): [("U",), ("D",)],
        ("D",): [("U",), ("D",)],
    }
    assert list(groups.values()) == [[("U",), ("D",)]]


def test_history_view_members_singleton_infoset():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.path("U"), "B", ["x", "y"])
    game.append_move(gbt.H.path("D"), "B", ["x", "y"])

    captured = {}

    def key(h):
        captured[h[:]] = h.members
        return None

    game._get_groups(gbt.H.path(...).by(key))
    assert captured == {("U",): [("U",)], ("D",): [("D",)]}


def test_history_view_members_on_event():
    game = gbt.Game.new_tree(players=["A"])
    game.append_event(gbt.H.path(), {"L": 0.5, "R": 0.5})
    game.append_event(gbt.H.plays, {"p": 0.5, "q": 0.5})

    captured = {}

    def key(h):
        captured[h[:]] = h.members
        return None

    game._get_groups(gbt.H.path(...).by(key))
    assert captured == {("L",): [("L",), ("R",)], ("R",): [("L",), ("R",)]}


def test_history_view_members_raises_on_terminal():
    game = gbt.Game.new_tree(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    def key(h):
        with pytest.raises(AttributeError):
            _ = h.members
        return None

    game._get_groups(gbt.H.plays.by(key))


def test_get_histories_root():
    game = gbt.Game.new_tree(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    assert game.get_histories(gbt.H.path()) == [()]


def test_get_histories_multiple():
    game = gbt.Game.new_tree(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    assert game.get_histories(gbt.H.path(...)) == [("U",), ("D",)]
    assert game.get_histories(gbt.H.plays) == [
        ("U", "x"), ("U", "y"), ("D", "x"), ("D", "y"),
    ]


def test_get_histories_plays_from_non_root():
    """`.plays` chained after a path prefix is the terminal frontier reachable
    from that point, not from the whole game."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")

    assert set(game.get_histories(gbt.H.path("L").plays)) == {
        ("L", "R"), ("L", "L", "r"), ("L", "L", "l"),
    }


def test_get_histories_empty():
    game = gbt.Game.new_tree(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    assert game.get_histories(gbt.H.after("nonexistent")) == []


def test_get_histories_after_strategic_game_raises():
    """`H.after()`, used bare, enumerates every node -- the replacement for the
    removed `Game.nodes` -- so it inherits the same tree-only restriction."""
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.get_histories(gbt.H.after())


def test_get_histories_requires_selector():
    game = gbt.Game.new_tree(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    with pytest.raises(TypeError):
        game.get_histories(())
    with pytest.raises(TypeError):
        game.get_histories("U")
