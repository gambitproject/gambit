//
// test file for gPoly1
//
// $Id$
//

#include "gpoly.h"
#include "rational.h"

void ptest(term_order*);


// This program was created as a quick test for the gpoly class.
// It creates a gPolyFamily of 3 variables (by default named n1, n2, n3)
// It then creates 4 polynomials of this class.
// When prompted for which polynomial, type in a number from 1 to 4.
// When entering a polynomial as a string, remember that the variables
// are named n1, n2, n3.
// The test option (option 98) is the same test suite as in the old 
// polytest.cc file.


int main()
{
  gSpace t(3);  // Declare a variable list of 3 variables

  ORD_PTR ptr = &lex;
  term_order* Lex = new term_order(&t, ptr);

  // create an array of 4 polynomials belonging to the family above
  gArray< gPoly <gRational> *> test(4);  //
  int k;
  for(k = 1; k<=4; k++) test[k] = new gPoly<gRational>(&t,Lex);  //

  int cont = 1;
  gFileInput in(stdin);
  gFileOutput out(stdout);
  int menuchoice;
  int poly1, poly2, input, input2;
  gRational coeff;  //
  gRational evalvalue; //
  gArray<int> a(3);
  gArray<gRational> eval(3);  //
  gList<exp_vect> exp_vect_list;
  char c;
  gString inputstring;

//  ptest(Lex);        exit(0);
 
  while (cont) {
/*
    out << "-----------------------------\n";
    out << "Menu:\n";
    out << " 0> Quit\n";
//    out << " 1) SetCoeff\n"; - DISABLED
    out << " 2) GetCoeff\n";
    out << " 3) Display Space Info\n";
    out << " 4) Display Polynomials\n";
    out << " 5) Set two polynomials equal\n";
    out << " 6) Enter a polynomial\n";
    out << " 7) Evaluate a polynomial\n";
    out << " 8) Add two polynomials\n";
    out << " 9) Sub two polynomials\n";
    out << "10) Mult two polynomials\n";
    out << "11) Negate a polynomial\n";
    out << "12) Mult by a constant \n";
    out << "13) Evaluate one Variable\n";
    out << "14) Divide by a constant \n";
    out << "15) Check if two polys are equal\n";
    out << "16) Newton polytope of polynomial\n";
    out << "17) Degree, in a variable, of a polynomial\n";
    out << "18) Total degree of a polynomial\n";
    out << "98> A Test\n";
    out << "99> Clear all polynomials\n";
    out << "-----------------------------\n";
    out << "? ";
*/
    in >> menuchoice;
    
    switch (menuchoice) {
    case 0:
      cont = 0;
      break;
/*
    case 1:
      do {
	out << "Please input which polynomial\n";
	in >> input;
      } while (input < 1 || input > 4);
      out << "Please input the powers of the term (a,b,c)\n";
      in >> a[1] >> a[2] >> a[3];
      out << a[1] << a[2] << a[3] << "\n";
      out << "Please input the new coefficient\n";
      in >> coeff;
      out << coeff << "\n";
      test[input]->SetCoef(a,coeff);
      break;
*/
    case 2:
      do {
	out << "Please input which polynomial\n";
	in >> input;
      } while (input < 1 || input > 4);
      out << "Please input the pwers of the term (a,b,c)\n";
      in >> a[1] >> a[2] >> a[3];
      coeff = test[input]->GetCoef(a);
      out << "the requested coefficient is: " << coeff << "\n";
      break;
    case 3:
      t.Dump(out);
      break;
    case 4:
      do {
	out << "Please input which polynomial\n";
	in >> input;
      } while (input < 1 || input > 4);
      out << *(test[input]); //->Print(out);
      out << "\n";
      break;
    case 5:
      do {
	out << "Please input which two polynomials\n";
	in >> poly1 >> poly2;
      } while ( poly1 < 1 || poly1 > 4 || poly2 < 1 || poly2 > 4);
      *(test[poly1]) = *(test[poly2]);
      break;
    case 6:
      do { 
	out << "Please input which polynomial\n";
	in >> input;
      } while ( input < 1 || input > 4);
      out << "Please type in a string\n";
      inputstring = "";
      c = getchar(); // throw away the extra CR.
      do {
	c = getchar();
	if ( c != 10 ) inputstring += c;
      } while (c != 10);
      out << inputstring << "\n";
      *(test[input]) = inputstring;
      break;
    case 7:
      do {
	out << "Please input which polynomial\n";
	in >> input;
      } while (input < 1 || input > 4);
      out << "Please input the values\n";
      in >> eval[1] >> eval[2] >> eval[3];
      out << test[input]->Evaluate(eval) << "\n";
      break;
    
    case 8:
      do {
	out << "Please input which two polynomials\n";
	in >> poly1 >> poly2;
	out << "Please input where to put the output\n";
	in >> input;
      } while ( poly1 < 1 || poly1 > 4 || poly2 < 1 || poly2 > 4
	       || input < 1 || input > 4);
      *(test[input]) = *(test[poly1]) + *(test[poly2]);
      break;

    case 9:
      do {
	out << "Please input which two polynomials\n";
	in >> poly1 >> poly2;
	out << "Please input where to put the output\n";
	in >> input;
      } while ( poly1 < 1 || poly1 > 4 || poly2 < 1 || poly2 > 4
	       || input < 1 || input > 4);
      *(test[input]) = *(test[poly1]) - *(test[poly2]);
      break;
    case 10:      
      do {
	out << "Please input which two polynomials\n";
	in >> poly1 >> poly2;
	out << "Please input where to put the output\n";
	in >> input;
      } while ( poly1 < 1 || poly1 > 4 || poly2 < 1 || poly2 > 4
	       || input < 1 || input > 4);
      *(test[input]) = *(test[poly1]) * *(test[poly2]);
      break;

    case 11:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
	out << "Please input where to put the output\n";
	in >> input;
      } while ( poly1 < 1 || poly1 > 4
	       || input < 1 || input > 4);
      *(test[input]) = - *(test[poly1]);
      break;

    case 12:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
	out << "Please input where to put the output\n";
	in >> input;
	out << "Please input the constant\n";
	in >> coeff; 
      } while ( poly1 < 1 || poly1 > 4 || input < 1 || input > 4);

//      assert( ((*(test[poly1])) * coeff) == (coeff * (*(test[poly1]))) );

      *(test[input]) = (*(test[poly1])) * coeff;
      break;
      
    case 13:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
	out << "Please input where to put the output\n";
	in >> poly2;;
	out << "Please input the variable to specialize\n";
	in >> input2; 
	out << "Please input the value of this variable\n";
	in >> evalvalue;
      } while ( poly1 < 1 || poly1 > 4 || poly2 < 1 || poly2 > 4);
      *(test[poly2]) = (*(test[poly1])).EvaluateOneVar(input2, evalvalue);
      break;

    case 14:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
	out << "Please input where to put the output\n";
	in >> input;
	out << "Please input the constant\n";
	in >> coeff; 
      } while ( poly1 < 1 || poly1 > 4 
	       || input < 1 || input > 4);
      *(test[input]) = (*(test[poly1])) / coeff;
      break;

    case 15:
      do {
	out << "Please input which two polynomials\n";
	in >> poly1;
	in >> poly2;
      } while ( poly1 < 1 || poly1 > 4 
	       || poly2 < 1 || poly2 > 4);
      if ( *(test[poly1]) == *(test[poly2]) ) out << "They are Equal!!!\n";
      else out << "They are not equal\n";
      break;

    case 16:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
      } while ( poly1 < 1 || poly1 > 4);
      exp_vect_list = test[poly1]->ExponentVectors();
      out << "The newton polytope of " << *(test[poly1]) << " is:\n";
      for (int i = 1; i <= exp_vect_list.Length(); i++)
	out << exp_vect_list[i] << "\n";
      break;
      
    case 17:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
	out << "Please input the variable of which to take degree\n";
	in >> input2; 
      } while ( poly1 < 1 || poly1 > 4 );
      out << "The degree of " << *(test[poly1]) << " with respect to "
	<< t[input2]->Name << " is " 
	  << (*test[poly1]).DegreeOfVar(input2) << ".\n";
      break;
      
    case 18:
      do {
	out << "Please input which polynomial\n";
	in >> poly1;
      } while ( poly1 < 1 || poly1 > 4 );
      out << "The total degree of " << *(test[poly1]) << " is " 
	<< (*test[poly1]).Degree() << ".\n";
      break;
            
    case 98:
      c = getchar(); // throw away extra carriage return
      ptest(Lex);
      break;


    case 99:
      for (k = 1; k<= 4; k++) *(test[k]) = "";
      break;
    }
  }
  
for (k = 1; k <= 4; k++) delete test[k]; 
}





void ptest(term_order* Lex)
{
  gout << "\n*******   BEGIN   **********\n";
  char continuing;

//  gPolyFamily<double> Fam(3);
  gSpace Space(3);
  gPoly<double> px(&Space, Lex);
  gPoly<double> py(&Space, Lex);
  gPoly<double> pz(&Space, Lex);

  gString gx = "-1.0 + 6 n1 + 1.5 n1^2";
  gString gy = "2.0 + 3.0 n2 + 4.0 n2^2";
  gString gz = "1 + 0.5 n3 + 2.0 n3^2";
  gString temp;

  px = gx; py = gy; pz = gz;
  
  gout << "Here are the polynomials.\n";
  gout << "px = " << px << "\n";
  gout << "py = " << py << "\n";
  gout << "pz = " << pz << "\n";

  gPoly<double> pRes(&Space, Lex);
  
  gout << "\n Evaluating polynomials.\n";
  pRes = px.EvaluateOneVar(1,2);
  gout << "  x = 2 =>  " << pRes << "\n";
  pRes = py.EvaluateOneVar(2,3);
  gout << "  y = 3 =>  " << pRes << "\n";
  
    gout << "\nAdding polynomials.\n";
  pRes = px + py;
  gout << "  " << px << "\n";
  gout << "+ " << py << "\n";
  gout << "= " << pRes << "\n\n";

  gout << "Negating polynomials.\n";
  pRes = - px;
  gout << "- " << px << "\n";
  gout << "= " << pRes << "\n\n";

  gout << "Subtracting polynomials.\n";
  pRes = px - py;
  gout << "  " << px << "\n";
  gout << "- " << py << "\n";
  gout << "= " << pRes << "\n\n";

  gout << "Multiplying polynomials.\n";
  pRes = px * py;
  gout << "  " << px << "\n";
  gout << "* " << py << "\n";
  gout << "= " << pRes << "\n\n";

  gout << "Dividing polynomials (by a constant).\n";
  pRes = px / (double) 4;
  gout << "  " << px << "\n";
  gout << "/ 4\n";
  gout << "= " << pRes << "\n\n";

  gout << "Evaluating " << (pRes = px * py) << "\n";
  pRes = pRes.EvaluateOneVar(1,1);
  gout << "  x = 1 => " << pRes << "\n";
  pRes = px * py;
  pRes = pRes.EvaluateOneVar(2,3);
  gout << " y = 3 => " << pRes << "\n";
  pRes = pRes.EvaluateOneVar(1,1);
  gout << " x = 1, y = 3 => " << pRes << "\n";

  gout << "Taking partial derivatives of " << (pRes = px * py) << ":\n";
  gout << "  partial wrt n1: " << pRes.PartialDerivative(1) << "\n";
  gout << "  partial wrt n2: " << pRes.PartialDerivative(2) << "\n";
  gout << "  partial wrt n3: " << pRes.PartialDerivative(3) << "\n";

  gout << "Finding leading coefficients of " << (pRes = px * py) << ":\n";
  gout << "  lead coef wrt n1: " << pRes.LeadingCoefficient(1) << "\n";
  gout << "  lead coef wrt n2: " << pRes.LeadingCoefficient(2) << "\n";
  gout << "  lead coef wrt n3: " << pRes.LeadingCoefficient(3) << "\n";

  gout << "Finding degrees (wrt a variable) of " << (pRes = px * py) << ":\n";
  gout << "  degree wrt n1: " << pRes.DegreeOfVar(1) << "\n";
  gout << "  degree wrt n2: " << pRes.DegreeOfVar(2) << "\n";
  gout << "  degree wrt n3: " << pRes.DegreeOfVar(3) << "\n";

  gout << "Finding (total) degree of " << (pRes = px * py) << ":\n";
  gout << "  total degree: " << pRes.Degree() << "\n";

//  gout << "\n ****** Touch Me ***********                  (Press Return)\n";
//  gin >> continuing;

  gPoly<double> px2(&Space, Lex);
  gPoly<double> py2(&Space, Lex);

  temp = "2 + 1 n2 + -1 n2^2";
  py2 = temp;
  temp = "5 - 2n1 + 0.5 n1^2";
  px2 = temp;

  gPoly<double> pMult1(px * py);
  gPoly<double> pMult2(px2 * py2);

  gout << "\nCross-term addition\n";
  gout << "  " << pMult1 << "\n";
  gout << "+ " << pMult2 << "\n";
  gout << "= " << (pMult1 + pMult2) << "\n";

  gout << "\nInner-term addition\n";
  gout << "  " << pMult1 << "\n";
  gout << "+ " << px2 << "\n";
  gout << "= " << (pMult1 + px2) << "\n";

  gout << "\nCross-term multiplication\n";
  gout << "  " << pMult1 << "\n";
  gout << "* " << pMult2 << "\n";
  gout << "= " << (pMult1 * pMult2) << "\n";

  gout << "\nCross-term division (by a constant)\n";
  gout << pMult1 << "/ 4\n";
  pRes = pMult1 / (double) 4;
  gout << "= " << pRes << "\n";

  gout << "\nBig addition.\n";
  pRes = px + py + pz;
  gout << "  " << px << "\n";
  gout << "+ " << py << "\n";
  gout << "+ " << pz << "\n";
  gout << "= " << pRes << "\n";

  gout << "\nBig multiplication.\n";
  pRes = px * py * pz;
  gout << "  " << px << "\n";  
  gout << "* " << py << "\n";
  gout << "* " << pz << "\n";
  gout << "= " << pRes << "\n";

  pRes = pRes.EvaluateOneVar(2, 3);
  pRes = pRes.EvaluateOneVar(1, 2);
  pRes = pRes.EvaluateOneVar(3, 4);
  gout << "\n x=2, y=3, z=4 => " << pRes << "\n";

//  gout << "\n ****** Touch Me ***********                  (Press Return)\n";
//  gin >> continuing;

  int num_players = 2;
  int *num_strat;
  num_strat = new int[2];
  num_strat[0] = 3;
  num_strat[1] = 2;
  int tot_strat = 5;
  int count_strat, i;

  gout << "\nTo find the equilibrium of this game: \n";
  gout << "              |      Player #2    |\n";
  gout << "              |  Set 1     Set 2  |\n";
  gout << "        ------+---------+---------+-\n";
  gout << "Player  Set 1 |   3, 2  |   2, 3  |\n";
  gout << " #1     Set 2 |   2, 3  |   3, 2  |\n";
  gout << "        Set 3 |   1, 2  |   1, 2  |\n";
  gout << "\n";
  
  gSpace Space2(5);
  Space2.SetVariableName(1, "pp1s1");
  Space2.SetVariableName(2, "pp1s2");
  Space2.SetVariableName(3, "pp1s3");
  Space2.SetVariableName(4, "pp2s1");
  Space2.SetVariableName(5, "pp2s2");


  gPoly<gRational> pp1s1(&Space2, Lex);
  gPoly<gRational> pp1s2(&Space2, Lex);
  gPoly<gRational> pp1s3(&Space2, Lex);
  gPoly<gRational> pp2s1(&Space2, Lex);
  gPoly<gRational> pp2s2(&Space2, Lex);
  
  gout << "\nThe sum of a player's probabilities should equal 1,\n";
  gout << "  so these should equal 0:\n";
 
  gPoly<gRational> *probsums;
  probsums = new (gPoly<gRational>[num_players])(&Space2, Lex);
  gPoly<gRational> zeroPoly(&Space2, Lex);
  temp = "-1";
  probsums[0] = temp;
   temp = "1 pp1s1";
  pp1s1 = temp;
  temp = "1 pp1s2";
  pp1s2 = temp;
  temp = "1 pp1s3";
  pp1s3 = temp;
  probsums[0] = probsums[0] + pp1s1 + pp1s2 + pp1s3;  
 
  temp = "-1";
  probsums[1] = temp;
  temp = "1 pp2s1";
  pp2s1 = temp;
  temp = "1 pp2s2";
  pp2s2 = temp;
  probsums[1] = probsums[1] + pp2s1 + pp2s2;
  for (i = 1; i<= num_players; i++){
    gout << probsums[i-1] << "\n";
  }
  gout << "\n";
  
  gout << "Individual probabilities: (greater than or equal to 0)\n";

  gPoly<gRational> *probs;
  probs = new gPoly<gRational>[tot_strat](&Space2, Lex);
  probs[0] = pp1s1;
  probs[1] = pp1s2;
  probs[2] = pp1s3;
  probs[3] = pp2s1;
  probs[4] = pp2s2;
  for (count_strat = 1; count_strat <= tot_strat; count_strat++) {
    gout << probs[count_strat-1] << "\n";
  }
  gout << "\n";
  
  gout << "Expected payoffs for each strategy: \n";
  gPoly<gRational> *payoffs;
  payoffs = new gPoly<gRational>[tot_strat](&Space2, Lex);
  payoffs[0] = (3 * pp2s1) + (2 * pp2s2);
  payoffs[1] = (2 * pp2s1) + (3 * pp2s2);
  payoffs[2] = (1 * pp2s1) + (1 * pp2s2);
  payoffs[3] = (2 * pp1s1) + (3 * pp1s2) + (2 * pp1s3);
  payoffs[4] = (3 * pp1s1) + (2 * pp1s2) + (2 * pp1s3);

  for (count_strat = 1; count_strat <= tot_strat; count_strat++) {
    gout << payoffs[count_strat-1] << "\n";
  }
  gout << "\n";
  
  gout << "Done.\n\n";
//  gout << "\n ****** Touch Me ***********                  (Press Return)\n";
//  gin >> continuing;
  
  
}
 
