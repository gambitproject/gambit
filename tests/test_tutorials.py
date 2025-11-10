import contextlib
from pathlib import Path

import nbformat
import pytest
from nbclient import NotebookClient
from nbclient.exceptions import CellExecutionError


def _find_tutorial_notebooks():
    """Return a sorted list of notebook Paths under doc/tutorials.

    Skips the entire module if the tutorials directory does not exist.
    """
    root = Path(__file__).resolve().parents[1] / "doc" / "tutorials"
    if not root.exists():
        pytest.skip(f"Tutorials folder not found: {root}")
    notebooks = sorted(root.rglob("*.ipynb"))
    if not notebooks:
        pytest.skip(f"No tutorial notebooks found in: {root}")
    return notebooks


# Discover notebooks at import time so pytest can parametrize them.
_NOTEBOOKS = _find_tutorial_notebooks()


@pytest.mark.parametrize("nb_path", _NOTEBOOKS, ids=[p.name for p in _NOTEBOOKS])
def test_execute_notebook(nb_path):
    """Execute a single Jupyter notebook and fail if any cell errors occur.

    This uses nbclient.NotebookClient to run the notebook in its parent directory
    so relative paths within the notebook resolve correctly.
    """
    nb = nbformat.read(str(nb_path), as_version=4)

    # Prefer the notebook's kernelspec if provided, otherwise let nbclient pick the default.
    kernel_name = nb.metadata.get("kernelspec", {}).get("name")

    client = NotebookClient(
        nb,
        timeout=600,
        kernel_name=kernel_name,
        resources={"metadata": {"path": str(nb_path.parent)}},
    )

    try:
        client.execute()
    except CellExecutionError as exc:
        # Re-raise with more context so pytest shows which notebook failed.
        raise AssertionError(f"Error while executing notebook {nb_path}: {exc}") from exc
    finally:
        # Ensure kernel is shut down.
        with contextlib.suppress(Exception):
            client.shutdown_kernel()
