import contextlib
import os
import sys
from pathlib import Path

import nbformat
import pytest

# Ensure Jupyter uses the new platformdirs paths to avoid DeprecationWarning
# This will become the default in `jupyter_core` v6
os.environ.setdefault("JUPYTER_PLATFORM_DIRS", "1")

from nbclient import NotebookClient  # noqa: E402
from nbclient.exceptions import CellExecutionError  # noqa: E402


def _find_tutorial_notebooks():
    """Return a sorted list of notebook Paths under doc/tutorials.

    Skips the entire module if the tutorials directory does not exist.
    """
    root = Path(__file__).resolve().parents[1] / "doc" / "tutorials"
    if not root.exists():
        pytest.skip(f"Tutorials folder not found: {root}")

    # Collect all notebooks under doc/tutorials (including any subfolders).
    # Exclude Jupyter checkpoint files
    notebooks = sorted(
        p for p in root.rglob("*.ipynb")
        if ".ipynb_checkpoints" not in p.parts
    )

    if not notebooks:
        pytest.skip(f"No tutorial notebooks found in: {root}")
    return notebooks


# Discover notebooks at import time so pytest can parametrize them.
_NOTEBOOKS = _find_tutorial_notebooks()

# Notebooks known to fail for reasons outside this repo's control, with the reason why.
# `strict=True` so that once the underlying issue is fixed, the resulting XPASS is reported
# as a failure -- a prompt to remove the marker rather than a result that's easy to miss.
_XFAIL_NOTEBOOKS = {
    "03_stripped_down_poker.ipynb": (
        "gtdraw <= 0.12.1 reads node.prior_action.prob, which was removed from pygambit in "
        "17.0.0-alpha.2 (see ChangeLog); needs a gtdraw update to use Node.action_probs instead"
    ),
    "openspiel.ipynb": (
        "gtdraw <= 0.12.1 reads node.prior_action.prob, which was removed from pygambit in "
        "17.0.0-alpha.2 (see ChangeLog); needs a gtdraw update to use Node.action_probs instead"
    ),
}


def _notebook_param(nb_path: Path) -> pytest.param:
    reason = _XFAIL_NOTEBOOKS.get(nb_path.name)
    marks = [pytest.mark.xfail(reason=reason, strict=True)] if reason else []
    return pytest.param(nb_path, marks=marks, id=nb_path.name)


@pytest.mark.tutorials
@pytest.mark.parametrize("nb_path", [_notebook_param(p) for p in _NOTEBOOKS])
def test_execute_notebook(nb_path):
    """Execute a single Jupyter notebook and fail if any cell errors occur.

    This uses nbclient.NotebookClient to run the notebook in its parent directory
    so relative paths within the notebook resolve correctly.
    """

    # Skip OpenSpiel notebook on Windows
    # (OpenSpiel is not available on Windows without manual install)
    if sys.platform == "win32" and "openspiel" in nb_path.name.lower():
        pytest.skip("OpenSpiel notebook requires OpenSpiel, which is not available on Windows")

    # GAMUT notebook requires Java and gamut.jar; outputs are pre-saved for docs builds
    if "gamut" in nb_path.name.lower():
        pytest.skip("GAMUT notebook requires Java and gamut.jar (see catalog documentation)")

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
