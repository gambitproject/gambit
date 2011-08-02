"""
Provides support for level-k/cognitive hierarchy modeling
"""

import math
import scipy.stats

def logit_br(game, profile, lam):
    br = game.mixed_profile()
    payoffs = [ math.exp(lam*v) for v in profile.strategy_values() ]
    s = sum(payoffs)
    for i in xrange(len(br)):
        br[i] = payoffs[i] / s
    return br


class CognitiveHierarchyProfile(object):
    """
    Container class representing a CH solution.
    """
    def __init__(self, tau, lam, profile):
        self.tau = tau
        self.lam = lam
        self.profile = profile
    def __len__(self):    return len(self.profile)
    def __getitem__(self, i):   return self.profile[i]
    def __getattr__(self, attr):  return getattr(self.profile, attr)
    def __repr__(self):
        return "<CognitiveHierarchyProfile for tau=%f, lam=%f: %s>" % \
            (self.tau, self.lam, self.profile)

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
