import pytest

from pygambit import Game, catalog
from pygambit.catalog import CatalogGame, PrisonersDilemma


class TestGame(CatalogGame):
    """
    Test game description.
    """

    game_type = "efg"

    @staticmethod
    def _game(some_param: bool = False):
        if some_param:
            return Game.new_tree(
                players=["A", "B"], title="Test game T"
            )
        return Game.new_tree(
            players=["A", "B"], title="Test game F"
        )


class TestCatalogGame:
    """Tests for CatalogGame base class and subclassing."""

    def test_catalog_game_not_instantiable(self):
        """CatalogGame should not be directly instantiable."""
        with pytest.raises(NotImplementedError):
            CatalogGame()

    def test_custom_game_subclass_extracts_metadata(self):
        """Custom CatalogGame subclasses should extract metadata from _game()."""
        assert TestGame.num_players == 2
        assert TestGame.game_type == "efg"
        assert TestGame.title == "Test game F"
        assert len(TestGame.description) > 0

    def test_catalog_py_game_with_parameters(self):
        """
        Custom CatalogGame subclass should return Game
        and support parameters.
        """
        assert isinstance(TestGame(some_param=False), Game)
        assert isinstance(TestGame(some_param=True), Game)

    def test_catalog_yml_game_instantiation(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert isinstance(PrisonersDilemma(), Game)


class TestGamesFunction:
    """Tests for the games() query function."""

    def test_games_returns_list_of_strings(self):
        """games() should return a list of class name strings."""
        result = catalog.games()
        assert isinstance(result, list)
        assert all(isinstance(name, str) for name in result)

    def test_games_filter_by_game_type(self):
        """Filtering should split games into NFG/EFG."""
        nfg_games = catalog.games(game_type="nfg")
        efg_games = catalog.games(game_type="efg")
        all_games = catalog.games(game_type="all")
        assert len(all_games) == len(set(nfg_games + efg_games))

    def test_games_filter_by_num_players(self):
        """games(num_players=n) should return only n-player games."""
        two_player_games = catalog.games(num_players=3)
        for game_name in two_player_games:
            game_class = getattr(catalog, game_name)
            assert game_class.num_players == 3

#     def test_games_combined_filters(self):
#         """games() should support combining multiple filters."""
#         result = gbt.catalog.games(game_type="nfg", num_players=2)
#         assert isinstance(result, list)
#         for game_name in result:
#             game_class = getattr(gbt.catalog, game_name)
#             assert game_class.game_type == "nfg"
#             assert game_class.num_players == 2

#     def test_games_filter_by_custom_metadata(self):
#         """games() should filter by custom metadata fields."""
#         # Assuming PrisonersDilemma has tutorial: 1 in metadata
#         tutorial_games = gbt.catalog.games(tutorial=1)
#         assert isinstance(tutorial_games, list)
#         if tutorial_games:
#             for game_name in tutorial_games:
#                 game_class = getattr(gbt.catalog, game_name)
#                 assert hasattr(game_class, "tutorial")
#                 assert game_class.tutorial == 1

#     def test_games_all_filter(self):
#         """games(game_type='all') should return all games."""
#         all_games = gbt.catalog.games(game_type="all")
#         nfg_games = gbt.catalog.games(game_type="nfg")
#         efg_games = gbt.catalog.games(game_type="efg")
#         # All games should be union of NFG and EFG
#         assert len(all_games) == len(set(nfg_games + efg_games))

#     def test_games_nonexistent_filter(self):
#         """games() with non-matching filters should return empty list."""
#         result = gbt.catalog.games(num_players=999)
#         assert result == []

#     def test_games_excludes_base_classes(self):
#         """games() should not include base classes like CatalogGameFromContrib."""
#         result = gbt.catalog.games()
#         assert "CatalogGame" not in result
#         assert "CatalogGameFromContrib" not in result
