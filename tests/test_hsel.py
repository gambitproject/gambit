import pytest

import pygambit as gbt


def test_history_view_members_on_shared_infoset():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    captured = {}

    def key(h):
        captured[h[:].actions] = [m.actions for m in h.members]
        return frozenset(m.actions for m in h.members)

    groups = game._get_groups(gbt.H.path(...).by(key))
    assert captured == {
        ("U",): [("U",), ("D",)],
        ("D",): [("U",), ("D",)],
    }
    assert [[m.actions for m in g] for g in groups.values()] == [[("U",), ("D",)]]


def test_history_view_members_singleton_infoset():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.path("U"), "B", ["x", "y"])
    game.append_move(gbt.H.path("D"), "B", ["x", "y"])

    captured = {}

    def key(h):
        captured[h[:].actions] = [m.actions for m in h.members]
        return None

    game._get_groups(gbt.H.path(...).by(key))
    assert captured == {("U",): [("U",)], ("D",): [("D",)]}


def test_history_view_members_on_event():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_event(gbt.H.path(), {"L": 0.5, "R": 0.5})
    game.append_event(gbt.H.plays, {"p": 0.5, "q": 0.5})

    captured = {}

    def key(h):
        captured[h[:].actions] = [m.actions for m in h.members]
        return None

    game._get_groups(gbt.H.path(...).by(key))
    assert captured == {("L",): [("L",), ("R",)], ("R",): [("L",), ("R",)]}


def test_history_view_members_raises_on_terminal():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    def key(h):
        with pytest.raises(AttributeError):
            _ = h.members
        return None

    game._get_groups(gbt.H.plays.by(key))


def test_get_histories_root():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    assert [h.actions for h in game.get_histories(gbt.H.path())] == [()]


def test_get_histories_multiple():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    assert [h.actions for h in game.get_histories(gbt.H.path(...))] == [("U",), ("D",)]
    assert [h.actions for h in game.get_histories(gbt.H.plays)] == [
        ("U", "x"), ("U", "y"), ("D", "x"), ("D", "y"),
    ]


def test_get_histories_plays_from_non_root():
    """`.plays` chained after a path prefix is the terminal frontier reachable
    from that point, not from the whole game."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")

    assert {h.actions for h in game.get_histories(gbt.H.path("L").plays)} == {
        ("L", "R"), ("L", "L", "r"), ("L", "L", "l"),
    }


def test_get_histories_empty():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    assert game.get_histories(gbt.H.after("nonexistent")) == []


def test_get_histories_after_strategic_game_raises():
    """`H.after()`, used bare, enumerates every node -- the replacement for the
    removed `Game.nodes` -- so `get_histories` inherits the same tree-only
    restriction: it does not exist on a strategic game."""
    game = gbt.StrategicGame([2, 2])
    with pytest.raises(AttributeError):
        game.get_histories(gbt.H.after())


def test_get_histories_requires_selector():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    with pytest.raises(TypeError):
        game.get_histories(())
    with pytest.raises(TypeError):
        game.get_histories("U")


def test_last_action_raises_on_invalid_player():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    def key(h):
        with pytest.raises(KeyError):
            h.last_action("NoSuchPlayer")
        return None

    game._get_groups(gbt.H.plays.by(key))


def test_last_action_raises_on_non_str_player():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    def key(h):
        with pytest.raises(TypeError):
            h.last_action(1)
        return None

    game._get_groups(gbt.H.plays.by(key))


def test_last_action_raises_on_empty_player():
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])

    def key(h):
        with pytest.raises(ValueError):
            h.last_action("  ")
        return None

    game._get_groups(gbt.H.plays.by(key))


def test_by_last_action_raises_on_invalid_player():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    grouped = gbt.H.path().by(lambda h: None).by_last_action("NoSuchPlayer").plays
    with pytest.raises(KeyError):
        game._get_groups(grouped)


def test_by_last_action_refines_key_by_last_action():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.plays, "B", ["x", "y"])

    grouped = gbt.H.path().by(lambda h: None).by_last_action("A").plays
    groups = game._get_groups(grouped)
    keyed_by_action = {k[1]: [h.actions for h in v] for k, v in groups.items()}
    assert {label: histories for (_, label), histories in keyed_by_action.items()} == {
        "U": [("U", "x"), ("U", "y")],
        "D": [("D", "x"), ("D", "y")],
    }


def test_by_last_action_distinguishes_infosets_sharing_a_label():
    # A acts at two distinct infosets (one per branch of the chance move), both
    # offering an action labelled "U". Grouping only by the label would wrongly
    # merge these into one group, breaking perfect recall by construction --
    # `.by_last_action` must key on the (infoset, label) pair instead.
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_event(gbt.H.path(), {"L": gbt.Rational(1, 2), "R": gbt.Rational(1, 2)})
    game.append_move(gbt.H.path("L"), "A", ["U", "D"])
    game.append_move(gbt.H.path("R"), "A", ["U", "D"])

    grouped = gbt.H.path(...).by(lambda h: None).by_last_action("A").plays
    groups = game._get_groups(grouped)
    assert len(groups) == 4
    assert {frozenset(h.actions for h in v) for v in groups.values()} == {
        frozenset({("L", "U")}),
        frozenset({("L", "D")}),
        frozenset({("R", "U")}),
        frozenset({("R", "D")}),
    }


def test_selector_deduplicates_plays():
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["go", "stop"])
    game.append_move(gbt.H.path("go"), "B", ["go", "stay"])
    plays = game.get_histories(gbt.H.after("go").plays)
    assert {h.actions for h in plays} == {("go", "go"), ("go", "stay")}
    assert len(plays) == 2, f"expected 2 unique plays, got {len(plays)}: {plays}"


def test_grouped_mutation_raises_on_empty_group():
    # A post-op (`.after`) can filter one group's members down to none while
    # leaving others nonempty. That must raise, the same as an ungrouped
    # selector matching no nodes, rather than being silently skipped.
    game = gbt.ExtensiveGame(players=["A", "B"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.path("U"), "B", ["x", "y"])
    game.append_move(gbt.H.path("D"), "B", ["z", "w"])

    grouped = gbt.H.plays.by(lambda h: h[:].actions[0]).after("x", "z")
    with pytest.raises(ValueError):
        game.append_move(grouped, "A", ["p", "q"])


def test_grouped_mutation_validates_all_groups_before_mutating_any():
    # If a later group fails validation, an earlier group must not have been
    # mutated -- grouped mutation calls validate every group before applying
    # any of them.
    game = gbt.ExtensiveGame(players=["A"])
    game.append_move(gbt.H.path(), "A", ["U", "D"])
    game.append_move(gbt.H.path("U"), "A", ["x", "y"])  # U-group node no longer terminal

    grouped = gbt.H.path(...).by(lambda h: h[:].actions[0])
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_move(grouped, "A", ["m", "n"])
    assert game.get_actions(gbt.H.path("D")) == []
