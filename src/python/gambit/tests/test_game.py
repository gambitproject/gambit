import gambit
import fractions
import nose.tools
from nose.tools import assert_raises
from gambit.lib.error import UndefinedOperationError

class TestGambitGame(object):
    def setUp(self):
        self.game = gambit.new_table([2,2])
        self.extensive_game = gambit.read_game("test_games/basic_extensive_game.efg")
    
    def tearDown(self):
        del self.game
        del self.extensive_game

    def test_game_get_outcome_with_ints(self):
        "To test getting the first outcome"
        assert self.game.outcomes[0] == self.game[0,0]

    def test_game_get_outcome_with_incorrect_tuple_size(self):
        "To test getting an outcome with a bad tuple size"
        assert_raises(KeyError, self.game.__getitem__, (0,0,0))


    def test_game_get_outcome_with_bad_ints(self):
        "To test getting an index error with ints"
        assert_raises(IndexError,self.game.__getitem__,(0,3))

    def test_game_get_outcome_with_strings(self):
        "To test getting the first outcome with strategy labels"
        self.game.players[0].strategies[0].label = "defect"
        self.game.players[1].strategies[0].label = "cooperate"
        assert self.game.outcomes[0] == self.game["defect","cooperate"]

    def test_game_get_outcome_with_bad_strings(self):
        "To test getting the first outcome with strategy labels"
        self.game.players[0].strategies[0].label = "defect"
        self.game.players[1].strategies[0].label = "cooperate"
        assert_raises(IndexError,self.game.__getitem__,("defect","defect"))


    def test_game_get_outcome_with_strategies(self):
        "To test getting the first outcome with strategies"
        assert self.game.outcomes[0] == self.game[self.game.players[0].strategies[0], self.game.players[1].strategies[0]]

    def test_game_get_outcome_with_bad_strategies(self):
        "To test getting the first outcome with incorrect strategies"
        assert_raises(IndexError, self.game.__getitem__, (self.game.players[0].strategies[0], self.game.players[0].strategies[0]))


    def test_game_outcomes_non_tuple(self):
        "To test when attempting to find outcome with a non-tuple-like object"
        assert_raises(TypeError, self.game.__getitem__, 42)
        
    def test_game_outcomes_type_exception(self):
        "To test when attempting to find outcome with invalid input"
        assert_raises(TypeError, self.game.__getitem__, (1.23,1))


    def test_game_is_const_sum(self):
        "To test checking if the game is constant sum"
        game = gambit.read_game("test_games/const_sum_game.nfg")
        assert game.is_const_sum

    def test_game_is_not_const_sum(self):
        "To test checking if the game is not constant sum"
        game = gambit.read_game("test_games/non_const_sum_game.nfg")
        assert not game.is_const_sum

    def test_game_get_min_payoff(self):
        "To test getting the minimum payoff of the game"
        game = gambit.read_game("test_games/payoff_game.nfg")
        assert game.min_payoff == fractions.Fraction(1,1)

    def test_game_get_max_payoff(self):
        "To test getting the maximum payoff of the game"
        game = gambit.read_game("test_games/payoff_game.nfg")
        assert game.max_payoff == fractions.Fraction(10,1)

    def test_game_is_perfect_recall(self):
        "To test checking if the game is of perfect recall"
        game = gambit.read_game("test_games/perfect_recall.efg")
        assert game.is_perfect_recall

    def test_game_is_not_perfect_recall(self):
        "To test checking if the game is not of perfect recall"
        game = gambit.read_game("test_games/not_perfect_recall.efg")
        assert not game.is_perfect_recall

    def test_game_behav_profile_error(self):
        "To test raising an error when trying to create a BehavProfile from \
        a game without a tree representation"
        assert_raises(UndefinedOperationError, self.game.behav_profile)
        assert_raises(UndefinedOperationError, self.game.behav_profile, True)

    def test_game_title(self):
        assert self.game.title == ""
        self.game.title = "Test Title"
        assert self.game.title == "Test Title"

    def test_game_comment(self):
        assert self.game.comment == ""
        self.game.comment = "Test Comment"
        assert self.game.comment == "Test Comment"

    def test_game_actions(self):
        assert self.extensive_game.actions[0] == self.extensive_game.players[0].infosets[0].actions[0]
        assert self.extensive_game.actions[1] == self.extensive_game.players[0].infosets[0].actions[1]
        assert self.extensive_game.actions[2] == self.extensive_game.players[1].infosets[0].actions[0]
        assert self.extensive_game.actions[3] == self.extensive_game.players[1].infosets[0].actions[1]
        assert self.extensive_game.actions[4] == self.extensive_game.players[2].infosets[0].actions[0]
        assert self.extensive_game.actions[5] == self.extensive_game.players[2].infosets[0].actions[1]

    @nose.tools.raises(UndefinedOperationError)
    def test_strategic_game_actions_error(self):
        "Test to ensure an error is raised when trying to access actions "\
        "of a game without a tree representation"
        self.game.actions


    def test_game_infosets(self):
        assert self.extensive_game.infosets[0] == self.extensive_game.players[0].infosets[0]
        assert self.extensive_game.infosets[1] == self.extensive_game.players[1].infosets[0]
        assert self.extensive_game.infosets[2] == self.extensive_game.players[2].infosets[0]

    @nose.tools.raises(UndefinedOperationError)
    def test_strategic_game_infosets_error(self):
        "Test to ensure an error is raised when trying to access infosets "\
        "of a game without a tree representation"
        self.game.infosets

    def test_game_strategies(self):
        assert self.game.strategies[0] == self.game.players[0].strategies[0]
        assert self.game.strategies[1] == self.game.players[0].strategies[1]
        assert self.game.strategies[2] == self.game.players[1].strategies[0]
        assert self.game.strategies[3] == self.game.players[1].strategies[1]

    def test_game_get_root(self):
        "Test retrieving the root node of a game"
        root = self.extensive_game.root
        root.label = "Test"
        assert self.extensive_game.root.label == root.label

    @nose.tools.raises(UndefinedOperationError)
    def test_game_get_root_error(self):
        "Test to ensure an error is raised when trying to get the root"\
        "node of a game without a tree representation"
        self.game.root

    def test_game_num_nodes(self):
        "Test retrieving the number of nodes of a game"
        assert self.extensive_game.num_nodes() == 15
        assert self.game.num_nodes() == 0

    @nose.tools.raises(RuntimeError)
    def test_game_dereference_invalid(self):
        "Test referencing an invalid game member object"
        g = gambit.new_tree()
        g.players.add("One")
        s = g.players[0].strategies[0]
        g.root.append_move(g.players[0], 2)
        s.number
