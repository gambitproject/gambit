from pathlib import Path

from pygambit import Game, read_efg, read_nfg

_CATALOG_DIR = Path(__file__).parent
_GAMEFILES_DIR = _CATALOG_DIR / "gamefiles"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """
    # Subclasses must define these
    game_file: str | None = None

    def __new__(cls) -> Game:
        if cls.game_file is None:
            raise NotImplementedError(f"{cls.__name__} must define 'game_file'")
        cls.game_type = cls.game_file.split(".")[-1]  # infer game type from file extension

        # Load the appropriate game type
        file_path = _GAMEFILES_DIR / cls.game_file
        if cls.game_type == "nfg":
            game = read_nfg(str(file_path))
        elif cls.game_type == "efg":
            game = read_efg(str(file_path))
        else:
            raise ValueError(f"Game file extension must be 'nfg' or 'efg', got '{cls.game_type}'")

        return game

    def __init_subclass__(cls, **kwargs):
        """Validate that subclasses define required attributes."""
        super().__init_subclass__(**kwargs)
        # This runs when a subclass is defined
        if not hasattr(cls, "game_file") or cls.game_file is None:
            raise TypeError(f"{cls.__name__} must define 'game_file' class attribute")


class PrisonersDilemma(CatalogGame):
    """Prisoner's Dilemma game."""
    game_file = "pd.nfg"


class TwoStageMatchingPennies(CatalogGame):
    """Two-Stage Matching Pennies game."""
    game_file = "2smp.efg"
