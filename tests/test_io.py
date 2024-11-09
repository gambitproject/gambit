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
