//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/liblinear/tableau.imp
// Implementation of tableau class
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

#include "tableau.h"

namespace Gambit::linalg {

// ---------------------------------------------------------------------------
//                   Tableau<double> method definitions
// ---------------------------------------------------------------------------

//
// Constructors and destructor
//

Tableau<double>::Tableau(const Matrix<double> &A, const Vector<double> &b)
  : TableauBase<double>(A, b), m_luDecomposition(*this),
    m_scratchColumn(b.front_index(), b.back_index())
{
  Solve(b, m_solution);
}

Tableau<double>::Tableau(const Matrix<double> &A, const Array<int> &art, const Vector<double> &b)
  : TableauBase<double>(A, art, b), m_luDecomposition(*this),
    m_scratchColumn(b.front_index(), b.back_index())
{
  Solve(b, m_solution);
}

Tableau<double>::Tableau(const Tableau<double> &orig)
  : TableauBase<double>(orig), m_luDecomposition(orig.m_luDecomposition, *this),
    m_scratchColumn(orig.m_scratchColumn)
{
}

Tableau<double> &Tableau<double>::operator=(const Tableau<double> &orig)
{
  TableauBase<double>::operator=(orig);
  if (this != &orig) {
    m_luDecomposition.Copy(orig.m_luDecomposition, *this);
    m_scratchColumn = orig.m_scratchColumn;
  }
  return *this;
}

//
// Pivoting
//

void Tableau<double>::Pivot(int outrow, int col)
{
  if (!IsRowIndex(outrow) || !IsValidIndex(col)) {
    throw BadPivot();
  }
  m_basis.Pivot(outrow, col);

  m_luDecomposition.update(outrow, col);
  Solve(m_b, m_solution);
}

void Tableau<double>::SolveColumn(int col, Vector<double> &out)
{
  Vector<double> tmpcol2(MinRow(), MaxRow());
  GetColumn(col, tmpcol2);
  Solve(tmpcol2, out);
}

void Tableau<double>::GetBasisVector(Vector<double> &out) const { out = m_solution; }

//
// Raw Tableau functions
//

void Tableau<double>::Refactor()
{
  m_luDecomposition.refactor();
  //** is re-solve necessary here?
  Solve(m_b, m_solution);
}

void Tableau<double>::SetConst(const Vector<double> &bnew)
{
  if (bnew.front_index() != m_b.front_index() || bnew.back_index() != m_b.back_index()) {
    throw DimensionException();
  }
  m_b = bnew;
  Solve(m_b, m_solution);
}

void Tableau<double>::Solve(const Vector<double> &b, Vector<double> &x)
{
  m_luDecomposition.solve(b, x);
}

void Tableau<double>::SolveT(const Vector<double> &c, Vector<double> &y)
{
  m_luDecomposition.solveT(c, y);
}

bool Tableau<double>::IsLexMin()
{
  for (int i = MinRow(); i <= MaxRow(); i++) {
    if (IsEqZero(m_solution[i])) {
      for (int j = -MaxRow(); j < GetLabel(i); j++) {
        if (j != 0) {
          SolveColumn(j, m_scratchColumn);
          if (IsLtZero(m_scratchColumn[i])) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

BFS<double> Tableau<double>::GetFullBFS() const
{
  Vector<double> sol(m_basis.GetFirst(), m_basis.GetLast());
  GetBasisVector(sol);

  BFS<double> cbfs;
  for (int i = -MaxRow(); i <= -MinRow(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  for (int i = MinCol(); i <= MaxCol(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  return cbfs;
}

BFS<double> Tableau<double>::GetColumnBFS() const
{
  Vector<double> sol(m_basis.GetFirst(), m_basis.GetLast());
  GetBasisVector(sol);

  BFS<double> cbfs;
  for (int i = MinCol(); i <= MaxCol(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  return cbfs;
}

// ---------------------------------------------------------------------------
//                   Tableau<Rational> method definitions
// ---------------------------------------------------------------------------

Integer find_lcd(const Matrix<Rational> &mat)
{
  Integer lcd(1);
  for (int i = mat.MinRow(); i <= mat.MaxRow(); i++) {
    for (int j = mat.MinCol(); j <= mat.MaxCol(); j++) {
      lcd = lcm(mat(i, j).denominator(), lcd);
    }
  }
  return lcd;
}

Integer find_lcd(const Vector<Rational> &vec)
{
  Integer lcd(1);
  for (int i = vec.front_index(); i <= vec.back_index(); i++) {
    lcd = lcm(vec[i].denominator(), lcd);
  }
  return lcd;
}

//
// Constructors and destructor
//

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Vector<Rational> &b)
  : TableauBase<Rational>(A, b), m_tableauData(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol()),
    m_scaledRHS(b.front_index(), b.back_index()), m_pivotDenominator(1),
    m_scratchColumn(b.front_index(), b.back_index()), m_nonbasicLabels(A.MinCol(), A.MaxCol())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    m_nonbasicLabels[j] = j;
  }

  m_totalDenominator = lcm(find_lcd(A), find_lcd(b));
  if (m_totalDenominator <= 0) {
    throw BadDenom();
  }

  for (int i = b.front_index(); i <= b.back_index(); i++) {
    const Rational x = b[i] * static_cast<Rational>(m_totalDenominator);
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    m_scaledRHS[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= MaxCol(); j++) {
      const Rational x = A(i, j) * static_cast<Rational>(m_totalDenominator);
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      m_tableauData(i, j) = x.numerator();
    }
  }
  for (int i = b.front_index(); i <= b.back_index(); i++) {
    m_solution[i] = static_cast<Rational>(m_scaledRHS[i]);
  }
}

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Array<int> &art,
                           const Vector<Rational> &b)
  : TableauBase<Rational>(A, art, b),
    m_tableauData(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol() + art.size()),
    m_scaledRHS(b.front_index(), b.back_index()), m_pivotDenominator(1),
    m_scratchColumn(b.front_index(), b.back_index()),
    m_nonbasicLabels(A.MinCol(), A.MaxCol() + art.size())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    m_nonbasicLabels[j] = j;
  }

  m_totalDenominator = lcm(find_lcd(A), find_lcd(b));
  if (m_totalDenominator <= 0) {
    throw BadDenom();
  }

  for (int i = b.front_index(); i <= b.back_index(); i++) {
    const Rational x = b[i] * static_cast<Rational>(m_totalDenominator);
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    m_scaledRHS[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= A.MaxCol(); j++) {
      const Rational x = A(i, j) * static_cast<Rational>(m_totalDenominator);
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      m_tableauData(i, j) = x.numerator();
    }
    for (int j = A.MaxCol() + 1; j <= MaxCol(); j++) {
      m_tableauData(m_artificialColumns[j], j) = m_totalDenominator;
    }
  }
  for (int i = b.front_index(); i <= b.back_index(); i++) {
    m_solution[i] = static_cast<Rational>(m_scaledRHS[i]);
  }
}

// Aligns the column indexes

int Tableau<Rational>::GetNonbasicPosition(int col_index) const
{
  int i = m_nonbasicLabels.front_index();
  while (i <= m_nonbasicLabels.back_index() && m_nonbasicLabels[i] != col_index) {
    i++;
  }
  if (i > m_nonbasicLabels.back_index()) {
    throw DimensionException();
  }
  return i;
}

Matrix<Rational> Tableau<Rational>::ComputeInverse()
{
  Vector<Rational> mytmpcol(m_scratchColumn.front_index(), m_scratchColumn.back_index());
  Matrix<Rational> inv(MinRow(), MaxRow(), MinRow(), MaxRow());
  for (int i = inv.MinCol(); i <= inv.MaxCol(); i++) {
    MySolveColumn(-i, mytmpcol);
    inv.SetColumn(i, mytmpcol);
  }
  return inv;
}

//
// Pivoting
//

void Tableau<Rational>::Pivot(int outrow, int in_col)
{
  if (!IsRowIndex(outrow) || !IsValidIndex(in_col)) {
    throw BadPivot();
  }
  const int outlabel = GetLabel(outrow);

  int col;
  const int row(outrow);
  int i, j; // loop-control variables

  col = GetNonbasicPosition(in_col);

  // Pivot Algorithm:
  // i* denotes Pivot Row
  // j* denotes Pivot Column
  // C is the Tableau
  // Cij is the (i,j)th component of C
  // X denotes multiplication
  // d is the denominator (initially 1)
  //
  // 1: Copy row i (don't need to implement this)
  // 2: Zero column j excepting the Pivot Element (done second)
  // 3: Cij=(Ci*j*XCij-Ci*jXCij*)/d for all other elements (done first)
  // 4: d=Ci*j* (done last)

  // Step 3

  for (i = m_tableauData.MinRow(); i <= m_tableauData.MaxRow(); ++i) {
    if (i != row) {
      for (j = m_tableauData.MinCol(); j <= m_tableauData.MaxCol(); ++j) {
        if (j != col) {
          m_tableauData(i, j) = (m_tableauData(row, col) * m_tableauData(i, j) -
                                 m_tableauData(row, j) * m_tableauData(i, col)) /
                                m_pivotDenominator;
        }
      }
      m_scaledRHS[i] =
          (m_tableauData(row, col) * m_scaledRHS[i] - m_scaledRHS[row] * m_tableauData(i, col)) /
          m_pivotDenominator;
    }
  }
  // Step 2
  // Note: here we are moving the old basis column into column 'col'
  for (i = m_tableauData.MinRow(); i <= m_tableauData.MaxRow(); ++i) {
    if (i != row) {
      m_tableauData(i, col) = -m_tableauData(i, col);
    }
  }
  // Step 4
  const Integer old_denom = m_pivotDenominator;
  m_pivotDenominator = m_tableauData(row, col);
  m_tableauData(row, col) = old_denom;

  m_basis.Pivot(outrow, in_col);
  m_nonbasicLabels[col] = outlabel;

  for (i = m_solution.front_index(); i <= m_solution.back_index(); i++) {
    m_solution[i] =
        static_cast<Rational>(m_scaledRHS[i] * sign(m_pivotDenominator * m_totalDenominator));
  }
}

void Tableau<Rational>::SolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(m_scratchColumn.front_index(), m_scratchColumn.back_index());
  if (IsMember(in_col)) {
    out = Rational{0};
    out[GetPosition(in_col)] = static_cast<Rational>(abs(m_pivotDenominator));
  }
  else {
    const int col = GetNonbasicPosition(in_col);
    m_tableauData.GetColumn(col, tempcol);
    for (int i = tempcol.front_index(); i <= tempcol.back_index(); i++) {
      out[i] = static_cast<Rational>(tempcol[i]) *
               static_cast<Rational>(sign(m_pivotDenominator * m_totalDenominator));
    }
  }
  out = out / static_cast<Rational>(abs(m_pivotDenominator));
  if (in_col < 0) {
    out *= static_cast<Rational>(m_totalDenominator);
  }
  for (int i = out.front_index(); i <= out.back_index(); i++) {
    if (GetLabel(i) < 0) {
      out[i] = out[i] / static_cast<Rational>(m_totalDenominator);
    }
  }
}

void Tableau<Rational>::MySolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(m_scratchColumn.front_index(), m_scratchColumn.back_index());
  if (IsMember(in_col)) {
    out = Rational{0};
    out[GetPosition(in_col)] = static_cast<Rational>(abs(m_pivotDenominator));
  }
  else {
    const int col = GetNonbasicPosition(in_col);
    m_tableauData.GetColumn(col, tempcol);
    for (int i = tempcol.front_index(); i <= tempcol.back_index(); i++) {
      out[i] = static_cast<Rational>(tempcol[i]) *
               static_cast<Rational>(sign(m_pivotDenominator * m_totalDenominator));
    }
  }
}

void Tableau<Rational>::GetColumn(int col, Vector<Rational> &out) const
{
  TableauBase<Rational>::GetColumn(col, out);
  if (col >= 0) {
    out *= static_cast<Rational>(m_totalDenominator);
  }
}

//
// Raw Tableau functions
//

void Tableau<Rational>::Refactor()
{
  Vector<Rational> mytmpcol(m_scratchColumn);
  m_totalDenominator = lcm(find_lcd(m_A), find_lcd(m_b));
  if (m_totalDenominator <= 0) {
    throw BadDenom();
  }
  int i, j;
  const Matrix<Rational> inv(ComputeInverse());
  Matrix<Rational> Tabnew(m_tableauData.MinRow(), m_tableauData.MaxRow(), m_tableauData.MinCol(),
                          m_tableauData.MaxCol());
  for (i = m_nonbasicLabels.front_index(); i <= m_nonbasicLabels.back_index(); i++) {
    GetColumn(m_nonbasicLabels[i], mytmpcol);
    Tabnew.SetColumn(i, inv * mytmpcol *
                            static_cast<Rational>(sign(m_pivotDenominator * m_totalDenominator)));
  }

  Vector<Rational> Coeffnew(m_scaledRHS.front_index(), m_scaledRHS.back_index());
  Coeffnew = inv * m_b * static_cast<Rational>(m_totalDenominator) *
             static_cast<Rational>(sign(m_pivotDenominator * m_totalDenominator));
  for (i = m_tableauData.MinRow(); i <= m_tableauData.MaxRow(); i++) {
    if (Coeffnew[i].denominator() != 1) {
      throw BadDenom();
    }
    m_scaledRHS[i] = Coeffnew[i].numerator();
    for (j = m_tableauData.MinCol(); j <= m_tableauData.MaxCol(); j++) {
      if (Tabnew(i, j).denominator() != 1) {
        throw BadDenom();
      }
      m_tableauData(i, j) = Tabnew(i, j).numerator();
    }
  }
  // BigDump(gout);
}

void Tableau<Rational>::SetConst(const Vector<Rational> &bnew)
{
  m_b = bnew;
  Refactor();
}

// solve M x = b
void Tableau<Rational>::Solve(const Vector<Rational> &b, Vector<Rational> &x)
{
  // Here, we do x = V * b, where V = M inverse
  x = (ComputeInverse() * b) / static_cast<Rational>(abs(m_pivotDenominator));
}

// solve y M = c
void Tableau<Rational>::SolveT(const Vector<Rational> &c, Vector<Rational> &y)
{
  // Here we do y = c * V, where V = M inverse
  y = (c * ComputeInverse()) / static_cast<Rational>(abs(m_pivotDenominator));
}

bool Tableau<Rational>::IsLexMin()
{
  for (int i = MinRow(); i <= MaxRow(); i++) {
    if (IsEqZero(m_solution[i])) {
      for (int j = -MaxRow(); j < GetLabel(i); j++) {
        if (j != 0) {
          SolveColumn(j, m_scratchColumn);
          if (IsLtZero(m_scratchColumn[i])) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void Tableau<Rational>::GetBasisVector(Vector<Rational> &out) const
{
  out = m_solution;
  out = out / static_cast<Rational>(abs(m_pivotDenominator));
  for (int i = out.front_index(); i <= out.back_index(); i++) {
    if (GetLabel(i) < 0) {
      out[i] = out[i] / static_cast<Rational>(m_totalDenominator);
    }
  }
}

BFS<Rational> Tableau<Rational>::GetFullBFS() const
{
  Vector<Rational> sol(m_basis.GetFirst(), m_basis.GetLast());
  GetBasisVector(sol);

  BFS<Rational> cbfs;
  for (int i = -MaxRow(); i <= -MinRow(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  for (int i = MinCol(); i <= MaxCol(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  return cbfs;
}

BFS<Rational> Tableau<Rational>::GetColumnBFS() const
{
  Vector<Rational> sol(m_basis.GetFirst(), m_basis.GetLast());
  GetBasisVector(sol);

  BFS<Rational> cbfs;
  for (int i = MinCol(); i <= MaxCol(); i++) {
    if (IsMember(i)) {
      cbfs.insert(i, sol[m_basis.GetPosition(i)]);
    }
  }
  return cbfs;
}

} // end namespace Gambit::linalg
