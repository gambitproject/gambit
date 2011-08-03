"""
Provides support for level-k/cognitive hierarchy modeling
"""

import math
import scipy.stats
from gambit.profiles import Solution

def logit_br(game, profile, lam):
    br = game.mixed_profile()
    payoffs = [ math.exp(lam*v) for v in profile.strategy_values() ]
    s = sum(payoffs)
    for i in xrange(len(br)):
        br[i] = payoffs[i] / s
    return br

class CognitiveHierarchyProfile(Solution):
    """
    Container class representing a CH solution.
    """
    def __init__(self, tau, lam, profile):
        Solution.__init__(self, profile)
        self._tau = tau
        self._lam = lam
    def __repr__(self):
        return "<CognitiveHierarchyProfile for tau=%f, lam=%f: %s>" % \
            (self._tau, self._lam, self._profile)
    @property
    def tau(self):       return self._tau
    @property
    def lam(self):       return self._lam

def compute_coghier(game, tau, lam):
    """
    Compute the cognitive hierarchy prediction for 'game', with
    a Poisson distribution of types with mean 'tau', with precision of
    best responses 'lam'.
    """
    p = game.mixed_profile()
    accum = float(scipy.stats.poisson(tau).pmf(0))
    for k in xrange(1, max(3, int(5*tau))):
        br = logit_br(game, p, lam=lam)
        prob = float(scipy.stats.poisson(tau).pmf(k))
        p = (1.0/(prob+accum)) * (prob*br + accum*p)
        accum += prob
    return CognitiveHierarchyProfile(tau, lam, p)
