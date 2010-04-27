
class Game(object):
    """
    An abstract class defining a game. Pre-provides some expected
    interface elements (raising the NotImplemented error)
    """
    def __init__(self):   pass

    def efg_file(self):   raise NotImplementedError
    def nfg_file(self):   raise NotImplementedError

    def is_symmetric(self):   raise NotImplementedError

    def mixed_profile(self):  raise NotImplementedError

    def qre(self):   raise NotImplementedError
    
