"""
Base classes for strategy profiles.
"""

class Solution(object):
    """
    Generic object representing a strategy profile which is
    (part of) a solution of a game.
    """
    def __init__(self, profile):  self._profile = profile
    def __len__(self):            return len(self._profile)
    def __getitem__(self, i):     return self._profile[i]
    def __setitem__(self, i, v):
        raise TypeError, "solution profile object does not support probability assignment"
    def __getattr__(self, attr):  return getattr(self._profile, attr)
