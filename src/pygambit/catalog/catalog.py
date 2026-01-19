import inspect
import sys
from pathlib import Path
from typing import Literal

import yaml

from pygambit.gambit import Game, read_efg, read_nfg

_GAMEFILES_DIR = Path(__file__).parent.parent.parent.parent / "contrib/games"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    game: Game | None = None

    def __new__(cls, *args, **kwargs) -> Game:
        """Create a game instance by calling the _game() method."""
        if cls.game is None:
            cls.game = cls._game(*args, **kwargs)
        return cls.game

    @staticmethod
    def _game() -> Game:
        """Override this method in subclasses to define the game."""
        raise NotImplementedError("Subclasses must implement _game() method")

    @classmethod
    def _extract_description(cls, game: Game) -> None:
        """Extract game description from docstring and apply to game."""
        cleaned_docstring = ""
        if cls.__doc__:
            cleaned_docstring = inspect.cleandoc(cls.__doc__)
        if len(cleaned_docstring) > 0:
            game.description = cleaned_docstring

    def __init_subclass__(cls, **kwargs):
        """Extract metadata when subclass is defined (if not a file-based game)."""
        super().__init_subclass__(**kwargs)

        # Skip if this is CatalogGameFromContrib or its subclasses
        if cls.__name__ == "CatalogGameFromContrib" or issubclass(cls, CatalogGameFromContrib):
            return

        # Load game and extract metadata immediately when class is defined
        cls.game = cls._game()
        cls._extract_description(cls.game)


class CatalogGameFromContrib(CatalogGame):
    """
    Base class for catalog games loaded from files.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    game_file: str

    def __new__(cls) -> Game:
        if cls.game is None:
            cls.game = cls._load_game()
        return cls.game

    @classmethod
    def _load_game(cls) -> Game:
        """Load the game from file."""
        if not hasattr(cls, "game_file") or cls.game_file is None:
            raise TypeError(f"{cls.__name__} must define 'game_file' class attribute")

        game_type = cls.game_file.split(".")[-1]
        file_path = _GAMEFILES_DIR / cls.game_file

        if game_type == "nfg":
            return read_nfg(str(file_path))
        elif game_type == "efg":
            return read_efg(str(file_path))
        else:
            raise ValueError(f"Game file extension must be 'nfg' or 'efg', got '{game_type}'")

    def __init_subclass__(cls, **kwargs):
        """Validate and extract metadata when subclass is defined."""
        super().__init_subclass__(**kwargs)

        # Load game and extract metadata immediately when class is defined
        cls.game = cls._load_game()
        cls._extract_description(cls.game)


def games(
    game_type: Literal["all", "nfg", "efg"] = "all",
    num_players: int | None = None,
    **metadata_filters,
) -> list[str]:
    """
    Return a list of catalog game class names.

    Parameters
    ----------
    game_type : {"all", "nfg", "efg"}, default "all"
        Filter games by type:
        - "all": return all games
        - "nfg": return only normal-form (strategic) games
        - "efg": return only extensive-form games
    num_players : int | None, default None
        If specified, only return games with the given number of players.
    **metadata_filters
        Additional keyword arguments to filter by catalog.yml metadata fields.
        For example, `x=1` filters for games with `x: 1` in metadata.

    Returns
    -------
    list[str]
        List of game class names matching the specified filters.

    Examples
    --------
    >>> games(x=1)  # Games with a custom metadata field 'x' equal to 1
    >>> games(game_type="efg", num_players=2)  # 2-player extensive-form games
    """

    def get_all_subclasses(cls):
        """Recursively get all subclasses."""
        all_subclasses = []
        for subclass in cls.__subclasses__():

            # Don't include CatalogGameFromContrib in result
            if subclass.__name__ in ["CatalogGameFromContrib"]:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            # Instantiate game to access metadata
            g = subclass()

            # Check standard filters
            g_game_type = "nfg"
            if g.is_tree:
                g_game_type = "efg"
            if game_type != "all" and g_game_type != game_type:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if num_players is not None and len(g.players) != num_players:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            # Check metadata filters
            metadata_match = True
            for key, value in metadata_filters.items():
                if not hasattr(subclass, key) or getattr(subclass, key) != value:
                    metadata_match = False
                    break

            if metadata_match:
                all_subclasses.append(subclass.__name__)

            all_subclasses.extend(get_all_subclasses(subclass))
        return all_subclasses
    # Sort alphabetically for consistency
    return sorted(get_all_subclasses(CatalogGame))


_CATALOG_YAML = Path(__file__).parent / "catalog.yml"


def _load_catalog_from_yaml(path: Path) -> dict[str, dict]:
    if not path.exists():
        raise FileNotFoundError(f"Catalog YAML not found: {path}")
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def _generate_contrib_game_classes(catalog: dict[str, dict]) -> None:
    """
    Dynamically generate CatalogGameFromContrib subclasses from YAML
    and attach them to this module's namespace.
    """
    module = sys.modules[__name__]

    for class_name, entry in catalog.items():
        if "file" not in entry:
            raise ValueError(f"Missing 'file' for catalog entry '{class_name}'")

        game_file = entry["file"]
        metadata = entry.get("metadata", {})

        # Build class attributes dict
        class_attrs = {
            "game_file": game_file,
            "__module__": __name__,
            "__doc__": entry.get("description", None),
        }

        # Add metadata fields as class attributes
        if metadata and "valid_game" in metadata and metadata["valid_game"] is False:
            pass  # Marked as invalid game, do not create class
        else:
            if metadata:
                for key, value in metadata.items():
                    class_attrs[key] = value

            cls = type(
                class_name,
                (CatalogGameFromContrib,),
                class_attrs,
            )

            setattr(module, class_name, cls)


# Generate classes at import time
_catalog_data = _load_catalog_from_yaml(_CATALOG_YAML)
_generate_contrib_game_classes(_catalog_data)

# Import all coded games to ensure they are registered in the catalog
from .coded_games import *  # noqa: E402, F403, F401
