//#
//# FILE: gpvector.h -- profile vector class
//#
//# $Id$
//#

#ifndef GPVECTOR_H
#define GPVECTOR_H

#include "gvector.h"

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type.
#endif   // __GNUC__, __BORLANDC__


template <class T> class gPVector: public gVector<T> {
 private:
  int sum( gTuple<int> &V )
    {
      int total= 0;
      for(int i= V.First(); i<=V.Last(); i++)
	total+= V[i];
      return total;
    }

  int setindex()
    {
      int index= First();
      for(int i=svlen.First(); i<=svlen.Last(); i++)
	{
	  svptr[i]= data+index-1;
	  index+= svlen[i];
	}
      assert( index==Last()+1 );
    }

 protected:
  gTuple<T *> svptr;
  gTuple<int> svlen;

  int Check(const gPVector<T> &v) const
    {
      return( v.min==min && v.max==max && v.svlen==svlen );
    }

 public:

  // constructors

  gPVector(void)
    : gVector<T>(), svptr(), svlen()
    { }

  gPVector(const gTuple<int> &sig)
    : gVector<T>(sum(sig)), svptr(sig.First(),sig.Last()), svlen(sig)
    {
      setindex();
    }

  gPVector(const gVector<T> &val, const gTuple<int> &sig)
    : gVector<T>( val ), svptr(sig.First(),sig.Last()), svlen(sig)
    {
      assert( sum(svlen)==val.Length() );
      setindex();
    }

  gPVector(const gPVector<T> &v)
    : gVector<T>(v), svptr(v.svlen.First(),v.svlen.Last()), svlen(v.svlen)
    {
      setindex();
    }

  virtual ~gPVector();


  // element access operators
  T& operator()(int a, int b)
    {
      assert( svlen.First()<=a && a<=svlen.Last() );
      assert( 1<=b && b<=svlen[a] );

      return svptr[a][b];
    }

  const T& operator()(int a, int b) const
    {
      assert( svlen.First()<=a && a<=svlen.Last() );
      assert( 1<=b && b<=svlen[a] );

      return svptr[a][b];
    }

  // extract a subvector
  gVector<T> GetRow(int row) const
    {
      assert( svlen.First()<=row && row<=svlen.Last() );
      gVector<T> v(1, svlen[row]);

      for(int i=v.First(); i<=v.Last(); i++)
	v[i]= (*this)(row,i);
      return v;
    }

  void GetRow(int row, gVector<T> &v) const
    {
      assert( svlen.First()<=row && row<=svlen.Last() );
      assert( v.First()==1 && v.Last()== svlen[row] );

      for(int i=v.First(); i<=v.Last(); i++)
	v[i]= (*this)(row,i);
    }

  void SetRow(int row, const gVector<T> &v)
    {
      assert( svlen.First()<=row && row<=svlen.Last() );
      assert( v.First()==1 && v.Last()== svlen[row] );

      for(int i=v.First(); i<=v.Last(); i++)
	(*this)(row,i)= v[i];
    }


  // more operators

  gPVector<T>& operator=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      gVector<T>::operator=(v);
      return (*this);
    }
  gPVector<T>& operator=(const gVector<T> &v)
    {
      gVector<T>::operator=(v);
      return (*this);
    }
  gPVector<T>& operator=(T c)
    {
      gVector<T>::operator=(c);
      return (*this);
    }

  gPVector<T> operator+(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      gPVector<T> tmp(*this);
      tmp.gVector<T>::operator+=(v);
      return tmp;
    }
  gPVector<T>& operator+=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      gVector<T>::operator+=(v);
      return (*this);
    }
  gPVector<T> operator-(void) const
    {
      gPVector<T> tmp(*this);
      for(int i=First(); i<=Last(); i++)
	tmp[i]= -tmp[i];
      return tmp;
    }
  gPVector<T> operator-(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      gPVector<T> tmp(*this);
      tmp.gVector<T>::operator-=(v);
      return tmp;
    }
  gPVector<T>& operator-=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      gVector<T>::operator-=(v);
      return (*this);
    }
  T operator*(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator*(v);
    }
  gPVector<T>& operator*=(const T c)
    {
      gVector<T>::operator*=(c);
      return (*this);
    }
  gPVector<T> operator/(T c)
    {
      gPVector<T> tmp(*this);
      tmp= tmp.gVector<T>::operator/(c);
      return tmp;
    }
  gPVector<T> operator/(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      gPVector<T> tmp(*this);
      tmp.gVector<T>::operator/=(v);
      return tmp;
    }
  gPVector<T>& operator/=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      gVector<T>::operator/=(v);
      return (*this);
    }

  int operator==(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator==(v);
    }
  int operator!=(const gPVector<T> &v)
    { return ! ((*this)==v); }

  // parameter access functions
  const gTuple<int>& Lengths()
    { return svlen; }

  void Dump(gOutput &) const;
};


// virtual destructor
template <class T>
gPVector<T>::~gPVector()
{ }

// method implementations

// stream output

template <class T> gOutput &
operator<<(gOutput &to, const gPVector<T> &v)
{
  v.Dump(to); return to;
}

template <class T> void
gPVector<T>::Dump(gOutput &to) const
{
  for(int i=svlen.First(); i<=svlen.Last(); i++)
    {
      to<<"{ ";
      for(int j=1; j<=svlen[i]; j++)
	to<<(*this)(i,j)<<" ";
      to<<"}";
    }
}

#endif   //# GPVECTOR_H


