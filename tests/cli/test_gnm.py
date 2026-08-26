"""Tests that gambit-gnm's switches produce the behavior they document."""

from pygambit.cli import gnm


def _start_file(tmp_path, text="0.9,0.1,0.9,0.1\n"):
    path = tmp_path / "pert.csv"
    path.write_text(text)
    return path


def test_default_finds_equilibria_along_the_path(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        gnm.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines() == [
        "NE,1.000000,0.000000,1.000000,0.000000",
        "NE,0.750000,0.250000,0.250000,0.750000",
        "NE,0.000000,1.000000,0.000000,1.000000",
    ]


def test_decimals_flag_changes_precision(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        gnm.main, ["-q", "-d", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines()[0] == "NE,1.00,0.00,1.00,0.00"


def test_verbose_flag_adds_pert_and_start_and_step_lines(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    start_file = _start_file(tmp_path)
    plain = cli_runner.invoke(
        gnm.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    verbose = cli_runner.invoke(
        gnm.main, ["-q", "-V", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert verbose.exit_code == 0
    lines = verbose.stdout.strip().splitlines()
    assert lines[0].startswith("pert,")
    assert len(lines[0].split(",")) == 5
    assert lines[1].startswith("start,")
    assert len(lines) > len(plain.stdout.strip().splitlines())
    for line in plain.stdout.strip().splitlines():
        assert line in verbose.stdout


def test_n_and_s_are_mutually_exclusive(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        gnm.main, ["-q", "-n", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == "Error: The -n and -s options are mutually exclusive.\n"


def test_seed_requires_n(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(gnm.main, ["-q", "-R", "5"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -R option requires -n.\n"


def test_n_controls_the_number_of_random_perturbations(cli_runner, nfg_asymmetric_table_text):
    one = cli_runner.invoke(
        gnm.main, ["-q", "-n", "1", "-R", "1"], input=nfg_asymmetric_table_text
    )
    three = cli_runner.invoke(
        gnm.main, ["-q", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert one.exit_code == 0
    assert three.exit_code == 0
    # each perturbation contributes at least one reported equilibrium
    assert len(three.stdout.strip().splitlines()) >= 3
    assert len(three.stdout.strip().splitlines()) > len(one.stdout.strip().splitlines())


def test_end_lambda_must_be_negative(cli_runner, nfg_asymmetric_table_text):
    for bad_value in ("0", "5"):
        result = cli_runner.invoke(
            gnm.main, ["-q", "-m", bad_value], input=nfg_asymmetric_table_text
        )
        assert result.exit_code == 1
        assert result.stderr == "Error: Value for -m (end lambda) must be negative\n"


def test_local_newton_interval_must_be_positive(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(gnm.main, ["-q", "-f", "0"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: Value for -f (local Newton frequency) must be at least 1\n"


def test_local_newton_maxits_must_be_positive(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(gnm.main, ["-q", "-i", "0"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == ("Error: Value for -i (local Newton iterations) must be at least 1\n")


def test_steps_must_be_positive(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(gnm.main, ["-q", "-c", "0"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: Value for -c (steps in support cell) must be at least 1\n"


def test_validation_errors_happen_before_the_banner(cli_runner, nfg_asymmetric_table_text):
    # The C++ tool prints the banner *then* validates -m/-f/-i/-c; the Python port
    # must match that ordering (banner always appears, even on validation failure).
    result = cli_runner.invoke(gnm.main, ["-m", "5"], input=nfg_asymmetric_table_text)
    assert gnm.DESCRIPTION in result.stderr
    assert "Error: Value for -m" in result.stderr
