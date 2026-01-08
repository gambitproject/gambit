from .catalog import games

# Dynamically import all catalog games
_all_games = games()
_game_classes = {}

for game_name in _all_games:
    # Import each game class from catalog module
    from . import catalog
    _game_classes[game_name] = getattr(catalog, game_name)

# Add to module namespace
globals().update(_game_classes)

# Build __all__ dynamically
__all__ = ["games"] + list(_all_games)  # type: ignore[assignment]
