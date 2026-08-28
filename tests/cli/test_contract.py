"""Tests for behavior every Gambit CLI tool is supposed to share: the banner,
``-q``/``-h``/``-v``, reading a game from a file or from standard input, and
reporting a missing file the way the C++ tools' ``perror()`` call does.

Each tool's tool-specific options are tested in their own test module.
"""

import pytest

from pygambit.cli import (
    enummixed,
    enumpoly,
    enumpure,
    gnm,
    ipa,
    lcp,
    liap,
    logit,
    lp,
    simpdiv,
)

TOOLS = [
    enumpure,
    logit,
    enummixed,
    lcp,
    lp,
    liap,
    simpdiv,
    gnm,
    ipa,
    enumpoly,
]


@pytest.fixture(params=TOOLS, ids=[t.PROG_NAME for t in TOOLS])
def tool(request):
    return request.param


def test_quiet_suppresses_banner(cli_runner, tool, nfg_matching_pennies_text):
    result = cli_runner.invoke(tool.main, ["-q"], input=nfg_matching_pennies_text)
    assert tool.DESCRIPTION not in result.stderr
    assert result.exit_code == 0


def test_banner_shown_by_default(cli_runner, tool, nfg_matching_pennies_text):
    result = cli_runner.invoke(tool.main, [], input=nfg_matching_pennies_text)
    assert tool.DESCRIPTION in result.stderr
    assert "Copyright" in result.stderr
    assert result.exit_code == 0


def test_version_prints_banner_and_exits_without_reading_a_game(cli_runner, tool):
    result = cli_runner.invoke(tool.main, ["-v"])
    assert result.exit_code == 0
    assert tool.DESCRIPTION in result.stderr
    assert result.stdout == ""


def test_help_exits_zero(cli_runner, tool):
    result = cli_runner.invoke(tool.main, ["-h"])
    assert result.exit_code == 0
    result = cli_runner.invoke(tool.main, ["--help"])
    assert result.exit_code == 0


def test_missing_file_exits_1_and_names_the_program(cli_runner, tool, tmp_path):
    missing = tmp_path / "does_not_exist.nfg"
    result = cli_runner.invoke(tool.main, ["-q", str(missing)])
    assert result.exit_code == 1
    assert result.stderr == f"{tool.PROG_NAME}: {missing}: No such file or directory\n"
    assert result.stdout == ""


def test_reads_game_from_stdin_when_no_file_given(cli_runner, tool, nfg_matching_pennies_text):
    result = cli_runner.invoke(tool.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0


def test_reads_game_from_file_argument(cli_runner, tool, tmp_path, nfg_matching_pennies_text):
    path = tmp_path / "game.nfg"
    path.write_text(nfg_matching_pennies_text)
    result = cli_runner.invoke(tool.main, ["-q", str(path)])
    assert result.exit_code == 0


def test_empty_game_file_is_a_clean_error(cli_runner, tool, tmp_path):
    path = tmp_path / "empty.nfg"
    path.write_text("")
    result = cli_runner.invoke(tool.main, ["-q", str(path)])
    assert result.exit_code == 1
    assert result.stderr == "Error: Empty file or string provided\n"


def test_unknown_option_is_a_usage_error(cli_runner, tool, nfg_matching_pennies_text):
    result = cli_runner.invoke(
        tool.main, ["--this-flag-does-not-exist"], input=nfg_matching_pennies_text
    )
    assert result.exit_code != 0
