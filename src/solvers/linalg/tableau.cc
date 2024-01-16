//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {

namespace linalg {

// ---------------------------------------------------------------------------
//                   Tableau<double> method definitions
// ---------------------------------------------------------------------------

// Constructors and Destructor

Tableau<double>::Tableau(const Matrix<double> &A, const Vector<double> &b)
  : TableauInterface<double>(A, b), B(*this), tmpcol(b.First(), b.Last())
{
  Solve(b, solution);
}

Tableau<double>::Tableau(const Matrix<double> &A, const Array<int> &art, const Vector<double> &b)
  : TableauInterface<double>(A, art, b), B(*this), tmpcol(b.First(), b.Last())
{
  Solve(b, solution);
}

Tableau<double>::Tableau(const Tableau<double> &orig)
  : TableauInterface<double>(orig), B(orig.B, *this), tmpcol(orig.tmpcol)
{
}

Tableau<double>::~Tableau() = default;

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

bool Tableau<double>::CanPivot(int outlabel, int col) const
{
  const_cast<Tableau<double> *>(this)->SolveColumn(col, tmpcol);
  double val = tmpcol[basis.Find(outlabel)];
  if (val <= eps2 && val >= -eps2) {
    return false;
  }
  return true;
}

void Tableau<double>::Pivot(int outrow, int col)
{
  if (!RowIndex(outrow) || !ValidIndex(col)) {
    throw BadPivot();
  }

  // int outlabel = Label(outrow);
  // gout << "\noutrow:" << outrow;
  // gout << " outlabel: " << outlabel;
  // gout << " inlabel: " << col;
  // BigDump(gout);
  basis.Pivot(outrow, col);

  B.update(outrow, col);
  Solve(*b, solution);
  npivots++;
  // BigDump(gout);
}

void Tableau<double>::SolveColumn(int col, Vector<double> &out)
{
  //** can we use tmpcol here, instead of allocating new vector?
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
  Solve(*b, solution);
}

void Tableau<double>::SetRefactor(int n) { B.SetRefactor(n); }

void Tableau<double>::SetConst(const Vector<double> &bnew)
{
  if (bnew.First() != b->First() || bnew.Last() != b->Last()) {
    throw DimensionException();
  }
  b = &bnew;
  Solve(*b, solution);
}

//** this function is not currently used.  Drop it?
void Tableau<double>::SetBasis(const Basis &in)
{
  basis = in;
  B.refactor();
  Solve(*b, solution);
}

void Tableau<double>::Solve(const Vector<double> &b, Vector<double> &x) { B.solve(b, x); }

void Tableau<double>::SolveT(const Vector<double> &c, Vector<double> &y)
{
  B.solveT(c, y);
  //** gout << "\nTableau<double>::SolveT(), y: " << y;
  //   gout << "\nc: " << c;
}

bool Tableau<double>::IsFeasible()
{
  //** is it really necessary to solve first here?
  Solve(*b, solution);
  for (int i = solution.First(); i <= solution.Last(); i++) {
    if (solution[i] >= eps2) {
      return false;
    }
  }
  return true;
}

bool Tableau<double>::IsLexMin()
{
  int i, j;
  for (i = MinRow(); i <= MaxRow(); i++) {
    if (EqZero(solution[i])) {
      for (j = -MaxRow(); j < Label(i); j++) {
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
  for (int i = vec.First(); i <= vec.Last(); i++) {
    lcd = lcm(vec[i].denominator(), lcd);
  }
  return lcd;
}

// Constructors and Destructor

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Vector<Rational> &b)
  : TableauInterface<Rational>(A, b), Tabdat(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol()),
    Coeff(b.First(), b.Last()), denom(1), tmpcol(b.First(), b.Last()),
    nonbasic(A.MinCol(), A.MaxCol())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    nonbasic[j] = j;
  }

  totdenom = lcm(find_lcd(A), find_lcd(b));
  if (totdenom <= 0) {
    throw BadDenom();
  }

  for (int i = b.First(); i <= b.Last(); i++) {
    Rational x = b[i] * (Rational)totdenom;
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= MaxCol(); j++) {
      Rational x = A(i, j) * (Rational)totdenom;
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      Tabdat(i, j) = x.numerator();
    }
  }
  for (int i = b.First(); i <= b.Last(); i++) {
    solution[i] = (Rational)Coeff[i];
  }
}

Tableau<Rational>::Tableau(const Matrix<Rational> &A, const Array<int> &art,
                           const Vector<Rational> &b)
  : TableauInterface<Rational>(A, art, b),
    Tabdat(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol() + art.Length()),
    Coeff(b.First(), b.Last()), denom(1), tmpcol(b.First(), b.Last()),
    nonbasic(A.MinCol(), A.MaxCol() + art.Length())
{
  for (int j = MinCol(); j <= MaxCol(); j++) {
    nonbasic[j] = j;
  }

  totdenom = lcm(find_lcd(A), find_lcd(b));
  if (totdenom <= 0) {
    throw BadDenom();
  }

  for (int i = b.First(); i <= b.Last(); i++) {
    Rational x = b[i] * (Rational)totdenom;
    if (x.denominator() != 1) {
      throw BadDenom();
    }
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow(); i <= MaxRow(); i++) {
    for (int j = MinCol(); j <= A.MaxCol(); j++) {
      Rational x = A(i, j) * (Rational)totdenom;
      if (x.denominator() != 1) {
        throw BadDenom();
      }
      Tabdat(i, j) = x.numerator();
    }
    for (int j = A.MaxCol() + 1; j <= MaxCol(); j++) {
      Tabdat(artificial[j], j) = totdenom;
    }
  }
  for (int i = b.First(); i <= b.Last(); i++) {
    solution[i] = (Rational)Coeff[i];
  }
}

Tableau<Rational>::Tableau(const Tableau<Rational> &orig)

    = default;

Tableau<Rational>::~Tableau() = default;

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
  int i = nonbasic.First();
  while (i <= nonbasic.Last() && nonbasic[i] != col_index) {
    i++;
  }
  if (i > nonbasic.Last()) {
    throw DimensionException();
  }
  return i;
}

Matrix<Rational> Tableau<Rational>::GetInverse()
{
  Vector<Rational> mytmpcol(tmpcol.First(), tmpcol.Last());
  Matrix<Rational> inv(MinRow(), MaxRow(), MinRow(), MaxRow());
  for (int i = inv.MinCol(); i <= inv.MaxCol(); i++) {
    MySolveColumn(-i, mytmpcol);
    inv.SetColumn(i, mytmpcol);
  }
  return inv;
}

// pivoting operations

bool Tableau<Rational>::CanPivot(int outlabel, int col) const
{
  const_cast<Tableau<Rational> *>(this)->MySolveColumn(col, tmpcol);
  Rational val = tmpcol[basis.Find(outlabel)];
  if (val == (Rational)0) {
    return false;
  }
  //   if(val <=eps2 && val >= -eps2) return 0;
  return true;
}

void Tableau<Rational>::Pivot(int outrow, int in_col)
{
  // gout << "\nIn Tableau<Rational>::Pivot() ";
  // gout << " outrow:" << outrow;
  // gout << " inlabel: " << in_col;
  if (!RowIndex(outrow) || !ValidIndex(in_col)) {
    throw BadPivot();
  }
  int outlabel = Label(outrow);

  // gout << "\noutrow:" << outrow;
  // gout << " outlabel: " << outlabel;
  // gout << " inlabel: " << in_col;

  // BigDump(gout);
  // gout << "\ndenom: " << denom << " totdenom: " << totdenom;
  // gout << " product: " << denom*totdenom;
  // gout << "\nTabdat: loc 1\n " << Tabdat;
  // gout << "\nInverse: loc 1\n " << GetInverse();

  int col;
  int row(outrow);
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
  Integer old_denom = denom;
  denom = Tabdat(row, col);
  Tabdat(row, col) = old_denom;
  // BigDump(gout);
  npivots++;

  basis.Pivot(outrow, in_col);
  nonbasic[col] = outlabel;

  for (i = solution.First(); i <= solution.Last(); i++) {
    //** solution[i] = (Rational)(Coeff[i])/(Rational)(denom*totdenom);
    solution[i] = Rational(Coeff[i] * sign(denom * totdenom));
  }

  // gout << "Bottom \n" << Tabdat << '\n';
  //  BigDump(gout);
  //  gout << "\ndenom: " << denom << " totdenom: " << totdenom;
  //  gout << "\nTabdat: loc 2\n " << Tabdat;
  //  gout << "\nInverse: loc 2\n " << GetInverse();

  // Refactor();
}

void Tableau<Rational>::SolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(tmpcol.First(), tmpcol.Last());
  if (Member(in_col)) {
    out = (Rational)0;
    out[Find(in_col)] = Rational(abs(denom));
  }
  else {
    int col = remap(in_col);
    Tabdat.GetColumn(col, tempcol);
    for (int i = tempcol.First(); i <= tempcol.Last(); i++) {
      out[i] = (Rational)(tempcol[i]) * (Rational)(sign(denom * totdenom));
    }
  }
  out = out / (Rational)abs(denom);
  if (in_col < 0) {
    out *= Rational(totdenom);
  }
  for (int i = out.First(); i <= out.Last(); i++) {
    if (Label(i) < 0) {
      out[i] = (Rational)out[i] / (Rational)totdenom;
    }
  }
}

void Tableau<Rational>::MySolveColumn(int in_col, Vector<Rational> &out)
{
  Vector<Integer> tempcol(tmpcol.First(), tmpcol.Last());
  if (Member(in_col)) {
    out = (Rational)0;
    out[Find(in_col)] = Rational(abs(denom));
  }
  else {
    int col = remap(in_col);
    Tabdat.GetColumn(col, tempcol);
    for (int i = tempcol.First(); i <= tempcol.Last(); i++) {
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
  // BigDump(gout);
  //** Note -- we may need to recompute totdenom here, if A and b have changed.
  // gout << "\ndenom: " << denom << " totdenom: " << totdenom;
  totdenom = lcm(find_lcd(*A), find_lcd(*b));
  if (totdenom <= 0) {
    throw BadDenom();
  }
  // gout << "\ndenom: " << denom << " totdenom: " << totdenom;

  int i, j;
  Matrix<Rational> inv(GetInverse());
  Matrix<Rational> Tabnew(Tabdat.MinRow(), Tabdat.MaxRow(), Tabdat.MinCol(), Tabdat.MaxCol());
  for (i = nonbasic.First(); i <= nonbasic.Last(); i++) {
    GetColumn(nonbasic[i], mytmpcol);
    //    if(nonbasic[i]>=0) mytmpcol*=Rational(totdenom);
    Tabnew.SetColumn(i, inv * mytmpcol * (Rational)sign(denom * totdenom));
    // gout << "\nMyTmpCol \n" << mytmpcol;
  }

  // gout << "\nInv: \n" << inv;
  // gout << "\nTabdat:\n" << Tabdat;
  // gout << "\nTabnew:\n" << Tabnew;

  Vector<Rational> Coeffnew(Coeff.First(), Coeff.Last());
  Coeffnew = inv * (*b) * Rational(totdenom) * Rational(sign(denom * totdenom));

  // gout << "\nCoeff:\n" << Coeff;
  // gout << "\nCoeffew:\n" << Coeffnew;

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

void Tableau<Rational>::SetRefactor(int) {}

void Tableau<Rational>::SetConst(const Vector<Rational> &bnew)
{
  b = &bnew;
  Refactor();
}

//** this function is not currently used.  Drop it?
void Tableau<Rational>::SetBasis(const Basis &in)
{
  basis = in;
  //** this has to be changed -- Need to start over and pivot to new basis.
  // B.refactor();
  // B.solve(*b, solution);
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

bool Tableau<Rational>::IsFeasible()
{
  for (int i = solution.First(); i <= solution.Last(); i++) {
    if (solution[i] >= eps2) {
      return false;
    }
  }
  return true;
}

bool Tableau<Rational>::IsLexMin()
{
  int i, j;
  for (i = MinRow(); i <= MaxRow(); i++) {
    if (EqZero(solution[i])) {
      for (j = -MaxRow(); j < Label(i); j++) {
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
  for (int i = out.First(); i <= out.Last(); i++) {
    if (Label(i) < 0) {
      out[i] = out[i] / (Rational)totdenom;
    }
  }
}

Integer Tableau<Rational>::TotDenom() const { return totdenom; }

} // namespace linalg

} // end namespace Gambit
