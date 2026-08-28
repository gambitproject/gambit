"""Tests that gambit-ipa's switches produce the behavior they document."""

from pygambit.cli import ipa


def _start_file(tmp_path, text="0.9,0.1,0.9,0.1\n"):
    path = tmp_path / "pert.csv"
    path.write_text(text)
    return path


def test_default_finds_an_equilibrium(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        ipa.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.750000,0.250000,0.250000,0.750000"


def test_starting_file_accepts_exact_fractions(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path, "9/10,1/10,9/10,1/10\n")
    result = cli_runner.invoke(
        ipa.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.750000,0.250000,0.250000,0.750000"


def test_decimals_flag_changes_precision(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        ipa.main, ["-q", "-d", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.75,0.25,0.25,0.75"


def test_verbose_flag_adds_iteration_labeled_lines(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    start_file = _start_file(tmp_path)
    plain = cli_runner.invoke(
        ipa.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    verbose = cli_runner.invoke(
        ipa.main, ["-q", "-V", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert verbose.exit_code == 0
    lines = verbose.stdout.strip().splitlines()
    assert lines[-1] == plain.stdout.strip()
    assert all(line.startswith("iter-") for line in lines[:-1])
    # iteration numbers appear in increasing order
    iterations = [int(line.split(",")[0].removeprefix("iter-")) for line in lines[:-1]]
    assert iterations == sorted(iterations)


def test_n_and_s_are_mutually_exclusive(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = _start_file(tmp_path)
    result = cli_runner.invoke(
        ipa.main, ["-q", "-n", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == "Error: The -n and -s options are mutually exclusive.\n"


def test_seed_requires_n(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(ipa.main, ["-q", "-R", "5"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -R option requires -n.\n"


def test_n_controls_the_number_of_random_perturbations(cli_runner, nfg_asymmetric_table_text):
    one = cli_runner.invoke(
        ipa.main, ["-q", "-n", "1", "-R", "1"], input=nfg_asymmetric_table_text
    )
    three = cli_runner.invoke(
        ipa.main, ["-q", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert one.exit_code == 0
    assert three.exit_code == 0
    assert len(one.stdout.strip().splitlines()) == 1
    assert len(three.stdout.strip().splitlines()) == 3
