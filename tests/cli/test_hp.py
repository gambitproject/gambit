"""Tests that gambit-hp's switches produce the behavior they document."""

from pygambit.cli import hp


def _start_file(tmp_path, text="0.9,0.1,0.9,0.1\n"):
    path = tmp_path / "prior.csv"
    path.write_text(text)
    return path


def test_default_reports_one_equilibrium_per_prior(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        hp.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines() == ["NE,1.000000,0.000000,1.000000,0.000000"]


def test_starting_file_accepts_exact_fractions(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path, "9/10,1/10,9/10,1/10\n")
    result = cli_runner.invoke(
        hp.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines() == ["NE,1.000000,0.000000,1.000000,0.000000"]


def test_decimals_flag_changes_precision(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        hp.main, ["-q", "-d", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines() == ["NE,1.00,0.00,1.00,0.00"]


def test_verbose_flag_adds_prior_and_step_lines(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    plain = cli_runner.invoke(
        hp.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    verbose = cli_runner.invoke(
        hp.main, ["-q", "-V", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert verbose.exit_code == 0
    lines = verbose.stdout.strip().splitlines()
    # The first line reports the (generally mixed) prior itself; the second is the
    # t=0 step, the best response to that prior, which is pure by construction.
    assert lines[0] == "prior,0.900000,0.100000,0.900000,0.100000"
    assert lines[1].startswith("0,")
    assert len(lines[1].split(",")) == 5
    assert len(lines) > len(plain.stdout.strip().splitlines())
    for line in plain.stdout.strip().splitlines():
        assert line in verbose.stdout


def test_n_and_s_are_mutually_exclusive(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        hp.main, ["-q", "-n", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == "Error: The -n and -s options are mutually exclusive.\n"


def test_seed_requires_n(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(hp.main, ["-q", "-R", "5"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -R option requires -n.\n"


def test_n_controls_the_number_of_priors_reported(cli_runner, nfg_asymmetric_table_text):
    one = cli_runner.invoke(hp.main, ["-q", "-n", "1", "-R", "1"], input=nfg_asymmetric_table_text)
    three = cli_runner.invoke(
        hp.main, ["-q", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert one.exit_code == 0
    assert three.exit_code == 0
    # Each prior contributes exactly one reported equilibrium.
    assert len(one.stdout.strip().splitlines()) == 1
    assert len(three.stdout.strip().splitlines()) == 3


def test_degenerate_prior_is_a_clean_error(cli_runner, nfg_coordination_text, tmp_path):
    # A prior exactly tied between two best responses has no unique best response
    # at t=0, which HPStrategySolve rejects rather than picking one arbitrarily.
    start_file = _start_file(tmp_path, "0.5,0.5,0.5,0.5\n")
    result = cli_runner.invoke(
        hp.main, ["-q", "-s", str(start_file)], input=nfg_coordination_text
    )
    assert result.exit_code == 1
    assert "Multiple best responses" in result.stderr
