"""Tests that gambit-enumpoly's switches produce the behavior they document."""

import pathlib

from pygambit.cli import enumpoly


def test_default_output(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enumpoly.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.500000,0.500000,0.500000,0.500000"


def test_decimals_flag_changes_precision(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enumpoly.main, ["-q", "-d", "2"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.50,0.50,0.50,0.50"


def test_verbose_flag_adds_support_lines(cli_runner, nfg_coordination_text):
    plain = cli_runner.invoke(enumpoly.main, ["-q"], input=nfg_coordination_text)
    verbose = cli_runner.invoke(enumpoly.main, ["-q", "-V"], input=nfg_coordination_text)
    assert plain.exit_code == 0
    assert verbose.exit_code == 0
    assert "candidate," not in plain.stdout
    assert "candidate," in verbose.stdout
    # every NE line in the plain run also appears in the verbose run
    for line in plain.stdout.strip().splitlines():
        assert line in verbose.stdout


def test_verbose_support_lines_mark_which_strategies_are_active(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(enumpoly.main, ["-q", "-V"], input=nfg_coordination_text)
    assert result.exit_code == 0
    candidates = [line for line in result.stdout.splitlines() if line.startswith("candidate,")]
    assert set(candidates) == {"candidate,10,10", "candidate,01,01", "candidate,11,11"}


def test_stop_after_limits_number_of_equilibria(cli_runner, nfg_coordination_text):
    unlimited = cli_runner.invoke(enumpoly.main, ["-q"], input=nfg_coordination_text)
    limited = cli_runner.invoke(enumpoly.main, ["-q", "-e", "1"], input=nfg_coordination_text)
    assert unlimited.exit_code == 0
    assert limited.exit_code == 0
    assert len(unlimited.stdout.strip().splitlines()) > 1
    assert len(limited.stdout.strip().splitlines()) == 1


def test_stop_after_rejects_non_positive_integer(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enumpoly.main, ["-q", "-e", "abc"], input=nfg_matching_pennies_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: -e argument must be a positive integer; got 'abc'.\n"


def test_strategic_flag_switches_tree_game_to_strategy_representation(
    cli_runner, efg_asymmetric_tree_text
):
    behavior_result = cli_runner.invoke(enumpoly.main, ["-q"], input=efg_asymmetric_tree_text)
    strategic_result = cli_runner.invoke(
        enumpoly.main, ["-q", "-S"], input=efg_asymmetric_tree_text
    )
    assert behavior_result.exit_code == 0
    assert strategic_result.exit_code == 0
    behavior_lines = behavior_result.stdout.strip().splitlines()
    strategic_lines = strategic_result.stdout.strip().splitlines()
    assert behavior_lines and strategic_lines
    assert all(len(line.split(",")) == 8 for line in behavior_lines)
    assert all(len(line.split(",")) == 9 for line in strategic_lines)


def test_imperfect_recall_is_rejected(cli_runner):
    imperfect_recall_efg = pathlib.Path("tests/test_games/gilboa_two_am_agents.efg").read_text()
    result = cli_runner.invoke(enumpoly.main, ["-q"], input=imperfect_recall_efg)
    assert result.exit_code == 1
    assert result.stderr == (
        "Error: Computing equilibria of games with imperfect recall is not supported.\n"
    )
