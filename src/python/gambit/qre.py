#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/qre.py
# A set of utilities for copmuting and analyzing quantal response equilibria
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
"""
A set of utilities for computing and analyzing quantal response equilbria
"""

import math
import numpy
import pctrace

from gambit.profiles import Solution

def sym_compute_lhs(game, point):
    """
    Compute the LHS for the set of equations for a symmetric logit QRE
    of a symmetric game.
    """
    profile = game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]])
    logprofile = point[:-1]
    lam = point[-1]

    lhs = numpy.zeros(len(profile))

    for (st, cont) in enumerate(game.choices):
        if st == 0:
            # sum-to-one equation
            lhs[st] = -1.0 + sum(profile)
        else:
            lhs[st] = (logprofile[st]-logprofile[0]-
                       lam * (profile.strategy_value(st)-
                              profile.strategy_value(0)))
    return lhs
            

def sym_compute_jac(game, point):
    """
    Compute the Jacobian for the set of equations for a symmetric logit QRE
    of a symmetric game.
    """
    profile = game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]])
    logprofile = point[:-1]
    lam = point[-1]

    matrix = numpy.zeros((len(point), len(profile)))

    for (st, cont) in enumerate(game.choices):
        if st == 0:
            # sum-to-one equation
            for (sto, conto) in enumerate(game.choices):
                matrix[sto, st] = profile[sto]

            # derivative wrt lambda is zero, so don't need to fill last col
        else:
            # this is a ratio equation
            for (sto, conto) in enumerate(game.choices):
                matrix[sto, st] = \
                           -(game.N-1) * lam * profile[sto] * \
                           (profile.strategy_value_deriv(st, sto) -
                            profile.strategy_value_deriv(0, sto))
                if sto == 0:
                    matrix[sto, st] -= 1.0
                elif sto == st:
                    matrix[sto, st] += 1.0

            # column wrt lambda
            matrix[-1][st] = profile.strategy_value(0) - \
                             profile.strategy_value(st)

    return matrix
            

def printer(game, point):
    profile = game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]])
    lam = point[-1]
    print(lam, profile)



class LogitQRE(Solution):
    """
    Container class representing a logit QRE
    """
    def __init__(self, lam, profile):
        Solution.__init__(self, profile)
        self._lam = lam
    def __repr__(self):
        return "<LogitQRE at lam=%f: %s>" % (self._lam, self._profile)
    @property
    def lam(self):      return self._lam
    @property
    def mu(self):       return 1.0 / self._lam

class StrategicQREPathTracer(object):
    """
    Compute the principal branch of the logit QRE correspondence of 'game'.
    """
    def __init__(self):
        self.h_start = 0.03
        self.max_decel = 1.1
    
    def trace_strategic_path(self, game, max_lambda=1000000.0, callback=None):
        points = [ ]
        def on_step(game, points, p, callback):
            qre = LogitQRE(p[-1],
                           game.mixed_strategy_profile(point=[math.exp(x) for x in p[:-1]]))
            points.append(qre)
            if callback:  callback(qre)

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            try:
                pctrace.trace_path([ math.log(x) for x in p.profile ],
                                   0.0, max_lambda,
                                   lambda x: sym_compute_lhs(game, x),
                                   lambda x: sym_compute_jac(game, x),
                                   hStart=self.h_start,
                                   maxDecel=self.max_decel,
                                   callback=lambda p: on_step(game, points, p, callback),
                                   crit=None,
                                   maxIter=100)
            except KeyboardInterrupt:
                pass

            return points
        else:
            raise NotImplementedError

    def compute_at_lambda(self, game, lam, callback=None):
        if callback is not None:
            on_step = lambda p: callback(LogitQRE(p[-1],
                                                  game.mixed_strategy_profile(point=[math.exp(x) for x in p[:-1]])))
        else:
            on_step = None

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       crit=lambda x,t: x[-1] - lam,
                                       callback=on_step,
                                       maxIter=100)

            return LogitQRE(point[-1],
                            game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]]))
        else:
            raise NotImplementedError

    def compute_max_like(self, game, data):
        log_like = lambda data, profile: \
                   sum([ x*math.log(y) for (x, y) in zip(data, profile) ])
        diff_log_like = lambda data, point, tangent: \
                        sum([ x*y for (x, y) in zip(data, tangent[:-1]) ])

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       hStart=1.0,
                                       crit=lambda x,t: diff_log_like(data,x,t),
                                       maxIter=100)

            qre = LogitQRE(point[-1],
                           game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]]))
            qre.logL = log_like(data, qre)
            return qre
        else:
            raise NotImplementedError

    def compute_fit_sshist(self, game, data, callback=None):
        """
        Find lambda parameter for which QRE best fits the data
        using the sum of squares of distances in the histogram of
        the data.
        """
        def diff_dist(data, point, tangent):
            return 2.0 * sum([ (math.exp(p)-d) * t * math.exp(p)
                               for (p, t, d) in zip(point, tangent, data) ])

        if game.is_symmetric:
            p = game.mixed_strategy_profile()
            point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       hStart=1.0,
                                       crit=lambda x,t: diff_dist(data,x,t),
                                       maxIter=100,
                                       callback=callback)

            qre = LogitQRE(point[-1],
                           game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]]))
            return qre
        else:
            raise NotImplementedError


    def compute_criterion(self, game, f):
        def criterion_wrap(x, t):
            """
            This translates the internal representation of the tracer
            into a QRE object.
            """
            return f(LogitQRE(x[-1],
                              game.mixed_strategy_profile(point=[math.exp(z) for z in x[:-1]])))
            
        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       hStart=1.0,
                                       crit=criterion_wrap,
                                       maxIter=100)

            return LogitQRE(point[-1],
                            game.mixed_strategy_profile(point=[math.exp(x) for x in point[:-1]]))
        else:
            raise NotImplementedError
        

from nash import ExternalSolver
    
class ExternalStrategicQREPathTracer(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for tracing a branch of the logit QRE correspondence.
    """
    def trace_strategic_path(self, game, max_lambda=1000000.0):
        profiles = [ ]
        command_line = "gambit-logit -d 20 -m %f" % max_lambda
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            profile = game.mixed_strategy_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(LogitQRE(float(entries[0]), profile))
        return profiles
        
    def compute_at_lambda(self, game, lam):
        command_line = "gambit-logit -d 20 -l %f" % lam
        line = list(self.launch(command_line, game))[-1]
        entries = line.strip().split(",")
        profile = game.mixed_strategy_profile()
        for (i, p) in enumerate(entries[1:]):
            profile[i] = float(p)
        return [ LogitQRE(float(entries[0]), profile) ]
        


