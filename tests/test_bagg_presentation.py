"""Tests of how a BAGG presents itself: real players, types as information sets, and the
strategy-shaped members that refuse."""

import pytest

from . import games


@pytest.mark.parametrize(
    "game_path,players,infosets",
    [
        ("Bayesian-Coffee-3-2-2-3.bagg", ["1", "2", "3"], ["1", "2"]),
        ("2x2.bagg", ["1", "2"], ["1"]),
        ("bayesian_2p2t_shared_node.bagg", ["1", "2"], ["1", "2"]),
    ],
)
def test_bagg_players_and_infosets(game_path, players, infosets):
    game = games.read_from_file(game_path)
    assert list(game.players) == players
    assert len(game.players) == len(players)
    for player in players:
        assert player in game.players
        assert game.get_infosets(player) == infosets
    assert "4" not in game.players


def test_bagg_infosets_differ_by_player_in_type_count():
    game = games.read_from_file("2x2_fraction_types.bagg")
    assert list(game.players) == ["1", "2"]
    assert game.get_infosets("1") == ["1", "2"]
    assert game.get_infosets("2") == ["1"]


def test_bagg_get_infosets_returns_fresh_list_of_str():
    game = games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg")
    infosets = game.get_infosets("1")
    infosets.append("x")
    assert game.get_infosets("1") == ["1", "2"]


@pytest.mark.parametrize(
    "player,exc",
    [("4", KeyError), ("a", KeyError), ("", ValueError), ("  ", ValueError), (1, TypeError)],
)
def test_bagg_get_infosets_bad_player(player, exc):
    game = games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg")
    with pytest.raises(exc):
        game.get_infosets(player)


def test_bagg_repr_html_does_not_raise():
    game = games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg")
    assert isinstance(game._repr_html_(), str)


@pytest.mark.parametrize("game_path", ["2x2.bagg", "Bayesian-Coffee-3-2-2-3.bagg"])
def test_bagg_game_level_members_unchanged(game_path):
    game = games.read_from_file(game_path)
    assert game.is_perfect_recall
    assert isinstance(game.title, str)
    assert isinstance(game.description, str)
    assert game.min_payoff <= game.max_payoff
    assert game.get_outcomes() == []


@pytest.mark.parametrize(
    "game_path", ["2x2.bagg", "2x2_fraction_types.bagg", "Bayesian-Coffee-3-2-2-3.bagg"]
)
def test_bagg_infosets_absent_on_other_game_types(game_path):
    game = games.read_from_file(game_path)
    assert not hasattr(game, "get_actions")
    assert not hasattr(game, "get_histories")


def test_other_games_do_not_get_bagg_infosets_semantics():
    game = games.read_from_file("2x2.agg")
    assert not hasattr(game, "get_infosets")
    assert list(game.players) == ["1", "2"]


def test_bagg_players_is_not_indexable_by_label():
    """Players collections identify players by label: there is nothing to look up."""
    game = games.read_from_file("2x2.bagg")
    with pytest.raises(TypeError):
        game.players["1"]
