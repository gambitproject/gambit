"""Tests that gambit-enummixed's switches produce the behavior they document."""

from pygambit.cli import enummixed


def test_default_output_is_exact_rational(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enummixed.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1/2,1/2,1/2,1/2"


def test_decimals_flag_switches_to_floating_point(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enummixed.main, ["-q", "-d", "3"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.500,0.500,0.500,0.500"


def test_decimals_digit_count_is_honored(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enummixed.main, ["-q", "-d", "1"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.5,0.5,0.5,0.5"


def test_cliques_flag_appends_convex_labeled_lines(cli_runner, nfg_matching_pennies_text):
    without = cli_runner.invoke(enummixed.main, ["-q"], input=nfg_matching_pennies_text)
    with_cliques = cli_runner.invoke(enummixed.main, ["-q", "-c"], input=nfg_matching_pennies_text)
    assert with_cliques.exit_code == 0
    assert with_cliques.stdout.startswith(without.stdout)
    extra = with_cliques.stdout[len(without.stdout) :]
    assert extra.strip().splitlines() == ["convex-1,1/2,1/2,1/2,1/2"]


def test_cliques_flag_numbers_each_equilibrium_class(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(enummixed.main, ["-q", "-c"], input=nfg_coordination_text)
    assert result.exit_code == 0
    labels = [line.split(",")[0] for line in result.stdout.strip().splitlines()]
    assert labels == ["NE", "NE", "NE", "convex-1", "convex-2", "convex-3"]


def test_without_cliques_flag_no_convex_lines_appear(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(enummixed.main, ["-q"], input=nfg_coordination_text)
    assert result.exit_code == 0
    assert "convex" not in result.stdout
