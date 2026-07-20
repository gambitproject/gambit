import io

import pytest

import pygambit as gbt

from . import games


def test_strategic_game_actions():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.actions


def test_strategic_game_player_actions():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        _ = player.actions


def test_strategic_game_infosets():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.infosets


def test_strategic_game_player_infosets():
    game = gbt.Game.new_table([2, 2])
    player, _ = game.players
    with pytest.raises(gbt.UndefinedOperationError):
        _ = player.infosets


def test_strategic_game_root():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.root


def test_strategic_game_nodes():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.nodes


def test_strategic_game_sort_infosets():
    game = gbt.Game.new_table([2, 2])
    with pytest.warns(FutureWarning), pytest.raises(gbt.UndefinedOperationError):
        _ = game.sort_infosets()


def test_game_behav_profile_error():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_behavior_profile()


def test_game_is_const_sum():
    game = games.read_from_file("const_sum_game.nfg")
    assert game.is_const_sum


def test_game_is_not_const_sum():
    game = games.read_from_file("non_const_sum_game.nfg")
    assert not game.is_const_sum


def test_game_get_min_payoff():
    game = games.read_from_file("mixed_strategy.nfg")
    assert game.min_payoff == 0


def test_game_get_max_payoff():
    game = games.read_from_file("mixed_strategy.nfg")
    assert game.max_payoff == 3


@pytest.mark.xfail(
    reason="Generated reduced-strategy labels are not injective",
    raises=AssertionError, strict=True,
)
def test_generated_strategy_labels_are_unique():
    """#981 established that a strategy label must be nonempty, valid, and
    unique among a player's strategies.  Labels generated for reduced
    strategies bypass the check: two 11-action information sets yield
    strategies (1, 11) and (11, 1), both rendering as "111"."""
    g = games.read_from_file("label-collision.efg")
    labels = [s.label for s in g.players["Player 1"].strategies]
    dups = sorted({lab for lab in labels if labels.count(lab) > 1})
    assert len(set(labels)) == len(labels), f"duplicate generated labels: {dups}"


@pytest.mark.xfail(
    reason="Generated reduced-strategy labels are not injective; "
           "duplicate labels break NFG re-read",
    raises=ValueError, strict=True,
)
def test_game_with_duplicate_generated_labels_roundtrips_through_nfg():
    """WriteNfgFile serialises the duplicate generated labels;
    ReadNfgFile then rejects its own output.  FAILS on 16.7.0.

    The game has two 11-action information sets for one player (imperfect
    recall); reduced strategies (1, 11) and (11, 1) both render as "111"."""
    g = games.read_from_file("label-collision.efg")
    gbt.read_nfg(io.StringIO(g.to_nfg()))
