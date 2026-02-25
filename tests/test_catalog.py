import pandas as pd
import pytest

import pygambit as gbt


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


def test_catalog_games():
    """Test games() function returns df of game slugs and titles"""
    all_games = gbt.catalog.games()
    assert isinstance(all_games, pd.DataFrame)
    assert len(all_games) > 0
    assert "myerson1991/fig4_2" in list(all_games.Game)
    assert "Myerson (1991) Figure 4.2" in list(all_games.Title)
