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

    title: str | None = None
    num_players: int | None = None
    game_type: str | None = None
    description: str | None = None
    citation: str | None = None

    def __new__(cls) -> Game:
        raise NotImplementedError("Subclasses must implement __new__ method")

    @classmethod
    def _extract_metadata_from_game(cls, game: Game) -> None:
        """Extract metadata from the game and set as class attributes."""
        cls.title = game.title
        cls.num_players = len(game.players)

    def __init_subclass__(cls, **kwargs):
        """Extract metadata when subclass is defined (if not a file-based game)."""
        super().__init_subclass__(**kwargs)

        # Skip if this is CatalogGameFromFile or its subclasses
        if cls.__name__ == "CatalogGameFromFile" or issubclass(cls, CatalogGameFromFile):
            return

        # For non-file-based games, create a temporary instance to extract metadata
        try:
            temp_game = cls.__new__(cls)
            cls._extract_metadata_from_game(temp_game)
        except NotImplementedError:
            # Base class, skip
            pass


class CatalogGameFromFile(CatalogGame):
    """
    Base class for catalog games loaded from files.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    game_file: str | None = None
    _cached_game: Game | None = None

    def __new__(cls) -> Game:
        # Return cached game if available, otherwise load it
        if cls._cached_game is None:
            cls._cached_game = cls._load_game()
        # Return a fresh instance (not the cached one)
        return cls._load_game()

    @classmethod
    def _load_game(cls) -> Game:
        """Load the game from file."""
        if cls.game_file is None:
            raise NotImplementedError(f"{cls.__name__} must define 'game_file'")

        cls.game_type = cls.game_file.split(".")[-1]
        file_path = _GAMEFILES_DIR / cls.game_file

        if cls.game_type == "nfg":
            return read_nfg(str(file_path))
        elif cls.game_type == "efg":
            return read_efg(str(file_path))
        else:
            raise ValueError(f"Game file extension must be 'nfg' or 'efg', got '{cls.game_type}'")

    def __init_subclass__(cls, **kwargs):
        """Validate and extract metadata when subclass is defined."""
        super().__init_subclass__(**kwargs)

        if not hasattr(cls, "game_file") or cls.game_file is None:
            raise TypeError(f"{cls.__name__} must define 'game_file' class attribute")

        # Load game and extract metadata immediately when class is defined
        cls._cached_game = cls._load_game()
        cls._extract_metadata_from_game(cls._cached_game)


class PrisonersDilemma(CatalogGameFromFile):
    game_file = "pd.nfg"
    description = "Prisoner's Dilemma game."
    citation = "Example citation for Prisoner's Dilemma."


class TwoStageMatchingPennies(CatalogGameFromFile):
    game_file = "2smp.efg"
    description = "Two-Stage Matching Pennies game."
    citation = "Example citation for Two-Stage Matching Pennies."


class PrisonersDilemmaTestgame(CatalogGame):
    title = "Test Prisoner's Dilemma"
    game_type = "nfg"
    description = "A simple test game based on the Prisoner's Dilemma."
    citation = "Example citation for Test Prisoner's Dilemma."

    def __new__(cls) -> Game:
        player1_payoffs = np.array([[-1, -3], [0, -2]])
        player2_payoffs = np.transpose(player1_payoffs)

        g1 = Game.from_arrays(
            player1_payoffs,
            player2_payoffs,
            title=cls.title,
        )

        return g1
