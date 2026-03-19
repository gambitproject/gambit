import pandas as pd
import pytest

import pygambit as gbt


@pytest.fixture
def game_slugs():
    """Fixture providing a set of all game slugs in the catalog."""
    game_slugs = set()
    for resource_path in gbt.catalog._CATALOG_RESOURCE.rglob("*"):
        if resource_path.is_file() and resource_path.suffix in gbt.catalog.READERS:
            rel_path = resource_path.relative_to(gbt.catalog._CATALOG_RESOURCE)
            slug = rel_path.with_suffix("").as_posix()
            game_slugs.add(slug)
    return game_slugs


def test_catalog_load_all_game_slugs(game_slugs):
    """Test loading all valid game files in the catalog."""
    for slug in game_slugs:
        g = gbt.catalog.load(slug)
        assert isinstance(g, gbt.Game)


def test_catalog_load_invalid_slug():
    """Test loading an invalid game slug."""
    with pytest.raises(FileNotFoundError):
        gbt.catalog.load("invalid_slug")


def test_catalog_games(game_slugs):
    """Test games() function returns df of game slugs and titles."""
    all_games = gbt.catalog.games()
    assert isinstance(all_games, pd.DataFrame)

    # The games() function should return exactly the set of slugs found above
    assert set(all_games["Game"]) == game_slugs

    # Test that standard columns are present
    assert "Game" in all_games.columns
    assert "Title" in all_games.columns
