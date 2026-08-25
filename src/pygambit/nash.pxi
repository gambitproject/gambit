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
import dataclasses
import enum
from libcpp.memory cimport shared_ptr, make_shared
from cython.operator cimport dereference as deref
from libcpp.list cimport list as stdlist
from libcpp.string cimport string


class GNMTerminationReason(enum.Enum):
    """Why a :ref:`global Newton method <gnm>` path-following run terminated."""
    NO_MORE_BOUNDARIES = 0
    NO_NEXT_BOUNDARY = 1
    NONFINITE_STRATEGY = 2
    LAMBDA_OUT_OF_RANGE = 3
    EXCESSIVE_ERROR = 4


@dataclasses.dataclass(frozen=True)
class GNMPerturbationEvent:
    """Reports the perturbation vector a GNM path is about to be traced from."""
    profile: MixedStrategyProfileDouble


@dataclasses.dataclass(frozen=True)
class GNMStartEvent:
    """Reports the starting point of a GNM path, after following the perturbation ray."""
    profile: MixedStrategyProfileDouble


@dataclasses.dataclass(frozen=True)
class GNMStepEvent:
    """Reports one predictor-corrector step taken while tracing a GNM path."""
    profile: MixedStrategyProfileDouble
    lam: float


@dataclasses.dataclass(frozen=True)
class GNMTerminationEvent:
    """Reports why a GNM path-following run terminated."""
    reason: GNMTerminationReason
    message: str


@dataclasses.dataclass(frozen=True)
class LiapStartEvent:
    """Reports the starting point of a :ref:`Lyapunov function minimization <liap>` run."""
    profile: MixedStrategyProfileDouble | MixedBehaviorProfileDouble


@dataclasses.dataclass(frozen=True)
class LiapEndEvent:
    """Reports the point, and regret attained, at the end of a
    :ref:`Lyapunov function minimization <liap>` run.
    """
    profile: MixedStrategyProfileDouble | MixedBehaviorProfileDouble
    regret: float


@dataclasses.dataclass(frozen=True)
class SimpdivStartEvent:
    """Reports the starting point of a :ref:`simplicial subdivision <simpdiv>` run."""
    profile: MixedStrategyProfileRational


@dataclasses.dataclass(frozen=True)
class SimpdivRefinementEvent:
    """Reports the profile, grid size, and regret reached each time
    :ref:`simplicial subdivision <simpdiv>` refines its triangulation.
    """
    profile: MixedStrategyProfileRational
    grid_size: Rational
    regret: Rational


class IPATerminationReason(enum.Enum):
    """Why a call to :ref:`iterated polymatrix approximation <ipa>` terminated."""
    CONVERGED = 0
    MAX_ITERATIONS_REACHED = 1
    NONFINITE_STRATEGY = 2


@dataclasses.dataclass(frozen=True)
class IPAStepEvent:
    """Reports the state of the approximating strategy profile at the end of one
    :ref:`iterated polymatrix approximation <ipa>` iteration.
    """
    profile: MixedStrategyProfileDouble
    iteration: int
    z_diff: float
    s_diff: float


@dataclasses.dataclass(frozen=True)
class IPATerminationEvent:
    """Reports why a call to :ref:`iterated polymatrix approximation <ipa>` terminated.

    Because IPA may internally restart the iteration from where it left off if it has
    not yet converged, this may be raised more than once in the course of solving.
    """
    reason: IPATerminationReason
    message: str


@dataclasses.dataclass(frozen=True)
class EnumPolyCandidateSupportEvent:
    """Reports a support profile examined by :ref:`enumpoly <enumpoly>` as a candidate
    to contain a totally-mixed equilibrium.
    """
    support: StrategySupportProfile


@dataclasses.dataclass(frozen=True)
class EnumPolySingularSupportEvent:
    """Reports a support profile skipped by :ref:`enumpoly <enumpoly>` because the
    system of equations over it was singular.
    """
    support: StrategySupportProfile


@dataclasses.dataclass(frozen=True)
class EnumPolyBudgetExceededSupportEvent:
    """Reports a support profile on which :ref:`enumpoly <enumpoly>` exhausted its
    rectangle budget before completing the search for roots.
    """
    support: StrategySupportProfile


cdef public string InvokeStrategyCallbackDouble(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]]
):
    try:
        callback(MixedStrategyProfileDouble.wrap(profile))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeStrategyCallbackRational(
        callback, profile: shared_ptr[c_MixedStrategyProfile[c_Rational]]
):
    try:
        callback(MixedStrategyProfileRational.wrap(profile))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeBehaviorCallbackDouble(
        callback, profile: shared_ptr[c_MixedBehaviorProfile[float]]
):
    try:
        callback(MixedBehaviorProfileDouble.wrap(profile))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeBehaviorCallbackRational(
        callback, profile: shared_ptr[c_MixedBehaviorProfile[c_Rational]]
):
    try:
        callback(MixedBehaviorProfileRational.wrap(profile))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitStrategyEventCallback(
        callback, qre: shared_ptr[c_LogitQREMixedStrategyProfile]
):
    try:
        callback(LogitQREMixedStrategyProfile.wrap(qre))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitBehaviorEventCallback(
        callback, qre: shared_ptr[c_LogitQREMixedBehaviorProfile]
):
    ret = LogitQREMixedBehaviorProfile()
    ret.thisptr = qre
    try:
        callback(ret)
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeGNMPerturbationEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]]
):
    try:
        callback(GNMPerturbationEvent(profile=MixedStrategyProfileDouble.wrap(profile)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeGNMStartEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]]
):
    try:
        callback(GNMStartEvent(profile=MixedStrategyProfileDouble.wrap(profile)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeGNMStepEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]], lam: float
):
    try:
        callback(GNMStepEvent(profile=MixedStrategyProfileDouble.wrap(profile), lam=lam))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeGNMTerminationEventCallback(
        callback, reason: cython.int, message: string
):
    try:
        callback(GNMTerminationEvent(
            reason=GNMTerminationReason(reason), message=message.decode("utf-8")
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLiapStrategyStartEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]]
):
    try:
        callback(LiapStartEvent(profile=MixedStrategyProfileDouble.wrap(profile)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLiapStrategyEndEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]], regret: float
):
    try:
        callback(LiapEndEvent(profile=MixedStrategyProfileDouble.wrap(profile), regret=regret))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLiapBehaviorStartEventCallback(
        callback, profile: shared_ptr[c_MixedBehaviorProfile[float]]
):
    try:
        callback(LiapStartEvent(profile=MixedBehaviorProfileDouble.wrap(profile)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLiapBehaviorEndEventCallback(
        callback, profile: shared_ptr[c_MixedBehaviorProfile[float]], regret: float
):
    try:
        callback(LiapEndEvent(profile=MixedBehaviorProfileDouble.wrap(profile), regret=regret))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeSimpdivStartEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[c_Rational]]
):
    try:
        callback(SimpdivStartEvent(profile=MixedStrategyProfileRational.wrap(profile)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeSimpdivRefinementEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[c_Rational]],
        grid_size: c_Rational, regret: c_Rational
):
    try:
        callback(SimpdivRefinementEvent(
            profile=MixedStrategyProfileRational.wrap(profile),
            grid_size=rat_to_py(grid_size), regret=rat_to_py(regret)
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeIPAStepEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]],
        iteration: cython.int, z_diff: float, s_diff: float
):
    try:
        callback(IPAStepEvent(
            profile=MixedStrategyProfileDouble.wrap(profile),
            iteration=iteration, z_diff=z_diff, s_diff=s_diff
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeIPATerminationEventCallback(
        callback, reason: cython.int, message: string
):
    try:
        callback(IPATerminationEvent(
            reason=IPATerminationReason(reason), message=message.decode("utf-8")
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyCandidateSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(EnumPolyCandidateSupportEvent(support=StrategySupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolySingularSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(EnumPolySingularSupportEvent(support=StrategySupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyBudgetExceededSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(
            EnumPolyBudgetExceededSupportEvent(support=StrategySupportProfile.wrap(support))
        )
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


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


def _enumpure_strategy_solve(
        game: Game, eqm_callback: object = None
) -> list[MixedStrategyProfile[c_Rational]]:
    return _convert_mspr(
        EnumPureStrategySolve(game.game, MakeStrategyCallback[c_Rational](eqm_callback))
    )


def _enumpure_agent_solve(
        game: Game, eqm_callback: object = None
) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(
        EnumPureAgentSolve(game.game, MakeBehaviorCallback[c_Rational](eqm_callback))
    )


def _enummixed_strategy_solve_double(
        game: Game, eqm_callback: object = None
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(
        EnumMixedStrategySolve[double](game.game, MakeStrategyCallback[double](eqm_callback))
    )


def _enummixed_strategy_solve_rational(
        game: Game, eqm_callback: object = None
) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(
        EnumMixedStrategySolve[c_Rational](
            game.game, MakeStrategyCallback[c_Rational](eqm_callback)
        )
    )


def _lcp_behavior_solve_double(
        game: Game, eqm_callback: object = None
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(
        LcpBehaviorSolve[double](game.game, MakeBehaviorCallback[double](eqm_callback))
    )


def _lcp_behavior_solve_rational(
        game: Game, eqm_callback: object = None
) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(
        LcpBehaviorSolve[c_Rational](game.game, MakeBehaviorCallback[c_Rational](eqm_callback))
    )


def _lcp_strategy_solve_double(
        game: Game, stop_after, max_depth: int, eqm_callback: object = None
) -> list[MixedStrategyProfileDouble]:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    return _convert_mspd(
        LcpStrategySolve[double](
            game.game, c_stop_after, max_depth, MakeStrategyCallback[double](eqm_callback)
        )
    )


def _lcp_strategy_solve_rational(
        game: Game, stop_after, max_depth: int, eqm_callback: object = None
) -> list[MixedStrategyProfileRational]:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    return _convert_mspr(
        LcpStrategySolve[c_Rational](
            game.game, c_stop_after, max_depth, MakeStrategyCallback[c_Rational](eqm_callback)
        )
    )


def _lp_behavior_solve_double(
        game: Game, eqm_callback: object = None
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(
        LpBehaviorSolve[double](game.game, MakeBehaviorCallback[double](eqm_callback))
    )


def _lp_behavior_solve_rational(
        game: Game, eqm_callback: object = None
) -> list[MixedBehaviorProfileRational]:
    return _convert_mbpr(
        LpBehaviorSolve[c_Rational](game.game, MakeBehaviorCallback[c_Rational](eqm_callback))
    )


def _lp_strategy_solve_double(
        game: Game, eqm_callback: object = None
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(
        LpStrategySolve[double](game.game, MakeStrategyCallback[double](eqm_callback))
    )


def _lp_strategy_solve_rational(
        game: Game, eqm_callback: object = None
) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(
        LpStrategySolve[c_Rational](game.game, MakeStrategyCallback[c_Rational](eqm_callback))
    )


def _liap_strategy_solve(start: MixedStrategyProfileDouble,
                         maxregret: float,
                         maxiter: int,
                         eqm_callback: object = None,
                         event_callback: object = None) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LiapStrategySolve(
        deref(start.profile), maxregret, maxiter,
        MakeStrategyCallback[double](eqm_callback),
        MakeLiapEventCallback[c_MixedStrategyProfile[double]](event_callback)
    ))


def _liap_behavior_solve(start: MixedBehaviorProfileDouble,
                         maxregret: float,
                         maxiter: int,
                         eqm_callback: object = None,
                         event_callback: object = None) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LiapAgentSolve(
        deref(start.profile), maxregret, maxiter,
        MakeBehaviorCallback[double](eqm_callback),
        MakeLiapEventCallback[c_MixedBehaviorProfile[double]](event_callback)
    ))


def _simpdiv_strategy_solve(
        start: MixedStrategyProfileRational, maxregret: Rational, gridstep: int, leash: int,
        eqm_callback: object = None, event_callback: object = None
) -> list[MixedStrategyProfileRational]:
    return _convert_mspr(SimpdivStrategySolve(
        deref(start.profile), to_rational(str(maxregret).encode("ascii")), gridstep, leash,
        MakeStrategyCallback[c_Rational](eqm_callback), MakeSimpdivEventCallback(event_callback)
    ))


def _ipa_strategy_solve(
        pert: MixedStrategyProfileDouble, eqm_callback: object = None,
        event_callback: object = None
) -> list[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(IPAStrategySolve(
            deref(pert.profile),
            MakeStrategyCallback[double](eqm_callback), MakeIPAEventCallback(event_callback)
        ))
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
        eqm_callback: object = None,
        event_callback: object = None,
) -> list[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(GNMStrategySolve(
            deref(pert.profile), end_lambda, steps, local_newton_interval, local_newton_maxits,
            MakeStrategyCallback[double](eqm_callback), MakeGNMEventCallback(event_callback)
        ))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _nashsupport_strategy_solve(
        game: Game
) -> typing.Generator[StrategySupportProfile, None, None]:
    generator: shared_ptr[c_PossibleNashStrategySupports] = (
        shared_ptr[c_PossibleNashStrategySupports](
            new c_PossibleNashStrategySupports(game.game)
        )
    )
    result: optional[c_StrategySupportProfile]
    while True:
        result = deref(generator).Next()
        if not result.has_value():
            return
        yield StrategySupportProfile.wrap(make_shared[c_StrategySupportProfile](result.value()))


def _enumpoly_strategy_solve(
        game: Game,
        stop_after,
        maxregret: float,
        max_rectangles: int,
        eqm_callback: object = None,
        event_callback: object = None,
) -> list[MixedStrategyProfileDouble]:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    return _convert_mspd(EnumPolyStrategySolve(
        game.game, c_stop_after, maxregret, max_rectangles,
        MakeStrategyCallback[double](eqm_callback),
        MakeEnumPolyEventCallback[c_StrategySupportProfile](event_callback)
    ))


def _enumpoly_behavior_solve(
        game: Game,
        stop_after,
        maxregret: float,
        max_rectangles: int,
        eqm_callback: object = None,
) -> list[MixedBehaviorProfileDouble]:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    return _convert_mbpd(EnumPolyBehaviorSolve(
        game.game, c_stop_after, maxregret, max_rectangles,
        MakeBehaviorCallback[double](eqm_callback)
    ))


def _logit_strategy_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
        event_callback: object = None,
) -> list[MixedStrategyProfileDouble]:
    return _convert_mspd(LogitStrategySolveWrapper(
        game.game, maxregret, first_step, max_accel,
        MakeLogitEventCallback[c_LogitQREMixedStrategyProfile](event_callback)
    ))


def _logit_behavior_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
        event_callback: object = None,
) -> list[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LogitBehaviorSolveWrapper(
        game.game, maxregret, first_step, max_accel,
        MakeLogitEventCallback[c_LogitQREMixedBehaviorProfile](event_callback)
    ))


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
