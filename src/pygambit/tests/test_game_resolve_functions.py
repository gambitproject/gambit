import unittest

import numpy as np

import pygambit as gbt

# funcname is used by all the resolve functions we test here, but is 
# not relevant to the substance of the tests
dummy_funcname = 'test'

class TestGambitResolveFunctions(unittest.TestCase):


    def setUp(self):
        self.game = gbt.Game.read_game(
            "./test_games/sample_extensive_game.efg"
        )

    def tearDown(self):
        del self.game

    def test_resolve_player_empty_string(self):
        "Test _resolve_player with the empty string"
        self.assertRaises(ValueError, self.game._resolve_player, player='', funcname='test')

    def test_resolve_player_nonempty_strings(self):
        "Test _resolve_player with non-empty strings, some that resolve some that don't"
        assert self.game._resolve_player(player='Player 1', funcname=dummy_funcname)
        assert self.game._resolve_player(player='Player 2', funcname=dummy_funcname)
        self.assertRaises(KeyError, self.game._resolve_player, player='Player 3', funcname=dummy_funcname)
