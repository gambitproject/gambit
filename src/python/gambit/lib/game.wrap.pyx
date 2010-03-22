cdef extern from "libgambit/libgambit.h":
    pass

cdef extern from "game.wrap.h":
    # This is a dummy header file which does nothing but include the
    # Gambit namespace.
    pass

cdef extern from "libgambit/game.h":
    ctypedef struct c_GameRep "GameRep":
        int NumNodes()

    ctypedef struct c_Game "GameObjectPtr<GameRep>":
        c_GameRep *deref "operator->"()

    c_Game NewTree()
    

cdef class Game:
    cdef c_Game game

    def __repr__(self):
        return "<This is a Game!>"

    def num_nodes(self):
        return self.game.deref().NumNodes()


def new_tree():
    cdef Game g
    g = Game()
    g.game = NewTree()
    return g

        
    
