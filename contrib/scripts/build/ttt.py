import copy

boardsize = 3

#
# Represents a tic-tac-toe board.
# Board coordinates are (i, j), i,j=0,1,2.
#
class TicTacToeHistory:
    def __init__(self, board = [ [ "" for i in range(boardsize) ] for j in range(boardsize) ]):
        self.board = copy.deepcopy(board)

    def WithAction(self, action):
        board = copy.deepcopy(self.board)
        board[eval(action)[0]][eval(action)[1]] = self.CurrentMove()
        return TicTacToeHistory(board)

    def CurrentMove(self):
        filled = len([ (row, col)
                       for row in xrange(boardsize) for col in xrange(boardsize)
                       if self.board[row][col] != "" ])
        if filled % 2 == 0:
            return "X"
        else:
            return "O"

    def IsWin(self, pl):
        winconfig = [ pl for i in xrange(boardsize) ]
        for row in xrange(boardsize):
            if self.board[row] == winconfig:
                return True
        for col in xrange(boardsize):
            if [ row[col] for row in self.board ] == winconfig:
                return True
        if [ self.board[i][i] for i in xrange(boardsize) ] == winconfig:
            return True
        if [ self.board[boardsize-i-1][i] for i in xrange(boardsize) ] == winconfig:
            return True
        return False

    def IsFull(self):
        return len([ (row, col) for row in xrange(boardsize) for col in xrange(boardsize)
                     if self.board[row][col] == "" ]) == 0

    def EmptySquares(self):
        return [ (row, col) for row in xrange(boardsize) for col in xrange(boardsize)
                 if self.board[row][col] == "" ]

class TicTacToeMove:
    def Apply(self, node, history):
        moves = history.EmptySquares()

        if history.CurrentMove() == "X":
            player = node.GetGame().GetPlayer(1)
        else:
            player = node.GetGame().GetPlayer(2)

        infoset = node.AppendMove(player, len(moves))
        for (i, move) in enumerate(moves):
            infoset.GetAction(i+1).SetLabel(str(move))

        return [ node.GetChild(i) for i in xrange(1, len(moves)+1) ]


import torr

#
# Here we make extensive use of anonymous 'lambda' functions
# to express the conditions, based on the board state
#

# Note that since rules are checked in order, we can just make the
# last condition "true" -- since if we get there, then the game is
# still going on and a move is to be made.
rules = [ { "condition": lambda history: history.IsWin("X"),
            "action":    torr.TerminalPayoff([1, -1]) },
          { "condition": lambda history: history.IsWin("O"),
            "action":    torr.TerminalPayoff([-1, 1]) },
          { "condition": lambda history: history.IsFull(),
            "action":    torr.TerminalPayoff([0, 0]) },
          { "condition": lambda history: True,
            "action":    TicTacToeMove() } ]


tree = torr.BuildTree([ "X", "O" ], rules, TicTacToeHistory)
print tree.AsEfgFile()

