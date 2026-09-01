//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/ludecomp.cc
// Instantiation of LU decomposition
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

#include "games.h"
#include "ludecomp.h"
#include "tableau.h"

namespace Gambit::linalg {

//
// Constructors and destructor
//

template <class T>
LUDecomposition<T>::LUDecomposition(const LUDecomposition<T> &a, Tableau<T> &t)
  : m_tableau(t), m_basis(t.GetBasis()), m_base(a.m_base), m_updateTail(a.m_updateTail),
    m_refactorInterval(a.m_refactorInterval),
    m_iterationsSinceRefactor(a.m_iterationsSinceRefactor), m_totalOperations(a.m_totalOperations)
{
}

template <class T>
LUDecomposition<T>::LUDecomposition(Tableau<T> &t, int rfac /* = 0 */)
  : m_tableau(t), m_basis(t.GetBasis()), m_base(std::make_shared<const BaseFactorization>()),
    m_refactorInterval(rfac), m_iterationsSinceRefactor(0)
{
  auto m = m_basis.GetLast() - m_basis.GetFirst() + 1;
  m_totalOperations = (m - 1) * m * (2 * m - 1) / 6;
}

//
// Operations
//

template <class T> void LUDecomposition<T>::Copy(const LUDecomposition<T> &orig, Tableau<T> &t)
{
  if (this != &orig) {
    m_tableau = t;
    m_basis = t.GetBasis();

    m_base = orig.m_base;
    m_updateTail = orig.m_updateTail;

    m_refactorInterval = orig.m_refactorInterval;
    m_iterationsSinceRefactor = orig.m_iterationsSinceRefactor;
    m_totalOperations = orig.m_totalOperations;
  }
}

template <class T> void LUDecomposition<T>::Update(int col, int matcol)
{
  const int m = m_basis.GetLast() - m_basis.GetFirst() + 1;

  m_iterationsSinceRefactor++;
  if ((m_refactorInterval > 0 && m_iterationsSinceRefactor >= m_refactorInterval) ||
      (m_refactorInterval == 0 && RefactorCheck())) {
    Refactor();
  }
  else {
    Vector<T> scratch(m_basis.GetFirst(), m_basis.GetLast());
    m_tableau.GetColumn(matcol, scratch);
    Solve(scratch, scratch);
    if (scratch[col] == T{0}) {
      throw BadPivot();
    }
    m_updateTail =
        std::make_shared<const EtaNode>(EtaNode{.eta = {col, scratch}, .prev = m_updateTail});

    m_totalOperations += m_iterationsSinceRefactor * m + 2 * m * m;
  }
}

template <class T> void LUDecomposition<T>::Refactor()
{
  m_base = std::make_shared<const BaseFactorization>(FactorBasis());
  m_updateTail = nullptr;

  m_iterationsSinceRefactor = 0;
  const int m = m_basis.GetLast() - m_basis.GetFirst() + 1;
  m_totalOperations = (m - 1) * m * (2 * m - 1) / 6;
}

template <class T> void LUDecomposition<T>::SolveT(const Vector<T> &c, Vector<T> &y) const
{
  if (c.front_index() != y.front_index() || c.back_index() != y.back_index()) {
    throw DimensionException();
  }
  if (c.front_index() != m_basis.GetFirst() || c.back_index() != m_basis.GetLast()) {
    throw DimensionException();
  }

  y = c;
  if (!m_basis.IsIdent()) {
    BTransE(y);
    FTransU(y);
    yLP_Trans(y);
  }
}

template <class T> void LUDecomposition<T>::Solve(const Vector<T> &a, Vector<T> &d) const
{
  if (a.front_index() != d.front_index() || a.back_index() != d.back_index()) {
    throw DimensionException();
  }
  if (a.front_index() != m_basis.GetFirst() || a.back_index() != m_basis.GetLast()) {
    throw DimensionException();
  }

  d = a;
  if (!m_basis.IsIdent()) {
    LPd_Trans(d);
    BTransU(d);
    FTransE(d);
  }
}

//
// Private members
//

template <class T> typename LUDecomposition<T>::BaseFactorization LUDecomposition<T>::FactorBasis()
{
  BaseFactorization result;

  Matrix<T> B(m_basis.GetFirst(), m_basis.GetLast(), m_basis.GetFirst(), m_basis.GetLast());

  Vector<T> scratch(m_basis.GetFirst(), m_basis.GetLast());
  for (int i = m_basis.GetFirst(); i <= m_basis.GetLast(); i++) {
    m_tableau.GetColumn(m_basis.GetLabel(i), scratch);
    B.SetColumn(i, scratch);
  }

  for (int i = B.MinRow(); i <= B.MaxRow(); i++) {
    T pivVal = Gambit::abs(B(i, i));
    int piv = i;
    for (int j = i + 1; j <= B.MaxRow(); j++) {
      if (B(j, i) * B(j, i) > pivVal * pivVal) {
        piv = j;
        pivVal = B(j, i);
      }
    }
    B.SwitchRows(i, piv);

    scratch = T{0};
    scratch[i] = T{1} / B(i, i);
    for (int j = i + 1; j <= B.MaxRow(); j++) {
      scratch[j] = -B(j, i) / B(i, i);
    }
    result.lowerFactors.push_back({piv, {i, scratch}});
    GaussElem(B, i, i);
  }
  for (int j = B.MinCol(); j <= B.MaxCol(); j++) {
    B.GetColumn(j, scratch);
    result.upperEtas.push_back({j, scratch});
  }
  return result;
}

template <class T> void LUDecomposition<T>::GaussElem(Matrix<T> &B, int row, int col)
{
  if (B(row, col) == T{0}) {
    throw BadPivot();
  }

  for (int j = col + 1; j <= B.MaxCol(); j++) {
    B(row, j) = B(row, j) / B(row, col);
  }

  for (int i = row + 1; i <= B.MaxRow(); i++) {
    for (int j = col + 1; j <= B.MaxCol(); j++) {
      B(i, j) = B(i, j) - (B(i, col) * B(row, j));
    }
  }

  for (int i = row + 1; i <= B.MaxRow(); i++) {
    B(i, col) = T{0};
  }

  B(row, col) = T{1};
}

template <class T> void LUDecomposition<T>::BTransE(Vector<T> &y) const
{
  for (auto node = m_updateTail; node; node = node->prev) {
    VectorEtaSolve(node->eta, y);
  }
}

template <class T> void LUDecomposition<T>::FTransU(Vector<T> &y) const
{
  for (const auto &eta : m_base->upperEtas) {
    VectorEtaSolve(eta, y);
  }
}

template <class T>
void LUDecomposition<T>::VectorEtaSolve(const EtaMatrix &eta, Vector<T> &y) const
{
  Vector<T> v = y;
  for (int i = v.front_index(); i <= v.back_index(); i++) {
    if (i == eta.col) {
      for (int j = v.front_index(); j <= v.back_index(); j++) {
        if (j != eta.col) {
          y[i] -= v[j] * eta.etadata[j];
        }
      }
      y[i] /= eta.etadata[i];
    }
  }
}

template <class T> void LUDecomposition<T>::FTransE(Vector<T> &y) const
{
  // Chain runs newest-to-oldest; buffer and reverse for oldest-to-newest.
  std::vector<const EtaMatrix *> etas;
  for (auto node = m_updateTail; node; node = node->prev) {
    etas.push_back(&node->eta);
  }
  for (auto it = etas.rbegin(); it != etas.rend(); ++it) {
    EtaVectorSolve(**it, y);
  }
}

template <class T> void LUDecomposition<T>::BTransU(Vector<T> &y) const
{
  std::for_each(m_base->upperEtas.rbegin(), m_base->upperEtas.rend(),
                [&](const EtaMatrix &m) { EtaVectorSolve(m, y); });
}

template <class T>
void LUDecomposition<T>::EtaVectorSolve(const EtaMatrix &eta, Vector<T> &d) const
{
  if (eta.etadata[eta.col] == T{0}) {
    throw BadPivot(); // or we would have a singular matrix
  }
  Vector<T> v = d;
  T temp = v[eta.col] / eta.etadata[eta.col];

  for (int i = v.front_index(); i <= v.back_index(); i++) {
    if (i == eta.col) {
      d[i] = temp;
    }
    else {
      d[i] = v[i] - temp * eta.etadata[i];
    }
  }
}

template <class T> void LUDecomposition<T>::yLP_Trans(Vector<T> &y) const
{
  Vector<T> scratch(m_basis.GetFirst(), m_basis.GetLast());
  for (int j = m_base->lowerFactors.size() - 1; j >= 0; j--) {
    yLP_mult(y, j, scratch);
    y = scratch;
  }
}

template <class T>
void LUDecomposition<T>::yLP_mult(const Vector<T> &y, int j, Vector<T> &ans) const
{
  const int ell = j + y.front_index();

  for (int i = y.front_index(); i <= y.back_index(); i++) {
    if (i != m_base->lowerFactors[j].second.col) {
      ans[i] = y[i];
    }
    else {
      T temp{0};
      for (int k = ans.front_index(); k <= ans.back_index(); k++) {
        temp += y[k] * m_base->lowerFactors[j].second.etadata[k];
      }
      ans[i] = temp;
    }
  }
  std::swap(ans[ell], ans[m_base->lowerFactors[j].first]);
}

template <class T> void LUDecomposition<T>::LPd_Trans(Vector<T> &d) const
{
  Vector<T> scratch(m_basis.GetFirst(), m_basis.GetLast());
  for (size_t j = 0; j < m_base->lowerFactors.size(); j++) {
    LPd_mult(d, j, scratch);
    d = scratch;
  }
}

template <class T> void LUDecomposition<T>::LPd_mult(Vector<T> &d, int j, Vector<T> &ans) const
{
  const int k = j + d.front_index();
  std::swap(d[k], d[m_base->lowerFactors[j].first]);
  for (int i = d.front_index(); i <= d.back_index(); i++) {
    if (i == m_base->lowerFactors[j].second.col) {
      ans[i] = d[i] * m_base->lowerFactors[j].second.etadata[i];
    }
    else {
      ans[i] =
          d[i] + d[m_base->lowerFactors[j].second.col] * m_base->lowerFactors[j].second.etadata[i];
    }
  }
  std::swap(d[m_base->lowerFactors[j].first], d[k]);
}

template <class T> bool LUDecomposition<T>::RefactorCheck()
{
  const int m = m_basis.GetLast() - m_basis.GetFirst() + 1;
  const int i = m_iterationsSinceRefactor * (m_iterationsSinceRefactor * m + 2 * m * m);
  const int k = m_totalOperations + m_iterationsSinceRefactor * m + 2 * m * m;
  return i > k;
}

template class LUDecomposition<double>;

} // end namespace Gambit::linalg
