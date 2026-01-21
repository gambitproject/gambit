from .catalog import CatalogGame, games

__all__ = ["games", "CatalogGame"]


def __getattr__(name: str):
    """Lazy load catalog games on access."""
    from . import catalog as _catalog_module
    return getattr(_catalog_module, name)
