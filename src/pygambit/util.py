import contextlib
import pathlib
import tempfile
import typing


@contextlib.contextmanager
def make_temporary(content: typing.Optional[str] = None) -> pathlib.Path:
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
