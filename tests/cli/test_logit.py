"""Tests that gambit-logit's switches produce the behavior they document."""

import pathlib

from pygambit.cli import logit


def test_default_streams_the_whole_branch_then_the_equilibrium(
    cli_runner, nfg_asymmetric_table_text
):
    result = cli_runner.invoke(logit.main, ["-q"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    lines = result.stdout.strip().splitlines()
    assert len(lines) > 2
    assert lines[0].startswith("0.000000,")
    assert lines[-1].startswith("NE,")


def test_terminal_only_flag_suppresses_the_trace(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(logit.main, ["-q", "-e"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    lines = result.stdout.strip().splitlines()
    assert len(lines) == 1
    assert lines[0].startswith("NE,")


def test_decimals_flag_controls_lambda_label_precision(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(logit.main, ["-q", "-d", "2"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    first_label = result.stdout.splitlines()[0].split(",")[0]
    assert first_label == "0.00"


def test_profile_values_use_general_not_fixed_formatting(cli_runner, nfg_asymmetric_table_text):
    # Values are trimmed of trailing zeros (0.51, not 0.510000), unlike the other
    # tools' fixed-decimal CSV renderer.
    result = cli_runner.invoke(logit.main, ["-q"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    assert "0.510000" not in result.stdout
    assert any(
        "," in line and not line.split(",")[1].endswith("00000")
        for line in result.stdout.splitlines()[1:5]
    )


def test_target_lambda_reports_only_the_requested_points(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(
        logit.main, ["-q", "-l", "1", "-l", "5"], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    lines = result.stdout.strip().splitlines()
    assert len(lines) >= 2
    assert lines[-2].startswith("1.000000,") or lines[-1].startswith("1.000000,")
    labels = [line.split(",")[0] for line in lines]
    assert "1.000000" in labels
    assert "5.000000" in labels
    # -l never reports a plain "NE" summary line -- every row is lambda-labeled.
    assert "NE" not in labels


def test_target_lambda_with_terminal_only_suppresses_the_trace_to_each_point(
    cli_runner, nfg_asymmetric_table_text
):
    full = cli_runner.invoke(logit.main, ["-q", "-l", "1"], input=nfg_asymmetric_table_text)
    terminal_only = cli_runner.invoke(
        logit.main, ["-q", "-e", "-l", "1"], input=nfg_asymmetric_table_text
    )
    assert full.exit_code == 0
    assert terminal_only.exit_code == 0
    assert len(terminal_only.stdout.strip().splitlines()) == 1
    assert len(full.stdout.strip().splitlines()) > 1


def test_strategic_flag_switches_tree_game_to_strategy_representation(
    cli_runner, efg_asymmetric_tree_text
):
    behavior_result = cli_runner.invoke(logit.main, ["-q", "-e"], input=efg_asymmetric_tree_text)
    strategic_result = cli_runner.invoke(
        logit.main, ["-q", "-e", "-S"], input=efg_asymmetric_tree_text
    )
    assert behavior_result.exit_code == 0
    assert strategic_result.exit_code == 0
    assert len(behavior_result.stdout.strip().split(",")) == 8
    assert len(strategic_result.stdout.strip().split(",")) == 9


def test_maximum_likelihood_estimate_appends_log_likelihood(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    frequencies = tmp_path / "freq.csv"
    frequencies.write_text("10,5,3,7\n")
    result = cli_runner.invoke(
        logit.main, ["-q", "-S", "-e", "-L", str(frequencies)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    fields = result.stdout.strip().split(",")
    # lambda + 4 profile values + a trailing (non-positive) log-likelihood
    assert len(fields) == 6
    assert float(fields[-1]) <= 0.0


def test_maximum_likelihood_estimate_requires_readable_frequency_file(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    missing = tmp_path / "does_not_exist.csv"
    result = cli_runner.invoke(
        logit.main, ["-q", "-S", "-L", str(missing)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == f"Error: Error reading strategy frequencies from '{missing}'.\n"


def test_imperfect_recall_is_rejected(cli_runner):
    imperfect_recall_efg = pathlib.Path("tests/test_games/gilboa_two_am_agents.efg").read_text()
    result = cli_runner.invoke(logit.main, ["-q"], input=imperfect_recall_efg)
    assert result.exit_code == 1
    assert result.stderr == (
        "Error: Computing equilibria of games with imperfect recall is not supported.\n"
    )
