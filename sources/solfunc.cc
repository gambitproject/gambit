//
// FILE: solfunc.cc -- GCL functions on profiles and solutions
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"


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

//------------------
// ActionValues
//------------------

Portion *GSM_ActionValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<double> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_ActionValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<gRational> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
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
    if( p1->Type() != porLIST )
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
      if( p2->Type() != porLIST )
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
	if( p3->Type() != porFLOAT )
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
    if( p1->Type() != porLIST )
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
      if( p2->Type() != porLIST )
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
	if( p3->Type() != porRATIONAL )
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
  if (S->BelongsTo().Type() == DOUBLE && param[1]->Type() & porFLOAT )
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
      if( p1->Type() != porLIST )
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
	if( p2->Type() != porLIST )
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
	  if( p3->Type() != porFLOAT )
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
  else if (S->BelongsTo().Type()== RATIONAL && param[1]->Type() & porRATIONAL )
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
      if( p1->Type() != porLIST )
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
	if( p2->Type() != porLIST )
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
	  if( p3->Type() != porRATIONAL )
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


/*
//------------------
// GobitLambda
//------------------

Portion* GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

//-----------------
// GobitValue
//-----------------

Portion* GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}
*/

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
  return new IntValPortion(P->IsNash());
}

Portion *GSM_IsNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsNash());
}

//--------------------
// IsSequential
//--------------------

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

//---------------
// List
//---------------

Portion *GSM_List_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new FloatValPortion( (*P)( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

}


Portion *GSM_List_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new RationalValPortion( (*P)( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

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
    if( p3->Type() == porLIST )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Type() == porFLOAT );
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
    if( p3->Type() == porLIST )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Type() == porRATIONAL );
    (*P)( PlayerNum, InfosetNum, k ) = ( (RationalPortion*) p3 )->Value();

    delete p3;
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



void Init_solfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ActionValues");
  FuncObj->SetFuncInfo(GSM_ActionValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 0, "strategy",
			porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 1, "infoset", porINFOSET);

  FuncObj->SetFuncInfo(GSM_ActionValuesRational, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 1, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj( "Behav" );
  FuncObj->SetFuncInfo( GSM_Behav_EfgFloat, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Behav_EfgFloat, 
			1, "list", porLIST | porFLOAT,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 3);

  FuncObj->SetFuncInfo( GSM_Behav_EfgRational, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Behav_EfgRational, 
			1, "list", porLIST | porRATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 3);

  FuncObj->SetFuncInfo( GSM_Behav_EFSupport, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EFSupport, 0, "support", porEF_SUPPORT,
			NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Behav_EFSupport, 
			1, "list", porLIST | porFLOAT | porRATIONAL );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Beliefs");
  FuncObj->SetFuncInfo(GSM_BeliefsFloat, 1);
  FuncObj->SetParamInfo(GSM_BeliefsFloat, 0, "strategy", 
			porBEHAV_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_BeliefsRational, 1);
  FuncObj->SetParamInfo(GSM_BeliefsRational, 0, "strategy",
			porBEHAV_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Centroid");
  FuncObj->SetFuncInfo(GSM_CentroidEfgFloat, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidEfgRational, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidEFSupport, 1);
  FuncObj->SetParamInfo(GSM_CentroidEFSupport, 0, "support",
			porEF_SUPPORT);

  FuncObj->SetFuncInfo(GSM_CentroidNfgFloat, 1);
  FuncObj->SetParamInfo(GSM_CentroidNfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidNfgRational, 1);
  FuncObj->SetParamInfo(GSM_CentroidNfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidNFSupport, 1);
  FuncObj->SetParamInfo(GSM_CentroidNFSupport, 0, "support",
			porNF_SUPPORT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InfosetProbs");
  FuncObj->SetFuncInfo(GSM_InfosetProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_InfosetProbsRational, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsNash");
  FuncObj->SetFuncInfo(GSM_IsNash_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsNash_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsNash_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsNash_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSequential");
  FuncObj->SetFuncInfo(GSM_IsSequential_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsSequential_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsSequential_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsSequential_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSubgamePerfect");
  FuncObj->SetFuncInfo(GSM_IsSubgamePerfect_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsSubgamePerfect_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsSubgamePerfect_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsSubgamePerfect_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapValue");
  FuncObj->SetFuncInfo(GSM_LiapValue_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);  
  FuncObj->SetFuncInfo(GSM_LiapValue_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

   
  FuncObj = new FuncDescObj( "ListForm" );
  FuncObj->SetFuncInfo( GSM_List_BehavFloat, 1 );
  FuncObj->SetParamInfo(GSM_List_BehavFloat, 
			0, "behav", porBEHAV_FLOAT );
  FuncObj->SetFuncInfo( GSM_List_BehavRational, 1 );
  FuncObj->SetParamInfo(GSM_List_BehavRational, 
			0, "behav", porBEHAV_RATIONAL );
  gsm->AddFunction( FuncObj );


  FuncObj = new FuncDescObj("NodeValues");
  FuncObj->SetFuncInfo(GSM_NodeValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 0, "strategy", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 1, "player", porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_NodeValuesRational, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 1, "player", porPLAYER_EFG);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RealizProbs");
  FuncObj->SetFuncInfo(GSM_RealizProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_RealizProbsRational, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj( "SetComponent" );
  FuncObj->SetFuncInfo( GSM_SetComponent_BehavFloat, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			0, "behav", porBEHAV_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			1, "infoset", porINFOSET );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			2, "list", porLIST | porFLOAT );

  FuncObj->SetFuncInfo( GSM_SetComponent_BehavRational, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			0, "behav", porBEHAV_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			1, "infoset", porINFOSET );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			2, "list", porLIST | porRATIONAL );

  gsm->AddFunction( FuncObj );


  FuncObj = new FuncDescObj("Support");
  FuncObj->SetFuncInfo(GSM_Support_Behav, 1);
  FuncObj->SetParamInfo(GSM_Support_Behav, 0, "strategy",
			porBEHAV, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);
}
