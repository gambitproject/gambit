import pandas as pd
import pytest

import pygambit as gbt


@pytest.fixture(scope="module")
def all_games():
    return gbt.catalog.games()


def test_catalog_load_efg():
    """Test loading an extensive form game"""
    g = gbt.catalog.load("2smp")
    assert isinstance(g, gbt.Game)
    assert g.title == "Two-stage matching pennies game"


def test_catalog_load_nfg():
    """Test loading a normal form game"""
    g = gbt.catalog.load("pd")
    assert isinstance(g, gbt.Game)
    assert g.title == "Two person Prisoner's Dilemma game"


def test_catalog_load_invalid_slug():
    """Test loading an invalid game slug"""
    with pytest.raises(FileNotFoundError):
        gbt.catalog.load("invalid_slug")


def test_catalog_load_subdir_slug():
    """Test loading a game from catalog/somedir"""
    g = gbt.catalog.load("myerson/fig_4_2")
    assert isinstance(g, gbt.Game)


def test_catalog_load_family_game():
    """Test loading a game generated from code with a game family func."""
    g = gbt.catalog.load("one_shot_trust")
    assert isinstance(g, gbt.Game)


def test_catalog_games(all_games):
    """Test games() function returns df of game slugs and titles"""
    slugs = list(all_games.Game)
    assert isinstance(all_games, pd.DataFrame)
    assert len(all_games) > 0
    assert "2smp" in slugs
    assert "Two-stage matching pennies game" in list(all_games.Title)
    # Check slug of game in subdir
    assert "myerson/fig_4_2" in slugs
    assert "myerson_fig_4_2" not in slugs
    # Check family game present
    assert "one_shot_trust" in slugs


def test_catalog_games_filter_n_actions(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'actions'"""
    filtered_games = gbt.catalog.games(n_actions=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.actions) == 2 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_contingencies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'contingencies'"""
    filtered_games = gbt.catalog.games(n_contingencies=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.contingencies) == 2 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_info_sets(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'info_sets'"""
    filtered_games = gbt.catalog.games(n_info_sets=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.info_sets) == 2 for g in list(filtered_games.Game))


def test_catalog_games_filter_is_constant_sum(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_constant_sum'"""
    filtered_games = gbt.catalog.games(is_constant_sum=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(g.is_constant_sum for g in list(filtered_games.Game))


def test_catalog_games_filter_is_perfect_recall(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_perfect_recall'"""
    filtered_games = gbt.catalog.games(is_perfect_recall=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(g.is_perfect_recall for g in list(filtered_games.Game))


def test_catalog_games_filter_is_tree(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_tree'"""
    filtered_games = gbt.catalog.games(is_tree=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(g.is_tree for g in list(filtered_games.Game))


def test_catalog_games_filter_min_payoff_and_max_payoff(all_games):
    """Test games() function can filter on min and max payoff values"""
    filtered_games = gbt.catalog.games(min_payoff=0, max_payoff=10)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(g.min_payoff >= 0 and g.max_payoff <= 10 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_nodes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'nodes'"""
    filtered_games = gbt.catalog.games(n_nodes=5)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.nodes) == 5 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_outcomes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'outcomes'"""
    filtered_games = gbt.catalog.games(n_outcomes=3)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.outcomes) == 3 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_players(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'players'"""
    filtered_games = gbt.catalog.games(n_players=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.players) == 2 for g in list(filtered_games.Game))


def test_catalog_games_filter_n_strategies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'strategies'"""
    filtered_games = gbt.catalog.games(n_strategies=4)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    assert all(len(g.strategies) == 4 for g in list(filtered_games.Game))
