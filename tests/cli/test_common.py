"""Unit tests for the shared infrastructure in src/pygambit/cli/common.py."""

import io
import sys

import pytest

import pygambit as gbt
from pygambit.cli import common


class TestReadGame:
    def test_reads_nfg(self, nfg_matching_pennies_text):
        game = common.read_game(io.BytesIO(nfg_matching_pennies_text.encode()))
        assert not game.is_tree
        assert len(game.players) == 2

    def test_reads_efg(self, efg_small_tree_text):
        game = common.read_game(io.BytesIO(efg_small_tree_text.encode()))
        assert game.is_tree
        assert len(game.players) == 2

    def test_reads_text_stream(self, nfg_matching_pennies_text):
        game = common.read_game(io.StringIO(nfg_matching_pennies_text))
        assert len(game.players) == 2

    def test_empty_input_raises(self):
        with pytest.raises(ValueError, match="Empty file or string provided"):
            common.read_game(io.BytesIO(b""))

    def test_whitespace_only_input_raises(self):
        with pytest.raises(ValueError, match="Empty file or string provided"):
            common.read_game(io.BytesIO(b"   \n  "))

    def test_unrecognized_format_raises(self):
        with pytest.raises(ValueError, match="Unrecognized file format"):
            common.read_game(io.BytesIO(b"this is not a game file"))

    def test_malformed_nfg_raises_parse_error(self):
        with pytest.raises(ValueError, match="Parse error in game file"):
            common.read_game(io.BytesIO(b'NFG 1 R "broken" { "1" "2" }\n\n{ garbage'))


class TestOpenGameFile:
    def test_none_returns_stdin(self):
        assert common.open_game_file(None, "prog") is sys.stdin.buffer

    def test_dash_returns_stdin(self):
        assert common.open_game_file("-", "prog") is sys.stdin.buffer

    def test_valid_path_is_opened(self, tmp_path):
        path = tmp_path / "game.nfg"
        path.write_text("NFG 1 R\n")
        f = common.open_game_file(str(path), "prog")
        try:
            assert f.read() == b"NFG 1 R\n"
        finally:
            f.close()

    def test_missing_file_exits_1_with_perror_style_message(self, tmp_path, capsys):
        missing = tmp_path / "does_not_exist.nfg"
        with pytest.raises(SystemExit) as excinfo:
            common.open_game_file(str(missing), "gambit-enumpure")
        assert excinfo.value.code == 1
        captured = capsys.readouterr()
        assert captured.err == f"gambit-enumpure: {missing}: No such file or directory\n"


class TestValidateStopAfter:
    def test_none_passes_through(self):
        assert common.validate_stop_after(None, None, None) is None

    def test_valid_positive_integer(self):
        assert common.validate_stop_after(None, None, "5") == 5

    def test_zero_is_rejected(self, capsys):
        with pytest.raises(SystemExit) as excinfo:
            common.validate_stop_after(None, None, "0")
        assert excinfo.value.code == 1
        assert "must be a positive integer; got '0'." in capsys.readouterr().err

    def test_non_numeric_is_rejected(self, capsys):
        with pytest.raises(SystemExit) as excinfo:
            common.validate_stop_after(None, None, "abc")
        assert excinfo.value.code == 1
        assert "must be a positive integer; got 'abc'." in capsys.readouterr().err

    def test_negative_is_rejected(self):
        with pytest.raises(SystemExit):
            common.validate_stop_after(None, None, "-3")


class TestHandleErrors:
    def test_success_passes_through(self, capsys):
        @common.handle_errors
        def f():
            print("ok")

        f()
        assert capsys.readouterr().out == "ok\n"

    @pytest.mark.parametrize("exc_type", [ValueError, RuntimeError, OSError])
    def test_known_exceptions_become_error_message_and_exit_1(self, capsys, exc_type):
        @common.handle_errors
        def f():
            raise exc_type("something went wrong")

        with pytest.raises(SystemExit) as excinfo:
            f()
        assert excinfo.value.code == 1
        assert capsys.readouterr().err == "Error: something went wrong\n"

    def test_unrelated_exception_propagates(self):
        @common.handle_errors
        def f():
            raise KeyError("not handled")

        with pytest.raises(KeyError):
            f()


class TestFormatValue:
    def test_rational_ignores_decimals_by_default(self):
        assert common.format_value(gbt.Rational(1, 3), 6) == "1/3"

    def test_rational_whole_number(self):
        assert common.format_value(gbt.Rational(1, 1), 0) == "1"

    def test_float_fixed(self):
        assert common.format_value(0.5, 3) == "0.500"

    def test_float_general_trims_trailing_zeroes(self):
        assert common.format_value(0.5, 6, fixed=False) == "0.5"

    def test_float_general_switches_to_scientific(self):
        assert common.format_value(1.38594e-08, 6, fixed=False) == "1.38594e-08"

    def test_rational_as_float_converts_and_formats_fixed(self):
        assert common.format_value(gbt.Rational(1, 3), 3, as_float=True) == "0.333"


class TestRenderProfileCsv:
    def test_strategy_profile(self, nfg_matching_pennies_text):
        game = gbt.read_nfg(io.BytesIO(nfg_matching_pennies_text.encode()))
        profile = game.mixed_strategy_profile(rational=True)
        assert common.render_profile_csv(profile, "NE", 0) == "NE,1/2,1/2,1/2,1/2"

    def test_behavior_profile(self, efg_small_tree_text):
        game = gbt.read_efg(io.BytesIO(efg_small_tree_text.encode()))
        profile = game.mixed_behavior_profile(rational=True)
        assert common.render_profile_csv(profile, "start", 0) == "start,1/2,1/2,1/2,1/2"

    def test_label_is_first_field(self, nfg_matching_pennies_text):
        game = gbt.read_nfg(io.BytesIO(nfg_matching_pennies_text.encode()))
        profile = game.mixed_strategy_profile(rational=True)
        assert common.render_profile_csv(profile, "custom-label", 0).startswith("custom-label,")


class TestRenderSupportCsv:
    def test_strategy_support_full_support(self, nfg_matching_pennies_text):
        game = gbt.read_nfg(io.BytesIO(nfg_matching_pennies_text.encode()))
        support = game.strategy_support_profile()
        assert common.render_support_csv(support, "candidate") == "candidate,11,11"

    def test_strategy_support_partial_support(self, nfg_matching_pennies_text):
        game = gbt.read_nfg(io.BytesIO(nfg_matching_pennies_text.encode()))
        support = game.strategy_support_profile()
        p1 = next(iter(game.players))
        first_strategy = next(iter(p1.strategies))
        support[p1.label] = [first_strategy.label]
        assert common.render_support_csv(support, "candidate") == "candidate,10,11"
