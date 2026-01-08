def __getattr__(name: str):
    """Lazily load game classes and the games function."""
    if name == "games":
        from .catalog import games
        return games

    # Try to import the game class
    try:
        from . import catalog
        if hasattr(catalog, name):
            return getattr(catalog, name)
    except ImportError:
        pass

    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


def __dir__():
    """Support dir() by lazily loading all game names."""
    from .catalog import games
    _all_games = games()
    return ["games"] + _all_games


__all__ = ["games"]  # type: ignore[assignment]
