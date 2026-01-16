# import pytest

from pygambit import Game
from pygambit.catalog import OneShotTrust, PrisonersDilemma


class TestCatalogGameBase:
    """Tests for CatalogGame base class and subclassing."""

    # This test will only work if CatalogGame added to __init__.py
    # def test_catalog_game_not_instantiable(self):
    #     """CatalogGame should not be directly instantiable."""
    #     with pytest.raises(NotImplementedError):
    #         gbt.catalog.CatalogGame()

    def test_custom_game_subclass_extracts_metadata(self):
        """Custom CatalogGame subclasses should extract metadata from _game()."""
        assert OneShotTrust.num_players == 2
        assert OneShotTrust.game_type == "efg"
        assert OneShotTrust.title == "One-shot trust game, after Kreps (1990)"

    def test_catalog_yml_game_instantiation(self):
        """Custom CatalogGame subclasses reading from catalog.yml should return Game instances."""
        assert isinstance(PrisonersDilemma(), Game)

    def test_catalog_py_game_with_parameters(self):
        """
            Custom CatalogGame subclass from catalog.py should return Game
            and support parameters.
        """
        assert isinstance(OneShotTrust(unique_NE_variant=False), Game)
        assert isinstance(OneShotTrust(unique_NE_variant=True), Game)
