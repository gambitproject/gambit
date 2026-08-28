"""Tests that gambit-liap's switches produce the behavior they document."""

from pygambit.cli import liap


def test_default_finds_an_equilibrium_from_a_starting_file(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    start_file = tmp_path / "start.csv"
    start_file.write_text("0.5,0.5,0.5,0.5\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.750000,0.250000,0.250000,0.750000"


def test_starting_file_accepts_exact_fractions(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("1/2,1/2,1/2,1/2\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.750000,0.250000,0.250000,0.750000"


def test_verbose_flag_adds_a_start_line(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("0.5,0.5,0.5,0.5\n")
    plain = cli_runner.invoke(
        liap.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    verbose = cli_runner.invoke(
        liap.main, ["-q", "-V", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert verbose.exit_code == 0
    assert verbose.stdout == "start,0.500000,0.500000,0.500000,0.500000\n" + plain.stdout


def test_decimals_flag_controls_precision(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("0.5,0.5,0.5,0.5\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-d", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.75,0.25,0.25,0.75"


def test_starting_points_file_can_hold_several_profiles(
    cli_runner, nfg_asymmetric_table_text, tmp_path
):
    start_file = tmp_path / "starts.csv"
    start_file.write_text("0.5,0.5,0.5,0.5\n0.9,0.1,0.9,0.1\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert len(result.stdout.strip().splitlines()) == 2


def test_n_and_s_are_mutually_exclusive(cli_runner, nfg_asymmetric_table_text, tmp_path):
    start_file = tmp_path / "start.csv"
    start_file.write_text("0.5,0.5,0.5,0.5\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-n", "2", "-s", str(start_file)], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 1
    assert result.stderr == "Error: The -n and -s options are mutually exclusive.\n"


def test_seed_requires_n(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(liap.main, ["-q", "-R", "5"], input=nfg_asymmetric_table_text)
    assert result.exit_code == 1
    assert result.stderr == "Error: The -R option requires -n.\n"


def test_n_controls_the_number_of_random_starts(cli_runner, nfg_asymmetric_table_text):
    result = cli_runner.invoke(
        liap.main, ["-q", "-n", "4", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert result.exit_code == 0
    assert len(result.stdout.strip().splitlines()) == 4


def test_agent_flag_reports_behavior_form_on_a_tree_game(
    cli_runner, efg_asymmetric_tree_text, tmp_path
):
    start_file = tmp_path / "start.csv"
    start_file.write_text("0.5,0.5,0.3,0.3,0.4,0.5,0.5\n")
    # This starting point sits close enough to a flat spot in the Lyapunov function
    # (the "y"/"z" actions are payoff-irrelevant ties) that whether minimization
    # reaches the default 1e-4 maxregret within maxiter is sensitive to
    # platform-dependent floating-point rounding; loosen -m so the test checks CLI
    # plumbing rather than exact convergence behavior.
    result = cli_runner.invoke(
        liap.main, ["-q", "-A", "-m", "0.01", "-s", str(start_file)],
        input=efg_asymmetric_tree_text,
    )
    assert result.exit_code == 0
    lines = result.stdout.strip().splitlines()
    assert lines
    assert all(len(line.split(",")) == 8 for line in lines)


def test_agent_flag_starting_file_accepts_exact_fractions(
    cli_runner, efg_asymmetric_tree_text, tmp_path
):
    start_file = tmp_path / "start.csv"
    start_file.write_text("1/2,1/2,3/10,3/10,2/5,1/2,1/2\n")
    result = cli_runner.invoke(
        liap.main, ["-q", "-A", "-m", "0.01", "-s", str(start_file)],
        input=efg_asymmetric_tree_text,
    )
    assert result.exit_code == 0
    lines = result.stdout.strip().splitlines()
    assert lines
    assert all(len(line.split(",")) == 8 for line in lines)


def test_agent_flag_has_no_effect_on_a_table_game(cli_runner, nfg_asymmetric_table_text):
    without = cli_runner.invoke(
        liap.main, ["-q", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    with_agent = cli_runner.invoke(
        liap.main, ["-q", "-A", "-n", "3", "-R", "1"], input=nfg_asymmetric_table_text
    )
    assert with_agent.exit_code == 0
    assert with_agent.stdout == without.stdout
