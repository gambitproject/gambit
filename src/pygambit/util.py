import contextlib
import pathlib
import tempfile
import typing

from pygambit.error import UnsupportedForBayesianGameError
from pygambit.gambit import BayesianActionGraphGame, Game


@contextlib.contextmanager
def make_temporary(content: str | None = None) -> typing.Generator[pathlib.Path, None, None]:
    """Context manager to create a temporary file containing `content', and
    provide the path to the temporary file.

    If `content' is none, the temporary file is created and then deleted, while
    returning the filename, for another process then to write to that file
    (under the assumption that it is extremely unlikely that another program
    will try to write to that same tempfile name).
    """
    with tempfile.NamedTemporaryFile("w", delete=(content is None)) as f:
        if content:
            f.write(content)
        filepath = pathlib.Path(f.name)
    try:
        yield filepath
    finally:
        filepath.unlink(missing_ok=True)


def reject_bayesian(game: Game, solver: str) -> None:
    """Raise if `game` is a Bayesian action-graph game.

    Parameters
    ----------
    game : Game
        The game a solver has been asked to solve.
    solver : str
        The name of the solver, used in the error message.

    Raises
    ------
    UnsupportedForBayesianGameError
        If `game` is a Bayesian action-graph game.
    """
    if isinstance(game, BayesianActionGraphGame):
        raise UnsupportedForBayesianGameError(f"{solver}(): not defined for Bayesian games")
