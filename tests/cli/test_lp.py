"""Tests that gambit-lp's switches produce the behavior they document."""

from pygambit.cli import lp


def test_default_output_is_exact_rational(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(lp.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,1/2,1/2,1/2,1/2"


def test_decimals_flag_switches_to_floating_point(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(lp.main, ["-q", "-d", "2"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout.strip() == "NE,0.50,0.50,0.50,0.50"


def test_detail_flag_switches_to_multiline_report(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(lp.main, ["-q", "-D"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert "NE," not in result.stdout
    assert "Strategy profile for player 1:" in result.stdout


def test_strategic_flag_switches_tree_game_to_strategy_representation(
    cli_runner, efg_small_tree_text
):
    # efg_small_tree_text is zero-sum (lp requires constant-sum) with one infoset
    # per player, so field counts alone won't distinguish -S; assert on the
    # representation via -D's report header instead, which names the game's
    # actual players/strategies only in strategy form.
    behavior_result = cli_runner.invoke(lp.main, ["-q", "-D"], input=efg_small_tree_text)
    strategic_result = cli_runner.invoke(lp.main, ["-q", "-D", "-S"], input=efg_small_tree_text)
    assert behavior_result.exit_code == 0
    assert strategic_result.exit_code == 0
    assert "Behavior profile for player 1:" in behavior_result.stdout
    assert "Strategy profile for player 1:" in strategic_result.stdout


def test_non_constant_sum_game_is_a_clean_error(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(lp.main, ["-q"], input=nfg_coordination_text)
    assert result.exit_code == 1
    assert result.stderr.startswith("Error:")
