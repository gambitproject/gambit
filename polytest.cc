#include <stdio.h>
#include "gambitio.h"
#include "rational.h"
#include "polynom.h"

main ()
{
  gout << "\nStarting program.\n";
  char continuing;

  gPolynom<double> px('x', 0, 2);
  gPolynom<double> py('y', 0, 2);
  gPolynom<double> pz('z', 1, 2);
  

  px[0] = -1.0;
  py[0] = 2.0;
  py[1] = 3.0;
  py[2] = 4.0;
  px[2] = 1.5;
  px[1] = 6;
  pz[0] = 1;
  pz[1] = 0.5;
  pz[2] = 2.0;
  gout << "Ready to start printing polynomials.\n";
  gout << "px = " << px << "\n";
  gout << "py = " << py << "\n";
  gout << "pz = " << pz << "\n";

  gPolynom<double> pRes('x', 0, 2);
  px.AutoSwap();
  py.AutoSwap();
  pz.AutoSwap();

  gout << "\nEvaluating polynomials.\n";
  pRes = px.Evaluate('x', 2);
  gout << "  x = 2 =>  " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";
  pRes = py.Evaluate('y', 3);
  gout << "  y = 3 =>  " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";

  gout << "\nAdding polynomials.\n";
  pRes = px + py;
  gout << "  " << px << "\n";
  gout << "+ " << py << "\n";
  gout << "= " << pRes << "\n\n";

  gout << "Inverting polynomials.\n";
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
  pRes = pRes.Evaluate('x', 1);
  gout << "  x = 1 => " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";
  pRes = px * py;
  pRes = pRes.Evaluate('y', 3);
  gout << "  y = 3 => " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";
  pRes = pRes.Evaluate('x', 1);
  gout << "  x = 1, y = 3 => " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";

  gout << "\n****************Press return to continue.  ";
  gin >> continuing;

  gPolynom<double> px2('x', 0, 2);
  gPolynom<double> py2('y', 0, 2);
    
  py2[0] = 2;
  py2[1] = 1;
  py2[2] = -1;
  px2[0] = 5;
  px2[1] = -2;
  px2[2] = 0.5;

  gPolynom<double> pMult1 = px * py;
  gPolynom<double> pMult2 = px2 * py2;
  pMult1.AutoSwap();
  pMult2.AutoSwap();

  gout << "\nCross-term addition\n";
  gout << "  " << pMult1 << "\n";
  gout << "+ " << pMult2 << "\n";
  gout << "= " << pMult1 + pMult2 << "\n";

  gout << "\nInner-term addition\n";
  gout << "  " << pMult1 << "\n";
  gout << "+ " << px2 << "\n";
  gout << "= " << pMult1 + px2 << "\n";

  gout << "\nCross-term multiplication\n";
  gout << "  " << pMult1 << "\n";
  gout << "* " << pMult2 << "\n";
  gout << "= " << pMult1 * pMult2 << "\n";

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

  pRes = pRes.Evaluate('y', 3);
  pRes = pRes.Evaluate('x', 2);
  pRes = pRes.Evaluate('z', 4);
  gout << "\n x=2, y=3, z=4 => " << pRes << "\n";
  if (pRes.isConst()) gout << "   a constant.\n";

  gout << "\n****************Press return to continue.  ";
  gin >> continuing;

  gPolynom<Rational> pb('b', 2, 4);
  gPolynom<Rational> pxx('x', 0, 1);
  gPolynom<Rational> pyv("yv", 0, 1);
  gPolynom<Rational> pzo('z', 1, 1);
  gPolynom<Rational> pzz("zz", 1, 1);
  
  pb[4] = (Rational) 1;
  pzz[1] = (Rational) 1/2;
  pb[3] = pzz;
  pzo[1] = (Rational) 3;
  pxx[1] = (Rational) 4;
  pxx[0] = pzo;
  pyv[1] = (Rational) 0.5;
  pyv[0]= pxx;
  pb[2] = pyv;
  pxx[1] = (Rational) 6;
  pxx[0] = (Rational) 0;
  pb[1] = pxx;
  pb[0] = (Rational) 7;
  gout << "\nA disordered polynomial, as input.\n";
  gout << pb << "\n";
  pb.AutoSwap();

  gout << "\nAfter changing to computational form: \n";
  gout << pb << "\n\nWith extra variables removed:\n";
  pb.Prune();
  gout << pb << "\n\n";

  pyv.AutoSwap();
  pyv.Prune();

  gout << "Addition:\n";
  gout << "  " << pb << "\n";
  gout << "+ " << pyv << "\n";
  gout << "= " << pb + pyv << "\n\n";

  gout << "Multiplication:\n";
  gout << "  " << pb << "\n";
  gout << "* " << pyv << "\n";
  gout << "= " << pb * pyv << "\n\n";

  gout << "Division (by constant)\n";
  gout << pb << " / 4\n";
  gout << "= " << pb / (Rational) 4 << "\n";

  gout << "\n****************Press return to continue.  ";
  gin >> continuing;

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

  gPolynom<Rational> pp1s1("pp1s1");
  gPolynom<Rational> pp1s2("pp1s2");
  gPolynom<Rational> pp1s3("pp1s3");
  gPolynom<Rational> pp2s1("pp2s1");
  gPolynom<Rational> pp2s2("pp2s2");

  gout << "\nThe sum of a player's probabilities should equal 1,\n";
  gout << "  so these should equal 0:\n";
  gPolynom<Rational> *probsums;
  probsums = new gPolynom<Rational>[num_players];
  gPolynom<Rational> zeroPoly;
  probsums[0] = zeroPoly;
  probsums[0].vcoef(0) = (Rational) -1;
  pp1s1[1] = (Rational) 1;
  pp1s2[1] = (Rational) 1;
  pp1s3[1] = (Rational) 1;
  probsums[0] = probsums[0] + pp1s1 + pp1s2 + pp1s3;
  probsums[1] = zeroPoly;
  probsums[1].vcoef(0) = (Rational) -1;
  pp2s1[1] = (Rational) 1;
  pp2s2[1] = (Rational) 1;
  probsums[1] = probsums[1] + pp2s1 + pp2s2;
  for (i = 1; i <= num_players; i++) {
    probsums[i-1].Prune();
    gout << probsums[i-1] << "\n";
  }
  gout << "\n";

  gout << "Individual probabilities: (greater than or equal to 0)\n";
  gPolynom<Rational> *probs;
  probs = new gPolynom<Rational>[tot_strat];
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
  gPolynom<Rational> *payoffs;
  payoffs = new gPolynom<Rational>[tot_strat];
  payoffs[0] = (3 * pp2s1) + (2 * pp2s2);
  payoffs[1] = (2 * pp2s1) + (3 * pp2s2);
  payoffs[2] = (1 * pp2s1) + (1 * pp2s2);
  payoffs[3] = (2 * pp1s1) + (3 * pp1s2) + (2 * pp1s3);
  payoffs[4] = (3 * pp1s1) + (2 * pp1s2) + (2 * pp1s3);
  for (count_strat = 1; count_strat <= tot_strat; count_strat++) {
    gout << payoffs[count_strat-1] << "\n";
  }
  gout << "\n";

  gout << "Done with bigtest program.\n\n";
}
