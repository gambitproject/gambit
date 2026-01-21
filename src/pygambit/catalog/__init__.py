from .catalog import CatalogGame, games

__all__ = ["games", "CatalogGame"]


def __getattr__(name: str):
    """Lazy load catalog games on access."""
    from . import catalog as _catalog_module

    # Don't try to load if already initializing to prevent recursion
    if not _catalog_module._catalog_initializing:
        # Ensure all catalog games are loaded
        _catalog_module._ensure_catalog_loaded()

    if hasattr(_catalog_module, name):
        return getattr(_catalog_module, name)

    raise AttributeError(f"module '{__name__}' has no attribute '{name}'")
