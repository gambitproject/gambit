#ifndef __GRAYCOMPOSITION_H
#define __GRAYCOMPOSITION_H

#include <vector>
using namespace std;

class GrayComposition {
friend class agg;
public:
  GrayComposition(int _n, int _k) :n(_n),k(_k),p(0),i(-1),d(-1),finished(false), current(k,0){
    current.at(0)=n;
  }

  bool eof() { return finished; }

  const vector<int>& get() { return current; }

  void incr() {
    int b=-1;
    if (finished) return;
    if (current[k-1]==n){
	finished=true;
	return;
    }
    if (p==0){
      for (b=1; b<k; ++b){
	if (current[b]>0) break;
      }
      if(b<k&& b==1){
	if(d==0 && current[0]==1) p=1;
      }
      else if ( (n-current[0])%2==0) {
	d=0;
	i=1;
	p=1;
      }
      else if (b<k&& current[b]%2==1) {
        d=0;
	i=b;
	p=b;
      }
      else {
	i=0;
	d=b;
      }
    } 
    else {
      if ( (n-current[p])%2==1 ){
        d=p;
	i=p-1;
	if (current[p]%2==0) i=0;
	p=i;
      }
      else if (current[p+1]%2==0){
	i=p+1;
	d=p;
	if(current[p]==1) p=p+1;
      }
      else{
	i=p;
	d=p+1;
      }
    }
    current[i]+=1;
    current[d]-=1;
    if(current[0]>0) p=0;
    return;
  }

private:
  int n,k;
  int p; // idx to first positive
  int i,d;
  bool finished;
  vector<int> current;

};


#endif
