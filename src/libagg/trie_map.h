//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/trie_map.h
// Trie with STL-like interfaces
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef __TRIE_MAP_H
#define __TRIE_MAP_H

//Mapping from vector of ints to type V.
//WARNING: traversal using the iterators is in the reverse order of insertion.

#include <math.h>
#include <list>
#include <iterator>

using std::ostream;
using std::endl;
using std::cout;
using std::ostream_iterator;

//forward declarations

template <class V>
class trie_map;

template <class V>
ostream& operator<< (ostream& s, const trie_map<V>& t);



template <class V>
struct TrieNode
{
  //members:
  std::vector<TrieNode*> children;
  typename std::list<std::pair<std::vector<int>,V> >::iterator val;


  //methods:

  //constructor  
  TrieNode(size_t branches, typename std::list<std::pair<std::vector<int>,V> >::iterator v):children(branches,(TrieNode*)NULL),val(v) {}
  

  inline TrieNode*& operator[](size_t i){
    if (i<children.size()) return children[i];
    else {children.resize(i+1,(TrieNode*)NULL); return children[i];}
  }
};

template <class V>
class trie_map {

public:
  //typedefs
  typedef std::vector<int>          key_type;
  typedef std::pair<std::vector<int>, V> value_type;

  typedef V* 		pointer;
  typedef V&		reference;

  typedef unsigned int  size_type;

  typedef typename std::list<value_type>::iterator iterator;
  typedef typename std::list<value_type>::const_iterator const_iterator;


  //friends
  friend ostream& operator<< <V>(ostream& s, const trie_map<V>& t);
  
  friend class agg;

  //constructors
  trie_map():initBranches(1),root(new TrieNode<V>(1,data.end())){}
  trie_map(int branches):initBranches(branches),root(new TrieNode<V>(branches,data.end())){}


  //copy constructor
  trie_map(const trie_map<V>& other);

  //destructor
  virtual ~trie_map(){deleteNodes(root);}

  //assignment
  inline trie_map<V>& operator=( const trie_map<V>& other);

  inline void swap (trie_map<V>& other);

  inline size_type size() const {return data.size();}
  inline bool empty() const {return data.empty();}
  inline iterator begin() {return data.begin();}
  inline iterator end() {return data.end();}
  inline const_iterator begin() const {return data.begin();}
  inline const_iterator end() const {return data.end();}

  //insert: same interface as in STL map
  inline std::pair<iterator,bool> insert (const value_type& x);

  template <class InputIterator>
  inline void insert(InputIterator f, InputIterator l){
    for (InputIterator p=f; p != l; ++p){
	insert( *p);
    }
  }

  //insert or add
  inline trie_map<V>& operator+=(const value_type& x){
    std::pair<typename trie_map<V>::iterator ,bool> r=insert(x);
    if (!r.second) (*r.first).second += x.second;
    return (*this);
  }


  //prefix matching
  //return a reference to iterator
  inline iterator& find (const key_type& k) const __attribute__((always_inline));

  //exact matching
  inline iterator findExact (const key_type& k);

  //clear the tree strucutre as well as data
  inline void clear(){
	deleteNodes(root);
	root=new TrieNode<V> (initBranches, data.end()); 
#ifdef AGGDEBUG
	iterator endp = data.end();
#endif
	data.clear();
#ifdef AGGDEBUG
	if(endp != end()) {std::cerr<<"Error: end() changed"<<endl; exit(1);}
#endif
	leaves.clear();
  }

  //clear data, but keep the tree structure
  inline void reset(){
    while(!leaves.empty()) {

#ifdef AGGDEBUG
      if(!leaves.back()){
	  cout<<"WARNING: NULL pointer in leaves. leaves are:"<<endl;
	  for(size_t i=0;i<leaves.size();++i){ 
	      cout << (long long) leaves[i]<<": ";
	      
	      if (leaves[i])cout<<leaves[i]->val->second;
	      cout<<endl;
	  }
	  cout<<endl<<"trie is:"<< *this<< endl;
	  cout<<"in order:"<<endl;
	  print_in_order();
      }
#endif


      leaves.back()->val=data.end();
      leaves.pop_back();
    }
    data.clear();
  }

  //number of elements with key exactly k.
  // returns 1 or 0 
  inline size_type count(const key_type& k) {
    size_t i=0;
    TrieNode<V>* ptr=root;
    for (;i<k.size()&&k[i]<(int)ptr->children.size()&&  ptr->children[k[i]];ptr=ptr->children[k[i++]]) ;
    
    return (i>=k.size()&&ptr->val!=data.end());
  }

  //in order traversal
  template <class UnaryFunction>
  inline void in_order(UnaryFunction f, bool debug=false){
	in_order_subtree(f, root, debug);
  }
  inline void print_in_order (){in_order(print_helper(end()),true);}

  //polynomial multiplication of t1 and t2, store the result in self
  void multiply (const trie_map<V>& t1,const trie_map<V>& t2,size_t keylen,
	 std::vector<proj_func*>& f)
  {
    static size_t i;
    static std::pair<std::vector<int>, V> v;
    const_iterator p1,p2;
    //assert(this!=&t1 && this != &t2);
    v.first.resize(keylen);
    reset();
    for (p1=t1.begin(); p1!=t1.end(); ++p1)if((*p1).second>(V)0){
      for(p2=t2.begin(); p2!=t2.end(); ++p2)if((*p2).second>(V)0){
	//assert((*p1).first.size()==keylen&& (*p2).first.size()==keylen);
	for (i=0;i<keylen;++i){
	  v.first[i]= (*(f[i])) ((*p1).first[i], (*p2).first[i]);
	}
	v.second = (V)((*p1).second * (*p2).second);
	(*this) += v;
      }//end for(p2
    }//end for(p1 
  }

  //polynomial multiplication
  //Taken from SEM
  //Do simplification when V is a class of symbolic expressions and there is strict independence
  //However, wouldn't it be sufficient to check if projectedStrat is a singleton?
  void multiply_smart (const trie_map<V>& P_k_minus_1,const trie_map<V>& projectedStrat,size_t keylen,
                        std::vector<proj_func*>& f)
        {
                static std::pair<std::vector<int>, V> v;
                v.first.resize(keylen);
                reset();

                for (const_iterator P_c_kminus1=P_k_minus_1.begin(); P_c_kminus1!=P_k_minus_1.end(); ++P_c_kminus1)if((*P_c_kminus1).second>V(0)) {
                        /*
                         take c_{k-1}, and apply the first a_k to it to get an updated
                         configuration c_k.  Then you'd apply every other a_k' to c_{k-1} and
                         see if it equals c_k.  If not, you'd break out of the check and do the
                         usual "for a_k" loop.  If every a_k' leads to c_k, then you can just
                         update Pk[c_k] += P_{k-1}[c_{k-1}]
                         */
                        bool canSimplify = true;
                        int prevConfigObtained[keylen];
                        const_iterator a_k = projectedStrat.begin();
                        for (size_t i=0;i<keylen;++i) {
                                prevConfigObtained[i]= (*(f[i])) ((*P_c_kminus1).first[i], (*a_k).first[i]); // keys
                                v.first[i] = prevConfigObtained[i];
                        }
                        a_k++;
                        for(; a_k!=projectedStrat.end(); a_k++)
                        {
                                if((*a_k).second> V(0)) { // FIXME what if played with prob 0????
                                        for (size_t i=0;i<keylen;++i) {
                                                v.first[i]= (*(f[i])) ((*P_c_kminus1).first[i], (*a_k).first[i]); // keys
                                                if( v.first[i] != prevConfigObtained[i] ) {
                                                        canSimplify = false;
                                                        break;
                                                }
                                        }
                                        if( !canSimplify )break;
                                }
                        }
                        if( canSimplify ){
                                // v.first should already be initialized
                                v.second = (V)(*P_c_kminus1).second;
                                (*this) += v;
                        }else{
                                for(const_iterator a_k=projectedStrat.begin(); a_k!=projectedStrat.end(); ++a_k)if((*a_k).second>V(0)) {
				    //assert((*P_c_kminus1).first.size()==keylen&& (*a_k).first.size()==keylen);
                                        for (size_t i=0;i<keylen;++i) {
                                                v.first[i]= (*(f[i])) ((*P_c_kminus1).first[i], (*a_k).first[i]); // keys
                                        }
                                        v.second = (V)((*P_c_kminus1).second * (*a_k).second); // this is the 'value'
                                        (*this) += v;
                                }//end for(p2
                        }
                }//end for(p1
        }

  //multiply in-place. other should not be the same object as self.
  void multiply (const trie_map<V>& other,size_t keylen, std::vector<proj_func*>& f);

  //squaring
  void square(trie_map<V>& dest, size_t keylen, std::vector<proj_func*>& f) const{
    static std::pair<std::vector<int>, V> v;
    v.first.resize(keylen);
    //assert(this!=&dest);
    dest.reset();
    for (const_iterator p1=begin(); p1!=end(); ++p1)if((*p1).second>(V)0){
      for(const_iterator p2=p1; p2!=end(); ++p2)if((*p2).second>(V)0){
	  //assert((*p1).first.size()==keylen&& (*p2).first.size()==keylen);
        for (size_t i=0;i<keylen;++i){
          v.first[i]= (*(f[i])) ((*p1).first[i], (*p2).first[i]);
        }
        v.second = (V)((*p1).second * (*p2).second);
        if(p1!=p2)v.second *=2;
        dest += v;
      }//end for(p2
    }//end for(p1
  }

  //squaring in-place
  void square(size_t keylen, std::vector<proj_func*>& f){
    static typename std::list<typename trie_map<V>::value_type>::iterator p1,p2;
    static std::pair<std::vector<int>, V> v;
    v.first.resize(keylen);
    std::list<typename trie_map<V>::value_type> data2;
    //data.swap(data2);
    data2=data;
    reset();
    for(p1=data2.begin();p1!=end();++p1)if((*p1).second>(V)0){
      for(p2=p1; p2!=data2.end(); ++p2)if((*p2).second>(V)0){
	  //assert((*p1).first.size()==keylen&& (*p2).first.size()==keylen);
        for (size_t i=0;i<keylen;++i){
          v.first[i]= (*(f[i])) ((*p1).first[i], (*p2).first[i]);
        }
        v.second = (V)((*p1).second * (*p2).second);
        if(p1!=p2)v.second *=2;
        (*this) += v;
      }//end for(p2
    }//end for(p1
  }

  //take power of self using repeated squaring. result stored in dest.
  //this is actually slower than power by straight multiplication, if the # of configurations grow polynomially
  //in the # of players.
  void power_repsq (size_t p, trie_map<V>& dest, size_t keylen, std::vector<proj_func*>& f) const{
    //assert(p>0 && this!=&dest );
    if(p==1){
      dest=*this;
      return;
    }
    if(p<=3){
      //dest.multiply(*this, *this, keylen, f);
      square(dest,keylen,f);
      if(p==3)dest.multiply(*this,keylen,f);
      return;
    }
    //power( p/2, scratch,dest, keylen, f);
    power_repsq( p/2,dest, keylen, f);
    //dest.multiply(scratch,scratch, keylen,f);
    dest.square(keylen,f); //take the square of scratch, store in dest

    if(p%2==1){
      dest.multiply(*this, keylen,f);
    }
  }

  void power(size_t p, trie_map<V> &dest,trie_map<V> &scratch, size_t keylen, std::vector<proj_func*> &f){
    //assert(p>0 && this!=&dest );
    if (p==1) {
      dest = *this;
      return;
    }
    square(dest,keylen,f);
    p-=2;
    if (p>1)scratch=dest;
    while(p>0){
      if(p==1){
        dest.multiply(*this,keylen,f);
        return;
      }
      dest.multiply(scratch,keylen,f);
      p-=2;
    }

  }

  //inner product
  V inner_prod( trie_map<V>& other, V init= (V)(0) ) const{
    V result(init);
    //V th(THRESH);
    for(const_iterator p=begin(); p!=end(); ++p)if((*p).second>(V)0){
	typename trie_map<V>::iterator p2=other.find( (*p).first);
	//assert(p2 != other.end());
	if (p2==other.end()){
	  if(p->second>(V) THRESH){
	    cout<<"inner_prod WARNING: discarding [";
	    copy(p->first.begin(),p->first.end(), ostream_iterator<int>(cout," "));
	    cout<<"] "<<p->second<<endl;
	  }
	} 
	else {
	  result+= (*p).second * (*p2).second;
	}
    }
    return result;
  }

  //first apply the action x, then inner prod
  V inner_prod(const std::vector<int>& x, size_t keylen, std::vector<proj_func*>& f,
	trie_map<V>& other, V init=(V)(0) ) const
  { 
    V result(init);
    V th(THRESH);
    static iterator p2;
    //V s(-1);
    for (const_iterator p=begin(); p!=end();++p)if((*p).second>(V)0){
      value_type y= *p;
      //assert(y.first.size()==keylen);
      for (size_t i=0; i<keylen;++i){
	y.first[i] = (*(f[i])) (y.first[i],x[i]); 
      }
      //s += y.second;

      p2 = other.find(y.first);
      //assert(p2!= other.end());
      if (p2==other.end()){
        if(y.second>th){
	  cout<<"inner_prod WARNING: discarding [";
	  copy(y.first.begin(),y.first.end(), ostream_iterator<int>(cout," "));
	  cout<<"] "<<y.second<<endl;
        }
      }
      else{
        result += y.second  * (*p2).second;
      }
    }

//    if (s>THRESH ||s<-THRESH){
//	cout<<"inner_prod WARNING: sum differ from 1.0: diff is"<<s<<endl;
//    } 
    return result;
  }

  
  //polynomial division
  inline trie_map<V>& operator/= (const std::vector<V>& denom);

  
private:
  //member variables:
  std::list<value_type> data;
  size_type initBranches; //default branching factor
  TrieNode<V> *root;

  std::vector<TrieNode<V>*> leaves;

  static const double  THRESH;
  
  
  struct print_helper {
   print_helper (typename trie_map<V>::iterator en):endp(en){}
   void operator()(typename trie_map<V>::iterator p){
    if (p== endp) {cout<< "leaf with no data"<<endl; return;}
    cout<<"[";
    copy(p->first.begin(),p->first.end(),ostream_iterator<int>(cout," "));
    cout <<"]: "<<p->second<<endl;
   }
   typename trie_map<V>::iterator endp;
  };



  //private methods:
  //div: helper for operator/=()
  void  div(const std::vector<V>& denom, TrieNode<V>* n, int current, int pivot);



  __attribute__((noinline)) void deleteNodes(TrieNode<V> *n){
	if (!n) return;
	size_type i,s=n->children.size();
	for (i=0;i<s;++i) if (n->children[i]) {
	  deleteNodes (n->children[i]);
	}
	delete n;
  }


  /*
  __attribute__((noinline)) void remove (TrieNode<V> *n){
	if (!n) return;
	size_type i, s=n->children.size();
	for (i=0;i<s;++i) if (n->children[i]){
	  remove(n->children[i]);
	}

	//if (n->val!=data.end()) {
        //    n->val->second=0;
        //    n->val= data.end();
        //}
	//else {
        //    delete n;
        //}
  }
  */


  //in order traversal
  template <class UnaryFunction>
  __attribute__((noinline)) void in_order_subtree(UnaryFunction& f, TrieNode<V> *n, bool debug=false)
  {
    if (n==NULL) return;
    size_type i,s=n->children.size();
    bool is_leaf=true;
    for (i=0;i<s;++i)if (n->children[i]){
	is_leaf=false;
	if (debug) cout<< i<<" ";
	in_order_subtree(f, n->children[i], debug);
    }
    if(is_leaf) {
	if(debug) cout<< "(leaf): ";
	f(n->val);
    }
    return;
  }    



  //private helper functor classes:

  struct div_helper :public std::unary_function<void, iterator>{
    div_helper(const std::vector<V>& den, int piv,iterator en)
	:pivot(piv), denom(den),endp(en){}
    inline void operator()(iterator p){
	if (p==endp) return;
	p->first[pivot]--;
	p->second /= denom[pivot];
    } 
    int pivot;
    const std::vector<V>& denom;
    iterator endp;
  };

  struct div_helper_mul: public std::unary_function<void,iterator>{
    div_helper_mul(const std::vector<V>& den, int piv, TrieNode<V>* des,iterator en)
	:pivot(piv),denom(den), dest(des),endp(en) {}

    void add(const std::vector<int>& conf, V y){
      size_t i,keylen=conf.size();
      double th(THRESH/(double)denom[pivot]);
      //if (y<=th&&y>=-th) return;

      if (!dest){
	if((double) y>th||(double)y<-th)
	  cout<<"division (pivot=" << denom[pivot]<<") WARNING: discarding "
	    <<y<<endl;
        return;
      }
      TrieNode<V>* ptr=dest;
      for (i=pivot+1; i<keylen; ++i){
	if( (*ptr)[conf[i]]==NULL){
	    if((double)y>th||(double)y<-th)
	      cout<<"division (pivot=" << denom[pivot]<<") WARNING: discarding "
	        <<y<<endl;
	    return;
	}
	ptr = (*ptr)[conf[i]];
      }
      ptr->val->second += y;
      //assert(ptr->val->second>-THRESH);
      if ((double)ptr->val->second <= -th)
	cout<<"division (pivot=" << denom[pivot]<<") WARNING: discarding "
	    <<ptr->val->second<<endl;
      if (ptr->val->second <(V)0) ptr->val->second=0;
    }
    void operator()(iterator p){
      if (p==endp) return;
      size_t i, keylen = p->first.size();
      //assert(keylen==denom.size());
      V null_prob(((V)1)-denom[pivot]);
      //V th(THRESH);
      for (i=pivot+1; i<keylen; ++i)if(denom[i]>(V)0){
	p->first[i]++;
	add(p->first, -denom[i]* p->second);
	p->first[i]--;
	null_prob-= denom[i];
      }
      
      if (null_prob>(V)0)
        add(p->first, -null_prob * p->second);
    }

    
    int pivot;
    const std::vector<V>& denom;
    TrieNode<V>* dest;
    iterator endp;
  };  //end struct div_helper_mul

};

template <class V> const double trie_map<V>::THRESH = 1e-12;


template <class V>
inline ostream& operator<< (ostream& s, const trie_map<V>& t)
{
  //s<<endl;
  for (typename trie_map<V>::const_iterator p=t.begin();p!=t.end();++p){
    s<<"[ ";
    copy(p->first.begin(),p->first.end(),ostream_iterator<int>(s, " "));
    s<< "] "<<p->second<<endl;
  }
  return s;
}



#include "trie_map.imp"
#endif
