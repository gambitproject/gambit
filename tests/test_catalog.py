import pytest

from pygambit import Game, catalog
from pygambit.catalog import CatalogGame, PrisonersDilemma


class ExampleGame(CatalogGame):

    @staticmethod
    def _game(some_param: bool = False):
        if some_param:
            g = Game.new_tree(
                players=["A", "B"], title="Test game T"
            )
        else:
            g = Game.new_tree(
                players=["A", "B"], title="Test game F"
            )
        g.description = "Test game description."
        return g


class ExampleGameWithDocstring(ExampleGame):
    """
    Alternative test game description.
    """


class TestCatalogGame:
    """Tests for CatalogGame base class and subclassing."""

    def test_catalog_game_not_instantiable(self):
        """CatalogGame should not be directly instantiable."""
        with pytest.raises(NotImplementedError):
            CatalogGame()

    def test_custom_game_subclass_extracts_metadata(self):
        """Custom CatalogGame subclasses should extract metadata from _game()."""
        assert ExampleGame().description == "Test game description."

    def test_can_get_game_description_from_docstring(self):
        """CatalogGame should get description from docstring over game description."""
        assert ExampleGameWithDocstring().description == "Alternative test game description."

    def test_catalog_py_game_with_parameters(self):
        """
        Custom CatalogGame subclass should return Game
        and support parameters.
        """
        assert ExampleGame(some_param=False).title == "Test game F"
        assert ExampleGame(some_param=True).title == "Test game T"

    def test_catalog_yml_game_instantiation(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert isinstance(PrisonersDilemma(), Game)

    def test_catalog_yml_game_description(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert (
            PrisonersDilemma().description
            == "A simple implementation of a two person Prisoner's Dilemma game."
        )


class TestGamesFunction:
    """Tests for the games() query function."""

    def test_games_returns_list_of_strings(self):
        """games() should return a list of class name strings."""
        result = catalog.games()
        assert isinstance(result, list)
        assert all(isinstance(name, str) for name in result)

    def test_games_filter_by_game_type(self):
        """Filtering should split games into NFG/EFG."""
        nfg_games = catalog.games(is_tree=False)
        efg_games = catalog.games(is_tree=True)
        all_games = catalog.games(is_tree=None)
        assert len(all_games) == len(set(nfg_games + efg_games))

    def test_games_filter_by_num_players(self):
        """games(num_players=n) should return only n-player games."""
        three_player_games = catalog.games(num_players=3)
        for game_name in three_player_games:
            game_class = getattr(catalog, game_name)
            assert len(game_class.game.players) == 3

    def test_games_filter_by_num_infosets(self):
        """games(num_infosets=n) should return only n-infoset games."""
        two_infoset_games = catalog.games(num_infosets=2)
        for game_name in two_infoset_games:
            game_class = getattr(catalog, game_name)
            assert len(game_class.game.infosets) == 2

    def test_games_filter_by_custom_metadata(self):
        """games() should filter by custom metadata fields."""
        custom_filter_games = catalog.games(test_suite=True)
        assert isinstance(custom_filter_games, list)
        for game_name in custom_filter_games:
            game_class = getattr(catalog, game_name)
            assert hasattr(game_class, "test_suite")
            assert game_class.test_suite is True

    def test_games_excludes_base_classes(self):
        """games() should not include base classes like CatalogGameFromContrib."""
        result = catalog.games()
        assert "CatalogGame" not in result
        assert "CatalogGameFromContrib" not in result

    def test_games_includes_coded_games(self):
        """games() should include manually coded games."""
        result = catalog.games(test_suite=True)
        assert "OneShotTrust" in result
