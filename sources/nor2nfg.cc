//#
//# FILE: nor2nfg.cc -- Simple .nor to .nfg conversion program
//#
//# $Id$
//#
//# To compile:  g++ -fno-implicit-templates nor2nfg.cc gambitio.cc -o nor2nfg
//# Usage:    nor2nfg < infile > outfile
//#

#include "garray.h"



//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
#ifdef __BORLANDC__
#define GHUGE	huge
#else
#define GHUGE
#endif
    T GHUGE *storage;
    gArray<int> dim;

    void ReadFrom(gInput &, const gArray<int> &, gArray<int> &, int);

  public:
    gNArray(const gArray<int> &d);
    ~gNArray();

    T &operator[](const gArray<int> &);

    void Input(gInput &, const gArray<int> &, int);
    void Output(gOutput &) const;
};

template <class T> gNArray<T>::gNArray(const gArray<int> &d)
  : dim(d)
{
  if (dim.Length() <= 0)   {
    storage = 0;
    storage_size = 0;
  }
  else   {
    assert(dim.First() == 1);
    storage_size = 1;
    
    for (int i = 1; i <= dim.Length(); i++)   {
      assert(dim[i] >= 1);
      storage_size *= dim[i];
    }

    storage = new GHUGE T[storage_size];
    for (i = 0; i < storage_size; storage[i++] = 0);
  }
}

template <class T> gNArray<T>::~gNArray()
{
  if (storage)    delete [] storage;
}

template <class T> T &gNArray<T>::operator[](const gArray<int> &v)
{
  assert(dim.Length() > 0 && dim.Length() == v.Length());
  
  for (int i = 1, location = 0, offset = 1; i <= dim.Length(); i++)   {
    assert(v[i] > 0 && v[i] <= dim[i]);
    location += (v[i] - 1) * offset;
    offset *= dim[i];
  }

  return storage[location];
}

template <class T> void gNArray<T>::Output(gOutput &f) const
{
  f << "NFG 1 D \"\" {";
  for (int i = 1; i < dim.Length(); i++)
    f << " \"\"";
  f << " } {";
  for (i = 1; i < dim.Length(); i++)
    f << ' ' << dim[i];
  f << " }\n\n";

  long ncont = storage_size / dim[dim.Last()];

  for (long l = 0; l < ncont; l++)
    for (int pl = 1; pl <= dim[dim.Last()]; pl++)
      f << storage[(pl - 1) * ncont + l] << ' ';

  f << '\n';
}

template <class T> 
void gNArray<T>::Input(gInput &f, const gArray<int> &norder, int i)
{
  gArray<int> strat(i);
  ReadFrom(f, norder, strat, i);
}

template <class T> 
void gNArray<T>::ReadFrom(gInput &f, const gArray<int> &norder,
			  gArray<int> &strat, int i)
{
  for (int j = 1; j <= dim[norder[i]]; j++)   {
    strat[norder[i]] = j;
    if (i > 1)
      ReadFrom(f, norder, strat, i - 1);
    else 
      f >> (*this)[strat];
  }
}

main()
{
  int nplayers;
  gin >> nplayers;

  gArray<int> size(nplayers);
  long outvect_offset = 1L;
  for (int i = 1; i <= nplayers; i++)  {
    gin >> size[i];
    outvect_offset *= size[i];
  }

  gArray<int> norder(nplayers + 1);
  for (i = 1; i <= nplayers + 1; gin >> norder[i++]);

  gArray<int> foo(size.Length() + 1);
  for (i = 1; i <= size.Length(); i++)
    foo[i] = size[i];
  foo[foo.Last()] = nplayers;
  gNArray<double> *payoffs = new gNArray<double>(foo);

  payoffs->Input(gin, norder, nplayers + 1);

  payoffs->Output(gout);

  delete payoffs;
}




#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__

#include "garray.imp"

TEMPLATE class gArray<int>;

TEMPLATE class gNArray<double>;
