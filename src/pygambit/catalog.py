import inspect
import sys
from importlib.resources import as_file, files

import yaml

import pygambit as gbt

_GAMEFILES_DIR = files(__package__) / "catalog_game_files"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    game: gbt.Game | None = None
    """Cached ``Game`` instance. Overwritten on each instantiation."""

    # Metadata fields
    test_suite: bool = False
    """Indicates if this game is included in the pygambit test suite."""

    def __new__(cls, *args, **kwargs) -> gbt.Game:
        """Create a game instance by calling the _game() method."""
        cls.game = cls._game(*args, **kwargs)
        cls._extract_description(cls.game)
        return cls.game

    @staticmethod
    def _game() -> gbt.Game:
        """Override this method in subclasses to define the game."""
        raise NotImplementedError("Subclasses must implement _game() method")

    @classmethod
    def _extract_description(cls, game: gbt.Game) -> None:
        """Extract game description from docstring and apply to game."""
        cleaned_docstring = ""
        if cls.__doc__:
            cleaned_docstring = inspect.cleandoc(cls.__doc__)
        if len(cleaned_docstring) > 0:
            game.description = cleaned_docstring

    def __init_subclass__(cls, **kwargs):
        """Extract metadata when subclass is defined (if not a file-based game)."""
        super().__init_subclass__(**kwargs)

        # Skip if this is CatalogGameFromFile or its subclasses
        if cls.__name__ == "CatalogGameFromFile" or issubclass(cls, CatalogGameFromFile):
            return

        # Load game and extract metadata immediately when class is defined
        cls.game = cls._game()
        cls._extract_description(cls.game)


class CatalogGameFromFile(CatalogGame):
    """
    Base class for catalog games loaded from files.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """

    game_file: str
    """Filename of the game file in catalog_game_files directory."""

    def __new__(cls) -> gbt.Game:
        if getattr(cls, "game", None) is None:
            cls.game = cls._load_game()
        return cls.game

    @classmethod
    def _load_game(cls) -> gbt.Game:
        """Load the game from file."""
        if not hasattr(cls, "game_file") or cls.game_file is None:
            raise TypeError(f"{cls.__name__} must define 'game_file'")

        game_type = cls.game_file.rsplit(".", 1)[-1]
        resource = _GAMEFILES_DIR / cls.game_file

        with as_file(resource) as path:
            if game_type == "nfg":
                return gbt.read_nfg(str(path))
            elif game_type == "efg":
                return gbt.read_efg(str(path))
            else:
                raise ValueError(
                    f"gbt.Game file extension must be 'nfg' or 'efg', got '{game_type}'"
                )

    def __init_subclass__(cls, **kwargs):
        """Validate and extract metadata when subclass is defined."""
        super().__init_subclass__(**kwargs)

        # Load game and extract metadata immediately when class is defined
        cls.game = cls._load_game()
        cls._extract_description(cls.game)


def games(
    num_actions: int | None = None,
    num_contingencies: int | None = None,
    num_infosets: int | None = None,
    is_const_sum: bool | None = None,
    is_perfect_recall: bool | None = None,
    is_tree: bool | None = None,
    num_nodes: int | None = None,
    num_outcomes: int | None = None,
    num_players: int | None = None,
    num_strategies: int | None = None,
    **metadata_filters,
) -> list[str]:
    """
    Return a list of catalog game class names.

    Parameters
    ----------
    num_actions : int | None, default None
        If specified, only return games with the given number of actions.
    num_contingencies : int | None, default None
        If specified, only return games with the given number of contingencies.
    num_infosets : int | None, default None
        If specified, only return games with the given number of information sets.
    is_const_sum : bool | None, default None
        If specified, only return games that are (or are not) constant-sum.
    is_perfect_recall : bool | None, default None
        If specified, only return games that have (or do not have) perfect recall.
    is_tree : bool | None, default None
        If specified, only return games that are (or are not) extensive-form.
    num_nodes : int | None, default None
        If specified, only return games with the given number of nodes.
    num_outcomes : int | None, default None
        If specified, only return games with the given number of outcomes.
    num_strategies : int | None, default None
        If specified, only return games with the given number of strategies.
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
    >>> games(is_tree=True, num_players=2)  # 2-player extensive-form games
    """

    # Filter by extensive-form if filtering by tree-specific attributes
    if (
        num_actions is not None or
        num_infosets is not None or
        num_nodes is not None
    ):
        is_tree = True

    def get_all_subclasses(cls):
        """Recursively get all subclasses."""
        all_subclasses = []
        for subclass in cls.__subclasses__():

            # Don't include CatalogGameFromFile in result
            if subclass.__name__ in ["CatalogGameFromFile"]:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if is_tree is not None and is_tree != subclass.game.is_tree:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if num_actions is not None:
                if not getattr(subclass.game, "is_tree", False):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue
                if num_actions != len(getattr(subclass.game, "actions", [])):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue

            if num_infosets is not None:
                if not getattr(subclass.game, "is_tree", False):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue
                if num_infosets != len(subclass.game.infosets):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue

            if num_nodes is not None:
                if not getattr(subclass.game, "is_tree", False):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue
                if num_nodes != len(subclass.game.nodes):
                    all_subclasses.extend(get_all_subclasses(subclass))
                    continue

            if (
                num_contingencies is not None
                and num_contingencies != len(subclass.game.contingencies)
            ):
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if is_const_sum is not None and is_const_sum != subclass.game.is_const_sum:
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if (
                is_perfect_recall is not None
                and is_perfect_recall != subclass.game.is_perfect_recall
            ):
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if (
                num_outcomes is not None
                and num_outcomes != len(subclass.game.outcomes)
            ):
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if (
                num_strategies is not None
                and num_strategies != len(subclass.game.strategies)
            ):
                all_subclasses.extend(get_all_subclasses(subclass))
                continue

            if (
                num_players is not None
                and num_players != len(subclass.game.players)
            ):
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


def _load_catalog_from_yaml() -> dict[str, dict]:
    path = files(__package__) / "catalog.yml"
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def _generate_game_classes_from_catalog(catalog: dict[str, dict]) -> None:
    """
    Dynamically generate CatalogGameFromFile subclasses from YAML
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
                (CatalogGameFromFile,),
                class_attrs,
            )

            setattr(module, class_name, cls)


def load_coded_games():
    """
    Lazy load coded games.
    This function is called in __init__.py to load manually coded games.
    """
    for name in dir(gbt.catalog_games):
        if not name.startswith("_"):
            obj = getattr(gbt.catalog_games, name)
            if isinstance(obj, type) and issubclass(obj, CatalogGame):
                globals()[name] = obj


# Generate classes at import time
_catalog_data = _load_catalog_from_yaml()
_generate_game_classes_from_catalog(_catalog_data)
