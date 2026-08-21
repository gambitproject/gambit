import pytest

import pygambit as gbt

from . import games


def test_player_count():
    game = gbt.Game.new_table([2, 2])
    assert len(game.players) == 2


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_player_label(label):
    game = gbt.Game.new_table([2, 2])
    player = next(iter(game.players))
    game.relabel_players({player.label: label})
    assert player.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_player_label_invalid_raises_valueerror(label):
    game = gbt.Game.new_table([2, 2])
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player.label: label})


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_player_label_unicode_accepted(label):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = gbt.Game.new_table([2, 2])
    player = next(iter(game.players))
    game.relabel_players({player.label: label})
    assert player.label == label


def test_set_players_requires_iterable_of_str():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        game.set_players("12")
    with pytest.raises(TypeError):
        game.set_players([1, 2])


def test_set_players_duplicate_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    labels = [player.label for player in game.players]
    with pytest.raises(ValueError):
        game.set_players(labels + [labels[0]])
    assert [player.label for player in game.players] == labels


def test_set_players_empty_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    labels = [player.label for player in game.players]
    with pytest.raises(ValueError):
        game.set_players(labels + [""])
    assert [player.label for player in game.players] == labels


def test_set_players_reserved_chance_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_tree()
    with pytest.raises(ValueError):
        game.set_players(["Chance"])
    assert len(game.players) == 0


def test_chance_player_has_label():
    """The chance player is labeled "Chance" by default."""
    game = gbt.Game.new_tree()
    assert game.players.chance.label == "Chance"


def test_chance_player_label_cannot_be_changed():
    """The chance player's label is reserved ("Chance") and cannot be changed."""
    game = gbt.Game.new_tree()
    with pytest.raises(ValueError):
        game.relabel_players({game.players.chance.label: "Nature"})


def test_regular_player_cannot_be_relabeled_to_chance():
    game = gbt.Game.new_tree()
    game.set_players(["Alice"])
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player.label: "Chance"})


def test_player_index_by_string():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_players({pl1.label: "Alphonse", pl2.label: "Gaston"})
    assert game.players["Alphonse"].label == "Alphonse"
    assert game.players["Gaston"].label == "Gaston"


def test_player_index_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.players[1.3]


def test_player_label_invalid():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.players["Not a player"]


def test_set_empty_player_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    player = next(iter(game.players))
    with pytest.raises(ValueError):
        game.relabel_players({player.label: ""})


def test_set_duplicate_player_raises_valueerror():
    game = games.create_stripped_down_poker_efg()
    pl1, pl2, *_ = game.players
    with pytest.raises(ValueError):
        game.relabel_players({pl1.label: pl2.label})


def test_relabel_players_swap():
    game = gbt.Game.new_table([2, 2])
    a, b = (player.label for player in game.players)
    game.relabel_players({a: b, b: a})
    assert [player.label for player in game.players] == [b, a]


def test_relabel_players_swap_tree():
    game = gbt.Game.new_tree(["Alice", "Bob"])
    game.relabel_players({"Alice": "Bob", "Bob": "Alice"})
    assert [player.label for player in game.players] == ["Bob", "Alice"]


def test_relabel_players_duplicate_raises_valueerror():
    game = gbt.Game.new_table([2, 2, 2])
    a, b, _ = (player.label for player in game.players)
    with pytest.raises(ValueError):
        game.relabel_players({a: b})
    with pytest.raises(ValueError):
        game.relabel_players({a: "X", b: "X"})


@pytest.mark.parametrize("bad", ["", " x"])
def test_relabel_players_bad_label_raises_and_leaves_game_unchanged(bad: str):
    game = gbt.Game.new_table([2, 2])
    a, b = (player.label for player in game.players)
    with pytest.raises(ValueError):
        game.relabel_players({a: "X", b: bad})
    assert [player.label for player in game.players] == [a, b]


def test_relabel_players_unknown_label_strictness():
    game = gbt.Game.new_table([2, 2])
    a = next(iter(game.players)).label
    with pytest.raises(KeyError):
        game.relabel_players({"no-such-player": "X"})
    game.relabel_players({"no-such-player": "X", a: "Y"}, strict=False)
    assert next(iter(game.players)).label == "Y"


def test_relabel_players_chance_key_raises_even_when_not_strict():
    """The chance player's label is reserved; a key equal to it is an error."""
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(ValueError):
        game.relabel_players({"Chance": "Nature"})
    with pytest.raises(ValueError):
        game.relabel_players({"Chance": "Nature"}, strict=False)


def test_strategic_game_set_players_add():
    game = gbt.Game.new_table([2, 2])
    labels = [player.label for player in game.players]
    game.set_players(labels + ["Player 3"])
    new_player = game.players["Player 3"]
    assert len(game.players) == 3
    assert len(new_player.strategies) == 1
    assert next(iter(new_player.strategies)).label == "1"


def test_extensive_game_set_players_add():
    game = gbt.Game.new_tree()
    game.set_players(["Alice"])
    pl1 = next(iter(game.players))
    assert len(game.players) == 1
    assert len(pl1.infosets) == 0
    assert len(pl1.strategies) == 1


def test_strategic_game_set_strategies_add():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.set_strategies(pl1, [s.label for s in pl1.strategies] + ["new strategy"])
    assert len(pl1.strategies) == 3
    # This second add also ensures that we are testing the case where there
    # are null outcomes in the table
    game.set_strategies(pl2, [s.label for s in pl2.strategies] + ["new strategy"])
    assert len(pl2.strategies) == 3


def test_extensive_game_set_strategies():
    game = gbt.Game.new_tree(["Alice"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_strategies(game.players["Alice"], ["new strategy"])


def _tag_contingencies(game: gbt.Game) -> None:
    """Gives every contingency's outcome a payoff vector that encodes the
    strategy labels of the contingency, so a contingency's expected payoffs
    can be recomputed from its strategies' labels after the game is mutated.
    """
    players = list(game.players)
    for contingency in game.contingencies:
        outcome = game[contingency]
        strategies = [list(p.strategies)[i] for p, i in zip(players, contingency, strict=True)]
        for pl_index, (player, strategy) in enumerate(zip(players, strategies, strict=True)):
            outcome[player] = int(f"{pl_index}{strategy.label}")


def test_strategic_game_set_strategies_drop_preserves_other_payoffs():
    game = gbt.Game.new_table([4, 2, 2])
    pl1, pl2, pl3 = game.players
    _tag_contingencies(game)

    # Record expected payoffs by label (a stable identity), for the
    # strategies of pl1 that survive dropping its second strategy.
    surviving = [s.label for s in pl1.strategies if s.label != "2"]
    expected = {
        (s1.label, s2.label, s3.label):
            tuple(game[s1, s2, s3][p] for p in (pl1, pl2, pl3))
        for s1 in pl1.strategies if s1.label in surviving
        for s2 in pl2.strategies for s3 in pl3.strategies
    }

    game.set_strategies(pl1, surviving, drop=True)

    assert [s.label for s in pl1.strategies] == surviving
    for s1 in pl1.strategies:
        for s2 in pl2.strategies:
            for s3 in pl3.strategies:
                key = (s1.label, s2.label, s3.label)
                actual = tuple(game[s1, s2, s3][p] for p in (pl1, pl2, pl3))
                assert actual == expected[key]


def test_strategic_game_set_strategies_drop_first_preserves_other_payoffs():
    game = gbt.Game.new_table([3, 2])
    pl1, pl2 = game.players
    _tag_contingencies(game)

    surviving = [s.label for s in pl1.strategies if s.label != "1"]
    expected = {
        (s1.label, s2.label): tuple(game[s1, s2][p] for p in (pl1, pl2))
        for s1 in pl1.strategies if s1.label in surviving
        for s2 in pl2.strategies
    }

    game.set_strategies(pl1, surviving, drop=True)

    assert [s.label for s in pl1.strategies] == surviving
    for s1 in pl1.strategies:
        for s2 in pl2.strategies:
            key = (s1.label, s2.label)
            assert tuple(game[s1, s2][p] for p in (pl1, pl2)) == expected[key]


def test_strategic_game_set_strategies_empty():
    game = gbt.Game.new_table([1, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_strategies(pl1, [], drop=True)


def test_player_strategy_by_label():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    game.relabel_strategies(pl1, {next(iter(pl1.strategies)).label: "Cooperate"})
    assert pl1.strategies["Cooperate"].label == "Cooperate"


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_set_strategies_label_valid(label):
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    game.set_strategies(pl1, [s.label for s in pl1.strategies] + [label])
    assert [s.label for s in pl1.strategies][-1] == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_set_strategies_label_invalid_raises_valueerror(label):
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(ValueError):
        game.set_strategies(pl1, [s.label for s in pl1.strategies] + [label])


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
    strategy = next(iter(pl1.strategies))
    with pytest.raises(ValueError):
        game.relabel_strategies(pl1, {strategy.label: ""})


def test_strategy_label_duplicate_within_player_raises_valueerror():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    s1, s2 = pl1.strategies
    with pytest.raises(ValueError):
        game.relabel_strategies(pl1, {s2.label: s1.label})


def test_player_strategy_bad_label():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(KeyError):
        _ = pl1.strategies["Cooperate"]


def test_player_strategy_bad_type():
    game = gbt.Game.new_table([2, 2])
    pl1 = next(iter(game.players))
    with pytest.raises(TypeError):
        _ = pl1.strategies[1.3]


def test_player_sequence_count():
    """Test the identity that the number of sequences is the number of actions plus one."""
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    for player in game.players:
        action_count = sum(len(infoset.actions) for infoset in player.infosets)
        assert len(player.sequences) == action_count + 1


def test_player_sequence_actions():
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    player = game.players["Alice"]
    sequences = set(tuple(seq.actions) for seq in player.sequences)
    reference = (
        set((action, ) for infoset in player.infosets for action in infoset.actions) |
        {tuple()}
    )
    assert sequences == reference


def test_player_sequence_tree():
    game = gbt.catalog.load("books/myerson1991/fig2_1")
    player = game.players["Alice"]
    for seq in player.sequences:
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
        assert player.min_payoff == exp_min
        assert player.max_payoff == exp_max


def test_player_get_min_payoff_nonterminal_outcomes():
    """Test whether `min_payoff` correctly reports minimum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.players["Alice"].min_payoff == -2
    assert game.players["Bob"].min_payoff == -2
    game.set_outcome(game.root, game.add_outcome("outcome", [-1, -1]))
    assert game.players["Alice"].min_payoff == -3
    assert game.players["Bob"].min_payoff == -3


def test_player_get_min_payoff_null_outcome():
    """Test whether `min_payoff` correctly reports minimum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[1, 1], [1, 1]], [[2, 2], [2, 2]])
    pl1, pl2 = game.players
    assert pl1.min_payoff == 1
    assert pl2.min_payoff == 2
    game.set_strategies(pl1, [s.label for s in pl1.strategies] + ["new strategy"])
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert player.min_payoff == 0


def test_player_get_max_payoff_nonterminal_outcomes():
    """Test whether `max_payoff` correctly reports maximum payoffs
    when there are non-terminal outcomes.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.players["Alice"].max_payoff == 2
    assert game.players["Bob"].max_payoff == 2
    game.set_outcome(game.root, game.add_outcome("outcome", [-1, -1]))
    assert game.players["Alice"].max_payoff == 1
    assert game.players["Bob"].max_payoff == 1


def test_player_get_max_payoff_null_outcome():
    """Test whether `max_payoff` correctly reports maximum payoffs
    in a strategic game with a null outcome."""
    game = gbt.Game.from_arrays([[-1, -1], [-1, -1]], [[-2, -2], [-2, -2]])
    pl1, pl2 = game.players
    assert pl1.max_payoff == -1
    assert pl2.max_payoff == -2
    game.set_strategies(pl1, [s.label for s in pl1.strategies] + ["new strategy"])
    # Currently the outcomes associated with the new entries in the table
    # are null outcomes.  So now minimum payoff should be zero from those.
    for player in game.players:
        assert player.max_payoff == 0


def test_set_strategies_duplicate_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    pl = next(iter(game.players))
    labels = [s.label for s in pl.strategies]
    with pytest.raises(ValueError):
        game.set_strategies(pl, labels + [labels[0]])
    assert [s.label for s in pl.strategies] == labels


def test_set_strategies_empty_label_raises_and_leaves_game_unchanged():
    game = gbt.Game.new_table([2, 2])
    pl = next(iter(game.players))
    labels = [s.label for s in pl.strategies]
    with pytest.raises(ValueError):
        game.set_strategies(pl, labels + [""])
    assert [s.label for s in pl.strategies] == labels


def test_set_players_empty_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_players([], drop=True)


def test_set_players_reorder_transposes_table():
    """Reordering the players permutes the axes of the payoff table."""
    game = gbt.Game.from_arrays([[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]])
    a, b = (player.label for player in game.players)
    game.set_players([b, a])
    assert [player.label for player in game.players] == [b, a]
    assert game.to_arrays()[0].tolist() == [[7, 10], [8, 11], [9, 12]]
    assert game.to_arrays()[1].tolist() == [[1, 4], [2, 5], [3, 6]]


def test_set_players_add_then_drop_round_trips():
    game = gbt.Game.from_arrays([[1, 2], [3, 4]], [[5, 6], [7, 8]])
    labels = [player.label for player in game.players]
    game.set_players(labels + ["X"])
    assert all(outcome["X"] == 0 for outcome in game.outcomes)
    game.set_players(labels, drop=True)
    assert [player.label for player in game.players] == labels
    assert game.to_arrays()[0].tolist() == [[1, 2], [3, 4]]


def test_set_players_drop_requires_deletable_player():
    game = gbt.Game.new_table([2, 2])
    a, _ = (player.label for player in game.players)
    with pytest.raises(gbt.UndefinedOperationError):
        game.set_players([a], drop=True)
    tree = games.create_stripped_down_poker_efg()
    with pytest.raises(gbt.UndefinedOperationError):
        tree.set_players(["Bob"], drop=True)


def test_set_players_unconfirmed_drop_and_disabled_add_raise():
    game = gbt.Game.new_table([2, 2])
    labels = [player.label for player in game.players]
    with pytest.raises(ValueError):
        game.set_players(labels[:1])
    with pytest.raises(ValueError):
        game.set_players(labels + ["X"], add=False)
    assert [player.label for player in game.players] == labels
