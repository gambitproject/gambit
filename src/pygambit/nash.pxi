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
import pathlib
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
class HPStepEvent:
    """Reports one point traced along the HP homotopy path."""
    profile: MixedStrategyProfileDouble
    t: float


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


class HPTerminationReason(enum.Enum):
    """Why a call to :ref:`the homotopy method of Herings and Peeters <hp>` did not
    return an accepted equilibrium.
    """
    CONVERGED = 0
    TRACE_FAILED = 1
    POLISH_FAILED = 2
    REGRET_TARGET_NOT_REACHED = 3


class LiapTerminationReason(enum.Enum):
    """Why a call to :ref:`Lyapunov function minimization <liap>` did not return an
    accepted equilibrium.
    """
    CONVERGED = 0
    MINIMIZER_FAILED = 1
    REGRET_TARGET_NOT_REACHED = 2


class LogitTerminationReason(enum.Enum):
    """Why tracing the :ref:`logit QRE correspondence <logit>` to an equilibrium did
    not return an accepted equilibrium.
    """
    CONVERGED = 0
    REGRET_TARGET_NOT_REACHED = 1


@dataclasses.dataclass(frozen=True)
class LogitPathEvent:
    """Reports one point traced along the principal branch of the logit QRE
    correspondence.
    """
    qre: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"


@dataclasses.dataclass(frozen=True)
class LogitBifurcationEvent:
    """Reports a bifurcation the moment it is detected while tracing the logit QRE
    correspondence, bracketed by the last point before, and first point after, the
    change in branch orientation.
    """
    before: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"
    after: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"


@dataclasses.dataclass(frozen=True)
class LogitPerturbationEvent:
    """Reports the tracer switching a symmetry-breaking perturbation on or off while
    attempting to cross a suspected bifurcation.
    """
    active: bool
    qre: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"


@dataclasses.dataclass(frozen=True)
class LogitBifurcation:
    """Reports a bifurcation detected while tracing the logit QRE correspondence,
    bracketed by the last point before, and first point after, the change in branch
    orientation. Unlike `LogitBifurcationEvent`, this is not reported through
    `event_callback` but accumulated into `LogitResult.bifurcations`.
    """
    before: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"
    after: "LogitQREMixedStrategyProfile | LogitQREMixedBehaviorProfile"


@dataclasses.dataclass(frozen=True)
class EnumPolyCandidateSupportEvent:
    """Reports a support profile examined by :ref:`enumpoly <enumpoly>` as a candidate
    to contain a totally-mixed equilibrium.
    """
    support: StrategySupportProfile | BehaviorSupportProfile


@dataclasses.dataclass(frozen=True)
class EnumPolySingularSupportEvent:
    """Reports a support profile skipped by :ref:`enumpoly <enumpoly>` because the
    system of equations over it was singular.
    """
    support: StrategySupportProfile | BehaviorSupportProfile


@dataclasses.dataclass(frozen=True)
class EnumPolyBudgetExceededSupportEvent:
    """Reports a support profile on which :ref:`enumpoly <enumpoly>` exhausted its
    rectangle budget before completing the search for roots.
    """
    support: StrategySupportProfile | BehaviorSupportProfile


MixedStrategyEquilibriumSet = list[MixedStrategyProfile]
MixedBehaviorEquilibriumSet = list[MixedBehaviorProfile]


@dataclasses.dataclass(frozen=True, kw_only=True)
class NashResultBase:
    """Common attributes shared by every result of a method which computes Nash
    equilibria in a game.  Each solution method returns its own dataclass, derived
    from this one, whose additional attributes reflect that method's own parameters
    and the way it can succeed or fail.

    Attributes
    ----------
    game : Game
        The game on which the method was run.
    rational : bool
        Whether the calculation used exact rational arithmetic (True) or floating-point
        (False).
    use_strategic : bool
        Whether the method solved using the strategic representation (True) or the
        extensive representation (False).
    """
    game: Game = dataclasses.field(repr=False)
    rational: bool
    use_strategic: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class EnumPureResult(NashResultBase):
    """The result of enumerating pure-strategy Nash equilibria (`enumpure_solve`) or
    pure-strategy agent Nash equilibria (`enumpure_agent_solve`).

    Attributes
    ----------
    equilibria : MixedStrategyEquilibriumSet or MixedBehaviorEquilibriumSet
        The pure-strategy equilibria found, represented as degenerate mixed profiles.
    success : bool
        Always True; enumeration always completes (an empty `equilibria` means the
        game genuinely has no pure-strategy equilibrium of this kind, not a failure).
    """
    equilibria: MixedStrategyEquilibriumSet | MixedBehaviorEquilibriumSet
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class EnumMixedResult(NashResultBase):
    """The result of `enummixed_solve`.

    Attributes
    ----------
    equilibria : MixedStrategyEquilibriumSet
        The extreme equilibria found.
    success : bool
        Always True; enumeration always completes.
    lrsnash_path : pathlib.Path or str, optional
        Set if `lrsnash` was used to solve the systems of equations.
    cliques : list of MixedStrategyEquilibriumSet, optional
        Set if `cliques=True` was requested: the sets of extreme equilibria which
        are connected to one another.
    """
    equilibria: MixedStrategyEquilibriumSet
    success: bool
    lrsnash_path: pathlib.Path | str | None = None
    cliques: list[MixedStrategyEquilibriumSet] | None = None


@dataclasses.dataclass(frozen=True, kw_only=True)
class LcpStrategyResult(NashResultBase):
    """The result of `lcp_solve` when solving on the strategic representation.

    Attributes
    ----------
    stop_after : int, optional
        The maximum number of equilibria which were to be computed, if specified.
    max_depth : int, optional
        The maximum depth of recursion in the search, if specified.
    equilibria : MixedStrategyEquilibriumSet
        The equilibria found.
    success : bool
        False if `max_depth` caused the search to be pruned before it could
        establish it had found all accessible equilibria; True otherwise.
    """
    stop_after: int | None
    max_depth: int | None
    equilibria: MixedStrategyEquilibriumSet
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class LcpBehaviorResult(NashResultBase):
    """The result of `lcp_solve` when solving on the extensive representation.

    Attributes
    ----------
    equilibrium : MixedBehaviorProfile, optional
        The equilibrium found.  Lemke's algorithm on the extensive form always
        finds exactly one equilibrium, so this is never None.
    success : bool
        Always True.
    """
    equilibrium: MixedBehaviorProfile | None
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class LpResult(NashResultBase):
    """The result of `lp_solve`.

    Attributes
    ----------
    equilibrium : MixedStrategyProfile or MixedBehaviorProfile, optional
        The equilibrium found.  Linear programming always finds exactly one
        equilibrium, so this is never None.
    success : bool
        Always True.
    """
    equilibrium: MixedStrategyProfile | MixedBehaviorProfile | None
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class LiapResult(NashResultBase):
    """The result of `liap_solve` or `liap_agent_solve`.

    Attributes
    ----------
    start : MixedStrategyProfileDouble or MixedBehaviorProfileDouble
        The starting profile function minimization was run from.
    maxregret : float
        The acceptance criterion for approximate Nash equilibrium which was used.
    maxiter : int
        The maximum number of iterations in function minimization which was allowed.
    equilibrium : MixedStrategyProfileDouble or MixedBehaviorProfileDouble, optional
        The equilibrium found, or None if minimization did not reach an accepted
        equilibrium.
    success : bool
        Whether `equilibrium` was accepted.
    reason : LiapTerminationReason
        Why minimization did not reach an accepted equilibrium, if it did not.
    """
    start: MixedStrategyProfileDouble | MixedBehaviorProfileDouble
    maxregret: float
    maxiter: int
    equilibrium: MixedStrategyProfileDouble | MixedBehaviorProfileDouble | None
    success: bool
    reason: LiapTerminationReason


@dataclasses.dataclass(frozen=True, kw_only=True)
class SimpdivResult(NashResultBase):
    """The result of `simpdiv_solve`.

    Attributes
    ----------
    start : MixedStrategyProfileRational
        The starting profile the algorithm was run from.
    maxregret : Rational
        The acceptance criterion for approximate Nash equilibrium which was used.
    refine : int
        The rate at which the triangulation was refined at each iteration.
    leash : int, optional
        The maximum number of grid steps the method was allowed to explore, if
        specified.
    equilibrium : MixedStrategyProfileRational, optional
        The equilibrium found.  Simplicial subdivision is guaranteed to converge,
        so this is never None unless `leash` caused the search to be cut short.
    success : bool
        False only if `leash` was reached before converging; True otherwise.
    """
    start: MixedStrategyProfileRational
    maxregret: Rational
    refine: int
    leash: int | None
    equilibrium: MixedStrategyProfileRational | None
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class IPAResult(NashResultBase):
    """The result of `ipa_solve`.

    Attributes
    ----------
    perturbation : MixedStrategyProfileDouble
        The perturbation vector the iteration was run from.
    equilibrium : MixedStrategyProfileDouble, optional
        The equilibrium found, or None if iteration did not reach an accepted
        equilibrium.
    success : bool
        Whether `equilibrium` was accepted.
    reason : IPATerminationReason
        Why iteration did not reach an accepted equilibrium, if it did not.
    """
    perturbation: MixedStrategyProfileDouble
    equilibrium: MixedStrategyProfileDouble | None
    success: bool
    reason: IPATerminationReason


@dataclasses.dataclass(frozen=True, kw_only=True)
class GNMResult(NashResultBase):
    """The result of `gnm_solve`.

    Attributes
    ----------
    perturbation : MixedStrategyProfileDouble
        The perturbation vector the path-following run was traced from.
    end_lambda : float
        The value of the perturbation magnitude lambda at which tracing was set
        to terminate.
    steps : int
        The number of steps taken within a support cell which was used.
    local_newton_interval : int
        The frequency of local Newton method correction steps which was used.
    local_newton_maxits : int
        The maximum number of iterations in a local Newton method step which
        was allowed.
    equilibria : MixedStrategyEquilibriumSet
        The equilibria found along the path.
    success : bool
        False if the run ended in a numerical breakdown (see `reason`); True if
        it ended normally (whether or not further equilibria might exist beyond
        where tracing stopped).
    reason : GNMTerminationReason
        Why path-following terminated.
    """
    perturbation: MixedStrategyProfileDouble
    end_lambda: float
    steps: int
    local_newton_interval: int
    local_newton_maxits: int
    equilibria: MixedStrategyEquilibriumSet
    success: bool
    reason: GNMTerminationReason


@dataclasses.dataclass(frozen=True, kw_only=True)
class EnumPolyResult(NashResultBase):
    """The result of `enumpoly_solve`.

    Attributes
    ----------
    stop_after : int, optional
        The maximum number of equilibria which were to be computed, if specified.
    maxregret : float
        The acceptance criterion for approximate Nash equilibrium which was used.
    max_rectangles : int, optional
        The maximum number of rectangles examined per support which was allowed.
        Not set when `phcpack_path` was used.
    phcpack_path : pathlib.Path or str, optional
        Set if PHCpack was used to solve the systems of equations.
    equilibria : MixedStrategyEquilibriumSet or MixedBehaviorEquilibriumSet
        The equilibria found.
    success : bool
        Always True.  (A support being skipped as singular, or exhausting its
        rectangle budget, is reported only via `event_callback`; it is not yet
        reflected here.)
    """
    stop_after: int | None
    maxregret: float
    max_rectangles: int | None
    phcpack_path: pathlib.Path | str | None = None
    equilibria: MixedStrategyEquilibriumSet | MixedBehaviorEquilibriumSet
    success: bool


@dataclasses.dataclass(frozen=True, kw_only=True)
class LogitResult(NashResultBase):
    """The result of `logit_solve`.

    Attributes
    ----------
    maxregret : float
        The acceptance criterion for approximate Nash equilibrium which was used.
    first_step : float
        The arclength of the initial step which was used.
    max_accel : float
        The maximum rate at which the arclength step size was allowed to lengthen.
    equilibrium : MixedStrategyProfileDouble or MixedBehaviorProfileDouble, optional
        The equilibrium found, or None if tracing did not reach an accepted
        equilibrium.
    success : bool
        Whether `equilibrium` was accepted.
    reason : LogitTerminationReason
        Why tracing did not reach an accepted equilibrium, if it did not.
    bifurcations : list of LogitBifurcation
        Any bifurcations detected while tracing towards `equilibrium`.
    """
    maxregret: float
    first_step: float
    max_accel: float
    equilibrium: MixedStrategyProfileDouble | MixedBehaviorProfileDouble | None
    success: bool
    reason: LogitTerminationReason
    bifurcations: list[LogitBifurcation]


@dataclasses.dataclass(frozen=True, kw_only=True)
class HPResult(NashResultBase):
    """The result of `hp_solve`.

    Attributes
    ----------
    prior : MixedStrategyProfileDouble
        The prior distribution the homotopy path was traced from.
    maxregret : float
        The acceptance criterion for approximate Nash equilibrium which was used.
    equilibrium : MixedStrategyProfileDouble, optional
        The equilibrium found, or None if tracing did not reach an accepted
        equilibrium.
    success : bool
        Whether `equilibrium` was accepted.
    reason : HPTerminationReason
        Why tracing did not reach an accepted equilibrium, if it did not.
    """
    prior: MixedStrategyProfileDouble
    maxregret: float
    equilibrium: MixedStrategyProfileDouble | None
    success: bool
    reason: HPTerminationReason


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


cdef public string InvokeLogitStrategyPathEventCallback(
        callback, qre: shared_ptr[c_LogitQREMixedStrategyProfile]
):
    try:
        callback(LogitPathEvent(qre=LogitQREMixedStrategyProfile.wrap(qre)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitBehaviorPathEventCallback(
        callback, qre: shared_ptr[c_LogitQREMixedBehaviorProfile]
):
    try:
        callback(LogitPathEvent(qre=LogitQREMixedBehaviorProfile.wrap(qre)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitStrategyBifurcationEventCallback(
        callback, before: shared_ptr[c_LogitQREMixedStrategyProfile],
        after: shared_ptr[c_LogitQREMixedStrategyProfile]
):
    try:
        callback(LogitBifurcationEvent(
            before=LogitQREMixedStrategyProfile.wrap(before),
            after=LogitQREMixedStrategyProfile.wrap(after),
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitBehaviorBifurcationEventCallback(
        callback, before: shared_ptr[c_LogitQREMixedBehaviorProfile],
        after: shared_ptr[c_LogitQREMixedBehaviorProfile]
):
    try:
        callback(LogitBifurcationEvent(
            before=LogitQREMixedBehaviorProfile.wrap(before),
            after=LogitQREMixedBehaviorProfile.wrap(after),
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitStrategyPerturbationEventCallback(
        callback, active: bool, qre: shared_ptr[c_LogitQREMixedStrategyProfile]
):
    try:
        callback(LogitPerturbationEvent(
            active=active, qre=LogitQREMixedStrategyProfile.wrap(qre)
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeLogitBehaviorPerturbationEventCallback(
        callback, active: bool, qre: shared_ptr[c_LogitQREMixedBehaviorProfile]
):
    try:
        callback(LogitPerturbationEvent(
            active=active, qre=LogitQREMixedBehaviorProfile.wrap(qre)
        ))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeHPStrategyEventCallback(
        callback, profile: shared_ptr[c_MixedStrategyProfile[float]], t: float
):
    try:
        callback(HPStepEvent(profile=MixedStrategyProfileDouble.wrap(profile), t=t))
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


cdef public string InvokeEnumPolyStrategyCandidateSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(EnumPolyCandidateSupportEvent(support=StrategySupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyStrategySingularSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(EnumPolySingularSupportEvent(support=StrategySupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyStrategyBudgetExceededSupportEventCallback(
        callback, support: shared_ptr[c_StrategySupportProfile]
):
    try:
        callback(
            EnumPolyBudgetExceededSupportEvent(support=StrategySupportProfile.wrap(support))
        )
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyBehaviorCandidateSupportEventCallback(
        callback, support: shared_ptr[c_BehaviorSupportProfile]
):
    try:
        callback(EnumPolyCandidateSupportEvent(support=BehaviorSupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyBehaviorSingularSupportEventCallback(
        callback, support: shared_ptr[c_BehaviorSupportProfile]
):
    try:
        callback(EnumPolySingularSupportEvent(support=BehaviorSupportProfile.wrap(support)))
    except BaseException as e:
        return f"{type(e).__name__}: {e}".encode("utf-8")
    return b""


cdef public string InvokeEnumPolyBehaviorBudgetExceededSupportEventCallback(
        callback, support: shared_ptr[c_BehaviorSupportProfile]
):
    try:
        callback(
            EnumPolyBudgetExceededSupportEvent(support=BehaviorSupportProfile.wrap(support))
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


@cython.cfunc
def _convert_msp_opt_d(
        opt: optional[c_MixedStrategyProfile[float]]
) -> MixedStrategyProfile[double] | None:
    if not opt.has_value():
        return None
    return MixedStrategyProfileDouble.wrap(
        make_shared[c_MixedStrategyProfile[double]](opt.value())
    )


@cython.cfunc
def _convert_msp_opt_r(
        opt: optional[c_MixedStrategyProfile[c_Rational]]
) -> MixedStrategyProfile[c_Rational] | None:
    if not opt.has_value():
        return None
    return MixedStrategyProfileRational.wrap(
        make_shared[c_MixedStrategyProfile[c_Rational]](opt.value())
    )


@cython.cfunc
def _convert_mbp_opt_d(
        opt: optional[c_MixedBehaviorProfile[float]]
) -> MixedBehaviorProfile[double] | None:
    if not opt.has_value():
        return None
    return MixedBehaviorProfileDouble.wrap(
        make_shared[c_MixedBehaviorProfile[double]](opt.value())
    )


@cython.cfunc
def _convert_mbp_opt_r(
        opt: optional[c_MixedBehaviorProfile[c_Rational]]
) -> MixedBehaviorProfile[c_Rational] | None:
    if not opt.has_value():
        return None
    return MixedBehaviorProfileRational.wrap(
        make_shared[c_MixedBehaviorProfile[c_Rational]](opt.value())
    )


@cython.cfunc
def _convert_logit_bifurcations_strategy(
        bifurcations: stdvector[c_LogitBifurcationStrategy]
) -> list:
    # Indexed rather than a `for b in bifurcations` loop: Cython would declare a
    # by-value loop variable of type c_LogitBifurcationStrategy, which (like QRE
    # itself) has no default constructor.
    return [
        LogitBifurcation(
            before=LogitQREMixedStrategyProfile.wrap(
                make_shared[c_LogitQREMixedStrategyProfile](bifurcations[i].before)
            ),
            after=LogitQREMixedStrategyProfile.wrap(
                make_shared[c_LogitQREMixedStrategyProfile](bifurcations[i].after)
            ),
        )
        for i in range(bifurcations.size())
    ]


@cython.cfunc
def _convert_logit_bifurcations_behavior(
        bifurcations: stdvector[c_LogitBifurcationBehavior]
) -> list:
    return [
        LogitBifurcation(
            before=LogitQREMixedBehaviorProfile.wrap(
                make_shared[c_LogitQREMixedBehaviorProfile](bifurcations[i].before)
            ),
            after=LogitQREMixedBehaviorProfile.wrap(
                make_shared[c_LogitQREMixedBehaviorProfile](bifurcations[i].after)
            ),
        )
        for i in range(bifurcations.size())
    ]


def _enumpure_strategy_solve(game: Game, nash_callback: object = None) -> EnumPureResult:
    result: c_EnumPureStrategyResult = EnumPureStrategySolve(
        game.game, MakeStrategyCallback[c_Rational](nash_callback)
    )
    return EnumPureResult(
        game=game, rational=True, use_strategic=True,
        equilibria=_convert_mspr(result.equilibria), success=result.success,
    )


def _enumpure_agent_solve(game: Game, nash_callback: object = None) -> EnumPureResult:
    result: c_EnumPureAgentResult = EnumPureAgentSolve(
        game.game, MakeBehaviorCallback[c_Rational](nash_callback)
    )
    return EnumPureResult(
        game=game, rational=True, use_strategic=False,
        equilibria=_convert_mbpr(result.equilibria), success=result.success,
    )


def _enummixed_strategy_solve_double(game: Game, nash_callback: object = None) -> EnumMixedResult:
    result: c_EnumMixedStrategyResult[float] = EnumMixedStrategySolve[double](
        game.game, MakeStrategyCallback[double](nash_callback)
    )
    return EnumMixedResult(
        game=game, rational=False, use_strategic=True,
        equilibria=_convert_mspd(result.equilibria), success=result.success,
    )


def _enummixed_strategy_solve_rational(
        game: Game, nash_callback: object = None
) -> EnumMixedResult:
    result: c_EnumMixedStrategyResult[c_Rational] = EnumMixedStrategySolve[c_Rational](
        game.game, MakeStrategyCallback[c_Rational](nash_callback)
    )
    return EnumMixedResult(
        game=game, rational=True, use_strategic=True,
        equilibria=_convert_mspr(result.equilibria), success=result.success,
    )


def _enummixed_strategy_solve_cliques_double(
        game: Game, nash_callback: object = None
) -> EnumMixedResult:
    result: pair[
        c_EnumMixedStrategyResult[float], stdlist[stdlist[c_MixedStrategyProfile[float]]]
    ] = EnumMixedStrategySolveCliquesWrapper[double](
        game.game, MakeStrategyCallback[double](nash_callback)
    )
    return EnumMixedResult(
        game=game, rational=False, use_strategic=True,
        equilibria=_convert_mspd(result.first.equilibria),
        success=result.first.success,
        cliques=[_convert_mspd(clique) for clique in result.second],
    )


def _enummixed_strategy_solve_cliques_rational(
        game: Game, nash_callback: object = None
) -> EnumMixedResult:
    result: pair[
        c_EnumMixedStrategyResult[c_Rational],
        stdlist[stdlist[c_MixedStrategyProfile[c_Rational]]]
    ] = EnumMixedStrategySolveCliquesWrapper[c_Rational](
        game.game, MakeStrategyCallback[c_Rational](nash_callback)
    )
    return EnumMixedResult(
        game=game, rational=True, use_strategic=True,
        equilibria=_convert_mspr(result.first.equilibria),
        success=result.first.success,
        cliques=[_convert_mspr(clique) for clique in result.second],
    )


def _lcp_behavior_solve_double(game: Game, nash_callback: object = None) -> LcpBehaviorResult:
    result: c_LcpBehaviorResult[float] = LcpBehaviorSolve[double](
        game.game, MakeBehaviorCallback[double](nash_callback)
    )
    return LcpBehaviorResult(
        game=game, rational=False, use_strategic=False,
        equilibrium=_convert_mbp_opt_d(result.equilibrium), success=result.success,
    )


def _lcp_behavior_solve_rational(game: Game, nash_callback: object = None) -> LcpBehaviorResult:
    result: c_LcpBehaviorResult[c_Rational] = LcpBehaviorSolve[c_Rational](
        game.game, MakeBehaviorCallback[c_Rational](nash_callback)
    )
    return LcpBehaviorResult(
        game=game, rational=True, use_strategic=False,
        equilibrium=_convert_mbp_opt_r(result.equilibrium), success=result.success,
    )


def _lcp_strategy_solve_double(
        game: Game, stop_after, max_depth: int, nash_callback: object = None
) -> LcpStrategyResult:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    result: c_LcpStrategyResult[float] = LcpStrategySolve[double](
        game.game, c_stop_after, max_depth, MakeStrategyCallback[double](nash_callback)
    )
    return LcpStrategyResult(
        game=game, rational=False, use_strategic=True,
        stop_after=stop_after, max_depth=max_depth or None,
        equilibria=_convert_mspd(result.equilibria), success=result.success,
    )


def _lcp_strategy_solve_rational(
        game: Game, stop_after, max_depth: int, nash_callback: object = None
) -> LcpStrategyResult:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    result: c_LcpStrategyResult[c_Rational] = LcpStrategySolve[c_Rational](
        game.game, c_stop_after, max_depth, MakeStrategyCallback[c_Rational](nash_callback)
    )
    return LcpStrategyResult(
        game=game, rational=True, use_strategic=True,
        stop_after=stop_after, max_depth=max_depth or None,
        equilibria=_convert_mspr(result.equilibria), success=result.success,
    )


def _lp_behavior_solve_double(game: Game, nash_callback: object = None) -> LpResult:
    result: c_LpBehaviorResult[float] = LpBehaviorSolve[double](
        game.game, MakeBehaviorCallback[double](nash_callback)
    )
    return LpResult(
        game=game, rational=False, use_strategic=False,
        equilibrium=_convert_mbp_opt_d(result.equilibrium), success=result.success,
    )


def _lp_behavior_solve_rational(game: Game, nash_callback: object = None) -> LpResult:
    result: c_LpBehaviorResult[c_Rational] = LpBehaviorSolve[c_Rational](
        game.game, MakeBehaviorCallback[c_Rational](nash_callback)
    )
    return LpResult(
        game=game, rational=True, use_strategic=False,
        equilibrium=_convert_mbp_opt_r(result.equilibrium), success=result.success,
    )


def _lp_strategy_solve_double(game: Game, nash_callback: object = None) -> LpResult:
    result: c_LpStrategyResult[float] = LpStrategySolve[double](
        game.game, MakeStrategyCallback[double](nash_callback)
    )
    return LpResult(
        game=game, rational=False, use_strategic=True,
        equilibrium=_convert_msp_opt_d(result.equilibrium), success=result.success,
    )


def _lp_strategy_solve_rational(game: Game, nash_callback: object = None) -> LpResult:
    result: c_LpStrategyResult[c_Rational] = LpStrategySolve[c_Rational](
        game.game, MakeStrategyCallback[c_Rational](nash_callback)
    )
    return LpResult(
        game=game, rational=True, use_strategic=True,
        equilibrium=_convert_msp_opt_r(result.equilibrium), success=result.success,
    )


def _liap_strategy_solve(
        start: MixedStrategyProfileDouble, maxregret: float, maxiter: int,
        nash_callback: object = None, event_callback: object = None
) -> LiapResult:
    result: c_LiapStrategyResult = LiapStrategySolve(
        deref(start.profile), maxregret, maxiter,
        MakeStrategyCallback[double](nash_callback),
        MakeLiapEventCallback[c_MixedStrategyProfile[double]](event_callback)
    )
    return LiapResult(
        game=start.game, rational=False, use_strategic=True,
        start=start, maxregret=maxregret, maxiter=maxiter,
        equilibrium=_convert_msp_opt_d(result.equilibrium), success=result.success,
        reason=LiapTerminationReason(<int>result.reason),
    )


def _liap_behavior_solve(
        start: MixedBehaviorProfileDouble, maxregret: float, maxiter: int,
        nash_callback: object = None, event_callback: object = None
) -> LiapResult:
    result: c_LiapAgentResult = LiapAgentSolve(
        deref(start.profile), maxregret, maxiter,
        MakeBehaviorCallback[double](nash_callback),
        MakeLiapEventCallback[c_MixedBehaviorProfile[double]](event_callback)
    )
    return LiapResult(
        game=start.game, rational=False, use_strategic=False,
        start=start, maxregret=maxregret, maxiter=maxiter,
        equilibrium=_convert_mbp_opt_d(result.equilibrium), success=result.success,
        reason=LiapTerminationReason(<int>result.reason),
    )


def _simpdiv_strategy_solve(
        start: MixedStrategyProfileRational, maxregret: Rational, gridstep: int, leash: int,
        nash_callback: object = None, event_callback: object = None
) -> SimpdivResult:
    result: c_SimpdivStrategyResult = SimpdivStrategySolve(
        deref(start.profile), to_rational(str(maxregret).encode("ascii")), gridstep, leash,
        MakeStrategyCallback[c_Rational](nash_callback), MakeSimpdivEventCallback(event_callback)
    )
    return SimpdivResult(
        game=start.game, rational=True, use_strategic=True,
        start=start, maxregret=maxregret, refine=gridstep, leash=leash or None,
        equilibrium=_convert_msp_opt_r(result.equilibrium), success=result.success,
    )


def _ipa_strategy_solve(
        pert: MixedStrategyProfileDouble, nash_callback: object = None,
        event_callback: object = None
) -> IPAResult:
    try:
        result: c_IPAStrategyResult = IPAStrategySolve(
            deref(pert.profile),
            MakeStrategyCallback[double](nash_callback), MakeIPAEventCallback(event_callback)
        )
        return IPAResult(
            game=pert.game, rational=False, use_strategic=True,
            perturbation=pert,
            equilibrium=_convert_msp_opt_d(result.equilibrium), success=result.success,
            reason=IPATerminationReason(<int>result.reason),
        )
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
        nash_callback: object = None,
        event_callback: object = None,
) -> GNMResult:
    try:
        result: c_GNMStrategyResult = GNMStrategySolve(
            deref(pert.profile), end_lambda, steps, local_newton_interval, local_newton_maxits,
            MakeStrategyCallback[double](nash_callback), MakeGNMEventCallback(event_callback)
        )
        return GNMResult(
            game=pert.game, rational=False, use_strategic=True,
            perturbation=pert, end_lambda=end_lambda, steps=steps,
            local_newton_interval=local_newton_interval,
            local_newton_maxits=local_newton_maxits,
            equilibria=_convert_mspd(result.equilibria), success=result.success,
            reason=GNMTerminationReason(<int>result.reason),
        )
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
        nash_callback: object = None,
        event_callback: object = None,
) -> EnumPolyResult:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    result: c_EnumPolyStrategyResult = EnumPolyStrategySolve(
        game.game, c_stop_after, maxregret, max_rectangles,
        MakeStrategyCallback[double](nash_callback),
        MakeEnumPolyEventCallback[c_StrategySupportProfile](event_callback)
    )
    return EnumPolyResult(
        game=game, rational=False, use_strategic=True,
        stop_after=stop_after, maxregret=maxregret, max_rectangles=max_rectangles,
        equilibria=_convert_mspd(result.equilibria), success=result.success,
    )


def _enumpoly_behavior_solve(
        game: Game,
        stop_after,
        maxregret: float,
        max_rectangles: int,
        nash_callback: object = None,
        event_callback: object = None,
) -> EnumPolyResult:
    cdef optional[size_t] c_stop_after
    if stop_after is not None:
        c_stop_after = <size_t>stop_after
    result: c_EnumPolyBehaviorResult = EnumPolyBehaviorSolve(
        game.game, c_stop_after, maxregret, max_rectangles,
        MakeBehaviorCallback[double](nash_callback),
        MakeEnumPolyEventCallback[c_BehaviorSupportProfile](event_callback)
    )
    return EnumPolyResult(
        game=game, rational=False, use_strategic=False,
        stop_after=stop_after, maxregret=maxregret, max_rectangles=max_rectangles,
        equilibria=_convert_mbpd(result.equilibria), success=result.success,
    )


def _logit_strategy_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
        event_callback: object = None,
) -> LogitResult:
    result: c_LogitStrategyResult = LogitStrategySolveEquilibrium(
        deref(make_shared[c_LogitQREMixedStrategyProfile](game.game)),
        maxregret, first_step, max_accel,
        MakeLogitEventCallback[c_LogitQREMixedStrategyProfile](event_callback)
    )
    return LogitResult(
        game=game, rational=False, use_strategic=True,
        maxregret=maxregret, first_step=first_step, max_accel=max_accel,
        equilibrium=_convert_msp_opt_d(result.equilibrium), success=result.success,
        reason=LogitTerminationReason(<int>result.reason),
        bifurcations=_convert_logit_bifurcations_strategy(result.bifurcations),
    )


def _logit_behavior_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
        event_callback: object = None,
) -> LogitResult:
    result: c_LogitBehaviorResult = LogitBehaviorSolveEquilibrium(
        deref(make_shared[c_LogitQREMixedBehaviorProfile](game.game)),
        maxregret, first_step, max_accel,
        MakeLogitEventCallback[c_LogitQREMixedBehaviorProfile](event_callback)
    )
    return LogitResult(
        game=game, rational=False, use_strategic=False,
        maxregret=maxregret, first_step=first_step, max_accel=max_accel,
        equilibrium=_convert_mbp_opt_d(result.equilibrium), success=result.success,
        reason=LogitTerminationReason(<int>result.reason),
        bifurcations=_convert_logit_bifurcations_behavior(result.bifurcations),
    )


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
                             max_accel: float = 1.1,
                             event_callback: object = None) -> LogitQREMixedStrategyProfile:
    """Estimate QRE corresponding to mixed strategy profile using
    maximum likelihood along the principal branch.
    """
    return LogitQREMixedStrategyProfile.wrap(
        LogitStrategyEstimateWrapper(
            profile.profile, local_max, first_step, max_accel,
            MakeLogitEventCallback[c_LogitQREMixedStrategyProfile](event_callback)
        )
    )


def _logit_strategy_lambda(game: Game,
                           lam: float | list[float],
                           first_step: float = .03,
                           max_accel: float = 1.1,
                           event_callback: object = None) -> list[LogitQREMixedStrategyProfile]:
    """Compute the first QRE encountered along the principal branch of the strategic
    game corresponding to lambda value `lam`.
    """
    try:
        iter(lam)
    except TypeError:
        lam = [lam]
    return [LogitQREMixedStrategyProfile.wrap(profile)
            for profile in LogitStrategyAtLambdaWrapper(
                game.game, lam, first_step, max_accel,
                MakeLogitEventCallback[c_LogitQREMixedStrategyProfile](event_callback)
            )]


def _logit_strategy_branch(game: Game,
                           maxregret: float,
                           first_step: float,
                           max_accel: float):
    solns = LogitStrategyPrincipalBranchWrapper(game.game, maxregret, first_step, max_accel)
    return [LogitQREMixedStrategyProfile.wrap(profile) for profile in make_list_of_pointer(solns)]


@cython.cclass
class LogitQREMixedBehaviorProfile:
    thisptr = cython.declare(shared_ptr[c_LogitQREMixedBehaviorProfile])

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create a LogitQREMixedBehaviorProfile outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(profile: shared_ptr[c_LogitQREMixedBehaviorProfile]) -> LogitQREMixedBehaviorProfile:
        obj: LogitQREMixedBehaviorProfile = (
            LogitQREMixedBehaviorProfile.__new__(LogitQREMixedBehaviorProfile)
        )
        obj.thisptr = profile
        return obj

    def __repr__(self):
        return f"LogitQREMixedBehaviorProfile(lam={self.lam},profile={self.profile})"

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
    def profile(self) -> MixedBehaviorProfileDouble:
        """The mixed strategy profile."""
        return MixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedBehaviorProfile[double]](deref(self.thisptr).GetProfile())
        )


def _logit_behavior_estimate(profile: MixedBehaviorProfileDouble,
                             local_max: bool = False,
                             first_step: float = .03,
                             max_accel: float = 1.1,
                             event_callback: object = None) -> LogitQREMixedBehaviorProfile:
    """Estimate QRE corresponding to mixed behavior profile using
    maximum likelihood along the principal branch.
    """
    return LogitQREMixedBehaviorProfile.wrap(
        LogitBehaviorEstimateWrapper(
            profile.profile, local_max, first_step, max_accel,
            MakeLogitEventCallback[c_LogitQREMixedBehaviorProfile](event_callback)
        )
    )


def _logit_behavior_lambda(game: Game,
                           lam: float | list[float],
                           first_step: float = .03,
                           max_accel: float = 1.1,
                           event_callback: object = None) -> list[LogitQREMixedBehaviorProfile]:
    """Compute the first QRE encountered along the principal branch of the extensive
    game corresponding to lambda value `lam`.
    """
    try:
        iter(lam)
    except TypeError:
        lam = [lam]
    return [LogitQREMixedBehaviorProfile.wrap(profile)
            for profile in LogitBehaviorAtLambdaWrapper(
                game.game, lam, first_step, max_accel,
                MakeLogitEventCallback[c_LogitQREMixedBehaviorProfile](event_callback)
            )]


def _logit_behavior_branch(game: Game,
                           maxregret: float,
                           first_step: float,
                           max_accel: float):
    solns = LogitBehaviorPrincipalBranchWrapper(game.game, maxregret, first_step, max_accel)
    return [LogitQREMixedBehaviorProfile.wrap(profile) for profile in make_list_of_pointer(solns)]


def _hp_strategy_solve(
        prior: MixedStrategyProfileDouble,
        maxregret: float,
        event_callback: object = None,
) -> HPResult:
    result: c_HPStrategyResult = HPStrategySolve(
        deref(prior.profile), maxregret, MakeStrategyCallback[double](None),
        MakeHPEventCallback(event_callback)
    )
    return HPResult(
        game=prior.game, rational=False, use_strategic=True,
        prior=prior, maxregret=maxregret,
        equilibrium=_convert_msp_opt_d(result.equilibrium), success=result.success,
        reason=HPTerminationReason(<int>result.reason),
    )
