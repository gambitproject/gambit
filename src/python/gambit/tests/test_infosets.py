import gambit

class TestGambitInfosets(object):
    def setUp(self):
        self.extensive_game = gambit.read_game("test_games/basic_extensive_game.efg")
        
    def tearDown(self):
        del self.extensive_game
        
    def test_infoset_set_label(self):
        "Test to ensure infoset labels work"
        assert self.extensive_game.players[0].infosets[0].label == ""
        self.extensive_game.players[0].infosets[0].label = "infoset 1"
        assert self.extensive_game.players[0].infosets[0].label == "infoset 1"

    def test_infoset_player_retrieval(self):
        "Test to ensure infoset returns correct player"
        assert self.extensive_game.players[0] == self.extensive_game.players[0].infosets[0].player
        
    def test_infoset_node_precedes(self):
        "Test to check if the infoset preceding check works"
        assert not self.extensive_game.players[0].infosets[0].precedes(self.extensive_game.root)
        assert self.extensive_game.players[1].infosets[0].precedes(self.extensive_game.root.children[0])
        
        
