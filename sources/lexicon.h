//
// FILE: lexicon.h -- Declaration of the efg<->nfg strategy correspondence
//
// $Id$
//

#ifndef LEXICON_H
#define LEXICON_H


typedef gArray<int> Correspondence;

class Lexicon   {
  public:
    BaseNfg *N;
    gArray<gList<Correspondence *> > strategies;

    Lexicon(const BaseEfg &);
    ~Lexicon();

    void MakeStrategy(EFPlayer *p);
    void MakeReducedStrats(EFPlayer *, Node *, Node *);
};


#endif   // LEXICON_H
