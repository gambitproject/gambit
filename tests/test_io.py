import os.path
from glob import glob

import pytest

import pygambit as gbt


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


@pytest.mark.parametrize(
        "game_path",
        [game for game in glob(os.path.join("tests", "test_games", "*.efg"))
         if game != os.path.join("tests", "test_games", "basic_extensive_game.efg")]
)
def test_read_write_efg(game_path):
    game = gbt.read_efg(game_path)
    saved_game = game.to_efg()
    with open(game_path) as f:
        original_game = f.read()
    assert saved_game == original_game


@pytest.mark.skip(reason="These nfg files are not saved in the identical to the original format")
@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.nfg")))
def test_read_write_nfg(game_path):
    game = gbt.read_nfg(game_path)
    saved_game = game.to_nfg()
    with open(game_path) as f:
        original_game = f.read()
    assert saved_game == original_game
