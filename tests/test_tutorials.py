import contextlib
import functools
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


@functools.lru_cache(maxsize=1)
def _available_kernels():
    """Return the set of Jupyter kernel names installed on this machine."""
    from jupyter_client.kernelspec import KernelSpecManager

    return set(KernelSpecManager().find_kernel_specs())


# Discover notebooks at import time so pytest can parametrize them.
_NOTEBOOKS = _find_tutorial_notebooks()


@pytest.mark.tutorials
@pytest.mark.parametrize("nb_path", _NOTEBOOKS, ids=[p.name for p in _NOTEBOOKS])
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

    # Notebooks for other systems (e.g. the SageMath tutorials, which declare the
    # "sagemath" kernel) can only be executed where that kernel is installed.
    # Their outputs are pre-saved for docs builds, so skip rather than fail.
    if kernel_name and kernel_name not in _available_kernels():
        pytest.skip(
            f"Notebook {nb_path.name} needs the {kernel_name!r} Jupyter kernel, "
            "which is not installed"
        )

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


@pytest.mark.tutorials
@pytest.mark.parametrize("nb_path", _NOTEBOOKS, ids=[p.name for p in _NOTEBOOKS])
def test_prerendered_notebooks_keep_outputs(nb_path):
    """Notebooks the docs build cannot execute must ship their stored outputs.

    ``nbsphinx`` only executes notebooks that have no stored outputs, so tutorials that
    cannot run on Read the Docs (no Java for GAMUT, no SageMath for the Sage tutorials)
    opt out with ``"nbsphinx": {"execute": "never"}`` and commit their outputs instead.
    If those outputs are ever stripped, the notebook silently renders as bare code with
    no results at all, which is easy to miss when reviewing a docs build.
    """
    nb = nbformat.read(str(nb_path), as_version=4)
    kernel_name = nb.metadata.get("kernelspec", {}).get("name")
    prerendered = nb.metadata.get("nbsphinx", {}).get("execute") == "never"

    # The docs build only has a plain Python kernel, so anything else must opt out.
    if kernel_name and kernel_name != "python3":
        assert prerendered, (
            f"{nb_path.name} declares the {kernel_name!r} kernel, which the docs build "
            'does not have. Set notebook metadata "nbsphinx": {"execute": "never"} and '
            "commit the notebook with its outputs."
        )

    if not prerendered:
        pytest.skip(f"{nb_path.name} is executed during the docs build")

    code_cells = [cell for cell in nb.cells if cell.cell_type == "code"]
    assert any(cell.get("outputs") for cell in code_cells), (
        f"{nb_path.name} sets nbsphinx execute='never' but has no stored outputs, so it "
        "would render without any results. Re-run the notebook and commit it."
    )
