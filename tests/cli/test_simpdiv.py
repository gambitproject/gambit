"""Tests that gambit-simpdiv's switches produce the behavior they document."""

from pygambit.cli import simpdiv


def test_default_output_is_exact_rational(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(simpdiv.main, ["-q"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1,0,1,0"


def test_decimals_flag_switches_to_floating_point(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(simpdiv.main, ["-q", "-d", "3"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1.000,0.000,1.000,0.000"


def test_verbose_flag_adds_start_and_refinement_lines(cli_runner, nfg_asymmetric_table_text):
    plain = cli_runner.invoke(simpdiv.main, ["-q"], input=nfg_asymmetric_table_text)
    verbose = cli_runner.invoke(simpdiv.main, ["-q", "-V"], input=nfg_asymmetric_table_text)
    assert verbose.exit_code == 0
    lines = verbose.stdout.strip().splitlines()
    assert lines[0].startswith("start,")
    assert lines[-1] == plain.stdout.strip()
    assert len(lines) > 2


def test_r_and_s_are_mutually_exclusive(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("1/2,1/2,1/2,1/2\n")
    result = cli_runner.invoke(
        simpdiv.main, ["-q", "-r", "5", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == "Error: The -r and -s options are mutually exclusive.\n"


def test_n_requires_r(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(simpdiv.main, ["-q", "-n", "3"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -n option requires -r.\n"


def test_seed_requires_n(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(simpdiv.main, ["-q", "-R", "5"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -R option requires -n.\n"


def test_n_controls_the_number_of_random_starts(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(
        simpdiv.main, ["-q", "-r", "6", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert len(result.stdout.strip().splitlines()) == 3


def test_starting_points_file_is_used_exactly(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("1/2,1/2,1/2,1/2\n")
    result = cli_runner.invoke(
        simpdiv.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1,0,1,0"


def test_maxregret_accepts_a_rational_string(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(simpdiv.main, ["-q", "-m", "1/10"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1,0,1,0"
