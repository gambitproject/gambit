from pathlib import Path

from pygambit import read_efg, read_nfg

_CATALOG_DIR = Path(__file__).parent
_GAMEFILES_DIR = _CATALOG_DIR / "gamefiles"


class CatalogGame:
    """
    Base class for catalog games.
    This class serves as a template for specific games in the catalog.
    Calling any subclass will return an instance of the corresponding game.
    """
    def __new__(cls):
        raise NotImplementedError("This method should be implemented by subclasses.")


class PrisonersDilemma(CatalogGame):
    """Prisoner's Dilemma game."""
    def __new__(cls):
        return read_nfg(str(_GAMEFILES_DIR / "pd.nfg"))


class TwoStageMatchingPennies(CatalogGame):
    """Two-Stage Matching Pennies game."""
    def __new__(cls):
        return read_efg(str(_GAMEFILES_DIR / "2smp.efg"))
