from pathlib import Path

import numpy as np

from pygambit import Game, read_efg, read_nfg

_CATALOG_DIR = Path(__file__).parent
_GAMEFILES_DIR = _CATALOG_DIR / "gamefiles"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    def __new__(cls) -> Game:
        raise NotImplementedError("Subclasses must implement __new__ method")


class CatalogGameFromFile(CatalogGame):
    """
    Base class for catalog games loaded from files.
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


class PrisonersDilemma(CatalogGameFromFile):
    """Prisoner's Dilemma game."""
    game_file = "pd.nfg"


class TwoStageMatchingPennies(CatalogGameFromFile):
    """Two-Stage Matching Pennies game."""
    game_file = "2smp.efg"


class PrisonersDilemmaTestgame(CatalogGame):
    """A simple test game based on the Prisoner's Dilemma."""
    def __new__(cls) -> Game:
        player1_payoffs = np.array([[-1, -3], [0, -2]])
        player2_payoffs = np.transpose(player1_payoffs)

        g1 = Game.from_arrays(
            player1_payoffs,
            player2_payoffs,
            title="Test Prisoner's Dilemma"
        )
        return g1
