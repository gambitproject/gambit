//
// FILE: nfgfunc.cc -- Normal form command language builtins
//
// $Id$
//


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "nfg.h"

#include "glist.h"
#include "mixed.h"


//
// Implementations of these are provided as necessary in gsmutils.cc
//
Portion *ArrayToList(const gArray<NFPlayer *> &);
Portion *ArrayToList(const gArray<Strategy *> &);

Portion *GSM_IsConstSumNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  return new BoolValPortion(N.IsConstSum());
}

Portion *GSM_NumPlayersNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new IntValPortion(N.NumPlayers());
}

Portion *GSM_NameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new TextValPortion(N.GetTitle());
}

Portion *GSM_PlayersNfg(Portion **param)
{
  BaseNfg &N = *((NfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(N.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  S->GetNFStrategySet(s->nfp->GetNumber())->AddStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  S->GetNFStrategySet(s->nfp->GetNumber())->RemoveStrategy(s);

  Portion* por = new NfSupportValPortion(S);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion* GSM_NameNfPlayer( Portion** param )
{
  NFPlayer *p = ( (NfPlayerPortion*) param[ 0 ] )->Value();
  return new TextValPortion( p->GetName() );
}

Portion* GSM_NameStrategy( Portion** param )
{
  Strategy *s = ( (StrategyPortion*) param[ 0 ] )->Value();
  return new TextValPortion( s->name );
}

Portion *GSM_SetNameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameNfPlayer(Portion **param)
{
  NFPlayer *p = ((NfPlayerPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameStrategy(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  s->name = name;
  return param[0]->ValCopy();
}






Portion *GSM_NumStrats( Portion** param )
{
  int i;
  gArray< int > dim;

  NFPlayer* P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ( (NfSupportPortion*) param[ 1 ] )->Value();

  if( s == 0 )
    return new IntValPortion( P->StrategyList().Length() );
  else
  {
    dim = s->SupportDimensions();
    for( i = 1; i <= dim.Length(); i++ )
    {
      if( &( s->GetNFStrategySet( i )->GetPlayer() ) == P )
	return new IntValPortion( s->NumStrats( i ) );
    }
  }
  return new ErrorPortion( "Specified player is not found in the support" );
}


Portion *GSM_Strategies(Portion **param)
{
  int i;
  gArray< int > dim;
  Portion* por = 0;

  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ( (NfSupportPortion*) param[ 1 ] )->Value();

  if( s == 0 )
    por = ArrayToList(P->StrategyList());
  else
  {
    dim = s->SupportDimensions();
    for( i = 1; i <= dim.Length(); i++ )
    {
      if( &( s->GetNFStrategySet( i )->GetPlayer() ) == P )
      {
	por = ArrayToList( s->GetStrategy( i ) );
	break;
      }
    }
  }

  if( por != 0 )
  {
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
  }
  else
  {
    por = new ErrorPortion( "Specified player is not found in the support" );
  }
  return por;
}


#include "gwatch.h"




//---------------------- ElimDom ----------------------------//

extern NFSupport *ComputeDominated(NFSupport &S, bool strong, 
				   const gArray<int> &players,
				   gOutput &tracefile);

Portion *GSM_ElimDom(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players, gout);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


Portion *GSM_ElimDom_Nfg(Portion **param)
{
  NFSupport *S = new NFSupport( * ((NfgPortion *) param[0])->Value() );
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players, gout);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}




//---------------------------- ElimAllDom ---------------------------//

Portion *GSM_ElimAllDom(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport* new_T = S;
  NFSupport* old_T = S;
  while( new_T )
  {
    old_T = new_T;
    new_T = ComputeDominated(*old_T, strong, players, gout);
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = new NfSupportValPortion( old_T );
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


Portion *GSM_ElimAllDom_Nfg(Portion **param)
{
  NFSupport *S = new NFSupport( * ((NfgPortion *) param[0])->Value() );
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport* new_T = S;
  NFSupport* old_T = S;
  while( new_T )
  {
    old_T = new_T;
    new_T = ComputeDominated(*old_T, strong, players, gout);
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = new NfSupportValPortion( old_T );
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

 
Portion *GSM_LoadNfg(Portion **param)
{
  gString file = ((TextPortion *) param[0])->Value();

  gFileInput f(file);

  if (f.IsValid())   {
    DataType type;
    bool valid;

    NfgFileType(f, valid, type);
    if (!valid)   return new ErrorPortion("Not a valid .nfg file");
    
    switch (type)   {
      case DOUBLE:  {
	Nfg<double> *N = 0;
	ReadNfgFile((gInput &) f, N);

	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      case RATIONAL:   {
	Nfg<gRational> *N = 0;
	ReadNfgFile((gInput &) f, N);
	
	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      default:
	assert(0);
	return 0;
    }
  }
  else
    return new ErrorPortion("Unable to open file for reading");

}

Portion *GSM_CompressNfg(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *M = new Nfg<double>(N);
  return new NfgValPortion(M);
}

extern Nfg<double> *ConvertNfg(const Nfg<gRational> &);

Portion *GSM_FloatNfg(Portion **param)
{
  Nfg<gRational> &orig = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion failed.");
}

extern Nfg<gRational> *ConvertNfg(const Nfg<double> &);

Portion *GSM_RationalNfg(Portion **param)
{
  Nfg<double> &orig = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<gRational> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion failed.");
}

Portion *GSM_NewNfg(Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  Portion* p;
  
  for (int i = 1; i <= dim->Length(); i++)
  {
    p = dim->Subscript(i);
    d[i] = ((IntPortion *) p)->Value();
    delete p;
  }

  BaseNfg* N;
  if( !( (BoolPortion*) param[ 1 ] )->Value() )
    N = new Nfg<double>( d );
  else
    N = new Nfg<gRational>( d );
  return new NfgValPortion(N);
}

Portion *GSM_RandomNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}  

Portion *GSM_NewNFSupport(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  Portion *p = new NfSupportValPortion(new NFSupport(N));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}


//---------------------------- SaveNfg ------------------------//

Portion *GSM_SaveNfg(Portion **param)
{
  BaseNfg* N = ((NfgPortion*) param[0])->Value();
  gString file = ((TextPortion *) param[1])->Value();
  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N->WriteNfgFile(f);

  return param[0]->RefCopy();
}

Portion *GSM_SaveNfg_Support(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  gString file = ((TextPortion *) param[1])->Value();
  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N->WriteNfgFile(f);

  Portion* por = param[ 0 ]->ValCopy();
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}


//--------------------------- List ---------------------------//


Portion *GSM_List_MixedFloat(Portion **param)
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


Portion *GSM_List_MixedRational(Portion **param)
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





//---------------------- Mixed -------------------//


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
    if( p1->Type() != porLIST )
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
      if( p2->Type() != porFLOAT )
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
    if( p1->Type() != porLIST )
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
      if( p2->Type() != porRATIONAL )
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
  PortionType datatype;
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  switch( param[ 0 ]->Owner()->Type() )
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
    if( p1->Type() != porLIST )
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
      if( p2->Type() != datatype )
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


//----------------------- SetComponent ---------------------------//

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
    if( p2->Type() == porLIST )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Type() == porFLOAT );
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
    if( p2->Type() == porLIST )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Type() == porRATIONAL );
    (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}


//-------------------- MixedSolution data members --------------------//

//------------ IsNash ---------------//

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

//------------ IsPerfect, IsProper ----------------//

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

//---------- IsntPerfect, IsntProper ------------//

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

//------------- Support -------------//

Portion* GSM_Support_MixedFloat(Portion** param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new NfSupportValPortion(new NFSupport(P->Support()));
}

Portion* GSM_Support_MixedRational(Portion** param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  return new NfSupportValPortion(new NFSupport(P->Support()));
}

//------------ GobitLambda, GobitValue --------------//

Portion* GSM_GobitLambda_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitValue_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_MixedRational(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}

//-------------- LiapValue ---------------//

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



//---------------------------------------------------------------------


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  //--------------------- ElimDom ---------------------------//

  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimDom, 3);
  FuncObj->SetParamInfo(GSM_ElimDom, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimDom, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_ElimDom_Nfg, 3);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("ElimAllDom");
  FuncObj->SetFuncInfo(GSM_ElimAllDom, 3);
  FuncObj->SetParamInfo(GSM_ElimAllDom, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimAllDom, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimAllDom, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_ElimAllDom_Nfg, 3);
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  //----------------------------------------------------------//

  FuncObj = new FuncDescObj("CompressNfg");
  FuncObj->SetFuncInfo(GSM_CompressNfg, 1);
  FuncObj->SetParamInfo(GSM_CompressNfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewNfg");
  FuncObj->SetFuncInfo(GSM_NewNfg, 2);
  FuncObj->SetParamInfo(GSM_NewNfg, 0, "dim", porLIST | porINTEGER);
  FuncObj->SetParamInfo(GSM_NewNfg, 1, "rational", porBOOL,
			new BoolValPortion(false));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatNfg, 1);
  FuncObj->SetParamInfo(GSM_FloatNfg, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_RationalNfg, 1);
  FuncObj->SetParamInfo(GSM_RationalNfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RandomNfg");
  FuncObj->SetFuncInfo(GSM_RandomNfgFloat, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgRational, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewSupport");
  FuncObj->SetFuncInfo(GSM_NewNFSupport, 1);
  FuncObj->SetParamInfo(GSM_NewNFSupport, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AddStrategy");
  FuncObj->SetFuncInfo(GSM_AddStrategy, 2);
  FuncObj->SetParamInfo(GSM_AddStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_AddStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveStrategy");
  FuncObj->SetFuncInfo(GSM_RemoveStrategy, 2);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumStrats");
  FuncObj->SetFuncInfo(GSM_NumStrats, 2);
  FuncObj->SetParamInfo(GSM_NumStrats, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_NumStrats, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Strategies");
  FuncObj->SetFuncInfo(GSM_Strategies, 2);
  FuncObj->SetParamInfo(GSM_Strategies, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_Strategies, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadNfg");
  FuncObj->SetFuncInfo(GSM_LoadNfg, 1);
  FuncObj->SetParamInfo(GSM_LoadNfg, 0, "file", porTEXT);
  gsm->AddFunction(FuncObj);


  //------------------------ SaveNfg ----------------------------//

  FuncObj = new FuncDescObj("SaveNfg");

  FuncObj->SetFuncInfo(GSM_SaveNfg, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg, 0, "nfg", porNFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SaveNfg, 1, "file", porTEXT);

  FuncObj->SetFuncInfo(GSM_SaveNfg_Support, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_SaveNfg_Support, 1, "file", porTEXT);

  gsm->AddFunction(FuncObj);


  //----------------------- Mixed --------------------------//
  FuncObj = new FuncDescObj( "Mixed" );
  FuncObj->SetFuncInfo( GSM_Mixed_NfgFloat, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Mixed_NfgFloat, 
			1, "list", porLIST | porFLOAT,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 2);

  FuncObj->SetFuncInfo( GSM_Mixed_NfgRational, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NfgRational, 0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Mixed_NfgRational, 
			1, "list", porLIST | porRATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 2);

  FuncObj->SetFuncInfo( GSM_Mixed_NFSupport, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NFSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_Mixed_NFSupport, 
			1, "list", porLIST | porFLOAT | porRATIONAL );
  gsm->AddFunction( FuncObj );

  
  //--------------------- SetComponent -------------------------//
  FuncObj = new FuncDescObj( "SetComponent" );

  FuncObj->SetFuncInfo( GSM_SetComponent_MixedFloat, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			0, "mixed", porMIXED_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			1, "player", porPLAYER_NFG );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			2, "list", porLIST | porFLOAT );

  FuncObj->SetFuncInfo( GSM_SetComponent_MixedRational, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			0, "mixed", porMIXED_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			1, "player", porPLAYER_NFG );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			2, "list", porLIST | porRATIONAL );

  gsm->AddFunction( FuncObj );


  //------------------------- MixedSolution member functions ----------//

  //----------------------- IsNash ------------------------//

  FuncObj = new FuncDescObj("IsNash");
  FuncObj->SetFuncInfo(GSM_IsNash_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_IsNash_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsNash_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_IsNash_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsPerfect");
  FuncObj->SetFuncInfo(GSM_IsPerfect_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_IsPerfect_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsPerfect_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_IsPerfect_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsProper");
  FuncObj->SetFuncInfo(GSM_IsProper_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_IsProper_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsProper_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_IsProper_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsntPerfect");
  FuncObj->SetFuncInfo(GSM_IsntPerfect_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_IsntPerfect_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsntPerfect_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_IsntPerfect_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsntProper");
  FuncObj->SetFuncInfo(GSM_IsntProper_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_IsntProper_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsntProper_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_IsntProper_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Support");
  FuncObj->SetFuncInfo(GSM_Support_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_Support_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_Support_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_Support_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapValue");
  FuncObj->SetFuncInfo(GSM_LiapValue_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_MixedFloat, 0, "strategy",
			porMIXED_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);  
  FuncObj->SetFuncInfo(GSM_LiapValue_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_MixedRational, 0, "strategy",
			porMIXED_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  //----------------------- List --------------------------//
  FuncObj = new FuncDescObj( "ListForm" );
  FuncObj->SetFuncInfo( GSM_List_MixedFloat, 1 );
  FuncObj->SetParamInfo(GSM_List_MixedFloat, 
			0, "mixed", porMIXED_FLOAT );
  FuncObj->SetFuncInfo( GSM_List_MixedRational, 1 );
  FuncObj->SetParamInfo(GSM_List_MixedRational, 
			0, "mixed", porMIXED_RATIONAL );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("SetName");
  FuncObj->SetFuncInfo(GSM_SetNameNfg, 2);
  FuncObj->SetParamInfo(GSM_SetNameNfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetNameNfg, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameNfPlayer, 2);
  FuncObj->SetParamInfo(GSM_SetNameNfPlayer, 0, "x", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_SetNameNfPlayer, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameStrategy, 2);
  FuncObj->SetParamInfo(GSM_SetNameStrategy, 0, "x", porSTRATEGY);
  FuncObj->SetParamInfo(GSM_SetNameStrategy, 1, "name", porTEXT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_NameNfPlayer, 1);
  FuncObj->SetParamInfo(GSM_NameNfPlayer, 0, "x", porPLAYER_NFG);

  FuncObj->SetFuncInfo(GSM_NameStrategy, 1);
  FuncObj->SetParamInfo(GSM_NameStrategy, 0, "x", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsConstSum");
  FuncObj->SetFuncInfo(GSM_IsConstSumNfg, 1);
  FuncObj->SetParamInfo(GSM_IsConstSumNfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_NameNfg, 1);
  FuncObj->SetParamInfo(GSM_NameNfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayersNfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayersNfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Players");
  FuncObj->SetFuncInfo(GSM_PlayersNfg, 1);
  FuncObj->SetParamInfo(GSM_PlayersNfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);
}





