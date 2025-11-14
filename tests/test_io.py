import io
import os.path
from glob import glob

import pytest

import pygambit as gbt

from .games import create_2x2_zero_nfg, create_selten_horse_game_efg


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.efg")))
def test_read_efg(game_path):
    game = gbt.read_efg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_efg_invalid():
    game_path = os.path.join("tests", "test_games", "2x2x2_nfg_with_two_pure_one_mixed_eq.nfg")
    with pytest.raises(ValueError):
        gbt.read_efg(game_path)


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.nfg")))
def test_read_nfg(game_path):
    game = gbt.read_nfg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_nfg_invalid():
    game_path = os.path.join("tests", "test_games", "cent3.efg")
    with pytest.raises(ValueError):
        gbt.read_nfg(game_path)


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.agg")))
def test_read_agg(game_path):
    game = gbt.read_agg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_agg_invalid():
    game_path = os.path.join("tests", "test_games", "2x2x2_nfg_with_two_pure_one_mixed_eq.nfg")
    with pytest.raises(ValueError):
        gbt.read_agg(game_path)


def test_read_gbt_invalid():
    game_path = os.path.join("tests", "test_games", "2x2x2_nfg_with_two_pure_one_mixed_eq.nfg")
    with pytest.raises(ValueError):
        gbt.read_gbt(game_path)


def test_write_efg():
    game = gbt.Game.new_tree()
    serialized_game = game.to_efg()
    assert serialized_game[:3] == "EFG"


def test_write_nfg():
    game = gbt.Game.new_table([2, 2])
    serialized_game = game.to_nfg()
    assert serialized_game[:3] == "NFG"


def test_write_efg_as_nfg():
    result = """
NFG 1 R "Centipede game. Three inning with probability of altruism.  " { "Player 1" "Player 2" }

{ { "1**111" "21*111" "221111" "222111" }
{ "1**111" "21*111" "221111" "222111" }
}
""

20027/25000 2689/12500
541/1250 3983/2500
3299/6250 5053/3125
227/250 214/125
5081/6250 3283/12500
19931/6250 2677/3125
5362/3125 19903/3125
262/125 808/125
2689/3125 5659/12500
10114/3125 3271/3125
39814/3125 10696/3125
856/125 3184/125
163/125 163/500
92/25 23/25
1648/125 412/125
256/5 64/5
"""
    game = gbt.read_efg("tests/test_games/cent3.efg")
    assert game.to_nfg().strip() == result.strip()


def test_write_html():
    game = gbt.Game.new_table([2, 2])
    serialized_game = game.to_html()
    assert isinstance(serialized_game, str)


def test_write_latex():
    game = gbt.Game.new_table([2, 2])
    serialized_game = game.to_latex()
    assert serialized_game.startswith(r"\begin{game}")


def test_read_write_efg():
    efg_game = create_selten_horse_game_efg()
    serialized_efg_game = efg_game.to_efg()
    deserialized_efg_game = gbt.read_efg(io.BytesIO(serialized_efg_game.encode()))
    double_serialized_efg_game = deserialized_efg_game.to_efg()
    assert serialized_efg_game == double_serialized_efg_game


def test_read_write_nfg():
    nfg_game = create_2x2_zero_nfg()
    serialized_nfg_game = nfg_game.to_nfg()
    deserialized_nfg_game = gbt.read_nfg(io.BytesIO(serialized_nfg_game.encode()))
    double_serialized_nfg_game = deserialized_nfg_game.to_nfg()
    assert serialized_nfg_game == double_serialized_nfg_game


def test_print_mixed_strategy_profile():
    game_path = os.path.join("tests", "test_games", "mixed_behavior_game.efg")
    test_game = gbt.read_efg(game_path)
    text_string = "\n".join(["Player 1 1 0.5",
                             "         2 0.5",
                             "Player 2 1 0.5",
                             "         2 0.5",
                             "Player 3 1 0.5",
                             "         2 0.5"])
    assert repr(test_game.mixed_strategy_profile()) == text_string


def test_print_mixed_behavior_profile():
    game_path = os.path.join("tests", "test_games", "mixed_behavior_game.efg")
    test_game = gbt.read_efg(game_path)
    text_string = "\n".join(["Player 1 Infoset 1:1 U1 0.5",
                             "                     D1 0.5",
                             "Player 2 Infoset 2:1 U2 0.5",
                             "                     D2 0.5",
                             "Player 3 Infoset 3:1 U3 0.5",
                             "                     D3 0.5"])
    assert repr(test_game.mixed_behavior_profile()) == text_string
