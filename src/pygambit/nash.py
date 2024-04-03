#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/nash.py
# A set of utilities for computing Nash equilibria
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
A set of utilities for computing Nash equilibria
"""

import dataclasses
import typing

import pygambit.gambit as libgbt

MixedStrategyEquilibriumSet = typing.List[libgbt.MixedStrategyProfile]
MixedBehaviorEquilibriumSet = typing.List[libgbt.MixedBehaviorProfile]


@dataclasses.dataclass(frozen=True)
class NashComputationResult:
    """Represents the result of a method which computes Nash equilibria in a game.

    Attributes
    ----------
    game : Game
        The game on which the method was run.
    method : str
        A string indicating the name of the method used.
    rational : bool
        Whether the calculation used exact rational arithmetic (True) or floating-point
        (False).
    use_strategic : bool
        Whether the method solved using the strategic representation (True) or the
        extensive representation (False).
    equilibria : MixedStrategyEquilibriumSet or MixedBehaviorEquilibriumSet
        The list of equilibrium profiles computed.
    parameters : dict
        A dictionary recording any additional algorithm parameters used.
    """
    game: libgbt.Game = dataclasses.field(repr=False)
    method: str
    rational: bool
    use_strategic: bool
    equilibria: typing.Union[MixedStrategyEquilibriumSet, MixedBehaviorEquilibriumSet]
    parameters: dict = dataclasses.field(default_factory=dict)


def enumpure_solve(game: libgbt.Game, use_strategic: bool = True) -> NashComputationResult:
    """Compute all :ref:`pure-strategy Nash equilibria <gambit-enumpure>` of game.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    use_strategic : bool, default True
        Whether to use the strategic form.  If False, computes all agent-form
        pure-strategy equilibria, which consider only unilateral deviations at each
        individual information set.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if not game.is_tree or use_strategic:
        return NashComputationResult(
            game=game,
            method="enumpure",
            rational=True,
            use_strategic=True,
            equilibria=libgbt._enumpure_strategy_solve(game)
        )
    else:
        return NashComputationResult(
            game=game,
            method="enumpure",
            rational=True,
            use_strategic=False,
            equilibria=libgbt._enumpure_agent_solve(game)
        )


def enummixed_solve(
        game: libgbt.Game,
        rational: bool = True,
        use_lrs: bool = False
) -> NashComputationResult:
    """Compute all :ref:`mixed-strategy Nash equilibria <gambit-enummixed>`
    of a two-player game using the strategic representation.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    rational : bool, default True
        Compute using rational numbers.  If `False`, using floating-point
        arithmetic.  Using rationals is more precise, but slower.
    use_lrs : bool, default False
        If `True`, use the implementation based on ``lrslib``.  This is experimental.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.

    Raises
    ------
    RuntimeError
        If game has more than two players.
    """
    if use_lrs:
        equilibria = libgbt._enummixed_strategy_solve_lrs(game)
    elif rational:
        equilibria = libgbt._enummixed_strategy_solve_rational(game)
    else:
        equilibria = libgbt._enummixed_strategy_solve_double(game)
    return NashComputationResult(
        game=game,
        method="enummixed",
        rational=use_lrs or rational,
        use_strategic=True,
        equilibria=equilibria
    )


def lcp_solve(
        game: libgbt.Game,
        rational: bool = True,
        use_strategic: bool = False,
        stop_after: typing.Optional[int] = None,
        max_depth: typing.Optional[int] = None
) -> NashComputationResult:
    """Compute Nash equilibria of a two-player game using :ref:`linear
    complementarity programming <gambit-lcp>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    rational : bool, default True
        Compute using rational numbers.  If `False`, using floating-point
        arithmetic.  Using rationals is more precise, but slower.
    use_strategic : bool, default False
        Whether to use the strategic form.  If `True`, always uses the strategic
        representation even if the game's native representation is extensive.
    stop_after : int, optional
        Maximum number of equilibria to compute.  If not specified, computes all
        accessible equilibria.
    max_depth : int, optional
        Maximum depth of recursion.  If specified, will limit the recursive search,
        but may result in some accessible equilibria not being found.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.

    Raises
    ------
    RuntimeError
        If game has more than two players.
    """
    if stop_after is None:
        stop_after = 0
    if max_depth is None:
        max_depth = 0
    if not game.is_tree or use_strategic:
        if rational:
            equilibria = libgbt._lcp_strategy_solve_rational(game, stop_after or 0, max_depth or 0)
        else:
            equilibria = libgbt._lcp_strategy_solve_double(game, stop_after or 0, max_depth or 0)
    elif rational:
        equilibria = libgbt._lcp_behavior_solve_rational(game, stop_after or 0, max_depth or 0)
    else:
        equilibria = libgbt._lcp_behavior_solve_double(game, stop_after or 0, max_depth or 0)
    return NashComputationResult(
        game=game,
        method="lcp",
        rational=rational,
        use_strategic=not game.is_tree or use_strategic,
        equilibria=equilibria,
        parameters={"stop_after": stop_after, "max_depth": max_depth}
    )


def lp_solve(
        game: libgbt.Game,
        rational: bool = True,
        use_strategic: bool = False
) -> NashComputationResult:
    """Compute Nash equilibria of a two-player constant-sum game using :ref:`linear
    programming <gambit-lp>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    rational : bool, default True
        Compute using rational numbers.  If `False`, using floating-point
        arithmetic.  Using rationals is more precise, but slower.
    use_strategic : bool, default False
        Whether to use the strategic form.  If `True`, always uses the strategic
        representation even if the game's native representation is extensive.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.

    Raises
    ------
    RuntimeError
        If game has more than two players or is not constant sum.
    """
    if not game.is_tree or use_strategic:
        if rational:
            equilibria = libgbt._lp_strategy_solve_rational(game)
        else:
            equilibria = libgbt._lp_strategy_solve_double(game)
    elif rational:
        equilibria = libgbt._lp_behavior_solve_rational(game)
    else:
        equilibria = libgbt._lp_behavior_solve_double(game)
    return NashComputationResult(
        game=game,
        method="lp",
        rational=rational,
        use_strategic=use_strategic,
        equilibria=equilibria
    )


def liap_solve(
        start: typing.Union[libgbt.MixedStrategyProfileDouble,
                            libgbt.MixedBehaviorProfileDouble],
        maxregret: float = 1.0e-4,
        maxiter: int = 1000
) -> NashComputationResult:
    """Compute approximate Nash equilibria of a game using
    :ref:`Lyapunov function minimization <gambit-liap>`.

    .. versionchanged:: 16.2.0

       Method now takes a starting point (as a mixed strategy or mixed behavior profile)
       instead of a game.  Implemented `maxregret` to specify acceptance criterion
       for approximation.

    Parameters
    ----------
    start : MixedStrategyProfileDouble or MixedBehaviorProfileDouble
        The starting profile for function minimization.  Up to one equilibrium will be found
        from any starting profile, and the equilibrium found may (and generally will)
        depend on the initial profile chosen.

    maxregret : float, default 1e-4
        The acceptance criterion for approximate Nash equilibrium; the maximum
        regret of any player must be no more than `maxregret` times the
        difference of the maximum and minimum payoffs of the game

        .. versionadded: 16.2.0

    maxiter : int, default 1000
        Maximum number of iterations in function minimization.

        .. versionadded: 16.2.0

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if maxregret <= 0.0:
        raise ValueError("liap_solve(): maxregret argument must be positive")
    if isinstance(start, libgbt.MixedStrategyProfileDouble):
        equilibria = libgbt._liap_strategy_solve(start,
                                                 maxregret=maxregret, maxiter=maxiter)
    elif isinstance(start, libgbt.MixedBehaviorProfileDouble):
        equilibria = libgbt._liap_behavior_solve(start,
                                                 maxregret=maxregret, maxiter=maxiter)
    else:
        raise TypeError(
            f"liap_solve(): start must be a MixedStrategyProfile or MixedBehaviorProfile, "
            f"not {start.__class__.__name__}"
        )
    return NashComputationResult(
        game=start.game,
        method="liap",
        rational=False,
        use_strategic=isinstance(start, libgbt.MixedStrategyProfileDouble),
        equilibria=equilibria,
        parameters={"start": start, "maxregret": maxregret, "maxiter": maxiter}
    )


def simpdiv_solve(
        start: libgbt.MixedStrategyProfileRational,
        maxregret: libgbt.Rational = None,
        refine: int = 2,
        leash: typing.Optional[int] = None
) -> NashComputationResult:
    """Compute Nash equilibria of a game using :ref:`simplicial
    subdivision <gambit-simpdiv>`.

    .. versionchanged:: 16.2.0

       Method now takes a starting point, as a mixed strategy profile, instead of a game.

    Parameters
    ----------
    start: MixedStrategyProfileRational
        The starting profile for the algorithm.  Up to one equilibrium will be found
        from any starting profile, and the equilibrium found may (and generally will)
        depend on the initial profile chosen.

    maxregret : Rational, default 1e-8
        The acceptance criterion for approximate Nash equilibrium; the maximum
        regret of any player must be no more than `maxregret` times the
        difference of the maximum and minimum payoffs of the game

        .. versionadded: 16.2.0

    refine : int, default 2
        This controls the rate at which the triangulation of the space of mixed strategy
        profiles is made more fine at each iteration.

    leash : int, optional
        Simplicial subdivision is guaranteed to converge to an (approximate) Nash equilibrium.
        The method may take arbitrarily long paths through the space of mixed strategies in
        doing so.  If specified, `leash` sets a maximum number of grid steps the method
        may explore.  This trades off the possibility of finding an equilibrium more
        quickly by giving up the guarantee than an equilibrium will necessarily be found.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if not isinstance(refine, int) or refine < 2:
        raise ValueError("simpdiv_solve(): refine must be an integer no less than 2")
    if leash is not None and (not isinstance(leash, int) or leash <= 0):
        raise ValueError("simpdiv_solve(): leash must be a non-negative integer")
    if maxregret is None:
        maxregret = libgbt.Rational(1, 10000000)
    elif maxregret < libgbt.Rational(0):
        raise ValueError("simpdiv_solve(): maxregret must be positive")
    equilibria = libgbt._simpdiv_strategy_solve(start, maxregret, refine, leash or 0)
    return NashComputationResult(
        game=start.game,
        method="simpdiv",
        rational=True,
        use_strategic=True,
        equilibria=equilibria,
        parameters={"start": start, "maxregret": maxregret, "refine": refine, "leash": leash}
    )


def ipa_solve(
        perturbation: typing.Union[libgbt.Game, libgbt.MixedStrategyProfileDouble]
) -> NashComputationResult:
    """Compute Nash equilibria of a game using :ref:`iterated polymatrix
    approximation <gambit-ipa>`.

    Parameters
    ----------
    perturbation : Game or MixedStrategyProfileDouble
        The perturbation vector to apply to the game.  If a ``Game`` is
        passed, the perturbation vector is set to be 1 for the first
        strategy for each player and 0 for all other strategies.

        .. versionchanged:: 16.2.0

           Allow selection of the perturbation vector

    Raises
    ------
    ValueError
        If the perturbation vector does not have a unique maximizer for
        each player

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if isinstance(perturbation, libgbt.Game):
        game = perturbation
        perturbation = game.mixed_strategy_profile(rational=False)
        for strategy in game.strategies:
            perturbation[strategy] = 0.0
        for player in game.players:
            perturbation[player.strategies[0]] = 1.0
    elif isinstance(perturbation, libgbt.MixedStrategyProfileDouble):
        game = perturbation.game
    else:
        raise TypeError(
            f"parameter must be Game or MixedStrategyProfileDouble, "
            f"not {perturbation.__class__.__name__}"
        )
    return NashComputationResult(
        game=game,
        method="ipa",
        rational=False,
        use_strategic=True,
        parameters={"perturbation": perturbation},
        equilibria=libgbt._ipa_strategy_solve(perturbation),
    )


def gnm_solve(
        perturbation: typing.Union[libgbt.Game, libgbt.MixedStrategyProfileDouble],
        end_lambda: float = -10.0,
        steps: int = 100,
        local_newton_interval: int = 3,
        local_newton_maxits: int = 10,
) -> NashComputationResult:
    """Compute Nash equilibria of a game using :ref:`a global Newton
    method <gambit-gnm>`.

    Parameters
    ----------
    perturbation : Game or MixedStrategyProfileDouble
        The perturbation vector to apply to the game.  If a ``Game`` is
        passed, the perturbation vector is set to be 1 for the first
        strategy for each player and 0 for all other strategies.

        .. versionchanged:: 16.2.0

           Allow selection of the perturbation vector

    end_lambda : float, default -10.0
        The value of the perturbation magnitude lambda at which to terminate
        tracing.  This must be a negative number.  This sets the point at
        which the algorithm assumes no further equilibria will be found along
        this ray.

        .. versionadded:: 16.2.0

    steps : int, default 100
        The number of steps to take within a support cell.  Lqrger values
        trade off speed for security in tracing the path.

        .. versionadded:: 16.2.0

    local_newton_interval : int, default 3
        The frequency to run a local Newton method step.  This is a
        correction step that reduces accumulated errors in the path-following.

        .. versionadded:: 16.2.0

    local_newton_maxits : int, default 10
        The maximum number of iterations in a local Newton method step.

        .. versionadded:: 16.2.0

    Raises
    ------
    ValueError
        If the perturbation vector does not have a unique maximizer for
        each player, or arguments controlling the behavior of the numerical
        tracing are not valid.

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if isinstance(perturbation, libgbt.Game):
        game = perturbation
        perturbation = game.mixed_strategy_profile(rational=False)
        for strategy in game.strategies:
            perturbation[strategy] = 0.0
        for player in game.players:
            perturbation[player.strategies[0]] = 1.0
    elif isinstance(perturbation, libgbt.MixedStrategyProfileDouble):
        game = perturbation.game
    else:
        raise TypeError(
            f"parameter must be Game or MixedStrategyProfileDouble, "
            f"not {perturbation.__class__.__name__}"
        )
    if end_lambda >= 0.0:
        raise ValueError(f"end_lambda must be a negative number; got {end_lambda}")
    if steps <= 0:
        raise ValueError(f"steps must be a positive integer; got {steps}")
    if local_newton_interval <= 0:
        raise ValueError(
            f"local_newton_interval must be a positive integer; got {local_newton_interval}"
        )
    if local_newton_maxits <= 0:
        raise ValueError(
            f"local_newton_maxits must be a positive integer; got {local_newton_maxits}"
        )
    try:
        return NashComputationResult(
            game=game,
            method="gnm",
            rational=False,
            use_strategic=True,
            parameters={"perturbation": perturbation,
                        "end_lambda": end_lambda,
                        "steps": steps,
                        "local_newton_interval": local_newton_interval,
                        "local_newton_maxits": local_newton_maxits},
            equilibria=libgbt._gnm_strategy_solve(perturbation, end_lambda,
                                                  steps,
                                                  local_newton_interval, local_newton_maxits)
        )
    except RuntimeError as e:
        if "at least one nonzero" in str(e):
            raise ValueError(str(e)) from None
        raise


def logit_solve(
        game: libgbt.Game,
        use_strategic: bool = False,
        maxregret: float = 1.0e-8,
        first_step: float = .03,
        max_accel: float = 1.1,
) -> NashComputationResult:
    """Compute Nash equilibria of a game using :ref:`the logit quantal response
    equilibrium correspondence <gambit-logit>`.

    Returns an approximation to the limiting point on the principal branch of
    the correspondence for the game.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.

    use_strategic : bool, default False
        Whether to use the strategic form.  If True, always uses the strategic
        representation even if the game's native representation is extensive.

    maxregret : float, default 1e-8
        The acceptance criterion for approximate Nash equilibrium; the maximum
        regret of any player must be no more than `maxregret` times the
        difference of the maximum and minimum payoffs of the game

        .. versionadded:: 16.2.0

    first_step : float, default .03
        The arclength of the initial step.

        .. versionadded:: 16.2.0

    max_accel : float, default 1.1
        The maximum rate at which to lengthen the arclength step size.

        .. versionadded:: 16.2.0

    Returns
    -------
    res : NashComputationResult
        The result represented as a ``NashComputationResult`` object.
    """
    if maxregret <= 0.0:
        raise ValueError("logit_solve(): maxregret argument must be positive")
    if not game.is_tree or use_strategic:
        equilibria = libgbt._logit_strategy_solve(game, maxregret, first_step, max_accel)
    else:
        equilibria = libgbt._logit_behavior_solve(game, maxregret, first_step, max_accel)
    return NashComputationResult(
        game=game,
        method="logit",
        rational=False,
        use_strategic=not game.is_tree or use_strategic,
        equilibria=equilibria,
        parameters={"first_step": first_step, "max_accel": max_accel},
    )


logit_atlambda = libgbt.logit_atlambda
logit_principal_branch = libgbt.logit_principal_branch
