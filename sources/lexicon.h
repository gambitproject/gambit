//
// FILE: lexicon.h -- Declaration of the efg<->nfg strategy correspondence
//
// $Id$
//

#ifndef LEXICON_H
#define LEXICON_H


#include "glist.h"

typedef gArray<int> Correspondence;
class Nfg;

class Lexicon   {
  public:
    Nfg *N;
    gArray<gList<Correspondence *> > strategies;

    Lexicon(const Efg &);
    ~Lexicon();

    void MakeStrategy(class EFPlayer *p);
    void MakeReducedStrats(const class EFSupport &, class EFPlayer *, class Node *, class Node *);
    void MakeLink(Efg *, Nfg *);
};


#endif   // LEXICON_H
