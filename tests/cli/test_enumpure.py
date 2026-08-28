"""Tests that gambit-enumpure's switches produce the behavior they document.

Rendering (CSV vs -D detail) is checked end to end on real, tiny games. Which
representation gets solved -- the -A/-S/is_tree logic -- is checked by spying on
the pygambit.nash functions the tool is supposed to call, since that logic is
about argument wiring, not about the numerics of any particular game.
"""

import pygambit as gbt
from pygambit.cli import enumpure


def test_default_finds_pure_equilibria(cli_runner, nfg_coordination_text):
    result = cli_runner.invoke(enumpure.main, ["-q"], input=nfg_coordination_text)
    assert result.exit_code == 0
    assert result.stdout.strip().splitlines() == ["NE,1,0,1,0", "NE,0,1,0,1"]


def test_no_pure_equilibria_is_empty_output(cli_runner, nfg_matching_pennies_text):
    result = cli_runner.invoke(enumpure.main, ["-q"], input=nfg_matching_pennies_text)
    assert result.exit_code == 0
    assert result.stdout == ""


def test_detail_flag_switches_to_multiline_report(cli_runner, nfg_coordination_text):
    csv_result = cli_runner.invoke(enumpure.main, ["-q"], input=nfg_coordination_text)
    detail_result = cli_runner.invoke(enumpure.main, ["-q", "-D"], input=nfg_coordination_text)
    assert detail_result.exit_code == 0
    assert "NE," not in detail_result.stdout
    assert "Strategy profile for player 1:" in detail_result.stdout
    assert "Strategy profile for player 2:" in detail_result.stdout
    assert len(detail_result.stdout.splitlines()) > len(csv_result.stdout.splitlines())


class TestRepresentationWiring:
    """-A only takes effect on a tree game; -S only converts *display*, not which
    solver runs. Verified by spying on the two entry points rather than needing a
    game whose behavior/strategy representations happen to render differently.
    """

    def test_table_game_always_uses_strategy_solve(
        self, cli_runner, monkeypatch, nfg_matching_pennies_text
    ):
        called = []
        monkeypatch.setattr(
            gbt.nash, "enumpure_solve", lambda *a, **k: called.append("strategy") or None
        )
        monkeypatch.setattr(
            gbt.nash, "enumpure_agent_solve", lambda *a, **k: called.append("agent") or None
        )
        for flags in ([], ["-A"], ["-S"], ["-A", "-S"]):
            called.clear()
            result = cli_runner.invoke(
                enumpure.main, ["-q", *flags], input=nfg_matching_pennies_text
            )
            assert result.exit_code == 0
            assert called == ["strategy"]

    def test_tree_game_agent_flag_selects_agent_solve(
        self, cli_runner, monkeypatch, efg_small_tree_text
    ):
        called = []
        monkeypatch.setattr(
            gbt.nash, "enumpure_solve", lambda *a, **k: called.append("strategy") or None
        )
        monkeypatch.setattr(
            gbt.nash, "enumpure_agent_solve", lambda *a, **k: called.append("agent") or None
        )
        result = cli_runner.invoke(enumpure.main, ["-q", "-A"], input=efg_small_tree_text)
        assert result.exit_code == 0
        assert called == ["agent"]

    def test_tree_game_without_agent_flag_selects_strategy_solve(
        self, cli_runner, monkeypatch, efg_small_tree_text
    ):
        called = []
        monkeypatch.setattr(
            gbt.nash, "enumpure_solve", lambda *a, **k: called.append("strategy") or None
        )
        monkeypatch.setattr(
            gbt.nash, "enumpure_agent_solve", lambda *a, **k: called.append("agent") or None
        )
        result = cli_runner.invoke(enumpure.main, ["-q"], input=efg_small_tree_text)
        assert result.exit_code == 0
        assert called == ["strategy"]

    def test_strategic_flag_converts_agent_result_for_display(
        self, cli_runner, efg_asymmetric_tree_text
    ):
        """-A alone reports the behavior-form profile (7 values, per the fixture's
        docstring); adding -S should convert each equilibrium found to its
        strategic form (8 values) before printing it, not just relabel the same
        7 numbers.
        """
        agent_result = cli_runner.invoke(
            enumpure.main, ["-q", "-A"], input=efg_asymmetric_tree_text
        )
        agent_strategic_result = cli_runner.invoke(
            enumpure.main, ["-q", "-A", "-S"], input=efg_asymmetric_tree_text
        )
        assert agent_result.exit_code == 0
        assert agent_strategic_result.exit_code == 0
        agent_lines = agent_result.stdout.strip().splitlines()
        strategic_lines = agent_strategic_result.stdout.strip().splitlines()
        assert agent_lines and strategic_lines
        assert all(len(line.split(",")) == 8 for line in agent_lines)
        assert all(len(line.split(",")) == 9 for line in strategic_lines)
