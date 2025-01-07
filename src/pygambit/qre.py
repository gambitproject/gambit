#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
from __future__ import annotations

import math

import scipy.optimize

import pygambit.gambit as libgbt


def logit_solve_branch(
        game: libgbt.Game,
        use_strategic: bool = False,
        maxregret: float = 1.0e-8,
        first_step: float = .03,
        max_accel: float = 1.1,
):
    if maxregret <= 0.0:
        raise ValueError("logit_solve_branch(): maxregret argument must be positive")
    if first_step <= 0.0:
        raise ValueError("logit_solve_branch(): first_step argument must be positive")
    if max_accel < 1.0:
        raise ValueError("logit_solve_branch(): max_accel argument must be at least 1.0")
    if not game.is_tree or use_strategic:
        return libgbt._logit_strategy_branch(game, maxregret, first_step, max_accel)
    else:
        return libgbt._logit_behavior_branch(game, maxregret, first_step, max_accel)


def logit_solve_lambda(
        game: libgbt.Game,
        lam: float | list[float],
        use_strategic: bool = False,
        first_step: float = .03,
        max_accel: float = 1.1,
):
    if first_step <= 0.0:
        raise ValueError("logit_solve_lambda(): first_step argument must be positive")
    if max_accel < 1.0:
        raise ValueError("logit_solve_lambda(): max_accel argument must be at least 1.0")
    if not game.is_tree or use_strategic:
        return libgbt._logit_strategy_lambda(game, lam, first_step, max_accel)
    else:
        return libgbt._logit_behavior_lambda(game, lam, first_step, max_accel)


class LogitQREMixedStrategyFitResult:
    """The result of fitting a QRE to a given probability distribution
    over strategies.

    See Also
    --------
    logit_estimate
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
    def data(self) -> libgbt.MixedStrategyProfileDouble:
        """The empirical strategy frequencies used to estimate the QRE."""
        return self._data

    @property
    def lam(self) -> float:
        """The value of lambda corresponding to the QRE."""
        return self._lam

    @property
    def profile(self) -> libgbt.MixedStrategyProfileDouble:
        """The mixed strategy profile corresponding to the QRE."""
        return self._profile

    @property
    def log_like(self) -> float:
        """The log-likelihood of the data at the estimated QRE."""
        return self._log_like

    def __repr__(self) -> str:
        return (
            f"<LogitQREMixedStrategyFitResult(method={self.method},"
            f"lam={self.lam},profile={self.profile})>"
        )


class LogitQREMixedBehaviorFitResult:
    """The result of fitting a QRE to a given probability distribution
    over actions.

    See Also
    --------
    logit_estimate
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
    def data(self) -> libgbt.MixedBehaviorProfileDouble:
        """The empirical actions frequencies used to estimate the QRE."""
        return self._data

    @property
    def lam(self) -> float:
        """The value of lambda corresponding to the QRE."""
        return self._lam

    @property
    def profile(self) -> libgbt.MixedBehaviorProfileDouble:
        """The mixed behavior profile corresponding to the QRE."""
        return self._profile

    @property
    def log_like(self) -> float:
        """The log-likelihood of the data at the estimated QRE."""
        return self._log_like

    def __repr__(self) -> str:
        return (
            f"<LogitQREMixedBehaviorFitResult(method={self.method},"
            f"lam={self.lam},profile={self.profile})>"
        )


def _estimate_strategy_fixedpoint(
        data: libgbt.MixedStrategyProfileDouble,
        local_max: bool = False,
        first_step: float = .03,
        max_accel: float = 1.1,
) -> LogitQREMixedStrategyFitResult:
    res = libgbt._logit_strategy_estimate(data, local_max=local_max,
                                          first_step=first_step, max_accel=max_accel)
    return LogitQREMixedStrategyFitResult(
        data, "fixedpoint", res.lam, res.profile, res.log_like
    )


def _estimate_behavior_fixedpoint(
        data: libgbt.MixedBehaviorProfileDouble,
        local_max: bool = False,
        first_step: float = .03,
        max_accel: float = 1.1,
) -> LogitQREMixedBehaviorFitResult:
    res = libgbt._logit_behavior_estimate(data, local_max=local_max,
                                          first_step=first_step, max_accel=max_accel)
    return LogitQREMixedBehaviorFitResult(
        data, "fixedpoint", res.lam, res.profile, res.log_like
    )


def _empirical_log_logit_probs(lam: float, regrets: list) -> list:
    """Given empirical choice regrets and a value of lambda (`lam`), compute the
    log-probabilities given by the logit choice model.
    """
    log_sums = [
        math.log(sum([math.exp(lam*r) for r in infoset]))
        for infoset in regrets
    ]
    return [lam*a - s for (r, s) in zip(regrets, log_sums) for a in r]


def _empirical_log_like(lam: float, regrets: list, flattened_data: list) -> float:
    """Given empirical choice regrets and a list of frequencies of choices, compute
    the log-likelihood of the choices given the regrets and assuming the logit
    choice model with lambda `lam`."""
    return sum([f*p for (f, p) in zip(flattened_data, _empirical_log_logit_probs(lam, regrets))])


def _estimate_strategy_empirical(
        data: libgbt.MixedStrategyProfileDouble
) -> LogitQREMixedStrategyFitResult:
    flattened_data = [data[s] for p in data.game.players for s in p.strategies]
    normalized = data.normalize()
    regrets = [[-normalized.strategy_regret(s) for s in player.strategies]
               for player in data.game.players]
    res = scipy.optimize.minimize(
        lambda x: -_empirical_log_like(x[0], regrets, flattened_data),
        (0.1,),
        bounds=((0.0, None),)
    )
    profile = data.game.mixed_strategy_profile()
    for strategy, log_prob in zip(data.game.strategies,
                                  _empirical_log_logit_probs(res.x[0], regrets)):
        profile[strategy] = math.exp(log_prob)
    return LogitQREMixedStrategyFitResult(
        data, "empirical", res.x[0], profile, -res.fun
    )


def _estimate_behavior_empirical(
        data: libgbt.MixedBehaviorProfileDouble,
) -> LogitQREMixedBehaviorFitResult:
    flattened_data = [data[a] for p in data.game.players for s in p.infosets for a in s.actions]
    normalized = data.normalize()
    regrets = [[-normalized.action_regret(a) for a in infoset.actions]
               for player in data.game.players for infoset in player.infosets]
    res = scipy.optimize.minimize(
        lambda x: -_empirical_log_like(x[0], regrets, flattened_data),
        (0.1,),
        bounds=((0.0, None),)
    )
    profile = data.game.mixed_behavior_profile()
    for action, log_prob in zip(data.game.actions, _empirical_log_logit_probs(res.x[0], regrets)):
        profile[action] = math.exp(log_prob)
    return LogitQREMixedBehaviorFitResult(
        data, "empirical", res.x[0], profile, -res.fun
    )


def logit_estimate(
        data: libgbt.MixedStrategyProfile | libgbt.MixedBehaviorProfile,
        use_empirical: bool = False,
        local_max: bool = False,
        first_step: float = .03,
        max_accel: float = 1.1,
) -> LogitQREMixedStrategyFitResult | LogitQREMixedBehaviorFitResult:
    """Use maximum likelihood estimation to find the logit quantal
    response equilibrium which best fits empirical frequencies of play.

    .. versionadded:: 16.3.0

    Parameters
    ----------
    data : MixedStrategyProfile or MixedBehaviorProfile
        The empirical distribution of play to which to fit the QRE.
        To obtain the correct resulting log-likelihood, these should
        be expressed as total counts of observations of each action
        rather than probabilities.  If a MixedBehaviorProfile is
        specified, estimation is done using the agent QRE.

    use_empirical : bool, default = False
        If specified and True, use the empirical payoff approach for
        estimation.  This replaces the payoff matrix of the game with an
        approximation as a collection of individual decision problems based
        on the empirical expected payoffs to strategies or actions.
        This is computationally much faster but in most cases produces
        estimates which deviate systematically from those obtained by
        computing the QRE correspondence of the game.  See the discussion
        in [1]_ for more details.

    local_max : bool, default False
        The default behavior is to find the global maximiser along
        the principal branch.  If this parameter is set to True,
        tracing stops at the first interior local maximiser found.

        .. note::

           This argument only has an effect when use_empirical is False.

    first_step : float, default .03
        The arclength of the initial step.

        .. note::

           This argument only has an effect when use_empirical is False.

    max_accel : float, default 1.1
        The maximum rate at which to lengthen the arclength step size.

        .. note::

           This argument only has an effect when use_empirical is False.

    Returns
    -------
    LogitQREMixedStrategyFitResult or LogitQREMixedBehaviorFitResult
        The result of the estimation represented as a
        ``LogitQREMixedStrategyFitResult`` or ``LogitQREMixedBehaviorFitResult``
        object, as appropriate.

    References
    ----------
    .. [1] Bland, J. R. and Turocy, T. L., 2023.  Quantal response equilibrium
        as a structural model for estimation: The missing manual.
        SSRN working paper 4425515.
    """
    if isinstance(data, libgbt.MixedStrategyProfile):
        if use_empirical:
            return _estimate_strategy_empirical(data)
        else:
            return _estimate_strategy_fixedpoint(data, local_max=local_max,
                                                 first_step=first_step, max_accel=max_accel)
    elif isinstance(data, libgbt.MixedBehaviorProfile):
        if use_empirical:
            return _estimate_behavior_empirical(data)
        else:
            return _estimate_behavior_fixedpoint(data, local_max=local_max,
                                                 first_step=first_step, max_accel=max_accel)
    else:
        raise TypeError("data must be specified as a MixedStrategyProfile or MixedBehaviorProfile")
