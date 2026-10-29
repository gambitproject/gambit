"""Tests of the command-line tools on a multi-type Bayesian action-graph game (BAGG).

The game has two players with two types each; each type has two actions.  Output
lists players, then types (information sets), then actions.
"""

import fractions
import pathlib

import pytest

import pygambit as gbt
from pygambit.cli import (
    enummixed,
    enumpoly,
    enumpure,
    gnm,
    hp,
    ipa,
    lcp,
    liap,
    logit,
    lp,
    simpdiv,
)
from pygambit.cli.common import render_profile_csv, render_profile_detail

BAGG_FILE = pathlib.Path(__file__).parent.parent / "test_games" / "bayesian_2p2t_shared_node.bagg"

SLOT_VALUES = 8
CSV_FIELDS = SLOT_VALUES + 1
START_FIELDS = "0.1,0.9,0.2,0.8,0.3,0.7,0.4,0.6"
PURE_START_FIELDS = "9/10,1/10,1/10,9/10,9/10,1/10,1/10,9/10"
EQUILIBRIUM_START_FIELDS = "1,0,0,1,1,0,0,1"


@pytest.fixture
def bagg_text() -> str:
    return BAGG_FILE.read_text()


def _fields(line: str) -> list[str]:
    return line.split(",")


def _write_start(tmp_path: pathlib.Path, fields: str) -> pathlib.Path:
    start_file = tmp_path / "start.csv"
    start_file.write_text(fields + "\n")
    return start_file


def _assert_valid_profile_line(line: str) -> None:
    values = [float(v) for v in _fields(line)[1:]]
    assert len(values) == SLOT_VALUES
    assert all(0 <= v <= 1 for v in values)
    for slot in range(0, SLOT_VALUES, 2):
        assert sum(values[slot : slot + 2]) == pytest.approx(1)


def test_csv_lists_players_then_types_then_actions(cli_runner, bagg_text, tmp_path):
    start_file = _write_start(tmp_path, START_FIELDS)
    result = cli_runner.invoke(
        liap.main, ["-q", "-V", "-d", "1", "-s", str(start_file)], input=bagg_text
    )
    assert result.exit_code == 0
    assert result.stdout.splitlines()[0] == "start,0.1,0.9,0.2,0.8,0.3,0.7,0.4,0.6"


def test_enumpure_prints_one_field_per_player_type_action(cli_runner, bagg_text):
    result = cli_runner.invoke(enumpure.main, ["-q"], input=bagg_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1,0,0,1,1,0,0,1"


def test_enumpoly_prints_the_same_pure_equilibrium(cli_runner, bagg_text):
    result = cli_runner.invoke(enumpoly.main, ["-q"], input=bagg_text)
    assert result.exit_code == 0
    assert (
        result.stdout.strip()
        == "NE,1.000000,0.000000,0.000000,1.000000,1.000000,0.000000,0.000000,1.000000"
    )


def test_detail_reports_probability_and_conditional_value_per_type_and_action(
    cli_runner, bagg_text
):
    result = cli_runner.invoke(enumpure.main, ["-q", "-D"], input=bagg_text)
    assert result.exit_code == 0
    rows = [line.split() for line in result.stdout.splitlines()]
    assert ["Behavior", "profile", "for", "1:"] in rows
    assert ["Behavior", "profile", "for", "2:"] in rows
    assert [row for row in rows if len(row) == 4 and row[0].isdigit()] == [
        ["1", "1", "1", "5/2"],
        ["1", "2", "0", "1/4"],
        ["2", "1", "0", "1/4"],
        ["2", "2", "1", "15/4"],
        ["1", "1", "1", "5/3"],
        ["1", "2", "0", "4/3"],
        ["2", "1", "0", "4/3"],
        ["2", "2", "1", "8/3"],
    ]


def test_detail_has_no_tree_only_sections(cli_runner, bagg_text):
    result = cli_runner.invoke(enumpure.main, ["-q", "-D"], input=bagg_text)
    assert "Belief" not in result.stdout
    assert "History" not in result.stdout
    assert "full histories" not in result.stdout


def test_liap_detail_reports_a_behavior_profile(cli_runner, bagg_text, tmp_path):
    start_file = _write_start(tmp_path, EQUILIBRIUM_START_FIELDS)
    result = cli_runner.invoke(liap.main, ["-q", "-D", "-s", str(start_file)], input=bagg_text)
    assert result.exit_code == 0, result.stderr
    assert "Behavior profile for 1:" in result.stdout
    assert "Behavior profile for 2:" in result.stdout
    assert "Strategy profile" not in result.stdout


def test_bagg_profile_is_classified_as_a_behavior_profile():
    game = gbt.read_bagg(BAGG_FILE)
    profile = game.mixed_behavior_profile(rational=True)
    csv_line = render_profile_csv(profile, "NE", 6)
    assert csv_line == "NE,1/2,1/2,1/2,1/2,1/2,1/2,1/2,1/2"
    assert render_profile_detail(profile, 6).startswith("Behavior profile for 1:")


@pytest.mark.parametrize("tool", [lcp, lp, enummixed])
def test_solvers_that_cannot_run_on_a_bagg_exit_cleanly(cli_runner, bagg_text, tool):
    result = cli_runner.invoke(tool.main, ["-q"], input=bagg_text)
    assert result.exit_code == 1
    assert result.stderr.startswith("Error: ")
    assert "Bayesian" in result.stderr
    assert "Traceback" not in result.stderr
    assert result.stdout == ""


@pytest.mark.parametrize("tool", [lcp, lp, enummixed])
def test_solvers_that_cannot_run_on_a_bagg_exit_cleanly_without_quiet(cli_runner, bagg_text, tool):
    result = cli_runner.invoke(tool.main, [], input=bagg_text)
    assert result.exit_code == 1
    assert "Bayesian" in result.stderr
    assert "Traceback" not in result.stderr
    assert "Traceback" not in result.stdout


def test_logit_prints_profiles_with_one_field_per_player_type_action(cli_runner, bagg_text):
    result = cli_runner.invoke(logit.main, ["-q", "-e"], input=bagg_text)
    assert result.exit_code == 0
    assert len(_fields(result.stdout.strip().splitlines()[-1])) == CSV_FIELDS


@pytest.mark.parametrize(
    ("tool", "echo_label"), [(liap, "start"), (simpdiv, "start"), (hp, "prior")]
)
def test_start_file_is_the_starting_profile_used(
    cli_runner, bagg_text, tmp_path, tool, echo_label
):
    start_file = _write_start(tmp_path, START_FIELDS)
    result = cli_runner.invoke(
        tool.main, ["-q", "-V", "-d", "1", "-s", str(start_file)], input=bagg_text
    )
    assert result.exit_code == 0, result.stderr
    assert result.stdout.splitlines()[0] == f"{echo_label},0.1,0.9,0.2,0.8,0.3,0.7,0.4,0.6"


def test_gnm_start_file_changes_the_perturbation(cli_runner, bagg_text, tmp_path):
    def perturbation(fields: str) -> str:
        start_file = _write_start(tmp_path, fields)
        result = cli_runner.invoke(gnm.main, ["-q", "-V", "-s", str(start_file)], input=bagg_text)
        assert result.exit_code == 0, result.stderr
        return result.stdout.splitlines()[0]

    first = perturbation(START_FIELDS)
    assert first.startswith("pert,")
    assert first != perturbation(PURE_START_FIELDS)


@pytest.mark.parametrize("tool", [liap, gnm, ipa, hp, simpdiv])
def test_tools_with_starting_profiles_accept_a_file_on_a_bagg(
    cli_runner, bagg_text, tmp_path, tool
):
    fields = EQUILIBRIUM_START_FIELDS if tool is liap else PURE_START_FIELDS
    start_file = _write_start(tmp_path, fields)
    result = cli_runner.invoke(tool.main, ["-q", "-s", str(start_file)], input=bagg_text)
    assert result.exit_code == 0, result.stderr
    lines = result.stdout.strip().splitlines()
    assert lines
    assert all(len(_fields(line)) == CSV_FIELDS for line in lines)


@pytest.mark.parametrize("tool", [liap, gnm, hp])
def test_random_starts_are_valid_profiles(cli_runner, bagg_text, tool):
    result = cli_runner.invoke(tool.main, ["-q", "-V", "-n", "2", "-R", "1"], input=bagg_text)
    assert result.exit_code == 0, result.stderr
    starts = [line for line in result.stdout.splitlines() if line.startswith(("start,", "prior,"))]
    assert starts
    for line in starts:
        _assert_valid_profile_line(line)


def test_ipa_random_starts_work_on_a_bagg(cli_runner, bagg_text):
    result = cli_runner.invoke(ipa.main, ["-q", "-n", "2", "-R", "1"], input=bagg_text)
    assert result.exit_code == 0, result.stderr
    assert all(len(_fields(line)) == CSV_FIELDS for line in result.stdout.strip().splitlines())


def test_simpdiv_default_start_is_each_types_first_action(cli_runner, bagg_text):
    result = cli_runner.invoke(simpdiv.main, ["-q", "-V"], input=bagg_text)
    assert result.exit_code == 0, result.stderr
    assert result.stdout.splitlines()[0] == "start,1,0,1,0,1,0,1,0"


def test_simpdiv_random_starts_are_valid_profiles(cli_runner, bagg_text):
    result = cli_runner.invoke(
        simpdiv.main, ["-q", "-V", "-r", "4", "-n", "2", "-R", "1"], input=bagg_text
    )
    assert result.exit_code == 0, result.stderr
    starts = [line for line in result.stdout.splitlines() if line.startswith("start,")]
    assert len(starts) == 2
    for line in starts:
        values = [fractions.Fraction(v) for v in _fields(line)[1:]]
        assert len(values) == SLOT_VALUES
        for slot in range(0, SLOT_VALUES, 2):
            assert sum(values[slot : slot + 2]) == 1
            assert all(v * 4 == int(v * 4) for v in values[slot : slot + 2])


def test_start_file_of_the_wrong_length_is_a_clean_error(cli_runner, bagg_text, tmp_path):
    start_file = _write_start(tmp_path, "1/2,1/2")
    result = cli_runner.invoke(liap.main, ["-q", "-s", str(start_file)], input=bagg_text)
    assert result.exit_code == 1
    assert result.stderr.startswith("Error:")
