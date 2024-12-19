from html.parser import HTMLParser

import pytest

import pygambit as gbt


class MyHTMLParser(HTMLParser):
    """https://stackoverflow.com/a/70214064"""

    def __init__(self):
        super().__init__()
        self.start_tags = list()
        self.end_tags = list()
        self.attributes = list()

    def is_text_html(self):
        return len(self.start_tags) == len(self.end_tags)

    def handle_starttag(self, tag, attrs):
        self.start_tags.append(tag)
        self.attributes.append(attrs)

    def handle_endtag(self, tag):
        self.end_tags.append(tag)

    def handle_data(self, data):
        pass


def test_read_efg():
    game_path = "tests/test_games/cent3.efg"
    game = gbt.read_efg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_efg_invalid():
    game_path = "tests/test_games/2x2x2_nfg_with_two_pure_one_mixed_eq.nfg"
    with pytest.raises(ValueError):
        gbt.read_efg(game_path)


def test_read_nfg():
    game_path = "tests/test_games/2x2x2_nfg_with_two_pure_one_mixed_eq.nfg"
    game = gbt.read_nfg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_nfg_invalid():
    game_path = "tests/test_games/cent3.efg"
    with pytest.raises(ValueError):
        gbt.read_nfg(game_path)


def test_read_agg():
    game_path = "tests/test_games/2x2.agg"
    game = gbt.read_agg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_agg_invalid():
    game_path = "tests/test_games/2x2x2_nfg_with_two_pure_one_mixed_eq.nfg"
    with pytest.raises(ValueError):
        gbt.read_agg(game_path)


def test_read_gbt_invalid():
    game_path = "tests/test_games/2x2x2_nfg_with_two_pure_one_mixed_eq.nfg"
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
    parser = MyHTMLParser()
    parser.feed(serialized_game)
    assert parser.is_text_html()


def test_write_latex():
    game = gbt.Game.new_table([2, 2])
    serialized_game = game.to_latex()
    assert serialized_game.startswith(r"\begin{game}")
