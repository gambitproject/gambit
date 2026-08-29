from unittest.mock import MagicMock

import pytest

import pygambit as gbt
from pygambit.nashlrs import _generate_lrs_input, _parse_lrs_output, lrsnash_solve


def test_generate_lrs_input_format():
    """The generated input is the game dimensions, then each player's payoff matrix as
    space-separated rows, matching lrsnash's expected input format.
    """
    game = gbt.Game.from_arrays([[1, 2], [3, 4]], [[5, 6], [7, 8]])
    assert _generate_lrs_input(game) == "2 2\n\n1 2\n3 4\n\n5 6\n7 8\n"


def test_generate_lrs_input_rectangular():
    game = gbt.Game.from_arrays([[1, 2, 3]], [[4, 5, 6]])
    assert _generate_lrs_input(game) == "1 3\n\n1 2 3\n\n4 5 6\n"


def test_parse_lrs_output_single_equilibrium():
    """Each equilibrium is one block of lines: player 1's rows (prefixed '1'), then
    player 2's rows (prefixed '2'), each ending in a value that is not part of the
    probability vector and is discarded.
    """
    game = gbt.Game.new_table([2, 2])
    txt = "1 1 0 3\n2 0 1 3\n"
    profiles = _parse_lrs_output(game, txt)
    assert len(profiles) == 1
    assert profiles[0]["1"] == {"1": 1, "2": 0}
    assert profiles[0]["2"] == {"1": 0, "2": 1}


def test_parse_lrs_output_multiple_equilibria():
    """Equilibria (components) are separated by a blank line."""
    game = gbt.Game.new_table([2, 2])
    txt = "1 1 0 3\n2 0 1 3\n\n1 0 1 3\n2 1 0 3\n"
    profiles = _parse_lrs_output(game, txt)
    assert len(profiles) == 2
    assert profiles[0]["1"] == {"1": 1, "2": 0}
    assert profiles[1]["1"] == {"1": 0, "2": 1}


def test_parse_lrs_output_ignores_comment_lines():
    """Lines starting with '*' (lrsnash diagnostic/banner output) are ignored."""
    game = gbt.Game.new_table([2, 2])
    txt = "* some banner text\n1 1 0 3\n2 0 1 3\n* another comment\n"
    profiles = _parse_lrs_output(game, txt)
    assert len(profiles) == 1


def test_lrsnash_solve_requires_two_players():
    game = gbt.Game.new_table([2, 2, 2])
    with pytest.raises(RuntimeError, match="two-player"):
        lrsnash_solve(game, "lrsnash")


def test_lrsnash_solve_mocked_subprocess(monkeypatch):
    """`lrsnash_solve` writes the game to a temp file, invokes the external tool, and
    parses its stdout -- verified here without requiring the real `lrsnash` binary.
    """
    game = gbt.Game.from_arrays([[1, -1], [-1, 1]], [[-1, 1], [1, -1]])
    captured = {}

    def _fake_run(cmd, **kwargs):
        captured["cmd"] = cmd
        result = MagicMock()
        result.returncode = 0
        result.stdout = "1 1 0 3\n2 1 0 3\n"
        return result

    monkeypatch.setattr("pygambit.nashlrs.subprocess.run", _fake_run)
    profiles = lrsnash_solve(game, "./lrsnash")
    assert len(profiles) == 1
    assert profiles[0]["1"] == {"1": 1, "2": 0}
    assert captured["cmd"][0] == "./lrsnash"


def test_lrsnash_solve_nonzero_returncode_raises(monkeypatch):
    game = gbt.Game.new_table([2, 2])

    def _fake_run(cmd, **kwargs):
        result = MagicMock()
        result.returncode = 1
        result.stdout = ""
        return result

    monkeypatch.setattr("pygambit.nashlrs.subprocess.run", _fake_run)
    with pytest.raises(ValueError, match="failed"):
        lrsnash_solve(game, "./lrsnash")
