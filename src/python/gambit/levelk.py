#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/levelk.py
# Provides support for level-k/cognitive hierarchy modeling
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
"""Provides support for level-k/cognitive hierarchy modeling
"""

from __future__ import print_function

import math
import scipy.optimize
import scipy.stats
from gambit.profiles import Solution

def logit_br(game, profile, lam):
    def do_sum(maxi, logpi, lam, values):
        logp = [ logpi + lam * (values[j] - values[maxi])
                for j in xrange(len(values)) ]
        return sum([ math.exp(p) for p in logp ]) - 1.0
    values = profile.strategy_values()
    maxi = values.index(max(values))
    logp0 = scipy.optimize.newton(lambda x: do_sum(maxi, x, lam, values),
                                  0.0)
    br = game.mixed_strategy_profile()
    for i in xrange(len(profile)):
        br[i] = math.exp(logp0 + lam*(values[i]-values[maxi]))
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
    p = game.mixed_strategy_profile()
    if tau <= 0.0 or lam <= 0.0:
        return CognitiveHierarchyProfile(tau, lam, p)
    accum = float(scipy.stats.poisson(tau).pmf(0))
    for k in xrange(1, max(3, int(5*tau))):
        br = logit_br(game, p, lam=lam)
        prob = float(scipy.stats.poisson(tau).pmf(k))
        p = (1.0/(prob+accum)) * (prob*br + accum*p)
        accum += prob
    return CognitiveHierarchyProfile(tau, lam, p)

def fit_coghier(game, data, min_tau, max_tau, min_lam, max_lam,
                grid_size=10, sample_cands=20, verbose=False):
    """
    Compute maximum likelihood estimate of (tau, lambda) parameters
    for cognitive hierarchy model.  Operates with initial grid
    search over [ min_tau, max_tau ] x [ min_lam, max_lam ],
    with 'grid_size' points in each direction.  Then, uses
    Newton function minimization starting at the top 'sample_cands'
    points, to polish the maximizer.
    """
    def log_like(profile, data):
        return sum([ math.log(p) * d for (p, d) in zip(profile, data) ])

    def objective(params, game, data):
        penalty = 0.0
        tau, lam = params
        if lam < 0.0:
            penalty += lam*lam
            lam = 0.0
        if tau < 0.0:
            penalty += tau*tau
            tau = 0.0
        profile = compute_coghier(game, tau, lam)
        logL = log_like(profile, data)
        return penalty - logL

    results = [ ]
    for lam in scipy.linspace(min_lam, max_lam, grid_size):
        if verbose:
            print("Searching lambda=%.3f" % lam)
        for tau in scipy.linspace(min_tau, max_tau, grid_size):
            profile = compute_coghier(game, tau, lam)
            profile.logL = log_like(profile, data)
            results.append(profile)
        results.sort(lambda x, y: cmp(y.logL, x.logL))
        results = results[:sample_cands]
        if verbose:
            print("tau,lam,logL")
            for profile in results:
                print("%f,%f,%f" % (profile.tau, profile.lam, profile.logL))
            print()

    if verbose: print
    for start in results[:sample_cands]:
        params = scipy.optimize.fmin(lambda x: objective(x, game, data),
                                     (start.tau, start.lam),
                                     disp=0)
        end_tau, end_lam = list(params)
        profile = compute_coghier(game, end_tau, end_lam)
        profile.logL = log_like(profile, data)
        results.append(profile)
        if verbose:
            print("%f,%f,%f" % (profile.tau, profile.lam, profile.logL))
    results.sort(lambda x, y: cmp(y.logL, x.logL))
    return results[0]
                            
            
