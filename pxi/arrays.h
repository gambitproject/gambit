#ifndef	ARRAYS_H
#define ARRAYS_H
#include <assert.h>
#include <iostream.h>
// ******************************** ONE DIMENTIONAL ARRAY ******************
template<class T>
class Array
{
private:
	int dim;
	T *data;
public:
	// Constructors
	Array(void) {data=NULL;dim=0;}
	Array(int _dim) {assert(_dim>0);dim=_dim;data=new T[dim];assert(data);}
	Array(const Array<T> &A);//{dim=A.dim;data=new T[dim];assert(data);};
	// Destructor
	~Array(void)	{if (data) delete [] data;}
	// Data access
	T& operator[](int i) {assert(i>=0 && i<dim);return data[i];}
	T operator[](int i) const {assert(i>=0 && i<dim);return data[i];}
	Array<T> &operator=(const Array<T> &A);
	// Output operator
	friend ostream &operator<<(ostream& os,const Array<T> &A);
};

// ******************************** TWO DIMENTIONAL ARRAY ******************
template <class T>
class TwoDArray
{
private:
	Array< Array<T> > data;
  Array<T>	*t_array;
	int	rows,cols;
public:
	// Constructor
	TwoDArray(int _rows,int _cols);
  TwoDArray(const TwoDArray<T> &A);
	// Data Access
	T& operator()(int i,int j) {assert(i>=0 && i<rows && j>=0 && j<cols);return data[i][j]; }
	T operator()(int i,int j) const {assert(i>=0 && i<rows && j>=0 && j<cols);return data[i][j];}
	Array<T> &operator[](int row)
	{
	assert(row>=0 && row<rows);
	if (t_array) delete t_array;
	t_array=new Array<T>(cols);
	for (int i=0;i<cols;i++) (*t_array)[i]=data[row][i];
	return (*t_array);
	}
};
/*************************** TWO DIMENTIONAL ARRAY *************************/
template <class T>
TwoDArray<T>::TwoDArray(int _rows,int _cols)
{
rows=_rows;cols=_cols;
data=Array< Array<T> >(rows);
assert(data);
for (int i=0;i<rows;i++) {assert(data[i]);data[i]=new T[cols];}
}
template <class T>
TwoDArray<T>::TwoDArray(const TwoDArray<T> &A)
{
rows=A.rows;cols=A.cols;
data=Array< Array<T> >(rows);
assert(data);
for (int i=0;i<rows;i++) {assert(data[i]);data[i]=new T[cols];}
}

#endif