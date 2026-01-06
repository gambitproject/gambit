#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/nash.pxi
# Cython wrapper for Nash equilibrium computations
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
import cython
from libcpp.memory cimport shared_ptr, make_shared
from cython.operator cimport dereference as deref
from libcpp.list cimport list as stdlist


@cython.cfunc
def _convert_mspd(
        inlist: stdlist[c_MixedStrategyProfile[float]]
) -> list[MixedStrategyProfile[double]]:
    return [MixedStrategyProfileDouble.wrap(profile)
            for profile in make_list_of_pointer(inlist)]


@cython.cfunc
def _convert_mspr(
        inlist: stdlist[c_MixedStrategyProfile[c_Rational]]
) -> list[MixedStrategyProfile[c_Rational]]:
    return [MixedStrategyProfileRational.wrap(profile)
            for profile in make_list_of_pointer(inlist)]


@cython.cfunc
def _convert_mbpd(
        inlist: stdlist[c_MixedBehaviorProfile[float]]
) -> list[MixedBehaviorProfile[double]]:
    return [MixedBehaviorProfileDouble.wrap(profile)
            for profile in make_list_of_pointer(inlist)]


@cython.cfunc
def _convert_mbpr(
        inlist: stdlist[c_MixedBehaviorProfile[c_Rational]]
) -> list[MixedBehaviorProfile[c_Rational]]:
    return [MixedBehaviorProfileRational.wrap(profile)
            for profile in make_list_of_pointer(inlist)]


def _enumpure_strategy_solve(game: Game) -> list[MixedStrategyProfile[c_Rational]]:
    return _convert_mspr(EnumPureStrategySolve(game.game))


def _enumpure_agent_solve(game: Game) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(EnumPureAgentSolve(game.game))


def _enummixed_strategy_solve_double(game: Game) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(EnumMixedStrategySolve[double](game.game))


def _enummixed_strategy_solve_rational(game: Game) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(EnumMixedStrategySolve[c_Rational](game.game))


def _lcp_behavior_solve_double(
        game: Game
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LcpBehaviorSolve[double](game.game))


def _lcp_behavior_solve_rational(
        game: Game
) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(LcpBehaviorSolve[c_Rational](game.game))


def _lcp_strategy_solve_double(
        game: Game, stop_after: int, max_depth: int
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LcpStrategySolve[double](game.game, stop_after, max_depth))


def _lcp_strategy_solve_rational(
        game: Game, stop_after: int, max_depth: int
) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(LcpStrategySolve[c_Rational](game.game, stop_after, max_depth))


def _lp_behavior_solve_double(game: Game) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LpBehaviorSolve[double](game.game))


def _lp_behavior_solve_rational(game: Game) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(LpBehaviorSolve[c_Rational](game.game))


def _lp_strategy_solve_double(game: Game) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LpStrategySolve[double](game.game))


def _lp_strategy_solve_rational(game: Game) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(LpStrategySolve[c_Rational](game.game))


def _liap_strategy_solve(start: MixedStrategyProfileDouble,
                         maxregret: float,
                         maxiter: int) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LiapStrategySolve(deref(start.profile), maxregret, maxiter))


def _liap_behavior_solve(start: MixedBehaviorProfileDouble,
                         maxregret: float,
                         maxiter: int) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LiapAgentSolve(deref(start.profile), maxregret, maxiter))


def _simpdiv_strategy_solve(
        start: MixedStrategyProfileRational, maxregret: Rational, gridstep: int, leash: int
) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(SimpdivStrategySolve(deref(start.profile),
                                              to_rational(str(maxregret).encode("ascii")),
                                              gridstep, leash))


def _ipa_strategy_solve(
        pert: MixedStrategyProfileDouble
) -> list[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(IPAStrategySolve(deref(pert.profile)))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _gnm_strategy_solve(
        pert: MixedStrategyProfileDouble,
        end_lambda: float,
        steps: int,
        local_newton_interval: int,
        local_newton_maxits: int,
) -> list[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(GNMStrategySolve(deref(pert.profile), end_lambda,
                                              steps, local_newton_interval, local_newton_maxits))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _nashsupport_strategy_solve(game: Game) -> list[StrategySupportProfile]:
    return [
        StrategySupportProfile.wrap(support)
        for support in make_list_of_pointer(
            deref(PossibleNashStrategySupports(game.game)).m_supports
        )
    ]


def _enumpoly_strategy_solve(
        game: Game,
        stop_after: int,
        maxregret: float,
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(EnumPolyStrategySolve(game.game, stop_after, maxregret))


def _enumpoly_behavior_solve(
        game: Game,
        stop_after: int,
        maxregret: float,
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(EnumPolyBehaviorSolve(game.game, stop_after, maxregret))


def _logit_strategy_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LogitStrategySolveWrapper(game.game, maxregret, first_step, max_accel))


def _logit_behavior_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LogitBehaviorSolveWrapper(game.game, maxregret, first_step, max_accel))


@cython.cclass
class LogitQREMixedStrategyProfile:
    thisptr = cython.declare(shared_ptr[c_LogitQREMixedStrategyProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a LogitQREMixedStrategyProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_LogitQREMixedStrategyProfile]) -> LogitQREMixedStrategyProfile:
        obj: LogitQREMixedStrategyProfile = (
            LogitQREMixedStrategyProfile.__new__(LogitQREMixedStrategyProfile)
        )
        obj.thisptr = profile
        return obj

    def __repr__(self):
        return "LogitQREMixedStrategyProfile(lam=%f,profile=%s)" % (self.lam, self.profile)

    def __len__(self):
        return deref(self.thisptr).size()

    def __getitem__(self, int i):
        return deref(self.thisptr).getitem(i+1)

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        return Game.wrap(deref(self.thisptr).GetGame())

    @property
    def lam(self) -> double:
        """The value of the precision parameter."""
        return deref(self.thisptr).GetLambda()

    @property
    def log_like(self) -> double:
        """The log-likelihood of the data."""
        return deref(self.thisptr).GetLogLike()

    @property
    def profile(self) -> MixedStrategyProfileDouble:
        """The mixed strategy profile."""
        return MixedStrategyProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](deref(self.thisptr).GetProfile())
        )


def _logit_strategy_estimate(profile: MixedStrategyProfileDouble,
                             local_max: bool = False,
                             first_step: float = .03,
                             max_accel: float = 1.1) -> LogitQREMixedStrategyProfile:
    """Estimate QRE corresponding to mixed strategy profile using
    maximum likelihood along the principal branch.
    """
    return LogitQREMixedStrategyProfile.wrap(
        LogitStrategyEstimateWrapper(profile.profile, local_max, first_step, max_accel)
    )


def _logit_strategy_lambda(game: Game,
                           lam: float | list[float],
                           first_step: float = .03,
                           max_accel: float = 1.1) -> list[LogitQREMixedStrategyProfile]:
    """Compute the first QRE encountered along the principal branch of the strategic
    game corresponding to lambda value `lam`.
    """
    try:
        iter(lam)
    except TypeError:
        lam = [lam]
    return [LogitQREMixedStrategyProfile.wrap(profile)
            for profile in LogitStrategyAtLambdaWrapper(game.game, lam, first_step, max_accel)]


def _logit_strategy_branch(game: Game,
                           maxregret: float,
                           first_step: float,
                           max_accel: float):
    solns = LogitStrategyPrincipalBranchWrapper(game.game, maxregret, first_step, max_accel)
    return [LogitQREMixedStrategyProfile.wrap(profile) for profile in make_list_of_pointer(solns)]


@cython.cclass
class LogitQREMixedBehaviorProfile:
    thisptr = cython.declare(shared_ptr[c_LogitQREMixedBehaviorProfile])

    def __init__(self, game=None):
        if game is not None:
            self.thisptr = make_shared[c_LogitQREMixedBehaviorProfile](
                cython.cast(Game, game).game
            )

    def __repr__(self):
        return f"LogitQREMixedBehaviorProfile(lam={self.lam},profile={self.profile})"

    def __len__(self):
        return deref(self.thisptr).size()

    def __getitem__(self, int i):
        return deref(self.thisptr).getitem(i+1)

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        g = Game()
        g.game = deref(self.thisptr).GetGame()
        return g

    @property
    def lam(self) -> double:
        """The value of the precision parameter."""
        return deref(self.thisptr).GetLambda()

    @property
    def log_like(self) -> double:
        """The log-likelihood of the data."""
        return deref(self.thisptr).GetLogLike()

    @property
    def profile(self) -> MixedBehaviorProfileDouble:
        """The mixed strategy profile."""
        profile = MixedBehaviorProfileDouble()
        profile.profile = (
            make_shared[c_MixedBehaviorProfile[double]](deref(self.thisptr).GetProfile())
        )
        return profile


def _logit_behavior_estimate(profile: MixedBehaviorProfileDouble,
                             local_max: bool = False,
                             first_step: float = .03,
                             max_accel: float = 1.1) -> LogitQREMixedBehaviorProfile:
    """Estimate QRE corresponding to mixed behavior profile using
    maximum likelihood along the principal branch.
    """
    ret = LogitQREMixedBehaviorProfile(profile.game)
    ret.thisptr = LogitBehaviorEstimateWrapper(profile.profile, local_max, first_step, max_accel)
    return ret


def _logit_behavior_lambda(game: Game,
                           lam: float | list[float],
                           first_step: float = .03,
                           max_accel: float = 1.1) -> list[LogitQREMixedBehaviorProfile]:
    """Compute the first QRE encountered along the principal branch of the extensive
    game corresponding to lambda value `lam`.
    """
    try:
        iter(lam)
    except TypeError:
        lam = [lam]
    ret = []
    for profile in LogitBehaviorAtLambdaWrapper(game.game, lam, first_step, max_accel):
        qre = LogitQREMixedBehaviorProfile()
        qre.thisptr = profile
        ret.append(qre)
    return ret


def _logit_behavior_branch(game: Game,
                           maxregret: float,
                           first_step: float,
                           max_accel: float):
    solns = LogitBehaviorPrincipalBranchWrapper(game.game, maxregret, first_step, max_accel)
    ret = []
    for profile_ptr in make_list_of_pointer(solns):
        p = LogitQREMixedBehaviorProfile()
        p.thisptr = profile_ptr
        ret.append(p)
    return ret
