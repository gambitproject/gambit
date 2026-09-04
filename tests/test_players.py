import pytest

import pygambit as gbt

from . import games


def test_player_count():
    game = gbt.Game.new_table([2, 2])
    assert len(game.players) == 2


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_player_label(label):
    game = gbt.Game.new_table([2, 2])
    player, other = game.players
    game.relabel_players({player: label})
    assert list(game.players) == [label, other]


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_player_label_invalid_raises_valueerror(label):
    game = gbt.Game.new_table([2, 2])
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player: label})


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_player_label_unicode_accepted(label):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = gbt.Game.new_table([2, 2])
    player, other = game.players
    game.relabel_players({player: label})
    assert list(game.players) == [label, other]


def test_set_players_requires_iterable_of_str():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        game.set_players("12")
    with pytest.raises(TypeError):
        game.set_players([1, 2])


def test_set_players_duplicate_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    labels = list(game.players)
    with pytest.raises(ValueError):
        game.set_players(labels + [labels[0]])
    assert list(game.players) == labels


def test_set_players_empty_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    labels = list(game.players)
    with pytest.raises(ValueError):
        game.set_players(labels + [""])
    assert list(game.players) == labels


def test_set_players_reserved_chance_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_tree()
    with pytest.raises(ValueError):
        game.set_players(["Chance"])
    assert len(game.players) == 0


def test_chance_player_label_cannot_be_changed():
    """The chance player's label is reserved ("Chance") and cannot be changed."""
    game = gbt.Game.new_tree()
    with pytest.raises(ValueError):
        game.relabel_players({"Chance": "Nature"})


def test_regular_player_cannot_be_relabeled_to_chance():
    game = gbt.Game.new_tree()
    game.set_players(["Alice"])
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player: "Chance"})


def test_player_relabel_visible_via_membership():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_players({pl1: "Alphonse", pl2: "Gaston"})
    assert "Alphonse" in game.players
    assert "Gaston" in game.players


def test_set_empty_player_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player: ""})


def test_set_duplicate_player_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    pl1, pl2, *_ = game.players
    with pytest.raises(ValueError):
        game.relabel_players({pl1: pl2})


def test_relabel_players_swap():
    game = gbt.Game.new_table([2, 2])
    a, b = game.players
    game.relabel_players({a: b, b: a})
    assert list(game.players) == [b, a]


def test_relabel_players_swap_tree():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.relabel_players({"Alice": "Bob", "Bob": "Alice"})
    assert list(game.players) == ["Bob", "Alice"]


def test_relabel_players_duplicate_raises_valueerror():
    game = gbt.Game.new_table([2, 2, 2])
    a, b, _ = game.players
    with pytest.raises(ValueError):
        game.relabel_players({a: b})
    with pytest.raises(ValueError):
        game.relabel_players({a: "X", b: "X"})


@pytest.mark.parametrize("bad", ["", " x"])
def test_relabel_players_bad_label_raises_and_leaves_game_unchanged(bad: str):
    game = gbt.Game.new_table([2, 2])
    a, b = game.players
    with pytest.raises(ValueError):
        game.relabel_players({a: "X", b: bad})
    assert list(game.players) == [a, b]


def test_relabel_players_unknown_label_strictness():
    game = gbt.Game.new_table([2, 2])
    a = next(iter(game.players))
    with pytest.raises(KeyError):
        game.relabel_players({"no-such-player": "X"})
    game.relabel_players({"no-such-player": "X", a: "Y"}, strict=False)
    assert next(iter(game.players)) == "Y"


def test_relabel_players_chance_key_raises_even_when_not_strict():
    """The chance player's label is reserved; a key equal to it is an error."""
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(ValueError):
        game.relabel_players({"Chance": "Nature"})
    with pytest.raises(ValueError):
        game.relabel_players({"Chance": "Nature"}, strict=False)


def test_strategic_game_set_players_add():
    game = gbt.Game.new_table([2, 2])
    labels = list(game.players)
    game.set_players(labels + ["Player 3"])
    new_player = "Player 3"
    assert len(game.players) == 3
    assert len(game.get_strategies(new_player)) == 1
    assert next(iter(game.get_strategies(new_player))) == "1"


def test_extensive_game_set_players_add():
    game = gbt.Game.new_tree()
    game.set_players(["Alice"])
    pl1 = next(iter(game.players))
    assert len(game.players) == 1
    assert len(game.get_infosets(pl1)) == 0
    assert len(game.get_strategies(pl1)) == 1


def test_strategic_game_set_strategies_add():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.set_strategies(pl1, list(game.get_strategies(pl1)) + ["new strategy"])
    assert len(game.get_strategies(pl1)) == 3
    # This second add also ensures that we are testing the case where there
    # are null outcomes in the table
    game.set_strategies(pl2, list(game.get_strategies(pl2)) + ["new strategy"])
    assert len(game.get_strategies(pl2)) == 3


def test_extensive_game_set_strategies():
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_strategies("Alice", ["new strategy"])


def _tag_contingencies(game: gbt.Game) -> None:
    """Gives every contingency's outcome a payoff vector that encodes the
    strategy labels of the contingency, so a contingency's expected payoffs
    can be recomputed from its strategies' labels after the game is mutated.
    """
    players = list(game.players)
    for n, contingency in enumerate(game.contingencies, start=1):
        payoffs = {
            player: int(f"{pl_index}{contingency[player]}")
            for pl_index, player in enumerate(players)
        }
        game.make_outcome(contingency, payoffs, f"c{n}")


def test_strategic_game_set_strategies_drop_preserves_other_payoffs():
    game = gbt.Game.new_table([4, 2, 2])
    pl1, pl2, pl3 = game.players
    _tag_contingencies(game)

    # Record expected payoffs by label (a stable identity), for the
    # strategies of pl1 that survive dropping its second strategy.
    surviving = [s for s in game.get_strategies(pl1) if s != "2"]
    expected = {
        (s1, s2, s3):
            game.get_payoffs({pl1: s1, pl2: s2, pl3: s3})
        for s1 in game.get_strategies(pl1) if s1 in surviving
        for s2 in game.get_strategies(pl2) for s3 in game.get_strategies(pl3)
    }

    game.set_strategies(pl1, surviving, drop=True)

    assert list(game.get_strategies(pl1)) == surviving
    for s1 in game.get_strategies(pl1):
        for s2 in game.get_strategies(pl2):
            for s3 in game.get_strategies(pl3):
                key = (s1, s2, s3)
                actual = game.get_payoffs(
                    {pl1: s1, pl2: s2, pl3: s3}
                )
                assert actual == expected[key]


def test_strategic_game_set_strategies_drop_first_preserves_other_payoffs():
    game = gbt.Game.new_table([3, 2])
    pl1, pl2 = game.players
    _tag_contingencies(game)

    surviving = [s for s in game.get_strategies(pl1) if s != "1"]
    expected = {
        (s1, s2): game.get_payoffs({pl1: s1, pl2: s2})
        for s1 in game.get_strategies(pl1) if s1 in surviving
        for s2 in game.get_strategies(pl2)
    }

    game.set_strategies(pl1, surviving, drop=True)

    assert list(game.get_strategies(pl1)) == surviving
    for s1 in game.get_strategies(pl1):
        for s2 in game.get_strategies(pl2):
            key = (s1, s2)
            actual = game.get_payoffs({pl1: s1, pl2: s2})
            assert actual == expected[key]


def test_strategic_game_set_strategies_empty():
    game = gbt.Game.new_table([1, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_strategies(pl1, [], drop=True)


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_set_strategies_label_valid(label):
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    game.set_strategies(pl1, list(game.get_strategies(pl1)) + [label])
    assert list(game.get_strategies(pl1))[-1] == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_set_strategies_label_invalid_raises_valueerror(label):
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(ValueError):
        game.set_strategies(pl1, list(game.get_strategies(pl1)) + [label])


def test_set_strategies_requires_iterable_of_str():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(TypeError):
        game.set_strategies(pl1, "12")
    with pytest.raises(TypeError):
        game.set_strategies(pl1, [1, 2])


def test_strategy_label_empty_raises_valueerror():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    strategy = next(iter(game.get_strategies(pl1)))
    with pytest.raises(ValueError):
        game.relabel_strategies(pl1, {strategy: ""})


def test_strategy_label_duplicate_within_player_raises_valueerror():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    s1, s2 = game.get_strategies(pl1)
    with pytest.raises(ValueError):
        game.relabel_strategies(pl1, {s2: s1})


def test_player_sequence_count():
    """Test the identity that the number of sequences is the number of actions plus one."""
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    for player in game.players:
        action_count = sum(
            len(node.infoset.actions) for node in game.get_infosets(player)
        )
        assert len(game.get_sequences(player)) == action_count + 1


def test_player_sequence_actions():
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    player = "Alice"
    sequences = set(tuple(seq.actions) for seq in game.get_sequences(player))
    reference = (
        set(
            (action, )
            for node in game.get_infosets(player)
            for action in node.infoset.actions
        ) |
        {tuple()}
    )
    assert sequences == reference


def test_player_sequence_tree():
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    player = "Alice"
    for seq in game.get_sequences(player):
        if not seq.parent:
            continue
        assert seq in seq.parent.children


@pytest.mark.parametrize(
    "game,exp_min_payoffs,exp_max_payoffs",
    [
        # NFGs
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [-1, 0, -1],
            [2, 4, 2]
        ),
        (games.read_from_file("mixed_strategy.nfg"), [0, 0], [2, 3]),
        # EFGs only terminal outcomes
        (games.create_kuhn_poker_efg(), [-2, -2], [2, 2]),
        (games.create_stripped_down_poker_efg(), [-2, -2], [2, 2]),
        # with non-terminal outcomes
        (games.create_kuhn_poker_efg(nonterm_outcomes=True), [-2, -2], [2, 2]),
        (games.create_stripped_down_poker_efg(nonterm_outcomes=True), [-2, -2], [2, 2]),
        # AGGs/BAGGs
        (games.read_from_file("2x2.agg"), [-10, -10], [95, 95]),
        (games.read_from_file("2x2.bagg"), [-10, -10], [95, 95]),
        (
            games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg"),
            [0, 0, 0, 0, 0, 0],
            [99, 90, 99, 90, 99, 90],
        ),
    ],
)
def test_player_get_min_max_payoff(game: gbt.Game, exp_min_payoffs: list, exp_max_payoffs: list):
    for player, exp_min, exp_max in zip(
        game.players, exp_min_payoffs, exp_max_payoffs, strict=True
    ):
        assert game.get_min_payoff(player) == exp_min
        assert game.get_max_payoff(player) == exp_max


def test_player_get_min_payoff_nonterminal_outcomes():
    """Test whether `get_min_payoff` correctly reports minimum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.get_min_payoff("Alice") == -2
    assert game.get_min_payoff("Bob") == -2
    game.make_outcome(gbt.H.path(), {"Alice": -1, "Bob": -1}, "outcome")
    assert game.get_min_payoff("Alice") == -3
    assert game.get_min_payoff("Bob") == -3


def test_player_get_min_payoff_null_outcome():
    """Test whether `get_min_payoff` correctly reports minimum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[1, 1], [1, 1]], [[2, 2], [2, 2]])
    pl1, pl2 = game.players
    assert game.get_min_payoff(pl1) == 1
    assert game.get_min_payoff(pl2) == 2
    game.set_strategies(pl1, list(game.get_strategies(pl1)) + ["new strategy"])
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert game.get_min_payoff(player) == 0


def test_player_get_max_payoff_nonterminal_outcomes():
    """Test whether `get_max_payoff` correctly reports maximum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.get_max_payoff("Alice") == 2
    assert game.get_max_payoff("Bob") == 2
    game.make_outcome(gbt.H.path(), {"Alice": -1, "Bob": -1}, "outcome")
    assert game.get_max_payoff("Alice") == 1
    assert game.get_max_payoff("Bob") == 1


def test_player_get_max_payoff_null_outcome():
    """Test whether `get_max_payoff` correctly reports maximum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[-1, -1], [-1, -1]], [[-2, -2], [-2, -2]])
    pl1, pl2 = game.players
    assert game.get_max_payoff(pl1) == -1
    assert game.get_max_payoff(pl2) == -2
    game.set_strategies(pl1, list(game.get_strategies(pl1)) + ["new strategy"])
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert game.get_max_payoff(player) == 0


def test_set_strategies_duplicate_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    pl = next(iter(game.players))
    labels = list(game.get_strategies(pl))
    with pytest.raises(ValueError):
        game.set_strategies(pl, labels + [labels[0]])
    assert list(game.get_strategies(pl)) == labels


def test_set_strategies_empty_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    pl = next(iter(game.players))
    labels = list(game.get_strategies(pl))
    with pytest.raises(ValueError):
        game.set_strategies(pl, labels + [""])
    assert list(game.get_strategies(pl)) == labels


def test_set_players_empty_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_players([], drop=True)


def test_set_players_reorder_transposes_table():
    """Reordering the players permutes the axes of the payoff table."""
    game = gbt.Game.from_arrays([[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]])
    a, b = game.players
    game.set_players([b, a])
    assert list(game.players) == [b, a]
    assert game.to_arrays()[0].tolist() == [[7, 10], [8, 11], [9, 12]]
    assert game.to_arrays()[1].tolist() == [[1, 4], [2, 5], [3, 6]]


def test_set_players_add_then_drop_round_trips():
    game = gbt.Game.from_arrays([[1, 2], [3, 4]], [[5, 6], [7, 8]])
    labels = list(game.players)
    game.set_players(labels + ["X"])
    assert all(outcome["X"] == 0 for outcome in game.outcomes)
    game.set_players(labels, drop=True)
    assert list(game.players) == labels
    assert game.to_arrays()[0].tolist() == [[1, 2], [3, 4]]


def test_set_players_drop_requires_deletable_player():
    game = gbt.Game.new_table([2, 2])
    a, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_players([a], drop=True)
    tree = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.UndefinedOperationError):
        tree.set_players(["Bob"], drop=True)


def test_set_players_unconfirmed_drop_and_disabled_add_raise():
    game = gbt.Game.new_table([2, 2])
    labels = list(game.players)
    with pytest.raises(ValueError):
        game.set_players(labels[:1])
    with pytest.raises(ValueError):
        game.set_players(labels + ["X"], add=False)
    assert list(game.players) == labels
