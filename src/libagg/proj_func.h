//proj_func: pre-defined functions for the contribution-independent function nodes

#ifndef __PROJ_FUNC_H
#define __PROJ_FUNC_H
#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include <cstdlib>

using namespace std;

//types of contribution-independent function:
//sum, existence, highest, lowest and their extended versions
typedef enum{
  P_SUM=0,
  P_EXIST=1,
  P_HIGH=2,
  P_LOW=3,
  P_SUM2=10,
  P_EXIST2=11,
  P_HIGH2=12,
  P_LOW2=13
} TypeEnum;

//proj_func: contribution-independent function
struct proj_func{
        TypeEnum Type;
	int Default;      //default value
	vector<int> weights;
	proj_func(TypeEnum tp,int def,vector<int>& wts):Type(tp),Default(def),weights(wts) {}
	proj_func(TypeEnum tp,int def): Type(tp),Default(def) {}
	proj_func(TypeEnum tp,istream& in,int S) : Type(tp){
	      in>>Default;
	      if(in.eof()||in.bad()) {
	                        cout << "proj_func() error: bad input.\n";
	                        exit(1);
	      }
	      int w;
	      char c;
	      in >>ws>>c;
	      if (in.eof()||in.bad()||c!='['){
	        cout<<"proj_func() error: [ expected.\n";
	        exit(1);
	      }
	      for (int i=0;i<S;i++) {
	        if(in.eof()||in.bad()) {
	                cout << "proj_func() error: bad input.\n";
	                exit(1);
	        }
	        in >> w;
	        weights.push_back(w);
	      }
	      in>>ws>>c;
	      if(in.eof()||in.bad()||c!=']'){
	        cout<<"proj_func() error: ] expected.\n";
	        exit(1);
	      }
	}
	virtual ~proj_func() {}
	inline bool operator==(const proj_func &v) const {
	  return Type==v.Type && Default==v.Default && weights==v.weights;
	}
	inline bool operator!=(const proj_func &v) const {
	  return Type!=v.Type || Default!=v.Default || weights !=v.weights;
	}
	virtual int operator()(int x,int y)   =0;
	virtual int operator()(multiset<int>& s) =0;
	virtual void print (ostream& out){
	  out<< Type;
	  out<<" "<<Default<<" ";
	  copy(weights.begin(),weights.end(),ostream_iterator<int>(out," "));
	  cout<<endl;
	}
};

struct proj_func_SUM: public proj_func {
    proj_func_SUM():proj_func(P_SUM,0) {}
    inline int operator() (int x,int y){return x+y;}
    inline int operator()(multiset<int>& s){return s.size();}
    void print (ostream& out){ out<<P_SUM<<endl;}
};

struct proj_func_SUM2: public proj_func{
    proj_func_SUM2(istream& in, int S):proj_func(P_SUM2,in,S){ }
    inline int operator()(int x, int y){return x+y;}
    inline int operator()(multiset<int>& s){
      int res=Default;
      for(multiset<int>::iterator it=s.begin();it!=s.end();it++){
        res+=weights.at(*it);
      }
      return res;
    }
};
struct proj_func_EXIST: public proj_func{
    proj_func_EXIST() :proj_func(P_EXIST,0) {}
    inline int operator() (int x, int y) {return (x+y>0);}
    inline int operator() (multiset<int>& s){ return (s.size()>0);}
    void print(ostream& out){out<<P_EXIST<<endl;}
};
struct proj_func_EXIST2: public proj_func{
    proj_func_EXIST2(istream& in, int S):proj_func(P_EXIST2,in,S){
      if (Default<0){
        cout<<"proj_func_EXIST2() error: default value should be nonnegative.\n";
        exit(1);
      }
      for (size_t i=0;i<weights.size();i++){
        if(weights[i]<0){
          cout<<"proj_func_EXIST2() error: weights should be nonnegative.\n";
          exit(1);
        }
      }
    }
    inline int operator()(int x, int y){return (x+y>0);}
    inline int operator()(multiset<int>& s){
      int res=Default;
      for(multiset<int>::iterator it=s.begin();it!=s.end();it++){
        res+=weights.at(*it);
      }
      return (res>0);
    }
};
struct proj_func_HIGH:public proj_func{
    proj_func_HIGH(int def): proj_func(P_HIGH,def) {}
    inline int operator()(int x, int y){
	if(x==Default) return y;
	if (y==Default)return x;
	return ( (x>y)?x:y );
    }
    inline int operator()(multiset<int>& s){
	if (s.empty()) return Default;
	return *(s.rbegin());
    }
    void print(ostream& out){out<<P_HIGH<<endl;}
};
struct proj_func_HIGH2: public proj_func{
    proj_func_HIGH2(istream& in, int S):proj_func(P_HIGH2,in,S){ }
    inline int operator()(int x, int y){
      if(x==Default) return y;
      if (y==Default)return x;
      return ( (x>y)?x:y );
    }
    inline int operator()(multiset<int>& s){
      if (s.empty()) return Default;
      multiset<int>::iterator it=s.begin();
      int res=weights.at(*it);
      for(;it!=s.end();it++){
        res=(res>weights.at(*it))?res:weights[*it];
      }
      return res;
    }
};
struct proj_func_LOW: public proj_func{
    proj_func_LOW(int def): proj_func(P_LOW,def) {}
    inline int operator()(int x, int y){
	if (x==Default) return y;
	if (y==Default )return x;
	return ( (x<y)?x:y);
    }
    inline int operator()(multiset<int>& s){
	if (s.empty() ) return Default;
	return *(s.begin());
    }
    void print(ostream& out){out<<P_LOW <<endl;}
};	
struct proj_func_LOW2: public proj_func{
    proj_func_LOW2(istream& in, int S):proj_func(P_LOW2,in,S){ }
    inline int operator()(int x, int y){
      if(x==Default) return y;
      if (y==Default)return x;
      return ( (x<y)?x:y );
    }
    inline int operator()(multiset<int>& s){
      if (s.empty()) return Default;
      multiset<int>::iterator it=s.begin();
      int res=weights.at(*it);
      for(;it!=s.end();it++){
        res=(res<weights.at(*it))?res:weights[*it];
      }
      return res;
    }
};

typedef proj_func* projtype;


inline proj_func* make_proj_func(TypeEnum type, istream& in,int S,int P){
  switch(type){
	case P_SUM: return (new proj_func_SUM);
	case P_EXIST: return (new proj_func_EXIST);
	case P_HIGH:  return (new proj_func_HIGH(S+P));
	case P_LOW:     return (new proj_func_LOW(S+P));
	case P_SUM2: return (new proj_func_SUM2(in,S));
	case P_EXIST2: return (new proj_func_EXIST2(in,S));
	case P_HIGH2: return (new proj_func_HIGH2(in,S));
	case P_LOW2: return (new proj_func_LOW2(in,S));
	default:
	  cout<<"error: function type is not recognized."<<endl;
	  return NULL;
  }
}

#endif
