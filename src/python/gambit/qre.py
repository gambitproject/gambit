"""
A set of utilities for computing and analyzing quantal response equilbria
"""

import math
import numpy
import pctrace

def sym_compute_lhs(game, point):
    """
    Compute the LHS for the set of equations for a symmetric logit QRE
    of a symmetric game.
    """
    profile = game.mixed_strategy(point=[math.exp(x) for x in point[:-1]])
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
    profile = game.mixed_strategy(point=[math.exp(x) for x in point[:-1]])
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
    profile = game.mixed_strategy(point=[math.exp(x) for x in point[:-1]])
    lam = point[-1]
    print lam, profile



class LogitQRE(object):
    """
    Container class representing a logit QRE
    """
    def __init__(self, lam, profile):
        self.lam = lam
        self.profile = profile

    def __getitem__(self, i):   return self.profile[i]

    def __repr__(self):
        return "<LogitQRE at lam=%f: %s>" % (self.lam, self.profile)


def trace_path(game, max_lambda=1000000.0, h_start=0.03, max_decel=1.1,
               callback=None):
    """
    Compute the principal branch of the logit QRE correspondence of 'game'.
    """
    if game.is_symmetric():
        p = game.mixed_strategy()

        point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                   0.0, max_lambda,
                                   lambda x: sym_compute_lhs(game, x),
                                   lambda x: sym_compute_jac(game, x),
                                   hStart=h_start,
                                   maxDecel=max_decel,
                                   callback=callback,
                                   crit=None,
                                   maxIter=100)

        return LogitQRE(point[-1],
                        game.mixed_strategy(point=[math.exp(x) for x in point[:-1]]))
    else:
        raise NotImplementedError

def compute_at_lambda(game, lam):
    if game.is_symmetric():
        p = game.mixed_strategy()

        point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                   0.0, 1000000.0,
                                   lambda x: sym_compute_lhs(game, x),
                                   lambda x: sym_compute_jac(game, x),
                                   callback=None,
                                   crit=lambda x,t: x[-1] - lam,
                                   maxIter=100)

        return LogitQRE(point[-1],
                        game.mixed_strategy(point=[math.exp(x) for x in point[:-1]]))
    else:
        raise NotImplementedError

    
def diff_log_like(data, point, tangent):
    return sum([ x*y for (x, y) in zip(data, tangent[:-1]) ])


def compute_max_like(game, data):
    if game.is_symmetric():
        p = game.mixed_strategy()

        point = pctrace.trace_path([ math.log(x) for x in p.profile ],
                                   0.0, 1000000.0,
                                   lambda x: sym_compute_lhs(game, x),
                                   lambda x: sym_compute_jac(game, x),
                                   callback=None,
                                   hStart=1.0,
                                   crit=lambda x,t: diff_log_like(data,x,t),
                                   maxIter=100)

        return LogitQRE(point[-1],
                        game.mixed_strategy(point=[math.exp(x) for x in point[:-1]]))
    else:
        raise NotImplementedError
    
