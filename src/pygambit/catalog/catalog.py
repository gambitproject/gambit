from pathlib import Path
from typing import Literal

# import numpy as np
from pygambit import Game, read_efg, read_nfg

_GAMEFILES_DIR = Path(__file__).parent.parent.parent.parent / "contrib/games"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    title: str
    num_players: int
    game_type: Literal["nfg", "efg"]
    description: str
    citation: str

    def __new__(cls, *args, **kwargs) -> Game:
        """Create a game instance by calling the _game() method."""
        if hasattr(cls, "_game") and cls._game is not CatalogGame._game:
            return cls._game(*args, **kwargs)
        raise NotImplementedError("Subclasses must implement _game() method")

    @staticmethod
    def _game() -> Game:
        """Override this method in subclasses to define the game."""
        raise NotImplementedError("Subclasses must implement _game() method")

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
            temp_game = cls._game()
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

    game_file: str
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


def games(
    game_type: Literal["all", "nfg", "efg"] = "all",
    num_players: int | None = None,
) -> list[str]:
    """
    Return a list of catalog game names.

    Parameters
    ----------
    game_type : {"all", "nfg", "efg"}, default "all"
        Filter games by type:
        - "all": return all games
        - "nfg": return only normal-form (strategic) games
        - "efg": return only extensive-form games
    num_players : int | None, default None
        If specified, only return games with the given number of players.

    Returns
    -------
    list[str]
        List of game class names matching the specified type.
    """
    def get_all_subclasses(cls):
        """Recursively get all subclasses."""
        all_subclasses = []
        for subclass in cls.__subclasses__():
            if subclass.__name__ not in ["CatalogGameFromFile"] and (
                game_type == "all" or subclass.game_type == game_type
            ) and (
                num_players is None or subclass.num_players == num_players
            ):
                all_subclasses.append(subclass.__name__)
            all_subclasses.extend(get_all_subclasses(subclass))
        return all_subclasses

    return get_all_subclasses(CatalogGame)


############################
# Catalog games from files #
############################


class PrisonersDilemma(CatalogGameFromFile):
    game_file = "pd.nfg"
    description = "Prisoner's Dilemma game."
    citation = "Example citation for Prisoner's Dilemma."


class TwoStageMatchingPennies(CatalogGameFromFile):
    game_file = "2smp.efg"
    description = "Two-Stage Matching Pennies game."
    citation = "Example citation for Two-Stage Matching Pennies."


##########################################
# Catalog games defined programmatically #
##########################################


class OneShotTrust(CatalogGame):
    game_type = "efg"
    description = """
    The unique_NE_variant makes Trust a dominant strategy, replacing the
    non-singleton equilibrium component from the standard version of the game
    where the Buyer plays "Not Trust" and the seller can play any mixture with
    < 0.5 probability on Honor with a unique NE where the Buyer plays Trust and
    the Seller plays Abuse.
    """
    citation = "Kreps (1990)"

    @staticmethod
    def _game(unique_NE_variant: bool = False):
        g = Game.new_tree(
            players=["Buyer", "Seller"], title="One-shot trust game, after Kreps (1990)"
        )
        g.append_move(g.root, "Buyer", ["Trust", "Not trust"])
        g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])
        g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, 1], label="Trustworthy"))
        if unique_NE_variant:
            g.set_outcome(
                g.root.children[0].children[1], g.add_outcome(["1/2", 2], label="Untrustworthy")
            )
        else:
            g.set_outcome(
                g.root.children[0].children[1], g.add_outcome([-1, 2], label="Untrustworthy")
            )
        g.set_outcome(g.root.children[1], g.add_outcome([0, 0], label="Opt-out"))
        return g
