//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

// ---------------------------------------------------------------------------
//                   Tableau<double> method definitions
// ---------------------------------------------------------------------------

// Constructors and Destructor
 
Tableau<double>::Tableau(const Gambit::Matrix<double> &A, const Gambit::Vector<double> &b)
  : TableauInterface<double>(A,b), B(*this), tmpcol(b.First(),b.Last())
{
  Solve(b, solution);
}

Tableau<double>::Tableau(const Gambit::Matrix<double> &A, const Gambit::Array<int> &art, 
			 const Gambit::Vector<double> &b)
  : TableauInterface<double>(A,art,b), B(*this), tmpcol(b.First(),b.Last())
{
  Solve(b, solution);
}


Tableau<double>::Tableau(const Tableau<double> &orig)
  : TableauInterface<double>(orig), B(orig.B,*this), tmpcol(orig.tmpcol)
{ }

Tableau<double>::~Tableau()
{ }

Tableau<double>& Tableau<double>::operator=(const Tableau<double> &orig)
{
  TableauInterface<double>::operator=(orig);
  if(this!= &orig) {
    B.Copy(orig.B,*this);
    tmpcol = orig.tmpcol;
  }
  return *this;
}

//
// pivoting operations
//

int Tableau<double>::CanPivot(int outlabel, int col)
{
  SolveColumn(col,tmpcol);
  double val = tmpcol[basis.Find(outlabel)];
  if(val <=eps2 && val >= -eps2) return 0;
  return 1;  
}

void Tableau<double>::Pivot(int outrow,int col)
{
  if(!RowIndex(outrow) || !ValidIndex(col)) throw BadPivot();

  // int outlabel = Label(outrow);
  // gout << "\noutrow:" << outrow;
  // gout << " outlabel: " << outlabel;
  // gout << " inlabel: " << col;
  // BigDump(gout);
  basis.Pivot(outrow,col);
  
  B.update(outrow, col);
  Solve(*b, solution);
  npivots++;
  // BigDump(gout);
}

void Tableau<double>::SolveColumn(int col, Gambit::Vector<double> &out)
{
  //** can we use tmpcol here, instead of allocating new vector?
  Gambit::Vector<double> tmpcol2(MinRow(),MaxRow());
  GetColumn(col,tmpcol2);
  Solve(tmpcol2,out);
}

void Tableau<double>::BasisVector(Gambit::Vector<double> &out) const
{
  out= solution;
}

//
// raw Tableau functions
//

void Tableau<double>::Refactor()
{
  B.refactor();
  //** is re-solve necessary here?
  Solve(*b, solution);
}

void Tableau<double>::SetRefactor(int n)
{
  B.SetRefactor(n);
}

void Tableau<double>::SetConst(const Gambit::Vector<double> &bnew)
{
  if(bnew.First()!=b->First() || bnew.Last()!=b->Last())
    throw Gambit::DimensionException();
  b=&bnew;
  Solve(*b, solution);
}

//** this function is not currently used.  Drop it?
void Tableau<double>::SetBasis(const Basis &in)
{
  basis= in;
  B.refactor();
  Solve(*b, solution);
}

void Tableau<double>::Solve(const Gambit::Vector<double> &b, Gambit::Vector<double> &x)
{
  B.solve(b,x);
}

void Tableau<double>::SolveT(const Gambit::Vector<double> &c, Gambit::Vector<double> &y)
{
  B.solveT(c,y);
  //** gout << "\nTableau<double>::SolveT(), y: " << y;
  //   gout << "\nc: " << c;
}

bool Tableau<double>::IsFeasible()
{
  //** is it really necessary to solve first here?
  Solve(*b, solution);
  for(int i=solution.First();i<=solution.Last();i++)
    if(solution[i]>=eps2) return false;
  return true;
}

bool Tableau<double>::IsLexMin()
{
  int i,j;
  for(i=MinRow();i<=MaxRow();i++)
    if(EqZero(solution[i]))
      for(j=-MaxRow();j<Label(i);j++) if(j!=0){
	SolveColumn(j,tmpcol);
	if(LtZero(tmpcol[i]))
	  return 0;
      }
  return 1;
}


// ---------------------------------------------------------------------------
//                   Tableau<gbtRational> method definitions
// ---------------------------------------------------------------------------


Gambit::Integer find_lcd(const Gambit::Matrix<Gambit::Rational> &mat)
{
  Gambit::Integer lcd(1);
  for(int i=mat.MinRow();i<=mat.MaxRow();i++)
    for(int j=mat.MinCol();j<=mat.MaxCol();j++) 
      lcd = lcm(mat(i,j).denominator(),lcd);
  return lcd;
}

Gambit::Integer find_lcd(const Gambit::Vector<Gambit::Rational> &vec)
{
  Gambit::Integer lcd(1);
  for(int i=vec.First();i<=vec.Last();i++)
    lcd = lcm(vec[i].denominator(),lcd);
  return lcd;
}

// Constructors and Destructor
 
Tableau<Gambit::Rational>::Tableau(const Gambit::Matrix<Gambit::Rational> &A, 
			    const Gambit::Vector<Gambit::Rational> &b) 
  : TableauInterface<Gambit::Rational>(A,b), 
    Tabdat(A.MinRow(),A.MaxRow(),A.MinCol(),A.MaxCol()),
    Coeff(b.First(),b.Last()), denom(1), tmpcol(b.First(),b.Last()), 
    nonbasic(A.MinCol(),A.MaxCol())
{
  int j;
  for(j=MinCol();j<=MaxCol();j++) 
    nonbasic[j] = j;
  
  totdenom = lcm(find_lcd(A),find_lcd(b));
  if(totdenom<=0) throw BadDenom();
  
  for (int i = b.First();i<=b.Last();i++) {
    Gambit::Rational x = b[i]*(Gambit::Rational)totdenom;
    if(x.denominator() != 1) throw BadDenom();
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow();i<=MaxRow();i++) 
    for (int j = MinCol();j<=MaxCol();j++) {
      Gambit::Rational x = A(i,j)*(Gambit::Rational)totdenom;
      if(x.denominator() != 1) throw BadDenom();
      Tabdat(i,j) = x.numerator();
    }
  for (int i = b.First();i<=b.Last();i++) 
    solution[i] = (Gambit::Rational)Coeff[i];
}

Tableau<Gambit::Rational>::Tableau(const Gambit::Matrix<Gambit::Rational> &A, 
			    const Gambit::Array<int> &art, 
			    const Gambit::Vector<Gambit::Rational> &b) 
  : TableauInterface<Gambit::Rational>(A,art,b), 
    Tabdat(A.MinRow(),A.MaxRow(),A.MinCol(),A.MaxCol()+art.Length()),
    Coeff(b.First(),b.Last()), denom(1), tmpcol(b.First(),b.Last()), 
    nonbasic(A.MinCol(),A.MaxCol()+art.Length())
{
  int j;
  for(j=MinCol();j<=MaxCol();j++) 
    nonbasic[j] = j;
  
  totdenom = lcm(find_lcd(A),find_lcd(b));
  if(totdenom<=0) throw BadDenom();
  
  for (int i = b.First();i<=b.Last();i++) {
    Gambit::Rational x = b[i]*(Gambit::Rational)totdenom;
    if(x.denominator() != 1) throw BadDenom();
    Coeff[i] = x.numerator();
  }
  for (int i = MinRow();i<=MaxRow();i++) {
    for (int j = MinCol();j<=A.MaxCol();j++) {
      Gambit::Rational x = A(i,j)*(Gambit::Rational)totdenom;
      if(x.denominator() != 1) throw BadDenom();
      Tabdat(i,j) = x.numerator();
    }
    for (int j = A.MaxCol()+1;j<=MaxCol();j++)
      Tabdat(artificial[j],j) = totdenom;
  }
  for (int i = b.First();i<=b.Last();i++) 
    solution[i] = (Gambit::Rational)Coeff[i];
}


Tableau<Gambit::Rational>::Tableau(const Tableau<Gambit::Rational> &orig) 
  : TableauInterface<Gambit::Rational>(orig), Tabdat(orig.Tabdat), Coeff(orig.Coeff), 
    totdenom(orig.totdenom), denom(orig.denom), 
    tmpcol(orig.tmpcol), nonbasic(orig.nonbasic)
{ }

Tableau<Gambit::Rational>::~Tableau()
{ }

Tableau<Gambit::Rational>& Tableau<Gambit::Rational>::operator=(const Tableau<Gambit::Rational> &orig)
{
  TableauInterface<Gambit::Rational>::operator=(orig);
  if(this!= &orig) {
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

int Tableau<Gambit::Rational>::remap(int col_index) const
{
  int i = nonbasic.First(); 
  while(i <= nonbasic.Last() && nonbasic[i] !=col_index) { i++;}
  if(i > nonbasic.Last()) throw Gambit::DimensionException();
  return i;
}

Gambit::Matrix<Gambit::Rational> Tableau<Gambit::Rational>::GetInverse()
{
  Gambit::Vector<Gambit::Rational> mytmpcol(tmpcol.First(),tmpcol.Last());
  Gambit::Matrix<Gambit::Rational> inv(MinRow(),MaxRow(),MinRow(),MaxRow());
  for(int i=inv.MinCol();i<=inv.MaxCol();i++){
    MySolveColumn(-i,mytmpcol);
    inv.SetColumn(i,mytmpcol);
  }
  return inv;
}


// pivoting operations

int Tableau<Gambit::Rational>::CanPivot(int outlabel, int col)
{
  MySolveColumn(col,tmpcol);
  Gambit::Rational val = tmpcol[basis.Find(outlabel)];
  if(val == (Gambit::Rational)0) return 0;
  //   if(val <=eps2 && val >= -eps2) return 0;
  return 1;  
}

void Tableau<Gambit::Rational>::Pivot(int outrow,int in_col)
{
  // gout << "\nIn Tableau<Gambit::Rational>::Pivot() ";
  // gout << " outrow:" << outrow;
  // gout << " inlabel: " << in_col;
  if(!RowIndex(outrow) || !ValidIndex(in_col)) 
    throw BadPivot();
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
  int i,j; // loop-control variables

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
  
  for(i=Tabdat.MinRow();i<=Tabdat.MaxRow();++i){
    if(i!=row){
      for(j=Tabdat.MinCol();j<=Tabdat.MaxCol();++j){
	if(j!=col){
	  Tabdat(i,j) = (Tabdat(row,col)*Tabdat(i,j)-Tabdat(row,j)*Tabdat(i,col))/denom;
	}
      }
      Coeff[i] = (Tabdat(row,col)*Coeff[i]-Coeff[row]*Tabdat(i,col))/denom;
    }
  }
  // Step 2
  // Note: here we are moving the old basis column into column 'col'
  for(i=Tabdat.MinRow();i<=Tabdat.MaxRow();++i){
    if(i!=row)
      Tabdat(i,col)=-Tabdat(i,col);
  }
  // Step 4
  Gambit::Integer old_denom = denom;
  denom=Tabdat(row,col);
  Tabdat(row,col)=old_denom;
  // BigDump(gout);
  npivots++;

  basis.Pivot(outrow,in_col);
  nonbasic[col] = outlabel;
  
  for (i = solution.First();i<=solution.Last();i++) 
    //** solution[i] = (Gambit::Rational)(Coeff[i])/(Gambit::Rational)(denom*totdenom);
    solution[i] = Gambit::Rational(Coeff[i]*sign(denom*totdenom));

  //gout << "Bottom \n" << Tabdat << '\n';
  // BigDump(gout);
  // gout << "\ndenom: " << denom << " totdenom: " << totdenom;
  // gout << "\nTabdat: loc 2\n " << Tabdat;
  // gout << "\nInverse: loc 2\n " << GetInverse();
  
  // Refactor();
}

void Tableau<Gambit::Rational>::SolveColumn(int in_col, Gambit::Vector<Gambit::Rational> &out)
{
  Gambit::Vector<Gambit::Integer> tempcol(tmpcol.First(),tmpcol.Last());
  if(Member(in_col)) {
    out = (Gambit::Rational)0;
    out[Find(in_col)] = Gambit::Rational(abs(denom));
  }
  else {
    int col = remap(in_col);
    Tabdat.GetColumn(col,tempcol);
    for(int i=tempcol.First();i<=tempcol.Last();i++)
      out[i] = (Gambit::Rational)(tempcol[i]) * (Gambit::Rational)(sign(denom*totdenom));
  }
  out=out/(Gambit::Rational)abs(denom);
  if(in_col < 0) out*=totdenom;
  for(int i=out.First();i<=out.Last();i++) 
    if(Label(i)<0) out[i]=(Gambit::Rational)out[i]/(Gambit::Rational)totdenom;
}

void Tableau<Gambit::Rational>::MySolveColumn(int in_col, Gambit::Vector<Gambit::Rational> &out)
{
  Gambit::Vector<Gambit::Integer> tempcol(tmpcol.First(),tmpcol.Last());
  if(Member(in_col)) {
    out = (Gambit::Rational)0;
    out[Find(in_col)] = Gambit::Rational(abs(denom));
  }
  else {
    int col = remap(in_col);
    Tabdat.GetColumn(col,tempcol);
    for(int i=tempcol.First();i<=tempcol.Last();i++)
      out[i] = (Gambit::Rational)(tempcol[i]) * (Gambit::Rational)(sign(denom*totdenom));
  }
}

void Tableau<Gambit::Rational>::GetColumn(int col, Gambit::Vector<Gambit::Rational> &out) const
{
  TableauInterface<Gambit::Rational>::GetColumn(col,out);
  if(col>=0) out*=Gambit::Rational(totdenom);

}

void Tableau<Gambit::Rational>::Refactor()
{ 
  Gambit::Vector<Gambit::Rational> mytmpcol(tmpcol);
  //BigDump(gout);
  //** Note -- we may need to recompute totdenom here, if A and b have changed. 
  //gout << "\ndenom: " << denom << " totdenom: " << totdenom;
  totdenom = lcm(find_lcd(*A),find_lcd(*b));
  if(totdenom<=0) throw BadDenom();
  // gout << "\ndenom: " << denom << " totdenom: " << totdenom;

  int i,j;
  Gambit::Matrix<Gambit::Rational> inv(GetInverse());
  Gambit::Matrix<Gambit::Rational> Tabnew(Tabdat.MinRow(),Tabdat.MaxRow(),Tabdat.MinCol(),Tabdat.MaxCol());
  for(i=nonbasic.First();i<=nonbasic.Last();i++) {
    GetColumn(nonbasic[i],mytmpcol);
    //    if(nonbasic[i]>=0) mytmpcol*=Gambit::Rational(totdenom);
    Tabnew.SetColumn(i,inv * mytmpcol * (Gambit::Rational)sign(denom*totdenom));
    //gout << "\nMyTmpCol \n" << mytmpcol;
  }

  //gout << "\nInv: \n" << inv;
  //gout << "\nTabdat:\n" << Tabdat;
  //gout << "\nTabnew:\n" << Tabnew;

  Gambit::Vector<Gambit::Rational> Coeffnew(Coeff.First(),Coeff.Last());
  Coeffnew = inv * (*b) * totdenom * (Gambit::Rational)sign(denom*totdenom);

  //gout << "\nCoeff:\n" << Coeff;
  //gout << "\nCoeffew:\n" << Coeffnew;

  for(i=Tabdat.MinRow();i<=Tabdat.MaxRow();i++) {
    if(Coeffnew[i].denominator() != 1) throw BadDenom();
    Coeff[i] = Coeffnew[i].numerator();
    for(j=Tabdat.MinCol();j<=Tabdat.MaxCol();j++) {
      if(Tabnew(i,j).denominator() != 1) throw BadDenom();
      Tabdat(i,j) = Tabnew(i,j).numerator();
    }
  }
  //BigDump(gout);
}
  
void Tableau<Gambit::Rational>::SetRefactor(int)
{ }

void Tableau<Gambit::Rational>::SetConst(const Gambit::Vector<Gambit::Rational> &bnew)
{
  b=&bnew;
  Refactor();
}


//** this function is not currently used.  Drop it?
void Tableau<Gambit::Rational>::SetBasis(const Basis &in)
{
  basis= in;
  //** this has to be changed -- Need to start over and pivot to new basis.  
  // B.refactor();
  // B.solve(*b, solution);
}

 // solve M x = b
void Tableau<Gambit::Rational>::Solve(const Gambit::Vector<Gambit::Rational> &b, Gambit::Vector<Gambit::Rational> &x)
{
  // Here, we do x = V * b, where V = M inverse
  x = (GetInverse() * b )/(Gambit::Rational)abs(denom);
}

 // solve y M = c
void Tableau<Gambit::Rational>::SolveT(const Gambit::Vector<Gambit::Rational> &c, Gambit::Vector<Gambit::Rational> &y)
{
  // Here we do y = c * V, where V = M inverse
  y = (c * GetInverse()) /(Gambit::Rational)abs(denom);
}

bool Tableau<Gambit::Rational>::IsFeasible()
{
  for(int i=solution.First();i<=solution.Last();i++)
    if(solution[i]>=eps2) return false;
  return true;
}

bool Tableau<Gambit::Rational>::IsLexMin()
{
  int i,j;
  for(i=MinRow();i<=MaxRow();i++)
    if(EqZero(solution[i]))
      for(j=-MaxRow();j<Label(i);j++) if(j!=0){
	SolveColumn(j,tmpcol);
	if(LtZero(tmpcol[i]))
	  return 0;
      }
  return 1;
}

void Tableau<Gambit::Rational>::BasisVector(Gambit::Vector<Gambit::Rational> &out) const
{
  out = solution;
  out= out/(Gambit::Rational)abs(denom) ;
  for(int i=out.First();i<=out.Last();i++) 
    if(Label(i)<0) out[i]=out[i]/(Gambit::Rational)totdenom;
}

Gambit::Integer Tableau<Gambit::Rational>::TotDenom() const
{
return totdenom;
}


