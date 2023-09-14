#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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

import subprocess
import typing
import warnings

import pygambit.gambit as libgbt


class ExternalSolver:
    """
    Base class for managing calls to external programs.
    """
    def _call(self, prog, game) -> str:
        """
        Helper function for launching calls to external programs.
        Calls the specified program 'prog', passing the game to standard
        input in .efg format (if a tree) or .nfg format (if a table).
        Returns the output of the external program.
        """
        warnings.warn(
            "Calling external command-line solvers is deprecated and support "
            "will be removed in a future version.",
            FutureWarning
        )
        p = subprocess.run(
            prog.split() + ["-q"],
            input=game.write(format='native'),
            encoding='utf-8',
            capture_output=True
        )
        p.check_returncode()
        return p.stdout

    def _parse_output(self, output: str, game, rational, extensive=False):
        profiles = []
        for line in output.splitlines():
            entries = line.strip().split(",")
            if entries[0] != "NE":
                continue
            if extensive:
                profile = game.mixed_behavior_profile(rational=rational)
            else:
                profile = game.mixed_strategy_profile(rational=rational)
            for (i, p) in enumerate(entries[1:]):
                if rational:
                    profile[i] = libgbt.Rational(p)
                else:
                    profile[i] = float(p)
            profiles.append(profile)
        return profiles


class ExternalEnumPureSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpure solver
    for computing pure-strategy equilibria.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-enumpure"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=True,
                                  extensive=game.is_tree and not use_strategic)


class ExternalLPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lp solver
    for computing equilibria in two-player games using linear programming.
    """
    def solve(self, game, rational=False, use_strategic=False):
        if len(game.players) != 2:
            raise RuntimeError("Method only valid for two-player games.")
        if not game.is_const_sum:
            raise RuntimeError("Method only valid for constant-sum games.")
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        if rational:
            command_line = "gambit-lp"
        else:
            command_line = "gambit-lp -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational,
                                  extensive=game.is_tree and not use_strategic)


class ExternalLCPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lcp solver
    for computing equilibria in two-player games using linear complementarity
    programming.
    """
    def solve(self, game, rational=False, use_strategic=False):
        if len(game.players) != 2:
            raise RuntimeError("Method only valid for two-player games.")
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        if rational:
            command_line = "gambit-lcp"
        else:
            command_line = "gambit-lcp -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational,
                                  extensive=game.is_tree and not use_strategic)


class ExternalEnumMixedSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enummixed solver
    for computing equilibria in two-player games using enumeration of
    extreme points.
    """
    def solve(self, game, rational=False):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        if rational:
            command_line = "gambit-enummixed"
        else:
            command_line = "gambit-enummixed -d 10"
        return self._parse_output(self._call(command_line, game),
                                  game, rational)


class ExternalSimpdivSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-simpdiv solver
    for computing equilibria in N-player games using simpicial subdivision.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-simpdiv"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=True)


class ExternalGlobalNewtonSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-gnm solver
    for computing equilibria in N-player games using the global Newton method.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-gnm -d 10"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=False)


class ExternalEnumPolySolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpoly solver
    for computing equilibria in N-player games systems of polynomial equations.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-enumpoly -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)


class ExternalLyapunovSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-liap solver
    for computing equilibria in N-player games using Lyapunov function
    minimization.
    """
    def solve(self, game, use_strategic=False, maxiter=100):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = f"gambit-liap -d 10 -i {maxiter}"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)


class ExternalIteratedPolymatrixSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-ipa solver
    for computing equilibria in N-player games using iterated
    polymatrix approximation.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-ipa -d 10"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=False)


class ExternalLogitSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for computing equilibria in N-player games using quantal response
    equilibrium.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError(
                "Computing equilibria of games with imperfect recall "
                "is not supported."
            )
        command_line = "gambit-logit -d 20 -e"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self._call(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)


def enumpure_solve(
        game: libgbt.Game,
        use_strategic: bool = True,
        external: bool = False
) -> typing.Union[typing.List[libgbt.MixedStrategyProfile],
                  typing.List[libgbt.MixedBehaviorProfile]]:
    """Compute all :ref:`pure-strategy Nash equilibria <gambit-enumpure>` of game.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    use_strategic : bool, default True
        Whether to use the strategic form.  If `False`, computes all agent-form
        pure-strategy equilibria, which treat only unilateral deviations at each
        individual information set.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of profiles
        List of mixed strategy or mixed behavior profiles computed.
    """
    if external:
        return ExternalEnumPureSolver().solve(game, use_strategic=True)
    if not game.is_tree or use_strategic:
        return libgbt._enumpure_strategy_solve(game)
    else:
        return libgbt._enumpure_agent_solve(game)


def enummixed_solve(
        game: libgbt.Game,
        rational: bool = True,
        external: bool = False,
        use_lrs: bool = False
) -> typing.List[libgbt.MixedStrategyProfile]:
    """Compute all :ref:`mixed-strategy Nash equilibria <gambit-enummixed>`
    of a two-player game using the strategic representation.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    rational : bool, default True
        Compute using rational numbers.  If `False`, using floating-point
        arithmetic.  Using rationals is more precise, but slower.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.
    use_lrs : bool, default False
        If `True`, use the implementation based on ``lrslib``.  This is experimental.

    Returns
    -------
    List of mixed strategy profiles
        The list of mixed strategy profiles computed.

    Raises
    ------
    RuntimeError
        If game has more than two players.
    """
    if external:
        return ExternalEnumMixedSolver().solve(game, rational=rational)
    if use_lrs:
        return libgbt._enummixed_strategy_solve_lrs(game)
    elif rational:
        return libgbt._enummixed_strategy_solve_rational(game)
    else:
        return libgbt._enummixed_strategy_solve_double(game)


def lcp_solve(
        game: libgbt.Game,
        rational: bool = True,
        use_strategic: bool = False,
        external: bool = False,
        stop_after: typing.Optional[int] = None,
        max_depth: typing.Optional[int] = None
) -> typing.Union[typing.List[libgbt.MixedStrategyProfile],
                  typing.List[libgbt.MixedBehaviorProfile]]:
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
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.
    stop_after : int, optional
        Maximum number of equilibria to compute.  If not specified, computes all
        accessible equilibria.
    max_depth : int, optional
        Maximum depth of recursion.  If specified, will limit the recursive search,
        but may result in some accessible equilibria not being found.

    Returns
    -------
    List of profiles
        List of mixed strategy or mixed behavior profiles computed.

    Raises
    ------
    RuntimeError
        If game has more than two players.
    """
    if stop_after is None:
        stop_after = 0
    if max_depth is None:
        max_depth = 0
    if external:
        return ExternalLCPSolver().solve(game, rational=rational,
                                         use_strategic=use_strategic)
    if not game.is_tree or use_strategic:
        if rational:
            return libgbt._lcp_strategy_solve_rational(game, stop_after, max_depth)
        else:
            return libgbt._lcp_strategy_solve_double(game, stop_after, max_depth)
    else:
        if rational:
            return libgbt._lcp_behavior_solve_rational(game, stop_after, max_depth)
        else:
            return libgbt._lcp_behavior_solve_double(game, stop_after, max_depth)


def lp_solve(
        game: libgbt.Game,
        rational: bool = True,
        use_strategic: bool = False,
        external: bool = False
) -> typing.Union[typing.List[libgbt.MixedStrategyProfile],
                  typing.List[libgbt.MixedBehaviorProfile]]:
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
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of profiles
        List of mixed strategy or mixed behavior profiles computed.

    Raises
    ------
    RuntimeError
        If game has more than two players or is not constant sum.
    """
    if external:
        return ExternalLPSolver().solve(game, rational=rational,
                                        use_strategic=use_strategic)
    if not game.is_tree or use_strategic:
        if rational:
            return libgbt._lp_strategy_solve_rational(game)
        else:
            return libgbt._lp_strategy_solve_double(game)
    else:
        if rational:
            return libgbt._lp_behavior_solve_rational(game)
        else:
            return libgbt._lp_behavior_solve_double(game)


def liap_solve(
        game: libgbt.Game,
        use_strategic: bool = True,
        maxiter: int = 100,
        external: bool = False
) -> typing.Union[typing.List[libgbt.MixedStrategyProfile],
                  typing.List[libgbt.MixedBehaviorProfile]]:
    """Compute Nash equilibria of a game using
    :ref:`Lyapunov function minimization <gambit-liap>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    use_strategic : bool, default False
        Whether to use the strategic form.  If `True`, always uses the strategic
        representation even if the game's native representation is extensive.
    maxiter : int, default 100
        Maximum number of iterations in function minimization.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of profiles
        List of mixed strategy or mixed behavior profiles computed.
    """
    if external:
        return ExternalLyapunovSolver().solve(game, use_strategic=True)
    if not game.is_tree or use_strategic:
        return libgbt._liap_strategy_solve(game, maxiter=maxiter)
    else:
        return libgbt._liap_behavior_solve(game, maxiter=maxiter)


def simpdiv_solve(
        game: libgbt.Game,
        external: bool = False
) -> typing.List[libgbt.MixedStrategyProfile]:
    """Compute Nash equilibria of a game using :ref:`simplicial
    subdivision <gambit-simpdiv>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of mixed strategy profiles
        The list of mixed strategy profiles computed.
    """
    if external:
        return ExternalSimpdivSolver().solve(game)
    return libgbt._simpdiv_strategy_solve(game)


def ipa_solve(
        game: libgbt.Game,
        external: bool = False
) -> typing.List[libgbt.MixedStrategyProfile]:
    """Compute Nash equilibria of a game using :ref:`iterated polymatrix
    approximation <gambit-ipa>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of mixed strategy profiles
        The list of mixed strategy profiles computed.
    """
    if external:
        return ExternalIteratedPolymatrixSolver().solve(game)
    return libgbt._ipa_strategy_solve(game)


def gnm_solve(
        game: libgbt.Game,
        external: bool = False
) -> typing.List[libgbt.MixedStrategyProfile]:
    """Compute Nash equilibria of a game using :ref:`a global Newton
    method <gambit-gnm>`.

    Parameters
    ----------
    game : Game
        The game to compute equilibria in.
    external : bool, default False
        Call the external command-line solver instead of the internally-linked
        implementation.  Requires the command-line solvers to be installed somewhere
        accessible in the system path.

    Returns
    -------
    List of mixed strategy profiles
        The list of mixed strategy profiles computed.
    """
    if external:
        return ExternalGlobalNewtonSolver().solve(game)
    return libgbt._gnm_strategy_solve(game)


logit_atlambda = libgbt.logit_atlambda
logit_principal_branch = libgbt.logit_principal_branch
