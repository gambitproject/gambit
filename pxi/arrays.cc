#include "arrays.h"

#ifdef __GNUC__
	#define INLINE inline
#elif defined(__BORLANDC__)
	#define INLINE
#else
	#error Unsupported compiler type.
#endif   // __GNUC__, __BORLANDC__

/*************************** ONE DIMENTIONAL ARRAY *************************/
template <class T>  INLINE
Array<T>::Array(const Array<T> &A)
{
if (this!=&A)		// beware of a=a
{
	if (A.dim>0)
	{
		dim=A.dim;
		data=new T[dim];
		for (int i=0;i<dim;i++) data[i]=A[i];
	}
	else
		data=NULL;
}
}

template <class T>  INLINE
Array<T> &Array<T>::operator=(const Array<T> &A)
{
if (this!=&A)		// beware of a=a
{
	if (data) delete [] data;
	if (A.dim>0)
	{
		dim=A.dim;
		data=new T[dim];
		for (int i=0;i<dim;i++) data[i]=A[i];
	}
	else
		data=NULL;
}
return (*this);
}

//template <class T>
ostream &operator<<(ostream& os,const Array<double> &A)
{
	os<<'(';
	for (int i=0;i<A.dim;i++)
		os<< (A[i]) <<',';
	os<<')';
	return os;
}

/*************************** TWO DIMENTIONAL ARRAY *************************/
template <class T> INLINE
TwoDArray<T>::TwoDArray(int _rows,int _cols)
{
t_array=NULL;
rows=_rows;cols=_cols;
data=new T *[rows];
for (int i=0;i<rows;i++) data[i]=new T[cols];
}

template <class T>  INLINE
Array<T> &TwoDArray<T>::operator[](int row)
{
assert(row>=0 && row<rows);
if (t_array) delete t_array;
t_array=new Array<T>(cols);
for (int i=0;i<cols;i++) (*t_array)[i]=data[row][i];
return (*t_array);
}



