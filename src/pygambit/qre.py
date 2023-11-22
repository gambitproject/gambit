#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
import scipy.optimize

from . import gambit
from . import pctrace
from .profiles import Solution


def sym_compute_lhs(game, point):
    """
    Compute the LHS for the set of equations for a symmetric logit QRE
    of a symmetric game.
    """
    profile = game.mixed_strategy_profile(
        point=[math.exp(x) for x in point[:-1]]
    )
    logprofile = point[:-1]
    lam = point[-1]

    lhs = numpy.zeros(len(profile))

    for (st, cont) in enumerate(game.choices):
        if st == 0:
            # sum-to-one equation
            lhs[st] = -1.0 + sum(profile)
        else:
            lhs[st] = (logprofile[st] - logprofile[0] -
                       lam * (profile.strategy_value(st) -
                              profile.strategy_value(0)))
    return lhs


def sym_compute_jac(game, point):
    """
    Compute the Jacobian for the set of equations for a symmetric logit QRE
    of a symmetric game.
    """
    profile = game.mixed_strategy_profile(
        point=[math.exp(x) for x in point[:-1]]
    )
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
            matrix[-1][st] = (
                profile.strategy_value(0) - profile.strategy_value(st)
            )

    return matrix


def printer(game, point):
    profile = game.mixed_strategy_profile(
        point=[math.exp(x) for x in point[:-1]]
    )
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
        return f"<LogitQRE at lam={self._lam:f}: {self._profile}>"

    @property
    def lam(self):
        return self._lam

    @property
    def mu(self):
        return 1.0 / self._lam


class StrategicQREPathTracer:
    """
    Compute the principal branch of the logit QRE correspondence of 'game'.
    """
    def __init__(self):
        self.h_start = 0.03
        self.max_decel = 1.1

    def trace_strategic_path(self, game, max_lambda=1000000.0, callback=None):
        def on_step(game, points, p, callback):
            qre = LogitQRE(
                p[-1],
                game.mixed_strategy_profile(
                    point=[math.exp(x) for x in p[:-1]]
                )
            )
            points.append(qre)
            if callback:
                callback(qre)

        points = []
        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            try:
                pctrace.trace_path(
                    [math.log(x) for x in p.profile],
                    0.0, max_lambda,
                    lambda x: sym_compute_lhs(game, x),
                    lambda x: sym_compute_jac(game, x),
                    hStart=self.h_start,
                    maxDecel=self.max_decel,
                    callback=lambda p: on_step(game, points, p, callback),
                    crit=None,
                    maxIter=100
                )
            except KeyboardInterrupt:
                pass

            return points
        else:
            raise NotImplementedError

    def compute_at_lambda(self, game, lam, callback=None):
        def on_step(p):
            return callback(
                LogitQRE(p[-1],
                         game.mixed_strategy_profile(
                             point=[math.exp(x) for x in p[:-1]]
                        ))
            )

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path(
                [math.log(x) for x in p.profile],
                0.0, 1000000.0,
                lambda x: sym_compute_lhs(game, x),
                lambda x: sym_compute_jac(game, x),
                crit=lambda x, t: x[-1] - lam,
                callback=on_step if callback is not None else None,
                maxIter=100
            )

            return LogitQRE(
                point[-1],
                game.mixed_strategy_profile(
                    point=[math.exp(x) for x in point[:-1]])
            )
        else:
            raise NotImplementedError

    def compute_max_like(self, game, data):
        def log_like(data, profile):
            return sum(x*math.log(y) for (x, y) in zip(data, profile))

        def diff_log_like(data, point, tangent):
            return sum(x*y for (x, y) in zip(data, tangent[:-1]))

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path(
                [math.log(x) for x in p.profile],
                0.0, 1000000.0,
                lambda x: sym_compute_lhs(game, x),
                lambda x: sym_compute_jac(game, x),
                hStart=1.0,
                crit=lambda x, t: diff_log_like(data, x, t),
                maxIter=100
            )

            qre = LogitQRE(
                point[-1],
                game.mixed_strategy_profile(
                    point=[math.exp(x) for x in point[:-1]]
                )
            )
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
            return 2.0 * sum((math.exp(p)-d) * t * math.exp(p)
                             for (p, t, d) in zip(point, tangent, data))

        if game.is_symmetric:
            p = game.mixed_strategy_profile()
            point = pctrace.trace_path([math.log(x) for x in p.profile],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       hStart=1.0,
                                       crit=lambda x, t: diff_dist(data, x, t),
                                       maxIter=100,
                                       callback=callback)

            qre = LogitQRE(
                point[-1],
                game.mixed_strategy_profile(
                    point=[math.exp(x) for x in point[:-1]]
                )
            )
            return qre
        else:
            raise NotImplementedError

    def compute_criterion(self, game, f):
        def criterion_wrap(x, t):
            """
            This translates the internal representation of the tracer
            into a QRE object.
            """
            return f(
                LogitQRE(
                    x[-1],
                    game.mixed_strategy_profile(
                        point=[math.exp(z) for z in x[:-1]]
                    )
                )
            )

        if game.is_symmetric:
            p = game.mixed_strategy_profile()

            point = pctrace.trace_path([math.log(x) for x in p.profile],
                                       0.0, 1000000.0,
                                       lambda x: sym_compute_lhs(game, x),
                                       lambda x: sym_compute_jac(game, x),
                                       hStart=1.0,
                                       crit=criterion_wrap,
                                       maxIter=100)

            return LogitQRE(
                point[-1],
                game.mixed_strategy_profile(
                    point=[math.exp(x) for x in point[:-1]]
                )
            )
        else:
            raise NotImplementedError


class LogitQREMixedStrategyFitResult:
    """The result of fitting a QRE to a given probability distribution
    over strategies.

    See Also
    --------
    fit_fixedpoint
    fit_empirical
    """
    def __init__(self, data, method, lam, profile, log_like):
        self._data = data
        self._method = method
        self._lam = lam
        self._profile = profile
        self._log_like = log_like

    @property
    def method(self) -> str:
        """The method used to estimate the QRE; either "fixedpoint" or "empirical"."""
        return self._method

    @property
    def data(self) -> gambit.MixedStrategyProfileDouble:
        """The empirical strategy frequencies used to estimate the QRE."""
        return self._data

    @property
    def lam(self) -> float:
        """The value of lambda corresponding to the QRE."""
        return self._lam

    @property
    def profile(self) -> gambit.MixedStrategyProfileDouble:
        """The mixed strategy profile corresponding to the QRE."""
        return self._profile

    @property
    def log_like(self) -> float:
        """The log-likelihood of the data at the estimated QRE."""
        return self._log_like

    def __repr__(self):
        return (
            f"<LogitQREMixedStrategyFitResult(method={self.method},"
            f"lam={self.lam},profile={self.profile})>"
        )


def fit_fixedpoint(
        data: gambit.MixedStrategyProfileDouble
) -> LogitQREMixedStrategyFitResult:
    """Use maximum likelihood estimation to find the logit quantal
    response equilibrium on the principal branch for a strategic game
    which best fits empirical frequencies of play. [1]_

    .. versionadded:: 16.1.0

    Parameters
    ----------
    data : MixedStrategyProfileDouble
        The empirical distribution of play to which to fit the QRE.
        To obtain the correct resulting log-likelihood, these should
        be expressed as total counts of observations of each strategy
        rather than probabilities.

    Returns
    -------
    LogitQREMixedStrategyFitResult
        The result of the estimation represented as a
        ``LogitQREMixedStrategyFitResult`` object.

    See Also
    --------
    fit_empirical : Estimate QRE by approximation of the correspondence
                    using independent decision problems.

    References
    ----------
    .. [1] Bland, J. R. and Turocy, T. L., 2023.  Quantal response equilibrium
        as a structural model for estimation: The missing manual.
        SSRN working paper 4425515.
    """
    res = gambit.logit_estimate(data)
    return LogitQREMixedStrategyFitResult(
        data, "fixedpoint", res.lam, res.profile, res.log_like
    )


def fit_empirical(
        data: gambit.MixedStrategyProfileDouble
) -> LogitQREMixedStrategyFitResult:
    """Use maximum likelihood estimation to estimate a quantal
    response equilibrium using the empirical payoff method.
    The empirical payoff method operates by ignoring the fixed-point
    considerations of the QRE and approximates instead by a collection
    of independent decision problems. [1]_

    .. versionadded:: 16.1.0

    Returns
    -------
    LogitQREMixedStrategyFitResult
        The result of the estimation represented as a
        ``LogitQREMixedStrategyFitResult`` object.

    See Also
    --------
    fit_fixedpoint : Estimate QRE precisely by computing the correspondence

    References
    ----------
    .. [1] Bland, J. R. and Turocy, T. L., 2023.  Quantal response equilibrium
        as a structural model for estimation: The missing manual.
        SSRN working paper 4425515.
    """
    def do_logit(lam: float):
        logit_probs = [[math.exp(lam*v) for v in player] for player in values]
        sums = [sum(v) for v in logit_probs]
        logit_probs = [[v/s for v in vv]
                       for (vv, s) in zip(logit_probs, sums)]
        logit_probs = [v for player in logit_probs for v in player]
        return [max(v, 1.0e-293) for v in logit_probs]

    def log_like(lam: float) -> float:
        logit_probs = do_logit(lam)
        return sum([f*math.log(p) for (f, p) in zip(list(flattened_data), logit_probs)])

    flattened_data = [data[s] for p in data.game.players for s in p.strategies]
    normalized = data.normalize()
    values = [[normalized.strategy_value(s) for s in p.strategies]
              for p in data.game.players]
    res = scipy.optimize.minimize(lambda x: -log_like(x[0]), (0.1,),
                                  bounds=((0.0, None),))
    return LogitQREMixedStrategyFitResult(
        data, "empirical", res.x[0], do_logit(res.x[0]), -res.fun
    )
