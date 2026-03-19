import pandas as pd
import pytest

import pygambit as gbt


@pytest.fixture(scope="module")
def all_games():
    return gbt.catalog.games()


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
    errors = []
    for slug in game_slugs:
        try:
            g = gbt.catalog.load(slug)
            assert isinstance(g, gbt.Game), f"Expected gbt.Game, got {type(g)}"
        except Exception as e:
            errors.append(f"Slug '{slug}' failed with {type(e).__name__}: {e}")

    if errors:
        pytest.fail(f"Errors loading {len(errors)} game(s):\n" + "\n".join(errors))


def test_catalog_load_invalid_slug():
    """Test loading an invalid game slug."""
    with pytest.raises(FileNotFoundError):
        gbt.catalog.load("invalid_slug")


def test_catalog_load_family_game():
    """Test loading a game generated from code with a game family func."""
    g = gbt.catalog.load("one_shot_trust")
    assert isinstance(g, gbt.Game)


def test_catalog_games(game_slugs, all_games):
    """Test games() function returns df of game slugs and titles."""
    assert isinstance(all_games, pd.DataFrame)

    # The games() function should return set of slugs plus family games
    fg = gbt.catalog.family_games().keys()
    assert set(all_games["Game"]) == game_slugs.union(fg)

    # Test that standard columns are present
    assert "Game" in all_games.columns
    assert "Title" in all_games.columns


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


def test_catalog_games_include_descriptions():
    """Test games() function can include descriptions"""
    games_with_desc = gbt.catalog.games(include_descriptions=True)
    assert "Description" in games_with_desc.columns
    assert "Download" in games_with_desc.columns
