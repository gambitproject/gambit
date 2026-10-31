import contextlib
import pathlib
import tempfile
import typing

from pygambit.error import UnsupportedForBayesianGameError
from pygambit.gambit import BayesianActionGraphGame, Game


_T = typing.TypeVar("_T")


def _call_strategic_solver(call: typing.Callable[[], _T]) -> _T:
    """Call a strategic-form solver, adding context to allocation failures."""
    try:
        return call()
    except RuntimeError as exc:
        message = str(exc)
        if message == "vector" or "bad_alloc" in message or "bad allocation" in message:
            raise RuntimeError(
                "The strategic representation is too large to solve in memory; "
                "the solver could not allocate its required vectors and matrices. "
                "For an extensive game, use the behavior representation instead "
                "(leave use_strategic=False)."
            ) from None
        raise


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
