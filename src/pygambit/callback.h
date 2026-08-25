//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/pygambit/callback.h
// Adapts Python callables into the C++ solver callback types
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GAMBIT_PYGAMBIT_CALLBACK_H
#define GAMBIT_PYGAMBIT_CALLBACK_H

#include <Python.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/rational.h"
#include "solvers/enumpoly/enumpoly.h"
#include "solvers/gnm/gnm.h"
#include "solvers/ipa/ipa.h"
#include "solvers/liap/liap.h"
#include "solvers/logit/logit.h"
#include "solvers/nash.h"
#include "solvers/simpdiv/simpdiv.h"

//
// Trampolines into Cython (defined as `cdef public` functions in nash.pxi),
// each invoking a Python callback with the profile/event wrapped as the
// corresponding pygambit type. They report a Python exception raised by the
// callback by returning its description; an empty string means success.
//
std::string
InvokeStrategyCallbackDouble(PyObject *p_callback,
                             std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile);
std::string InvokeStrategyCallbackRational(
    PyObject *p_callback,
    std::shared_ptr<Gambit::MixedStrategyProfile<Gambit::Rational>> p_profile);
std::string
InvokeBehaviorCallbackDouble(PyObject *p_callback,
                             std::shared_ptr<Gambit::MixedBehaviorProfile<double>> p_profile);
std::string InvokeBehaviorCallbackRational(
    PyObject *p_callback,
    std::shared_ptr<Gambit::MixedBehaviorProfile<Gambit::Rational>> p_profile);
std::string
InvokeLogitStrategyEventCallback(PyObject *p_callback,
                                 std::shared_ptr<Gambit::LogitQREMixedStrategyProfile> p_qre);
std::string
InvokeLogitBehaviorEventCallback(PyObject *p_callback,
                                 std::shared_ptr<Gambit::LogitQREMixedBehaviorProfile> p_qre);
std::string InvokeGNMPerturbationEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile);
std::string
InvokeGNMStartEventCallback(PyObject *p_callback,
                            std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile);
std::string
InvokeGNMStepEventCallback(PyObject *p_callback,
                           std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile,
                           double p_lambda);
std::string InvokeGNMTerminationEventCallback(PyObject *p_callback, int p_reason,
                                              std::string p_message);
std::string InvokeLiapStrategyStartEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile);
std::string
InvokeLiapStrategyEndEventCallback(PyObject *p_callback,
                                   std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile,
                                   double p_regret);
std::string InvokeLiapBehaviorStartEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::MixedBehaviorProfile<double>> p_profile);
std::string
InvokeLiapBehaviorEndEventCallback(PyObject *p_callback,
                                   std::shared_ptr<Gambit::MixedBehaviorProfile<double>> p_profile,
                                   double p_regret);
std::string InvokeSimpdivStartEventCallback(
    PyObject *p_callback,
    std::shared_ptr<Gambit::MixedStrategyProfile<Gambit::Rational>> p_profile);
std::string InvokeSimpdivRefinementEventCallback(
    PyObject *p_callback,
    std::shared_ptr<Gambit::MixedStrategyProfile<Gambit::Rational>> p_profile,
    Gambit::Rational p_gridSize, Gambit::Rational p_regret);
std::string
InvokeIPAStepEventCallback(PyObject *p_callback,
                           std::shared_ptr<Gambit::MixedStrategyProfile<double>> p_profile,
                           int p_iteration, double p_zDiff, double p_sDiff);
std::string InvokeIPATerminationEventCallback(PyObject *p_callback, int p_reason,
                                              std::string p_message);
std::string InvokeEnumPolyStrategyCandidateSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::StrategySupportProfile> p_support);
std::string InvokeEnumPolyStrategySingularSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::StrategySupportProfile> p_support);
std::string InvokeEnumPolyStrategyBudgetExceededSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::StrategySupportProfile> p_support);
std::string InvokeEnumPolyBehaviorCandidateSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::BehaviorSupportProfile> p_support);
std::string InvokeEnumPolyBehaviorSingularSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::BehaviorSupportProfile> p_support);
std::string InvokeEnumPolyBehaviorBudgetExceededSupportEventCallback(
    PyObject *p_callback, std::shared_ptr<Gambit::BehaviorSupportProfile> p_support);

namespace Gambit {

/// Raises a std::runtime_error carrying the description of a Python
/// exception caught by one of the Invoke*Callback trampolines above.
inline void ThrowIfPythonError(const std::string &p_error)
{
  if (!p_error.empty()) {
    throw std::runtime_error(p_error);
  }
}

} // namespace Gambit

///
/// Builds a Nash::StrategyCallbackType<T> which, when invoked, calls a
/// Python callable with the profile found so far, converted to the
/// corresponding pygambit type. A null callback (Python `None`) yields the
/// solver's own no-op default, at no cost beyond the check.
///
template <class T>
Gambit::Nash::StrategyCallbackType<T> MakeStrategyCallback(PyObject *p_callback);

template <>
inline Gambit::Nash::StrategyCallbackType<double>
MakeStrategyCallback<double>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullStrategyCallback<double>;
  }
  return [p_callback](const Gambit::MixedStrategyProfile<double> &p_profile) {
    Gambit::ThrowIfPythonError(InvokeStrategyCallbackDouble(
        p_callback, std::make_shared<Gambit::MixedStrategyProfile<double>>(p_profile)));
  };
}

template <>
inline Gambit::Nash::StrategyCallbackType<Gambit::Rational>
MakeStrategyCallback<Gambit::Rational>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullStrategyCallback<Gambit::Rational>;
  }
  return [p_callback](const Gambit::MixedStrategyProfile<Gambit::Rational> &p_profile) {
    Gambit::ThrowIfPythonError(InvokeStrategyCallbackRational(
        p_callback, std::make_shared<Gambit::MixedStrategyProfile<Gambit::Rational>>(p_profile)));
  };
}

template <class T>
Gambit::Nash::BehaviorCallbackType<T> MakeBehaviorCallback(PyObject *p_callback);

template <>
inline Gambit::Nash::BehaviorCallbackType<double>
MakeBehaviorCallback<double>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullBehaviorCallback<double>;
  }
  return [p_callback](const Gambit::MixedBehaviorProfile<double> &p_profile) {
    Gambit::ThrowIfPythonError(InvokeBehaviorCallbackDouble(
        p_callback, std::make_shared<Gambit::MixedBehaviorProfile<double>>(p_profile)));
  };
}

template <>
inline Gambit::Nash::BehaviorCallbackType<Gambit::Rational>
MakeBehaviorCallback<Gambit::Rational>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullBehaviorCallback<Gambit::Rational>;
  }
  return [p_callback](const Gambit::MixedBehaviorProfile<Gambit::Rational> &p_profile) {
    Gambit::ThrowIfPythonError(InvokeBehaviorCallbackRational(
        p_callback, std::make_shared<Gambit::MixedBehaviorProfile<Gambit::Rational>>(p_profile)));
  };
}

///
/// Builds a LogitEventCallbackType<QRE> which, when invoked with a
/// path-tracing event, calls a Python callable with the QRE point traced so
/// far. A null callback (Python `None`) yields the solver's own no-op
/// default.
///
template <class QRE>
Gambit::LogitEventCallbackType<QRE> MakeLogitEventCallback(PyObject *p_callback);

template <>
inline Gambit::LogitEventCallbackType<Gambit::LogitQREMixedStrategyProfile>
MakeLogitEventCallback<Gambit::LogitQREMixedStrategyProfile>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::NullLogitEventCallback<Gambit::LogitQREMixedStrategyProfile>;
  }
  return [p_callback](const Gambit::LogitEvent<Gambit::LogitQREMixedStrategyProfile> &p_event) {
    const auto &qre =
        std::get<Gambit::LogitPathEvent<Gambit::LogitQREMixedStrategyProfile>>(p_event).qre;
    Gambit::ThrowIfPythonError(InvokeLogitStrategyEventCallback(
        p_callback, std::make_shared<Gambit::LogitQREMixedStrategyProfile>(qre)));
  };
}

template <>
inline Gambit::LogitEventCallbackType<Gambit::LogitQREMixedBehaviorProfile>
MakeLogitEventCallback<Gambit::LogitQREMixedBehaviorProfile>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::NullLogitEventCallback<Gambit::LogitQREMixedBehaviorProfile>;
  }
  return [p_callback](const Gambit::LogitEvent<Gambit::LogitQREMixedBehaviorProfile> &p_event) {
    const auto &qre =
        std::get<Gambit::LogitPathEvent<Gambit::LogitQREMixedBehaviorProfile>>(p_event).qre;
    Gambit::ThrowIfPythonError(InvokeLogitBehaviorEventCallback(
        p_callback, std::make_shared<Gambit::LogitQREMixedBehaviorProfile>(qre)));
  };
}

///
/// Builds a Nash::GNMEventCallbackType which, when invoked, dispatches to
/// whichever Invoke*EventCallback trampoline matches the alternative held by
/// the event, calling a Python callable with the corresponding pygambit
/// event object. A null callback (Python `None`) yields the solver's own
/// no-op default.
///
inline Gambit::Nash::GNMEventCallbackType MakeGNMEventCallback(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullGNMEventCallback;
  }
  return [p_callback](const Gambit::Nash::GNMEvent &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::GNMPerturbationEvent>) {
            Gambit::ThrowIfPythonError(InvokeGNMPerturbationEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::GNMStartEvent>) {
            Gambit::ThrowIfPythonError(InvokeGNMStartEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::GNMStepEvent>) {
            Gambit::ThrowIfPythonError(InvokeGNMStepEventCallback(
                p_callback, std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile),
                event.lambda));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::GNMTerminationEvent>) {
            Gambit::ThrowIfPythonError(InvokeGNMTerminationEventCallback(
                p_callback, static_cast<int>(event.reason), event.message));
          }
        },
        p_event);
  };
}

///
/// Builds a Nash::LiapEventCallbackType<Profile> which, when invoked,
/// dispatches to whichever Invoke*EventCallback trampoline matches the
/// alternative held by the event (start of a minimization run, or its end
/// with the regret attained), calling a Python callable with the
/// corresponding pygambit event object. A null callback (Python `None`)
/// yields the solver's own no-op default.
///
template <class Profile>
Gambit::Nash::LiapEventCallbackType<Profile> MakeLiapEventCallback(PyObject *p_callback);

template <>
inline Gambit::Nash::LiapEventCallbackType<Gambit::MixedStrategyProfile<double>>
MakeLiapEventCallback<Gambit::MixedStrategyProfile<double>>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullLiapEventCallback<Gambit::MixedStrategyProfile<double>>;
  }
  return [p_callback](
             const Gambit::Nash::LiapEvent<Gambit::MixedStrategyProfile<double>> &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::LiapStartEvent<
                                                  Gambit::MixedStrategyProfile<double>>>) {
            Gambit::ThrowIfPythonError(InvokeLiapStrategyStartEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::LiapEndEvent<
                                                       Gambit::MixedStrategyProfile<double>>>) {
            Gambit::ThrowIfPythonError(InvokeLiapStrategyEndEventCallback(
                p_callback, std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile),
                event.regret));
          }
        },
        p_event);
  };
}

template <>
inline Gambit::Nash::LiapEventCallbackType<Gambit::MixedBehaviorProfile<double>>
MakeLiapEventCallback<Gambit::MixedBehaviorProfile<double>>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullLiapEventCallback<Gambit::MixedBehaviorProfile<double>>;
  }
  return [p_callback](
             const Gambit::Nash::LiapEvent<Gambit::MixedBehaviorProfile<double>> &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::LiapStartEvent<
                                                  Gambit::MixedBehaviorProfile<double>>>) {
            Gambit::ThrowIfPythonError(InvokeLiapBehaviorStartEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedBehaviorProfile<double>>(event.profile)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::LiapEndEvent<
                                                       Gambit::MixedBehaviorProfile<double>>>) {
            Gambit::ThrowIfPythonError(InvokeLiapBehaviorEndEventCallback(
                p_callback, std::make_shared<Gambit::MixedBehaviorProfile<double>>(event.profile),
                event.regret));
          }
        },
        p_event);
  };
}

///
/// Builds a Nash::SimpdivEventCallbackType which, when invoked, dispatches
/// to whichever Invoke*EventCallback trampoline matches the alternative
/// held by the event. A null callback (Python `None`) yields the solver's
/// own no-op default.
///
inline Gambit::Nash::SimpdivEventCallbackType MakeSimpdivEventCallback(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullSimpdivEventCallback;
  }
  return [p_callback](const Gambit::Nash::SimpdivEvent &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::SimpdivStartEvent>) {
            Gambit::ThrowIfPythonError(InvokeSimpdivStartEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedStrategyProfile<Gambit::Rational>>(event.profile)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::SimpdivRefinementEvent>) {
            Gambit::ThrowIfPythonError(InvokeSimpdivRefinementEventCallback(
                p_callback,
                std::make_shared<Gambit::MixedStrategyProfile<Gambit::Rational>>(event.profile),
                event.gridSize, event.regret));
          }
        },
        p_event);
  };
}

///
/// Builds a Nash::IPAEventCallbackType which, when invoked, dispatches to
/// whichever Invoke*EventCallback trampoline matches the alternative held
/// by the event. A null callback (Python `None`) yields the solver's own
/// no-op default.
///
inline Gambit::Nash::IPAEventCallbackType MakeIPAEventCallback(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullIPAEventCallback;
  }
  return [p_callback](const Gambit::Nash::IPAEvent &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::IPAStepEvent>) {
            Gambit::ThrowIfPythonError(InvokeIPAStepEventCallback(
                p_callback, std::make_shared<Gambit::MixedStrategyProfile<double>>(event.profile),
                event.iteration, event.zDiff, event.sDiff));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::IPATerminationEvent>) {
            Gambit::ThrowIfPythonError(InvokeIPATerminationEventCallback(
                p_callback, static_cast<int>(event.reason), event.message));
          }
        },
        p_event);
  };
}

///
/// Builds a Nash::EnumPolyEventCallbackType<Support> which, when invoked,
/// dispatches to whichever Invoke*EventCallback trampoline matches the
/// alternative held by the event, calling a Python callable with the
/// corresponding pygambit event object. A null callback (Python `None`)
/// yields the solver's own no-op default.
///
template <class Support>
Gambit::Nash::EnumPolyEventCallbackType<Support> MakeEnumPolyEventCallback(PyObject *p_callback);

template <>
inline Gambit::Nash::EnumPolyEventCallbackType<Gambit::StrategySupportProfile>
MakeEnumPolyEventCallback<Gambit::StrategySupportProfile>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullEnumPolyEventCallback<Gambit::StrategySupportProfile>;
  }
  return [p_callback](const Gambit::Nash::EnumPolyEvent<Gambit::StrategySupportProfile> &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::EnumPolyCandidateSupportEvent<
                                                  Gambit::StrategySupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyStrategyCandidateSupportEventCallback(
                p_callback, std::make_shared<Gambit::StrategySupportProfile>(event.support)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::EnumPolySingularSupportEvent<
                                                       Gambit::StrategySupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyStrategySingularSupportEventCallback(
                p_callback, std::make_shared<Gambit::StrategySupportProfile>(event.support)));
          }
          else if constexpr (std::is_same_v<Event,
                                            Gambit::Nash::EnumPolyBudgetExceededSupportEvent<
                                                Gambit::StrategySupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyStrategyBudgetExceededSupportEventCallback(
                p_callback, std::make_shared<Gambit::StrategySupportProfile>(event.support)));
          }
        },
        p_event);
  };
}

template <>
inline Gambit::Nash::EnumPolyEventCallbackType<Gambit::BehaviorSupportProfile>
MakeEnumPolyEventCallback<Gambit::BehaviorSupportProfile>(PyObject *p_callback)
{
  if (!p_callback || p_callback == Py_None) {
    return Gambit::Nash::NullEnumPolyEventCallback<Gambit::BehaviorSupportProfile>;
  }
  return [p_callback](const Gambit::Nash::EnumPolyEvent<Gambit::BehaviorSupportProfile> &p_event) {
    std::visit(
        [p_callback]<typename Event>(const Event &event) {
          if constexpr (std::is_same_v<Event, Gambit::Nash::EnumPolyCandidateSupportEvent<
                                                  Gambit::BehaviorSupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyBehaviorCandidateSupportEventCallback(
                p_callback, std::make_shared<Gambit::BehaviorSupportProfile>(event.support)));
          }
          else if constexpr (std::is_same_v<Event, Gambit::Nash::EnumPolySingularSupportEvent<
                                                       Gambit::BehaviorSupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyBehaviorSingularSupportEventCallback(
                p_callback, std::make_shared<Gambit::BehaviorSupportProfile>(event.support)));
          }
          else if constexpr (std::is_same_v<Event,
                                            Gambit::Nash::EnumPolyBudgetExceededSupportEvent<
                                                Gambit::BehaviorSupportProfile>>) {
            Gambit::ThrowIfPythonError(InvokeEnumPolyBehaviorBudgetExceededSupportEventCallback(
                p_callback, std::make_shared<Gambit::BehaviorSupportProfile>(event.support)));
          }
        },
        p_event);
  };
}

#endif // GAMBIT_PYGAMBIT_CALLBACK_H
