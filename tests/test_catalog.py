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
    all_games = gbt.catalog.games()
    slugs = list(all_games.Game)
    assert isinstance(all_games, pd.DataFrame)
    assert len(all_games) > 0
    # Check slug of game in subdir
    assert "myerson/fig_4_2" in slugs
    assert "myerson_fig_4_2" not in slugs
    # Check family game present
    assert "one_shot_trust" in slugs
