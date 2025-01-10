//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/rectangle.h
// A utility class representing a (generalised) rectangle in N-dimensional space
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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "gambit.h"

namespace Gambit {

/// A nonempty compact interval
template <class T> class Interval {
private:
  T lower_bd, upper_bd;

public:
  Interval(const Interval<T> &p_interval)
    : lower_bd(p_interval.lower_bd), upper_bd(p_interval.upper_bd)
  {
  }
  Interval(const T &p_low, const T &p_high) : lower_bd(p_low), upper_bd(p_high) {}
  ~Interval() = default;

  Interval<T> &operator=(const Interval<T> &y) = delete;

  bool operator==(const Interval<T> &p_rhs) const
  {
    return lower_bd == p_rhs.lower_bd && upper_bd == p_rhs.upper_bd;
  }
  bool operator!=(const Interval<T> &p_rhs) const
  {
    return lower_bd != p_rhs.lower_bd || upper_bd != p_rhs.upper_bd;
  }

  const T &LowerBound() const { return lower_bd; }
  const T &UpperBound() const { return upper_bd; }
  bool Contains(const T &p_number) const { return lower_bd <= p_number && p_number <= upper_bd; }
  bool Contains(const Interval<T> &p_interval) const
  {
    return lower_bd <= p_interval.lower_bd && p_interval.upper_bd <= upper_bd;
  }
  T Length() const { return upper_bd - lower_bd; }
  T Midpoint() const { return (upper_bd + lower_bd) / static_cast<T>(2); }
  Interval<T> LeftHalf() const { return {LowerBound(), Midpoint()}; }
  Interval<T> RightHalf() const { return {Midpoint(), UpperBound()}; }

  Interval<T> SameCenterDoubleLength() const
  {
    return {static_cast<T>(2) * LowerBound() - Midpoint(),
            static_cast<T>(2) * UpperBound() - Midpoint()};
  }
};

/// A cartesian product of intervals
template <class T> class Rectangle {
private:
  List<Interval<T>> sides;

  Rectangle() = default;

public:
  /// Represents the subdivision of a rectangle into its constitutent orthants
  class Orthants {
  private:
    Rectangle<T> m_rect;

  public:
    class iterator {
      friend class Orthants;

    private:
      const Orthants *m_orthants;
      int m_index;

      iterator(const Orthants *orth, int index) : m_orthants(orth), m_index(index) {}

    public:
      Rectangle<T> operator*()
      {
        Rectangle<T> ret;
        int tmp = m_index;
        for (const auto &side : m_orthants->m_rect.sides) {
          if (tmp % 2 == 1) {
            ret.sides.push_back(side.RightHalf());
            tmp--;
          }
          else {
            ret.sides.push_back(side.LeftHalf());
          }
          tmp /= 2;
        }
        return ret;
      }

      iterator &operator++()
      {
        m_index++;
        return *this;
      }

      bool operator==(const iterator &it) const
      {
        return (m_orthants == it.m_orthants) && (m_index == it.m_index);
      }
      bool operator!=(const iterator &it) const { return !(*this == it); }
    };

    Orthants(const Rectangle<T> &p_rect) : m_rect(p_rect) {}

    int size() const { return std::pow(2, m_rect.Dmnsn()); }
    iterator begin() const { return {this, 1}; }
    iterator end() const { return {this, size() + 1}; }
  };

  Rectangle(const Vector<T> &lower, const Vector<T> &upper)
  {
    std::transform(lower.begin(), lower.end(), upper.begin(), std::back_inserter(sides),
                   [](const T &x, const T &y) { return Interval<T>(x, y); });
  }
  Rectangle(const Rectangle<T> &) = default;
  ~Rectangle() = default;

  Rectangle<T> &operator=(const Rectangle<T> &) = delete;
  bool operator==(const Rectangle<T> &y) const { return sides == y.sides; }
  bool operator!=(const Rectangle<T> &y) const { return sides != y.sides; }

  int Dmnsn() const { return sides.size(); }
  Orthants Orthants() const { return {*this}; }
  const Rectangle<T> SameCenterDoubleLengths() const
  {
    Rectangle<T> ret;
    for (const auto &side : sides) {
      ret.sides.push_back(side.SameCenterDoubleLength());
    }
    return ret;
  }
  const Interval<T> &Side(const int dim) const { return sides[dim]; }
  Vector<T> SideLengths() const
  {
    Vector<T> answer(Dmnsn());
    std::transform(sides.begin(), sides.end(), answer.begin(),
                   [](const Interval<T> &x) { return x.Length(); });
    return answer;
  }
  T MaxSideLength() const
  {
    return std::accumulate(
        sides.begin(), sides.end(), static_cast<T>(0),
        [](const T &v, const Interval<T> &side) { return std::max(v, side.Length()); });
  }
  bool Contains(const Vector<T> &point, const T &eps = static_cast<T>(0)) const
  {
    return std::equal(sides.begin(), sides.end(), point.begin(),
                      [&](const Interval<T> &side, const T &v) {
                        return v + eps >= side.LowerBound() && side.UpperBound() + eps >= v;
                      });
  }
  Vector<T> Center() const
  {
    Vector<T> answer(Dmnsn());
    std::transform(sides.begin(), sides.end(), answer.begin(),
                   [](const Interval<T> &x) { return x.Midpoint(); });
    return answer;
  }
};

} // end namespace Gambit

#endif // RECTANGLE_H
