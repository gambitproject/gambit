import pandas as pd
import pytest

import pygambit as gbt


@pytest.fixture(scope="module")
def all_games():
    return gbt.catalog.games()


def test_catalog_load_efg():
    """Test loading an extensive form game"""
    g = gbt.catalog.load("selten1975/fig1")
    assert isinstance(g, gbt.Game)
    assert g.title == "Selten's horse (Selten IJGT 1975, Figure 1)"


# TODO: Reintroduce this test once we have a .nfg game in the catalog
# def test_catalog_load_nfg():
#     """Test loading a normal form game"""
#     g = gbt.catalog.load("pd")
#     assert isinstance(g, gbt.Game)
#     assert g.title == "Two person Prisoner's Dilemma game"


def test_catalog_load_invalid_slug():
    """Test loading an invalid game slug"""
    with pytest.raises(FileNotFoundError):
        gbt.catalog.load("invalid_slug")


def test_catalog_load_family_game():
    """Test loading a game generated from code with a game family func."""
    g = gbt.catalog.load("one_shot_trust")
    assert isinstance(g, gbt.Game)


def test_catalog_games(all_games):
    """Test games() function returns df of game slugs and titles"""
    slugs = list(all_games.Game)
    assert isinstance(all_games, pd.DataFrame)
    assert len(all_games) > 0
    # Check slug of game in subdir
    assert "myerson1991/fig4_2" in slugs
    # Check family game present
    assert "one_shot_trust" in slugs
    # Check a known game title is present
    assert "Myerson (1991) Figure 4.2" in list(all_games.Title)


def test_catalog_games_filter_n_actions(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'actions'"""
    filtered_games = gbt.catalog.games(n_actions=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.actions) == 2


def test_catalog_games_filter_n_contingencies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'contingencies'"""
    filtered_games = gbt.catalog.games(n_contingencies=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.contingencies) == 2


def test_catalog_games_filter_n_infosets(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'infosets'"""
    filtered_games = gbt.catalog.games(n_infosets=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.infosets) == 2


def test_catalog_games_filter_is_const_sum(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_const_sum'"""
    filtered_games = gbt.catalog.games(is_const_sum=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.is_const_sum


def test_catalog_games_filter_is_not_perfect_recall(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_perfect_recall'"""
    filtered_games = gbt.catalog.games(is_perfect_recall=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not g.is_perfect_recall


def test_catalog_games_filter_is_not_tree(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_tree'"""
    filtered_games = gbt.catalog.games(is_tree=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not g.is_tree


def test_catalog_games_filter_min_payoff_and_max_payoff(all_games):
    """Test games() function can filter on min and max payoff values"""
    filtered_games = gbt.catalog.games(min_payoff=0, max_payoff=10)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.min_payoff >= 0
        assert g.max_payoff <= 10


def test_catalog_games_filter_n_nodes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'nodes'"""
    filtered_games = gbt.catalog.games(n_nodes=5)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.nodes) == 5


def test_catalog_games_filter_n_outcomes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'outcomes'"""
    filtered_games = gbt.catalog.games(n_outcomes=3)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.outcomes) == 3


def test_catalog_games_filter_n_players(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'players'"""
    filtered_games = gbt.catalog.games(n_players=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.players) == 2


def test_catalog_games_filter_n_strategies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'strategies'"""
    filtered_games = gbt.catalog.games(n_strategies=4)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.strategies) == 4


def test_catalog_games_filter_bad_filter():
    """Test games() function raises error on invalid filter key"""
    with pytest.raises(TypeError):
        gbt.catalog.games(invalid_filter=123)
