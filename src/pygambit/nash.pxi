#
# This file is part of Gambit
# Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
from libcpp.vector cimport vector as stdvector


import typing


@cython.cfunc
def _convert_mspd(
        inlist: c_List[c_MixedStrategyProfile[float]]
) -> typing.List[MixedStrategyProfile[double]]:
    return [MixedStrategyProfileDouble.wrap(copyitem_list_mspd(inlist, i+1))
            for i in range(inlist.size())]


@cython.cfunc
def _std_convert_mspd(
        inlist: stdlist[c_MixedStrategyProfile[float]]
) -> typing.List[MixedStrategyProfile[double]]:
    cdef stdvector[c_MixedStrategyProfile[float]] vector
    vector =  stdvector[c_MixedStrategyProfile[float]](inlist.begin(), inlist.end())
    outlist = []
    cdef shared_ptr[c_MixedStrategyProfile[double]] pointer
    for i in range(vector.size()):
        pointer = make_shared[c_MixedStrategyProfile[double]](vector[i])
        outlist.append(MixedStrategyProfileDouble.wrap(pointer))
    return outlist


@cython.cfunc
def _convert_mspr(
        inlist: c_List[c_MixedStrategyProfile[c_Rational]]
) -> typing.List[MixedStrategyProfile[c_Rational]]:
    return [MixedStrategyProfileRational.wrap(copyitem_list_mspr(inlist, i+1))
            for i in range(inlist.size())]


@cython.cfunc
def _std_convert_mspr(
        inlist: stdlist[c_MixedStrategyProfile[c_Rational]]
) -> typing.List[MixedStrategyProfile[c_Rational]]:
    cdef stdvector[c_MixedStrategyProfile[c_Rational]] vector
    vector =  stdvector[c_MixedStrategyProfile[c_Rational]](inlist.begin(), inlist.end())
    outlist = []
    cdef shared_ptr[c_MixedStrategyProfile[c_Rational]] pointer
    for i in range(vector.size()):
        pointer = make_shared[c_MixedStrategyProfile[c_Rational]](vector[i])
        outlist.append(MixedStrategyProfileRational.wrap(pointer))
    return outlist


@cython.cfunc
def _convert_mbpd(
        inlist: c_List[c_MixedBehaviorProfile[float]]
) -> typing.List[MixedBehaviorProfile[double]]:
    return [MixedBehaviorProfileDouble.wrap(copyitem_list_mbpd(inlist, i+1))
            for i in range(inlist.size())]


@cython.cfunc
def _std_convert_mbpd(
        inlist: stdlist[c_MixedBehaviorProfile[float]]
) -> typing.List[MixedBehaviorProfile[double]]:
    cdef stdvector[c_MixedBehaviorProfile[float]] vector
    vector =  stdvector[c_MixedBehaviorProfile[float]](inlist.begin(), inlist.end())
    outlist = []
    cdef shared_ptr[c_MixedBehaviorProfile[double]] pointer
    for i in range(vector.size()):
        pointer = make_shared[c_MixedBehaviorProfile[double]](vector[i])
        outlist.append(MixedBehaviorProfileDouble.wrap(pointer))
    return outlist


@cython.cfunc
def _convert_mbpr(
        inlist: c_List[c_MixedBehaviorProfile[c_Rational]]
) -> typing.List[MixedBehaviorProfile[c_Rational]]:
    return [MixedBehaviorProfileRational.wrap(copyitem_list_mbpr(inlist, i+1))
            for i in range(inlist.size())]


@cython.cfunc
def _std_convert_mbpr(
        inlist: stdlist[c_MixedBehaviorProfile[c_Rational]]
) -> typing.List[MixedBehaviorProfile[c_Rational]]:
    cdef stdvector[c_MixedBehaviorProfile[c_Rational]] vector
    vector =  stdvector[c_MixedBehaviorProfile[c_Rational]](inlist.begin(), inlist.end())
    outlist = []
    cdef shared_ptr[c_MixedBehaviorProfile[c_Rational]] pointer
    for i in range(vector.size()):
        pointer = make_shared[c_MixedBehaviorProfile[c_Rational]](vector[i])
        outlist.append(MixedBehaviorProfileRational.wrap(pointer))
    return outlist


def _enumpure_strategy_solve(game: Game) -> typing.List[MixedStrategyProfile[c_Rational]]:
    return _std_convert_mspr(EnumPureStrategySolve(game.game))


def _enumpure_agent_solve(game: Game) -> typing.List[MixedBehaviorProfileRational]:
    return _std_convert_mbpr(EnumPureAgentSolve(game.game))


def _enummixed_strategy_solve_double(game: Game) -> typing.List[MixedStrategyProfileDouble]:
    return _std_convert_mspd(EnumMixedStrategySolve[double](game.game))


def _enummixed_strategy_solve_rational(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _std_convert_mspr(EnumMixedStrategySolve[c_Rational](game.game))


def _lcp_behavior_solve_double(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedBehaviorProfileDouble]:
    return _std_convert_mbpd(LcpBehaviorSolve[double](game.game, stop_after, max_depth))


def _lcp_behavior_solve_rational(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedBehaviorProfileRational]:
    return _std_convert_mbpr(LcpBehaviorSolve[c_Rational](game.game, stop_after, max_depth))


def _lcp_strategy_solve_double(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedStrategyProfileDouble]:
    return _std_convert_mspd(LcpStrategySolve[double](game.game, stop_after, max_depth))


def _lcp_strategy_solve_rational(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedStrategyProfileRational]:
    return _std_convert_mspr(LcpStrategySolve[c_Rational](game.game, stop_after, max_depth))


def _lp_behavior_solve_double(game: Game) -> typing.List[MixedBehaviorProfileDouble]:
    return _std_convert_mbpd(LpBehaviorSolve[double](game.game))


def _lp_behavior_solve_rational(game: Game) -> typing.List[MixedBehaviorProfileRational]:
    return _std_convert_mbpr(LpBehaviorSolve[c_Rational](game.game))


def _lp_strategy_solve_double(game: Game) -> typing.List[MixedStrategyProfileDouble]:
    return _std_convert_mspd(LpStrategySolve[double](game.game))


def _lp_strategy_solve_rational(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _std_convert_mspr(LpStrategySolve[c_Rational](game.game))


def _liap_strategy_solve(start: MixedStrategyProfileDouble,
                         maxregret: float,
                         maxiter: int) -> typing.List[MixedStrategyProfileDouble]:
    return _std_convert_mspd(LiapStrategySolve(deref(start.profile), maxregret, maxiter))


def _liap_behavior_solve(start: MixedBehaviorProfileDouble,
                         maxregret: float,
                         maxiter: int) -> typing.List[MixedBehaviorProfileDouble]:
    return _std_convert_mbpd(LiapBehaviorSolve(deref(start.profile), maxregret, maxiter))


def _simpdiv_strategy_solve(
        start: MixedStrategyProfileRational, maxregret: Rational, gridstep: int, leash: int
) -> typing.List[MixedStrategyProfileRational]:
    return _std_convert_mspr(SimpdivStrategySolve(deref(start.profile),
                                              to_rational(str(maxregret).encode("ascii")),
                                              gridstep, leash))


def _ipa_strategy_solve(
        pert: MixedStrategyProfileDouble
) -> typing.List[MixedStrategyProfileDouble]:
    try:
        return _std_convert_mspd(IPAStrategySolve(deref(pert.profile)))
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
) -> typing.List[MixedStrategyProfileDouble]:
    try:
        return _std_convert_mspd(GNMStrategySolve(deref(pert.profile), end_lambda,
                                              steps, local_newton_interval, local_newton_maxits))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _nashsupport_strategy_solve(game: Game) -> typing.List[StrategySupportProfile]:
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
) -> typing.List[MixedStrategyProfileDouble]:
    return _std_convert_mspd(EnumPolyStrategySolve(game.game, stop_after, maxregret))


def _enumpoly_behavior_solve(
        game: Game,
        stop_after: int,
        maxregret: float,
) -> typing.List[MixedBehaviorProfileDouble]:
    return _std_convert_mbpd(EnumPolyBehaviorSolve(game.game, stop_after, maxregret))


def _logit_strategy_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(LogitStrategySolveWrapper(game.game, maxregret, first_step, max_accel))


def _logit_behavior_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> typing.List[MixedBehaviorProfileDouble]:
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
                           lam: typing.Union[float, typing.List[float]],
                           first_step: float = .03,
                           max_accel: float = 1.1) -> typing.List[LogitQREMixedStrategyProfile]:
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
    return [LogitQREMixedStrategyProfile.wrap(copyitem_list_qrem(solns, i+1))
            for i in range(solns.size())]


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
                           lam: typing.Union[float, typing.List[float]],
                           first_step: float = .03,
                           max_accel: float = 1.1) -> typing.List[LogitQREMixedBehaviorProfile]:
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
    for i in range(solns.size()):
        p = LogitQREMixedBehaviorProfile()
        p.thisptr = copyitem_list_qreb(solns, i+1)
        ret.append(p)
    return ret
