//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/util.h
// Utility functions common to command-line tools
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

#ifndef TOOLS_UTIL_H
#define TOOLS_UTIL_H

#include "gambit.h"

namespace Gambit {

template <class T> class StrategyProfileRenderer {
public:
  virtual ~StrategyProfileRenderer() = default;
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
                      const std::string &p_label = "NE") const = 0;
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
                      const std::string &p_label = "NE") const = 0;
};

//
// Encapsulates the rendering of a mixed strategy profile to various text formats.
// Implements automatic conversion of behavior strategy profiles to mixed
// strategy profiles.
//
template <class T> class MixedStrategyRenderer : public StrategyProfileRenderer<T> {
public:
  ~MixedStrategyRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
              const std::string &p_label = "NE") const override = 0;
  void Render(const MixedBehaviorProfile<T> &p_profile,
              const std::string &p_label = "NE") const override
  {
    Render(p_profile.ToMixedProfile(), p_label);
  }
};

template <class T> class MixedStrategyCSVRenderer : public MixedStrategyRenderer<T> {
public:
  explicit MixedStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals)
  {
  }
  ~MixedStrategyCSVRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
              const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T> class MixedStrategyDetailRenderer : public MixedStrategyRenderer<T> {
public:
  explicit MixedStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals)
  {
  }
  ~MixedStrategyDetailRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
              const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

//
// Encapsulates the rendering of a behavior profile to various text formats.
//
template <class T> class BehavStrategyRenderer : public StrategyProfileRenderer<T> {
public:
  ~BehavStrategyRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
              const std::string &p_label = "NE") const override
  {
    Render(MixedBehaviorProfile<T>(p_profile), p_label);
  }
  void Render(const MixedBehaviorProfile<T> &p_profile,
              const std::string &p_label = "NE") const override = 0;
};

template <class T> class BehavStrategyCSVRenderer : public BehavStrategyRenderer<T> {
public:
  explicit BehavStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals)
  {
  }
  ~BehavStrategyCSVRenderer() override = default;
  void Render(const MixedBehaviorProfile<T> &p_profile,
              const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T> class BehavStrategyDetailRenderer : public BehavStrategyRenderer<T> {
public:
  explicit BehavStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals)
  {
  }
  ~BehavStrategyDetailRenderer() override = default;
  void Render(const MixedBehaviorProfile<T> &p_profile,
              const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T>
void MixedStrategyCSVRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile,
                                         const std::string &p_label) const
{
  m_stream << p_label;
  for (size_t i = 1; i <= p_profile.MixedProfileLength(); i++) {
    m_stream << "," << lexical_cast<std::string>(p_profile[i], m_numDecimals);
  }
  m_stream << std::endl;
}

template <class T>
void MixedStrategyDetailRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile,
                                            const std::string &p_label) const
{
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    m_stream << "Strategy profile for player " << player->GetNumber() << ":\n";

    m_stream << "Strategy   Prob          Value\n";
    m_stream << "--------   -----------   -----------\n";

    for (auto strategy : player->GetStrategies()) {
      if (!strategy->GetLabel().empty()) {
        m_stream << std::setw(8) << strategy->GetLabel() << "    ";
      }
      else {
        m_stream << std::setw(8) << strategy->GetNumber() << "    ";
      }
      m_stream << std::setw(10);
      m_stream << lexical_cast<std::string>(p_profile[strategy], m_numDecimals);
      m_stream << "   ";
      m_stream << std::setw(11);
      m_stream << lexical_cast<std::string>(p_profile.GetPayoff(strategy), m_numDecimals);
      m_stream << std::endl;
    }
  }
}

template <class T>
void BehavStrategyCSVRenderer<T>::Render(const MixedBehaviorProfile<T> &p_profile,
                                         const std::string &p_label) const
{
  m_stream << p_label;
  for (size_t i = 1; i <= p_profile.BehaviorProfileLength(); i++) {
    m_stream << "," << lexical_cast<std::string>(p_profile[i], m_numDecimals);
  }
  m_stream << std::endl;
}

template <class T>
void BehavStrategyDetailRenderer<T>::Render(const MixedBehaviorProfile<T> &p_profile,
                                            const std::string &p_label) const
{
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    m_stream << "Behavior profile for player " << player->GetNumber() << ":\n";

    m_stream << "Infoset    Action     Prob          Value\n";
    m_stream << "-------    -------    -----------   -----------\n";

    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        if (!infoset->GetLabel().empty()) {
          m_stream << std::setw(7) << infoset->GetLabel() << "    ";
        }
        else {
          m_stream << std::setw(7) << infoset->GetNumber() << "    ";
        }
        if (!action->GetLabel().empty()) {
          m_stream << std::setw(7) << action->GetLabel() << "   ";
        }
        else {
          m_stream << std::setw(7) << action->GetNumber() << "   ";
        }
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile[action], m_numDecimals);
        m_stream << "   ";
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetPayoff(action), m_numDecimals);
        m_stream << std::endl;
      }
    }

    m_stream << std::endl;
    m_stream << "Infoset    Node       Belief        Prob\n";
    m_stream << "-------    -------    -----------   -----------\n";

    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &node : infoset->GetMembers()) {
        if (!infoset->GetLabel().empty()) {
          m_stream << std::setw(7) << infoset->GetLabel() << "    ";
        }
        else {
          m_stream << std::setw(7) << infoset->GetNumber() << "    ";
        }
        if (!node->GetLabel().empty()) {
          m_stream << std::setw(7) << node->GetLabel() << "   ";
        }
        else {
          m_stream << std::setw(7) << node->GetNumber() << "   ";
        }
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetBeliefProb(node), m_numDecimals);
        m_stream << "   ";
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetRealizProb(node), m_numDecimals);
        m_stream << std::endl;
      }
    }
    m_stream << std::endl;
  }
}

} // namespace Gambit

#endif // TOOLS_UTIL_H
