cdef extern from "libgambit/libgambit.h":
    pass

cdef extern from "string":
    ctypedef struct cxx_string "string":
        char *c_str()
        cxx_string assign(char *)


cdef extern from "libgambit/game.h":
    ctypedef struct c_GameRep "GameRep":
        cxx_string GetTitle()
        void SetTitle(cxx_string)
        int NumNodes()

    ctypedef struct c_Game "GameObjectPtr<GameRep>":
        c_GameRep *deref "operator->"()

    c_Game NewTree()

cdef extern from "game.wrap.h":
    c_Game ReadGame(char *) except +IOError

cdef class Game:
    cdef c_Game game

    def __repr__(self):
        return "<This is a Game!>"

    def num_nodes(self):
        return self.game.deref().NumNodes()

    property title:
        def __get__(self):
            return self.game.deref().GetTitle().c_str()
        def __set__(self, char *value):
            cdef cxx_string s
            s.assign(value)
            self.game.deref().SetTitle(s)


def new_tree():
    cdef Game g
    g = Game()
    g.game = NewTree()
    return g

def read_game(char *fn):
    cdef Game g
    g = Game()
    try:
        g.game = ReadGame(fn)
    except IOError:
        raise IOError("Unable to read game from file '%s'" % fn)
    return g
        
