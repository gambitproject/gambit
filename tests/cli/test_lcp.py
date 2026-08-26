"""Tests that gambit-lcp's switches produce the behavior they document."""

from pygambit.cli import lcp


def test_default_output_is_exact_rational(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(lcp.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1/2,1/2,1/2,1/2"


def test_decimals_flag_switches_to_floating_point(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(lcp.main, ["-q", "-d", "2"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.50,0.50,0.50,0.50"


def test_detail_flag_switches_to_multiline_report(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(lcp.main, ["-q", "-D"], input=nfg_coordination_text)
    assert result.exit_code == 0
    assert "NE," not in result.stdout
    assert "Strategy profile for player 1:" in result.stdout


def test_strategic_flag_switches_tree_game_to_strategy_representation(
    cli_runner, efg_asymmetric_tree_text
):
    behavior_result = cli_runner.invoke(lcp.main, ["-q"], input=efg_asymmetric_tree_text)
    strategic_result = cli_runner.invoke(lcp.main, ["-q", "-S"], input=efg_asymmetric_tree_text)
    assert behavior_result.exit_code == 0
    assert strategic_result.exit_code == 0
    behavior_lines = behavior_result.stdout.strip().splitlines()
    strategic_lines = strategic_result.stdout.strip().splitlines()
    assert behavior_lines and strategic_lines
    assert all(len(line.split(",")) == 8 for line in behavior_lines)
    assert all(len(line.split(",")) == 9 for line in strategic_lines)


def test_stop_after_limits_number_of_equilibria(cli_runner, nfg_coordination_text):
    unlimited = cli_runner.invoke(lcp.main, ["-q"], input=nfg_coordination_text)
    limited = cli_runner.invoke(lcp.main, ["-q", "-e", "1"], input=nfg_coordination_text)
    assert unlimited.exit_code == 0
    assert limited.exit_code == 0
    assert len(unlimited.stdout.strip().splitlines()) > 1
    assert len(limited.stdout.strip().splitlines()) == 1
    assert limited.stdout.strip() == unlimited.stdout.strip().splitlines()[0]


def test_stop_after_rejects_non_positive_integer(cli_runner, nfg_matching_pennies_text):
    for bad_value in ("0", "-1", "abc", "1.5"):
        result = cli_runner.invoke(
            lcp.main, ["-q", "-e", bad_value], input=nfg_matching_pennies_text
        )
        assert result.exit_code == 1
        assert result.stderr == (
            f"Error: -e argument must be a positive integer; got '{bad_value}'.\n"
        )


def test_stop_after_and_max_depth_are_silently_ignored_on_tree_game_without_strategic(
    cli_runner, efg_asymmetric_tree_text
):
    # lcp_solve() itself raises if stop_after/max_depth are given while solving on the
    # tree representation; the CLI must not forward them in that case, matching the
    # C++ tool, which simply never wires -e/-r into the tree-solving code path.
    plain = cli_runner.invoke(lcp.main, ["-q"], input=efg_asymmetric_tree_text)
    with_e_and_r = cli_runner.invoke(
        lcp.main, ["-q", "-e", "1", "-r", "5"], input=efg_asymmetric_tree_text
    )
    assert with_e_and_r.exit_code == 0
    assert with_e_and_r.stdout == plain.stdout
