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
  int sum( gTuple<int> V )
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
    : gVector(), svptr(), svlen()
    { }

  gPVector(const gTuple<int> sig)
    : gVector(sum(sig)), svptr(sig.First(),sig.Last()), svlen(sig)
    {
      setindex();
    }

  gPVector(const gVector<T> val, const gTuple<int> sig)
    : gVector( val ), svptr(sig.First(),sig.Last()), svlen(sig)
    {
      assert( sum(svlen)==Length(val) );
      setindex();
    }

  virtual ~gPVector()
    { }


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

      for(int i=v.First(); i<=v.Last; i++)
	v[i]= (*this)(row,i);
      return v;
    }

  void GetRow(int row, gVector<T>&v) const
    {
      assert( svlen.First()<=row && row<=svlen.Last() );
      assert( v.First()==1 && v.Last()== svlen[row] );

      for(int i=v.First(); i<=v.Last(); i++)
	v[i]= (*this)(row,i);
    }

  void SetRow(int row, const gVector<T>&)
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
      return (*this).gVector<T>::operator=(v);
    }

  gPVector<T> operator+(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator+(v);
    }
  gPVector<T>& operator+=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator+=(v);
    }
  gPVector<T> operator-(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator-(v);
    }
  gPVector<T>& operator-=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator-=(v);
    }
  T operator*(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator*(v);
    }
  gPVector<T> operator/(const gPVector<T> &v) const
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator/(v);
    }
  gPVector<T>& operator/=(const gPVector<T> &v)
    {
      assert( svlen==v.svlen );
      return (*this).gVector<T>::operator/=(v);
    }

  int operator==()
  int operator!=()

  // parameter access functions
  gTuple<int> Lengths()
    { return svlen; }

  void Dump(gOutput &) const;
}


// method implementations

// stream output

template <class T> gOutput &
operator<<(gOutput &to, const gPVector<T> &v)
{
  v.Dump(to); return to;
}

template <class T> void
gPVector::Dump(gOutput &to) const
{
  for(int i=svlen.First(); i<=svlen.Last(); i++)
    {
      to<<"{ ";
      for(int j=1; j<=svlen[i]; j++)
	to<<(*this)(i,j)<<" ";
      to<<"}\n";
    }
}

#endif   //# GPVECTOR_H


