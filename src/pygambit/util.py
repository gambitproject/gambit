import contextlib
import pathlib
import tempfile
import typing


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


def warn_on_explicit_use_strategic_false(func):
    """Decorator to emit a UserWarning if `use_strategic=False` is explicitly 
    passed to a solver function for a game that doesn't have an extensive-form
    (tree) representation.
    """
    import functools
    import inspect
    import warnings

    sig = inspect.signature(func)

    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        bound = sig.bind(*args, **kwargs)
        if "use_strategic" in bound.arguments:
            if not bound.arguments["use_strategic"]:
                game = bound.arguments.get("game")
                if game is not None and not getattr(game, "is_tree", True):
                    warnings.warn(
                        "Game has no tree representation; using strategic form.",
                        UserWarning,
                        stacklevel=2,
                    )
        return func(*args, **kwargs)

    return wrapper
