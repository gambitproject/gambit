"""Subprocess-based smoke tests for the installed `gambit-*` console-script entry
points.

The rest of the CLI test suite drives each tool's `main()` in-process via
`click.testing.CliRunner`, which is fast but never executes the actual
`project.scripts` entry point, a real process exit code, or argv/stdin as the OS
would deliver them. These tests are the thin subprocess-based layer on top of
that: one process-level check per tool that the installed script runs, reads a
game from stdin, and produces the expected first line of output.
"""

import subprocess
import sys
from pathlib import Path

import pytest

_BIN_DIR = Path(sys.executable).parent

# (tool name, fixture providing its input game, expected first output line)
TOOLS_AND_EXPECTED_FIRST_LINE = [
    ("gambit-enumpure", "nfg_coordination_text", "NE,1,0,1,0"),
    ("gambit-enummixed", "nfg_matching_pennies_text", "NE,1/2,1/2,1/2,1/2"),
    ("gambit-lcp", "nfg_matching_pennies_text", "NE,1/2,1/2,1/2,1/2"),
    ("gambit-lp", "nfg_matching_pennies_text", "NE,1/2,1/2,1/2,1/2"),
    ("gambit-enumpoly", "nfg_matching_pennies_text", "NE,0.500000,0.500000,0.500000,0.500000"),
]


def _script_path(name: str) -> Path:
    path = _BIN_DIR / name
    if not path.exists():
        pytest.skip(f"{name} is not installed alongside {sys.executable}")
    return path


@pytest.mark.parametrize(
    "tool_name,game_fixture,expected_first_line",
    TOOLS_AND_EXPECTED_FIRST_LINE,
    ids=[t[0] for t in TOOLS_AND_EXPECTED_FIRST_LINE],
)
def test_installed_script_reads_stdin_and_runs(
    tool_name, game_fixture, expected_first_line, request
):
    script = _script_path(tool_name)
    game_text = request.getfixturevalue(game_fixture)
    proc = subprocess.run(
        [str(script), "-q"],
        input=game_text,
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert proc.returncode == 0, proc.stderr
    assert proc.stdout.strip().splitlines()[0] == expected_first_line


def test_installed_script_exits_nonzero_on_bad_option():
    script = _script_path("gambit-enumpure")
    proc = subprocess.run(
        [str(script), "--this-flag-does-not-exist"],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert proc.returncode != 0


def test_installed_script_help_exits_zero():
    script = _script_path("gambit-enumpure")
    proc = subprocess.run([str(script), "-h"], capture_output=True, text=True, timeout=30)
    assert proc.returncode == 0
    assert "enumerating extreme points" not in proc.stdout  # sanity: this is enumpure's help
    assert "pure strategies" in proc.stdout
