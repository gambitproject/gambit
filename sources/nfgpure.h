//
//#FILE purenash.h
//

class NashSolver{
 private:
  const BaseNormalForm &nf;
  /*************/
 public:
  NashSolver(const BaseNormalForm &N):nf(N){ }
  ~NashSolver() { }
  
  int Solve(void); /*calls the procedure for calcualtinf Nash equilibria*/
};

/*********************************************/

template <class T> class PureNashModule {
 private:
 
  const NormalForm<T> &rep;
  gBlock<long> solution; /**index of the nash equilibrium*****/
  gBlock<StrategyProfile> sol;/*solution for the nash equilibria in SP form*/
  gBlock<int> Num_strats;/***number strategys fot each player***/
 
  gBlock<long> sindex; /***corresponding index for the StrategyProfile***/
  int players; /*number of players**/

/********************/
 public: 
  PureNashModule(const NormalForm<T> &r);
  virtual ~PureNashModule(void);
  void SingleNash(long i);/*checks if the given SP in a Nash equilibria*/
  int Reverse (long index, int player);/*take index and the player number and 
					 returns number of the strategy*/
  int PureNash(void); /*takes the SP and return index*/
  int Number_Nash(){return(solution.Length());}/*return the number of NashEq*/
 
};




