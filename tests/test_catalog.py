from pygambit import Game
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


class TestCatalogGameBase:
    """Tests for CatalogGame base class and subclassing."""

    # This test will only work if CatalogGame added to __init__.py
    # def test_catalog_game_not_instantiable(self):
    #     """CatalogGame should not be directly instantiable."""
    #     with pytest.raises(NotImplementedError):
    #         gbt.catalog.CatalogGame()

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
