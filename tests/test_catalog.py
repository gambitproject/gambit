import pandas as pd
import pytest

import pygambit as gbt


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


def test_catalog_games():
    """Test games() function returns df of game slugs and titles"""
    all_games = gbt.catalog.games()
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
