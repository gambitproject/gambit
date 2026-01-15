from . import catalog
from .catalog import games

# Ensure catalog module is fully imported including all YAML-generated classes exist
_all_games = games()
_game_classes = {}

for game_name in _all_games:
    try:
        _game_classes[game_name] = getattr(catalog, game_name)
    except AttributeError as e:
        raise ImportError(
            f"Catalog game '{game_name}' listed but not found in catalog module"
        ) from e

# Add to module namespace
globals().update(_game_classes)

# Build __all__ dynamically
__all__ = ["games", *list(_all_games)]
