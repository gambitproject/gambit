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

// Constructors and Destructor

Tableau<double>::Tableau(const Matrix<double> &A, const Vector<double> &b)
  : TableauInterface<double>(A, b), B(*this), tmpcol(b.front_index(), b.back_index())
{
  Solve(b, solution);
}

Tableau<double>::Tableau(const Matrix<double> &A, const Array<int> &art, const Vector<double> &b)
  : TableauInterface<double>(A, art, b), B(*this), tmpcol(b.front_index(), b.back_index())
{
  Solve(b, solution);
}

Tableau<double>::Tableau(const Tableau<double> &orig)
  : TableauInterface<double>(orig), B(orig.B, *this), tmpcol(orig.tmpcol)
{
}

Tableau<double> &Tableau<double>::operator=(const Tableau<double> &orig)
{
  TableauInterface<double>::operator=(orig);
  if (this != &orig) {
    B.Copy(orig.B, *this);
    tmpcol = orig.tmpcol;
  }
  return *this;
}

//
// pivoting operations
//

void Tableau<double>::Pivot(int outrow, int col)
{
  if (!RowIndex(outrow) || !ValidIndex(col)) {
    throw BadPivot();
  }
  basis.Pivot(outrow, col);

  B.update(outrow, col);
  Solve(b, solution);
}

void Tableau<double>::SolveColumn(int col, Vector<double> &out)
{
  Vector<double> tmpcol2(MinRow(), MaxRow());
  GetColumn(col, tmpcol2);
  Solve(tmpcol2, out);
}

void Tableau<double>::BasisVector(Vector<double> &out) const { out = solution; }

//
// raw Tableau functions
//

void Tableau<double>::Refactor()
{
  B.refactor();
  //** is re-solve necessary here?
  Solve(b, solution);
}

void Tableau<double>::SetConst(const Vector<double> &bnew)
{
  if (bnew.front_index() != b.front_index() || bnew.back_index() != b.back_index()) {
    throw DimensionException();
  }
  b = bnew;
  Solve(b, solution);
}

void Tableau<double>::Solve(const Vector<double> &b, Vector<double> &x) { B.solve(b, x); }

void Tableau<double>::SolveT(const Vector<double> &c, Vector<double> &y) { B.solveT(c, y); }

bool Tableau<double>::IsLexMin()
{
  for (int i = MinRow(); i <= MaxRow(); i++) {
    if (EqZero(solution[i])) {
      for (int j = -MaxRow(); j < Label(i); j++) {
        if (j != 0) {
          SolveColumn(j, tmpcol);
          if (LtZero(tmpcol[i])) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
//                   Tableau<gbtRational> method definitions
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

// Constructors and Destructor

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Vector<Rational> &b)
  : TableauInterface<Rational>(A, b), Tabdat(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol()),
    Coeff(b.front_index(), b.back_index()), denom(1), tmpcol(b.front_index(), b.back_index()),
    nonbasic(A.MinCol(), A.MaxCol())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    nonbasic[j] = j;
  }

  totdenom = lcm(find_lcd(A), find_lcd(b));
  if (totdenom <= 0) {
    throw BadDenom();
  }

  for (int i = b.front_index(); i <= b.back_index(); i++) {
    const Rational x = b[i] * (Rational)totdenom;
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= MaxCol(); j++) {
      const Rational x = A(i, j) * (Rational)totdenom;
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      Tabdat(i, j) = x.numerator();
    }
  }
  for (int i = b.front_index(); i <= b.back_index(); i++) {
    solution[i] = (Rational)Coeff[i];
  }
}

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Array<int> &art,
                           const Vector<Rational> &b)
  : TableauInterface<Rational>(A, art, b),
    Tabdat(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol() + art.size()),
    Coeff(b.front_index(), b.back_index()), denom(1), tmpcol(b.front_index(), b.back_index()),
    nonbasic(A.MinCol(), A.MaxCol() + art.size())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    nonbasic[j] = j;
  }

  totdenom = lcm(find_lcd(A), find_lcd(b));
  if (totdenom <= 0) {
    throw BadDenom();
  }

  for (int i = b.front_index(); i <= b.back_index(); i++) {
    const Rational x = b[i] * (Rational)totdenom;
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= A.MaxCol(); j++) {
      const Rational x = A(i, j) * (Rational)totdenom;
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      Tabdat(i, j) = x.numerator();
    }
    for (int j = A.MaxCol() + 1; j <= MaxCol(); j++) {
      Tabdat(artificial[j], j) = totdenom;
    }
  }
  for (int i = b.front_index(); i <= b.back_index(); i++) {
    solution[i] = (Rational)Coeff[i];
  }
}

Tableau<Rational> &Tableau<Rational>::operator=(const Tableau<Rational> &orig)
{
  TableauInterface<Rational>::operator=(orig);
  if (this != &orig) {
    Tabdat = orig.Tabdat;
    Coeff = orig.Coeff;
    totdenom = orig.totdenom;
    denom = orig.denom;
    tmpcol = orig.tmpcol;
    nonbasic = orig.nonbasic;
  }
  return *this;
}

// Aligns the column indexes

int Tableau<Rational>::remap(int col_index) const
{
  int i = nonbasic.front_index();
  while (i <= nonbasic.back_index() && nonbasic[i] != col_index) {
    i++;
  }
  if (i > nonbasic.back_index()) {
    throw DimensionException();
  }
  return i;
}

Matrix<Rational> Tableau<Rational>::GetInverse()
{
  Vector<Rational> mytmpcol(tmpcol.front_index(), tmpcol.back_index());
  Matrix<Rational> inv(MinRow(), MaxRow(), MinRow(), MaxRow());
  for (int i = inv.MinCol(); i <= inv.MaxCol(); i++) {
    MySolveColumn(-i, mytmpcol);
    inv.SetColumn(i, mytmpcol);
  }
  return inv;
}

// pivoting operations
void Tableau<Rational>::Pivot(int outrow, int in_col)
{
  if (!RowIndex(outrow) || !ValidIndex(in_col)) {
    throw BadPivot();
  }
  const int outlabel = Label(outrow);

  int col;
  const int row(outrow);
  int i, j; // loop-control variables

  col = remap(in_col);

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

  for (i = Tabdat.MinRow(); i <= Tabdat.MaxRow(); ++i) {
    if (i != row) {
      for (j = Tabdat.MinCol(); j <= Tabdat.MaxCol(); ++j) {
        if (j != col) {
          Tabdat(i, j) =
              (Tabdat(row, col) * Tabdat(i, j) - Tabdat(row, j) * Tabdat(i, col)) / denom;
        }
      }
      Coeff[i] = (Tabdat(row, col) * Coeff[i] - Coeff[row] * Tabdat(i, col)) / denom;
    }
  }
  // Step 2
  // Note: here we are moving the old basis column into column 'col'
  for (i = Tabdat.MinRow(); i <= Tabdat.MaxRow(); ++i) {
    if (i != row) {
      Tabdat(i, col) = -Tabdat(i, col);
    }
  }
  // Step 4
  const Integer old_denom = denom;
  denom = Tabdat(row, col);
  Tabdat(row, col) = old_denom;

  basis.Pivot(outrow, in_col);
  nonbasic[col] = outlabel;

  for (i = solution.front_index(); i <= solution.back_index(); i++) {
    solution[i] = Rational(Coeff[i] * sign(denom * totdenom));
  }
}

void Tableau<Rational>::SolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(tmpcol.front_index(), tmpcol.back_index());
  if (Member(in_col)) {
    out = (Rational)0;
    out[Find(in_col)] = Rational(abs(denom));
  }
  else {
    const int col = remap(in_col);
    Tabdat.GetColumn(col, tempcol);
    for (int i = tempcol.front_index(); i <= tempcol.back_index(); i++) {
      out[i] = (Rational)(tempcol[i]) * (Rational)(sign(denom * totdenom));
    }
  }
  out = out / (Rational)abs(denom);
  if (in_col < 0) {
    out *= Rational(totdenom);
  }
  for (int i = out.front_index(); i <= out.back_index(); i++) {
    if (Label(i) < 0) {
      out[i] = (Rational)out[i] / (Rational)totdenom;
    }
  }
}

void Tableau<Rational>::MySolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(tmpcol.front_index(), tmpcol.back_index());
  if (Member(in_col)) {
    out = (Rational)0;
    out[Find(in_col)] = Rational(abs(denom));
  }
  else {
    const int col = remap(in_col);
    Tabdat.GetColumn(col, tempcol);
    for (int i = tempcol.front_index(); i <= tempcol.back_index(); i++) {
      out[i] = (Rational)(tempcol[i]) * (Rational)(sign(denom * totdenom));
    }
  }
}

void Tableau<Rational>::GetColumn(int col, Vector<Rational> &out) const
{
  TableauInterface<Rational>::GetColumn(col, out);
  if (col >= 0) {
    out *= Rational(totdenom);
  }
}

void Tableau<Rational>::Refactor()
{
  Vector<Rational> mytmpcol(tmpcol);
  totdenom = lcm(find_lcd(A), find_lcd(b));
  if (totdenom <= 0) {
    throw BadDenom();
  }
  int i, j;
  const Matrix<Rational> inv(GetInverse());
  Matrix<Rational> Tabnew(Tabdat.MinRow(), Tabdat.MaxRow(), Tabdat.MinCol(), Tabdat.MaxCol());
  for (i = nonbasic.front_index(); i <= nonbasic.back_index(); i++) {
    GetColumn(nonbasic[i], mytmpcol);
    Tabnew.SetColumn(i, inv * mytmpcol * (Rational)sign(denom * totdenom));
  }

  Vector<Rational> Coeffnew(Coeff.front_index(), Coeff.back_index());
  Coeffnew = inv * b * Rational(totdenom) * Rational(sign(denom * totdenom));
  for (i = Tabdat.MinRow(); i <= Tabdat.MaxRow(); i++) {
    if (Coeffnew[i].denominator() != 1) {
      throw BadDenom();
    }
    Coeff[i] = Coeffnew[i].numerator();
    for (j = Tabdat.MinCol(); j <= Tabdat.MaxCol(); j++) {
      if (Tabnew(i, j).denominator() != 1) {
        throw BadDenom();
      }
      Tabdat(i, j) = Tabnew(i, j).numerator();
    }
  }
  // BigDump(gout);
}

void Tableau<Rational>::SetConst(const Vector<Rational> &bnew)
{
  b = bnew;
  Refactor();
}

// solve M x = b
void Tableau<Rational>::Solve(const Vector<Rational> &b, Vector<Rational> &x)
{
  // Here, we do x = V * b, where V = M inverse
  x = (GetInverse() * b) / (Rational)abs(denom);
}

// solve y M = c
void Tableau<Rational>::SolveT(const Vector<Rational> &c, Vector<Rational> &y)
{
  // Here we do y = c * V, where V = M inverse
  y = (c * GetInverse()) / (Rational)abs(denom);
}

bool Tableau<Rational>::IsLexMin()
{
  for (int i = MinRow(); i <= MaxRow(); i++) {
    if (EqZero(solution[i])) {
      for (int j = -MaxRow(); j < Label(i); j++) {
        if (j != 0) {
          SolveColumn(j, tmpcol);
          if (LtZero(tmpcol[i])) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

void Tableau<Rational>::BasisVector(Vector<Rational> &out) const
{
  out = solution;
  out = out / (Rational)abs(denom);
  for (int i = out.front_index(); i <= out.back_index(); i++) {
    if (Label(i) < 0) {
      out[i] = out[i] / (Rational)totdenom;
    }
  }
}

} // end namespace Gambit::linalg
