import pytest

import pygambit as gbt


class ExampleGame(gbt.catalog.CatalogGame):

    @staticmethod
    def _game(some_param: bool = False):
        """Desc2"""
        if some_param:
            g = gbt.Game.new_tree(
                players=["A", "B"], title="Test game T"
            )
        else:
            g = gbt.Game.new_tree(
                players=["A", "B"], title="Test game F"
            )
        g.description = "Desc1"
        return g


class ExampleGameWithDocstring(ExampleGame):
    """
    Desc3
    """


class TestCatalogGame:
    """Tests for CatalogGame base class and subclassing."""

    def test_catalog_game_not_instantiable(self):
        """CatalogGame should not be directly instantiable."""
        with pytest.raises(NotImplementedError):
            gbt.catalog.CatalogGame()

    def test_description_handling_from_game(self):
        """Custom CatalogGame subclass should extract description from game object."""
        assert "Desc1" in ExampleGame().description
        # Also should include _game docstring
        assert "Desc2" in ExampleGame().description

    def test_description_handling_from_docstring(self):
        """Custom CatalogGame subclass should get docstring over game description."""
        assert "Desc3" in ExampleGameWithDocstring().description
        # Also should include _game docstring
        assert "Desc2" in ExampleGameWithDocstring().description

    def test_catalog_py_game_with_parameters(self):
        """
        Custom CatalogGame subclass should return Game
        and support parameters.
        """
        assert ExampleGame(some_param=False).title == "Test game F"
        assert ExampleGame(some_param=True).title == "Test game T"

    def test_catalog_yml_game_instantiation(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert isinstance(gbt.catalog.PrisonersDilemma(), gbt.Game)

    def test_catalog_yml_game_description(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert (
            "A simple implementation of a two person Prisoner's Dilemma game." in
            gbt.catalog.PrisonersDilemma().description
        )


class TestGamesFunction:
    """Tests for the games() query function."""

    def test_games_returns_list_of_strings(self):
        """games() should return a list of class name strings."""
        result = gbt.catalog.games()
        assert isinstance(result, list)
        assert all(isinstance(name, str) for name in result)

    def test_games_filter_by_game_type(self):
        """Filtering should split games into NFG/EFG."""
        nfg_games = gbt.catalog.games(is_tree=False)
        efg_games = gbt.catalog.games(is_tree=True)
        all_games = gbt.catalog.games(is_tree=None)
        assert len(all_games) == len(set(nfg_games + efg_games))

    def test_games_filter_by_num_players(self):
        """games(num_players=n) should return only n-player games."""
        three_player_games = gbt.catalog.games(num_players=3)
        for game_name in three_player_games:
            game_class = getattr(gbt.catalog, game_name)
            assert len(game_class.game.players) == 3

    def test_games_filter_by_num_infosets(self):
        """games(num_infosets=n) should return only n-infoset games."""
        two_infoset_games = gbt.catalog.games(num_infosets=2)
        for game_name in two_infoset_games:
            game_class = getattr(gbt.catalog, game_name)
            assert len(game_class.game.infosets) == 2

    def test_games_filter_by_custom_metadata(self):
        """games() should filter by custom metadata fields."""
        custom_filter_games = gbt.catalog.games(test_suite=True)
        assert isinstance(custom_filter_games, list)
        for game_name in custom_filter_games:
            game_class = getattr(gbt.catalog, game_name)
            assert hasattr(game_class, "test_suite")
            assert game_class.test_suite is True

    def test_games_excludes_base_classes(self):
        """games() should not include base classes like CatalogGameFromFile."""
        result = gbt.catalog.games()
        assert "CatalogGame" not in result
        assert "CatalogGameFromFile" not in result

    def test_games_includes_coded_games(self):
        """games() should include manually coded games."""
        result = gbt.catalog.games(test_suite=True)
        assert "OneShotTrust" in result
