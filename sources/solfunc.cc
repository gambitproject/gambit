//
// FILE: solfunc.cc -- GCL functions on profiles and solutions
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"
#include "nfg.h"
#include "behavsol.h"
#include "mixedsol.h"

#include "nfplayer.h"

#include "efgutils.h"

//
// Implementations of these are provided as necessary in gsmutils.cc
//
template <class T> Portion *ArrayToList(const gArray<T> &);
// these added to get around g++ 2.7.2 not properly completing type
// unification when gVector passed to the parameter...
extern Portion *ArrayToList(const gArray<double> &);
extern Portion *ArrayToList(const gArray<gRational> &);
template <class T> Portion *ArrayToList(const gList<T> &);
template <class T> Portion *gDPVectorToList(const gDPVector<T> &);


//----------------
// ActionProb
//----------------

Portion *GSM_ActionProb_Float(Portion **param)
{
  Portion *por;
  BehavSolution<double>* profile = 
    (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  EFPlayer* player = infoset->GetPlayer();
  
  if (player->IsChance())
    por = new FloatValPortion(((ChanceInfoset<double> *) infoset)->
			      GetActionProbs()[action->GetNumber()]);  
  else if (profile->GetEFSupport().Find(action))
    por = new FloatValPortion((*profile)(player->GetNumber(),
					 infoset->GetNumber(),
					 profile->GetEFSupport().Find(action)));
  else
    por = new FloatValPortion(0.0);

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_ActionProb_Rational(Portion **param)
{
  Portion *por;
  BehavSolution<gRational>* profile = 
    (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  EFPlayer* player = infoset->GetPlayer();
  
  if (player->IsChance())
    por = new RationalValPortion(((ChanceInfoset<gRational> *) infoset)->
				 GetActionProbs()[action->GetNumber()]);  
  else if (profile->GetEFSupport().Find(action))
    por = new RationalValPortion((*profile)(player->GetNumber(),
					    infoset->GetNumber(),
					    profile->GetEFSupport().Find(action)));
  else
    por = new RationalValPortion(0);

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


//------------------
// ActionValue
//------------------

Portion *GSM_ActionValue_Float(Portion **param)
{
  BehavSolution<double> *profile =
    (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->BelongsTo() != profile->BelongsTo())
    return new ErrorPortion("Profile and infoset must belong to same game");

  if (infoset->GetPlayer()->IsChance())
    return new NullPortion(porFLOAT);
  //return new ErrorPortion("Infoset must belong to personal player");
  else if (profile->GetEFSupport().Find(action))  {
    Efg<double> *efg = profile->BelongsTo();

    gDPVector<double> values(efg->Dimensionality());
    gPVector<double> probs(efg->Dimensionality().Lengths());

    profile->CondPayoff(values, probs);
  
    return new FloatValPortion(values(infoset->GetPlayer()->GetNumber(), 
				      infoset->GetNumber(),
				      profile->GetEFSupport().Find(action)));
  }
  else
    return new ErrorPortion("Not implemented yet for non-support actions");
}

Portion *GSM_ActionValue_Rational(Portion **param)
{
  BehavSolution<gRational> *profile =
    (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->BelongsTo() != profile->BelongsTo())
    return new ErrorPortion("Profile and infoset must belong to same game");

  if (infoset->GetPlayer()->IsChance())
    return new NullPortion(porRATIONAL);
  //return new ErrorPortion("Infoset must belong to personal player");
  else if (profile->GetEFSupport().Find(action))  {
    Efg<gRational> *efg = profile->BelongsTo();

    gDPVector<gRational> values(efg->Dimensionality());
    gPVector<gRational> probs(efg->Dimensionality().Lengths());

    profile->CondPayoff(values, probs);
  
    return new RationalValPortion(values(infoset->GetPlayer()->GetNumber(), 
					 infoset->GetNumber(),
					 profile->GetEFSupport().Find(action)));
  }
  else
    return new ErrorPortion("Not implemented yet for non-support actions");
}

//--------------
// Behav
//--------------

Portion *GSM_Behav_EfgFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().ListDepth == 0 )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Spec().Type != porFLOAT )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;

}

Portion *GSM_Behav_EfgRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().ListDepth == 0 )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Spec().Type != porRATIONAL )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_Behav_EFSupport(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por = 0;


  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  // This is incredibly redundent; must find a way to reuse the code
  // from the previous two functions.
  if (S->BelongsTo().Type() == DOUBLE && param[1]->Spec().Type & porFLOAT )
  {
    // The code here is completely copied from GSM_Behav_EfgFloat

    Efg<double> &E = * (Efg<double>*) &S->BelongsTo();
    BehavSolution<double> *P = new BehavSolution<double>(E);

    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Spec().ListDepth == 0 )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Spec().ListDepth == 0 )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Spec().Type != porFLOAT )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }
    por = new BehavValPortion(P);


  }
  else if (S->BelongsTo().Type()== RATIONAL && param[1]->Spec().Type & porRATIONAL )
  {
    // The code here is entirely copied from GSM_Behav_EfgRational()

    Efg<gRational> &E = * (Efg<gRational>*) &S->BelongsTo();
    BehavSolution<gRational> *P = new BehavSolution<gRational>(E);
    
    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Spec().ListDepth == 0 )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Spec().ListDepth == 0 )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Spec().Type != porRATIONAL )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }

    por = new BehavValPortion(P);
  }

  if( por == 0 )
    por = new ErrorPortion( "Mismatching EFG and list type" );
  else
  {
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
  }
  return por;
}

//-------------
// Belief
//-------------

Portion* GSM_Belief_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  gDPVector<double> values(bp->Beliefs());
  Infoset* s = n->GetInfoset();
  gArray<Node*> members = s->GetMemberList();

  if(s->IsChanceInfoset())
    return new NullPortion(porFLOAT);
  if(n->NumChildren() == 0)
    return new NullPortion(porFLOAT);

  int i = 0;
  int found = 0;
  for(i=1; i<=members.Length(); i++)
    if(members[i] == n)
      found = i;
  if(!found)
    return new NullPortion(porFLOAT);
  //return new ErrorPortion("Node not a member of belief vector");
  return new FloatValPortion(values(s->GetPlayer()->GetNumber(), 
				    s->GetNumber(), found));
}

Portion* GSM_Belief_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  gDPVector<gRational> values(bp->Beliefs());
  Infoset* s = n->GetInfoset();
  gArray<Node*> members = s->GetMemberList();

  if(s->IsChanceInfoset())
    return new NullPortion(porFLOAT);
  if(n->NumChildren() == 0)
    return new NullPortion(porFLOAT);

  int i = 0;
  int found = 0;
  for(i=1; i<=members.Length(); i++)
    if(members[i] == n)
      found = i;
  if(!found)
    return new NullPortion(porRATIONAL);
  //return new ErrorPortion("Node not a member of belief vector");
  return new RationalValPortion(values(s->GetPlayer()->GetNumber(), 
				       s->GetNumber(), found));
}

//-------------
// Beliefs
//-------------

Portion *GSM_BeliefsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

//--------------
// Centroid
//--------------
 
Portion *GSM_CentroidEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEFSupport(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  BaseBehavProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new BehavSolution<double>( *S );
  else
    P = new BehavSolution<gRational>( *S );

  Portion *por = new BehavValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<double> *P = new MixedSolution<double>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<gRational> *P = new MixedSolution<gRational>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNFSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  BaseMixedProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S);
  else
    P = new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);

  Portion *por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


//---------------
// Game
//---------------

Portion* GSM_Game_Mixed(Portion** param)
{
  if(param[0]->Owner())
    return param[0]->Owner()->RefCopy();  
  else
    return 0;
}

Portion* GSM_Game_Behav(Portion** param)
{
  if(param[0]->Owner())
    return param[0]->Owner()->RefCopy();  
  else
    return 0;
}

Portion* GSM_Game_NFSupport(Portion** param)
{
  if(param[0]->Owner())
    return param[0]->Owner()->RefCopy();  
  else
    return 0;
}

Portion* GSM_Game_EFSupport(Portion** param)
{
  if(param[0]->Owner())
    return param[0]->Owner()->RefCopy();  
  else
    return 0;
}


//---------------
// GobitLambda
//---------------

Portion* GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_BehavRational(Portion** param)
{  
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();  
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porRATIONAL);
  return new RationalValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porRATIONAL);
  return new RationalValPortion( bs->GobitLambda() );
}

//--------------
// GobitValue
//--------------

Portion* GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porRATIONAL);
  return new RationalValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porRATIONAL);
  return new RationalValPortion( bs->GobitValue() );
}

//----------------
// InfosetProb
//----------------

Portion *GSM_InfosetProb_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  int i;
  int found1 = 0;
  EFPlayer* p = 0;
  for(i=1; i<=E->NumPlayers(); i++)
    if(s->GetPlayer() == E->PlayerList()[i])
      found1 = i;
  
  if(!found1)
    return new NullPortion(porFLOAT);
  //return new ErrorPortion("Infoset not found in strategy set");
  else
    p = E->PlayerList()[found1];

  int found2 = 0;
  for(i=1; i<=p->NumInfosets(); i++)
    if(s == p->InfosetList()[i])
      found2 = i;
  
  if(!found2)
    return new ErrorPortion("Infoset not found in player");  
  
  return new FloatValPortion(probs(found1, found2));
}

Portion *GSM_InfosetProb_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  int i;
  int found1 = 0;
  EFPlayer* p = 0;
  for(i=1; i<=E->NumPlayers(); i++)
    if(s->GetPlayer() == E->PlayerList()[i])
      found1 = i;
  
  if(!found1)
    return new NullPortion(porRATIONAL);
  //return new ErrorPortion("Infoset not found in strategy set");
  else
    p = E->PlayerList()[found1];

  int found2 = 0;
  for(i=1; i<=p->NumInfosets(); i++)
    if(s == p->InfosetList()[i])
      found2 = i;
  
  if(!found2)
    return new ErrorPortion("Infoset not found in player");  
  
  return new RationalValPortion(probs(found1, found2));
}

//----------------
// InfosetProbs
//----------------

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_InfosetProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}


//----------------
// IsNash
//----------------

Portion *GSM_IsNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

Portion *GSM_IsNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}


//----------------
// IsntNash
//----------------

Portion *GSM_IsntNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

Portion *GSM_IsntNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

//----------------
// IsntPerfect
//----------------

Portion *GSM_IsntPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

Portion *GSM_IsntPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

//--------------
// IsntProper
//--------------

Portion *GSM_IsntProper_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_NO);
}

Portion *GSM_IsntProper_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_NO);
}

//-------------
// IsPerfect
//-------------

Portion *GSM_IsPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

Portion *GSM_IsPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

//------------
// IsProper
//------------

Portion *GSM_IsProper_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_YES);
}

Portion *GSM_IsProper_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new BoolValPortion(P->IsProper() == T_YES);
}

//----------------
// IsSequential
//----------------

Portion *GSM_IsSequential_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

Portion *GSM_IsSequential_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

//--------------------
// IsSubgamePerfect
//--------------------

Portion *GSM_IsSubgamePerfect_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

Portion *GSM_IsSubgamePerfect_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

//----------------
// LiapValue
//----------------

Portion *GSM_LiapValue_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new FloatValPortion(P->LiapValue());
}

Portion *GSM_LiapValue_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new RationalValPortion(P->LiapValue());
}


Portion *GSM_LiapValue_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new FloatValPortion(P->LiapValue());
}

Portion *GSM_LiapValue_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new RationalValPortion(P->LiapValue());
}

//---------------
// ListForm
//---------------

Portion *GSM_ListForm_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return gDPVectorToList(* (gDPVector<double>*) P);
}


Portion *GSM_ListForm_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return gDPVectorToList(* (gDPVector<gRational>*) P);
}

Portion *GSM_ListForm_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new FloatValPortion( (*P)( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}


Portion *GSM_ListForm_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new RationalValPortion( (*P)( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}



//---------------------------- Gripe ------------------------------//

Portion *GSM_Gripe_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();

  gPVector<double> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for(i=1; i <= P->Lengths().Length(); i++)
  {
    p1 = new ListValPortion();

    for(j=1; j <= P->Lengths()[i]; j++)
    {
      p2 = new FloatValPortion(v(i, j));
      ((ListValPortion*) p1)->Append(p2);
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}


Portion *GSM_Gripe_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;
  Portion* por;

  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();

  gPVector<gRational> v(*P);

  P->Gripe(v);

  por = new ListValPortion();

  for( i = 1; i <= P->Lengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->Lengths()[i]; j++ )
    {
      p2 = new RationalValPortion( v( i, j ) );
      ((ListValPortion*) p1)->Append( p2 );
    }

    ((ListValPortion*) por)->Append( p1 );
  }

  return por;
}




Portion *GSM_Gripe_MixedFloat_Strategy(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  gPVector<double> v(*P);
  P->Gripe(v);

  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->nfp;
  BaseNfg* n = p->BelongsTo();
  
  int i = 0;
  int player = 0;
  int strategy = 0;
  for(i=1; i<=p->NumStrats(); i++)
    if(p->StrategyList()[i] == s)
      strategy = i;
  for(i=1; i<=n->NumPlayers(); i++)
    if(n->PlayerList()[i] == p)
      player = i;
  assert(player > 0);
  assert(strategy > 0);

  return new FloatValPortion(v(player, strategy));
}

Portion *GSM_Gripe_MixedRational_Strategy(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();
  gPVector<gRational> v(*P);
  P->Gripe(v);

  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->nfp;
  BaseNfg* n = p->BelongsTo();
  
  int i = 0;
  int player = 0;
  int strategy = 0;
  for(i=1; i<=p->NumStrats(); i++)
    if(p->StrategyList()[i] == s)
      strategy = i;
  for(i=1; i<=n->NumPlayers(); i++)
    if(n->PlayerList()[i] == p)
      player = i;
  assert(player > 0);
  assert(strategy > 0);

  return new RationalValPortion(v(player, strategy));
}






Portion *GSM_Gripe_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  gDPVector<double> v(*P);
  P->Gripe(v);
  return gDPVectorToList(v);
}


Portion *GSM_Gripe_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  gDPVector<gRational> v(*P);
  P->Gripe(v);
  return gDPVectorToList(v);
}


Portion *GSM_Gripe_BehavFloat_Action(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();

  gDPVector<double> v(*P);
  P->Gripe(v);

  Action* a = ((ActionPortion*) param[1])->Value();
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();
  BaseEfg* e = p->BelongsTo();

  if(s->IsChanceInfoset())
    return new NullPortion(porFLOAT);
  
  int i = 0;
  int player = 0;
  int infoset = 0;
  int action = 0;
  for(i=1; i<=s->NumActions(); i++)
    if(s->GetActionList()[i] == a)
      action = i;
  for(i=1; i<=p->NumInfosets(); i++)
    if(p->InfosetList()[i] == s)
      infoset = i;
  for(i=1; i<=e->NumPlayers(); i++)
    if(e->PlayerList()[i] == p)
      player = i;
  assert(player > 0);
  assert(infoset > 0);
  assert(action > 0);

  if (!s->GetPlayer()->IsChance()) 
    return new FloatValPortion(v(player, infoset, action));
  else
    return new NullPortion(porFLOAT);
}


Portion *GSM_Gripe_BehavRational_Action(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();

  gDPVector<gRational> v(*P);
  P->Gripe(v);

  Action* a = ((ActionPortion*) param[1])->Value();
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();
  BaseEfg* e = p->BelongsTo();

  if(s->IsChanceInfoset())
    return new NullPortion(porRATIONAL);
  
  int i = 0;
  int player = 0;
  int infoset = 0;
  int action = 0;
  for(i=1; i<=s->NumActions(); i++)
    if(s->GetActionList()[i] == a)
      action = i;
  for(i=1; i<=p->NumInfosets(); i++)
    if(p->InfosetList()[i] == s)
      infoset = i;
  for(i=1; i<=e->NumPlayers(); i++)
    if(e->PlayerList()[i] == p)
      player = i;
  assert(player > 0);
  assert(infoset > 0);
  assert(action > 0);

  if (!s->GetPlayer()->IsChance()) 
    return new RationalValPortion(v(player, infoset, action));
  else
    return new NullPortion(porRATIONAL);
}


//----------
// Mixed
//----------

Portion *GSM_Mixed_NfgFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<double> *P = new MixedSolution<double>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != porFLOAT )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (FloatPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion *GSM_Mixed_NfgRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedSolution<gRational> *P = new MixedSolution<gRational>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != porRATIONAL )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion* GSM_Mixed_NFSupport( Portion** param )
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gArray<int> dim = S->SupportDimensions();
  BaseMixedProfile *P;
  Portion* por;
  unsigned long datatype;
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  switch( param[ 0 ]->Owner()->Spec().Type )
  {
  case porNFG_FLOAT:
    P = new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S);
    datatype = porFLOAT;
    break;
  case porNFG_RATIONAL:
    P = new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);
    datatype = porRATIONAL;
    break;
  default:
    assert( 0 );
  }


  if( ( (ListPortion*) param[1] )->Length() != dim.Length() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= dim.Length(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Spec().ListDepth == 0 )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != dim[ i ] )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }
    
    for( j = 1; j <= dim[ i ]; j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Spec().Type != datatype )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      switch( datatype )
      {
      case porFLOAT:
	( * (MixedSolution<double>*) P )( i, j ) = 
	  ( (FloatPortion*) p2 )->Value();
	break;
      case porRATIONAL:
	( * (MixedSolution<gRational>*) P )( i, j ) = 
	  ( (RationalPortion*) p2 )->Value();
	break;
      default:
	assert( 0 );
      }
      
      delete p2;
    }
    delete p1;
  }
  
  por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


//----------------
// NodeValue
//----------------

Portion *GSM_NodeValue_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();
  Node* n = ((NodePortion*) param[2])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;

  return new FloatValPortion(bp->NodeValues(p->GetNumber())[found]);
}

Portion *GSM_NodeValue_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();
  Node* n = ((NodePortion*) param[2])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;

  return new RationalValPortion(bp->NodeValues(p->GetNumber())[found]);
}


//----------------
// NodeValues
//----------------

Portion *GSM_NodeValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 
//----------------
// RealizProb
//----------------

Portion *GSM_RealizProb_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;
  
  return new FloatValPortion(bp->NodeRealizProbs()[found]);
}  

Portion *GSM_RealizProb_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;
  
  return new RationalValPortion(bp->NodeRealizProbs()[found]);
}  

//----------------
// RealizProbs
//----------------

Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}

//-----------------
// SetComponent
//-----------------

Portion *GSM_SetComponent_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<double>* P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<double>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Spec().ListDepth > 0 )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Spec().Type == porFLOAT );
    (*P)( PlayerNum, InfosetNum, k ) = ( (FloatPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<gRational>* P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<gRational>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Spec().ListDepth > 0 )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Spec().Type == porRATIONAL );
    (*P)( PlayerNum, InfosetNum, k ) = ( (RationalPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}

Portion *GSM_SetComponent_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<double>* P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<double>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Spec().ListDepth > 0 )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Spec().Type == porFLOAT );
    (*P)( PlayerNum, j ) = ( (FloatPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<gRational>* P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<gRational>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Spec().ListDepth > 0 )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Spec().Type == porRATIONAL );
    (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}

//---------------
// Support
//---------------

Portion* GSM_Support_Behav(Portion** param)
{
  BaseBehavProfile *P = ((BehavPortion*) param[0])->Value();
  return new EfSupportValPortion(new EFSupport(P->GetEFSupport()));
}

Portion* GSM_Support_Mixed(Portion** param)
{
  BaseMixedProfile *P = ((MixedPortion *) param[0])->Value();
  return new NfSupportValPortion(new NFSupport(P->GetNFSupport()));
}




void Init_solfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ActionProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionProb_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionProb_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValue_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionValue_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Behav", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Behav_EfgFloat, 
				       porBEHAV_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("list", PortionSpec(porFLOAT, 3),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Behav_EfgRational, 
				       porBEHAV_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("list", 
					    PortionSpec(porRATIONAL, 3),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Behav_EFSupport, 
				       porBEHAV, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porEF_SUPPORT,
					    REQUIRED));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Belief", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Belief_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Belief_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Beliefs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_BeliefsFloat, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_BeliefsRational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Centroid", 6);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CentroidEfgFloat, 
				       porBEHAV_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_CentroidEfgRational, 
				       porBEHAV_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("efg", porEFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_CentroidEFSupport, 
				       porBEHAV, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porEF_SUPPORT));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_CentroidNfgFloat, 
				       porMIXED_FLOAT, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("nfg", porNFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_CentroidNfgRational,
				       porMIXED_RATIONAL, 1));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("nfg", porNFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_CentroidNFSupport,
				       porMIXED, 1));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("support", porNF_SUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Game", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_Mixed, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("mixed", porMIXED));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_Behav, porEFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Game_NFSupport, porNFG, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Game_EFSupport, porEFG, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("support", porEF_SUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitLambda", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitLambda_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitLambda_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porMIXED_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GobitLambda_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GobitLambda_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("GobitValue", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitValue_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porMIXED_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GobitValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GobitValue_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("InfosetProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProb_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InfosetProb_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InfosetProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProbsFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InfosetProbsRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsntNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsntNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsntNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  
  FuncObj = new FuncDescObj("IsntPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsntProper", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsntProper_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsntProper_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porMIXED_RATIONAL,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED_RATIONAL, 
					    REQUIRED,BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsProper", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsProper_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsProper_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED_RATIONAL,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSequential", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSequential_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsSequential_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSubgamePerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSubgamePerfect_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsSubgamePerfect_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapValue", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LiapValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT, 
					    REQUIRED, BYREF));  
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LiapValue_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LiapValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("strategy", porMIXED_FLOAT, 
					    REQUIRED, BYREF));  
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_LiapValue_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("strategy",	porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


   
  FuncObj = new FuncDescObj("ListForm", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ListForm_BehavFloat, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ListForm_BehavRational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_ListForm_MixedFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_ListForm_MixedRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Gripe", 8);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Gripe_BehavFloat, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Gripe_BehavRational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Gripe_MixedFloat, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Gripe_MixedRational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Gripe_BehavFloat_Action, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("behav", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("action", porACTION));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Gripe_BehavRational_Action, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("behav", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Gripe_MixedFloat_Strategy, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("mixed", porMIXED_FLOAT));
  FuncObj->SetParamInfo(6, 1, ParamInfoType("strategy", porSTRATEGY));
  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Gripe_MixedRational_Strategy, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("mixed", porMIXED_RATIONAL));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("strategy", porSTRATEGY));

  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Mixed", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Mixed_NfgFloat, 
				       porMIXED_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT, 2),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Mixed_NfgRational, 
				       porMIXED_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("list", 
					    PortionSpec(porRATIONAL, 2),
					    REQUIRED, BYVAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Mixed_NFSupport, 
				       porMIXED, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("list", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 1)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NodeValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValue_Float, 
				       porFLOAT, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("node", porNODE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NodeValue_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValues", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValuesFloat, 
				       PortionSpec(porFLOAT, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porPLAYER_EFG));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NodeValuesRational, 
				       PortionSpec(porRATIONAL, 1), 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porPLAYER_EFG));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("RealizProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProb_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RealizProb_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RealizProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProbsFloat, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RealizProbsRational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("SetComponent", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetComponent_BehavFloat, 
				       porBEHAV_FLOAT, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("behav", porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("list", PortionSpec(porFLOAT, 1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetComponent_BehavRational, 
				       porBEHAV_RATIONAL, 3));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("behav", porBEHAV_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("list", 
					    PortionSpec(porRATIONAL,1)));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_SetComponent_MixedFloat, 
				       porMIXED_FLOAT, 3));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("mixed", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(2, 2, ParamInfoType("list", PortionSpec(porFLOAT,1)));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_SetComponent_MixedRational, 
				       porMIXED_RATIONAL, 3));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("mixed", porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("list", 
					    PortionSpec(porRATIONAL,1)));

  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("Support", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Behav, 
				       porEF_SUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy",	porBEHAV, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Support_Mixed, 
				       porNF_SUPPORT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy",	porMIXED, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);
}

