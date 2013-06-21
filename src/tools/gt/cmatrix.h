/* Copyright 2002 Ben Blum, Christian Shelton
 * 
 * This file is part of GameTracer.
 *
 * GameTracer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GameTracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GameTracer; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _CMATRIX_H_
#define _CMATRIX_H_

#include <cmath>
#ifdef SOLARIS
#include <ieeefp.h>
#endif
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iomanip>
#include <vector>

using namespace std;
class cmatrix;

class cmatrixrow {
public:
	double operator[](int) const {
		return 0.0;
	}
};

class cvector {
friend class cmatrix;
public:
 static int num_vec_cons; 
	inline cvector() {
	  cvector::num_vec_cons++;
		m = 1;
		x = new double[1];
	}
	inline cvector(int m) {
	  cvector::num_vec_cons++;
		this->m = m;
		x = new double[m];
	}
	~cvector(); 
	inline cvector(const cvector &v) {
	  cvector::num_vec_cons++;
		m = v.m;
		x = new double[m];
		//for(int i=0;i<m;i++) x[i] = v.x[i];
		memcpy(x,v.x,m*sizeof(double));
	}
	inline cvector(int m, const double &a) {
	  cvector::num_vec_cons++;
		this->m = m;
		x = new double[m];
		for(int i=0;i<m;i++) x[i] = a;
	}
	inline cvector(double *v, int m, bool keep=false) {
	  cvector::num_vec_cons++;
		this->m = m;
		if (keep) x = v;
		else {
			x = new double[m];
			//for(int i=0;i<m;i++) x[i] = v[i];
			memcpy(x,v,m*sizeof(double));
		}
	}
	inline cvector operator-() const {
		cvector ret(m);
		for(int i=0;i<m;i++) ret.x[i] = -x[i];
		return ret;
	}
	inline cvector& operator=(double a) {
		for(int i=0;i<m;i++) x[i] = a;
		return *this;
	}
	inline cvector& operator=(const cvector &v) {
		if (&v==this) return *this;
		if (v.m != m) {
			delete []x;
			m = v.m;
			x = new double[m];
		}
		//for(int i=0;i<m;i++) x[i] = v.x[i];
		memcpy(x,v.x,m*sizeof(double));
		return *this;
	}
	inline bool isvalid() const {
		for(int i=0;i<m;i++) if (!finite(x[i])) return false;
		return true;
	}
	inline double operator*(const cvector &v) const {
		if (m!=v.m) {
			cerr << "invalid cvector dot product" << endl;
			//assert(0);
		}
		double ret = 0.0;
		for(int i=0;i<m;i++) ret += x[i]*v.x[i];
		return ret;
	}
	inline double operator*(const double *v) const {
		double ret = 0.0;
		for(int i=0;i<m;i++) ret += x[i]*v[i];
		return ret;
	}
	inline cvector outer(const cvector &v) const {
		cvector ret(m*v.m);
		for(int i=0,c=0;i<m;i++)
			for(int j=0;j<v.m;j++,c++)
				ret.x[c] = x[i]*v.x[j];
		return ret;
	}
	inline double operator[](int i) const {
		return x[i];
	}
	inline double &operator[](int i) {
		return x[i];
	}
	inline cvector &operator+=(const cvector &v) {
		if (v.m!=m) {
			cerr << "invalid cvector addition" << endl;
			//assert(0);
		}
		for(int i=0;i<m;i++) x[i] += v.x[i];
		return *this;
	}
	inline cvector &operator-=(const cvector &v) {
		if (v.m!=m) {
			cerr << "invalid cvector subtraction" << endl;
			//assert(0);
		}
		for(int i=0;i<m;i++) x[i] -= v.x[i];
		return *this;
	}
	inline cvector &operator*=(const double &a) {
		for(int i=0;i<m;i++) x[i] *= a;
		return *this;
	}
	inline cvector &operator+=(const double &a) {
		for(int i=0;i<m;i++) x[i] += a;
		return *this;
	}
	inline cvector &operator-=(const double &a) {
		for(int i=0;i<m;i++) x[i] -= a;
		return *this;
	}
	inline cvector &operator/=(const double &a) {
		for(int i=0;i<m;i++) x[i] /= a;
		return *this;
	}
	inline double max() const {
		double t,ma = x[0];
		for(int i=1;i<m;i++)
			if((t=x[i])>ma) ma = t;
		return ma;
	}
	inline double min() const {
		double t,mi = x[0];
		for(int i=1;i<m;i++)
			if ((t=x[i])<mi) mi = t;
		return mi;
	}
	inline double absmax() const {
		double t,ma = x[0]>0?x[0]:-x[0];
		for(int i=1;i<m;i++)
			if ((t=(x[i]>0?x[i]:-x[i]))>ma) ma = t;
		return ma;
	}
	inline double absmin() const {
		double t,mi = x[0]>0?x[0]:-x[0];
		for(int i=1;i<m;i++)
			if ((t=(x[i]>0?x[i]:-x[i]))<mi) mi = t;
		return mi;
	}
	inline double normalize() {
		double norm = 0.0;
		for(int i = 0; i < m; i++)
			norm += x[i] * x[i];
		norm = sqrt(norm);
		for(int i = 0; i < m; i++)
			x[i] /= norm;
		return norm;
	}
	inline bool operator==(const cvector &v) const {
		if (m!=v.m) return false;
#if !defined(HAVE_BCMP)
		for(int i=0;i<m;i++) if (v.x[i]!=x[i]) return false;
		return true;
#else
		return bcmp(x,v.x,m*sizeof(double))==0;
#endif // HAVE_BCMP
	}
	inline bool IsEqual(cvector *v) const {
		if (m!=v->m) return false;
#if !defined(HAVE_BCMP)
		for(int i=0;i<m;i++) if (v->x[i]!=x[i]) return false;
		return true;
#else
		return bcmp(x,v->x,m*sizeof(double))==0;
#endif // HAVE_BCMP
	}	
	inline bool operator==(const double &a) const {
		for(int i=0;i<m;i++) if (a!=x[i]) return false;
		return true;
	}
	inline bool operator!=(const cvector &v) const {
		if (m!=v.m) return true;
#if !defined(HAVE_BCMP)
		for(int i=0;i<m;i++) if (v.x[i]!=x[i]) return true;
		return false;
#else
		return bcmp(x,v.x,m*sizeof(double))!=0;
#endif // HAVE_BCMP
	}
	inline bool operator!=(const double &a) const {
		for(int i=0;i<m;i++) if (a!=x[i]) return true;
		return false;
	}
	inline double norm2() const {
		double ret=x[0]*x[0];
		for(int i=1;i<m;i++) ret += x[i]*x[i];
		return ret;
	}
	inline double norm() const {
		return sqrt(norm2());
	}
	friend ostream& operator<<(ostream &s, const cvector &v);
	friend istream& operator>>(istream &s, cvector &v);

	inline double *values() {
		return x;
	}
	
	inline int getm() const { return m; }

	inline ostream &niceprint(ostream &s) {
		for(int i=0;i<m;i++)
			s << x[i] << ' ';
		s << endl;
		return s;
	}

	inline void unfuzz(double fuzz) {
	  for(int i=0; i < m; i++)
	    if(x[i] < fuzz) x[i] = 0.0;
	}

	inline double sum() {
	  double total = 0.0;
	  for(int i = 0; i < m; i++)
	    total += x[i];
	  return total;
	}

	inline void support(std::vector<int> &s) {
	  for(int i = 0; i < m; i++)
	    if(!s[i])
	      x[i] = 0.0;
	}

	inline void negate() {
	  for(int i = 0; i < m; i++) {
	    x[i] = -x[i];
	  }
	}

private:
	int m;
	double *x;
};

inline double max(double f1, double f2) {
        return ((f1 > f2) ? f1 : f2);
}
inline cvector operator+(const cvector &a, const cvector &b) {
	return cvector(a)+=b;
}
inline cvector operator-(const cvector &a, const cvector &b) {
	return cvector(a)-=b;
}
inline cvector operator+(const cvector &a, const double &b) {
	return cvector(a)+=b;
}
inline cvector operator-(const cvector &a, const double &b) {
	return cvector(a)-=b;
}
inline cvector operator+(const double &a, const cvector &b) {
	return cvector(b)+=a;
}
inline cvector operator-(const double &a, const cvector &b) {
	return cvector(b.getm(),a)-=b;
}
inline cvector operator*(const cvector &a, const double &b) {
	return cvector(a)*=b;
}
inline cvector operator*(const double &a, const cvector &b) {
	return cvector(b)*=a;
}
inline cvector operator/(const cvector &a, const double &b) {
	return cvector(a)/=b;
}

inline ostream &operator<<(ostream &s, const cvector& v) {
//  	s << v.m << ' ';
	for(int i=0;i<v.m;i++) { s << v.x[i]; if (i!=v.m) s << ' '; }
	return s;
}

inline istream &operator>>(istream &s, cvector& v) {
	int tm;
	s >> tm;
	if (tm!=v.m) {
		delete []v.x;
		v.m = tm;
		v.x = new double[tm];
	}
	for(int i=0;i<tm;i++) s >> v.x[i];
	return s;
}

class cmatrix {
public:
	inline cmatrix(int m=1, int n=1) {
		this->m = m; this->n = n;
		s = m*n;
		x = new double[s];
	}
	~cmatrix();
	inline cmatrix(const cmatrix &ma, bool transpose=false) {
		s = ma.m*ma.n;
		x = new double[s];
		if (transpose) {
			int i,j,c;
			n = ma.m; m = ma.n;
			c = 0;
			for(i=0;i<m;i++) for(j=0;j<n;j++,c++)
				x[c] = ma.x[i+j*m];
		} else {
			n = ma.n; m = ma.m;
			int i;
			for(i=0;i<s;i++) x[i] = ma.x[i];
		}
	}
	inline cmatrix(int m, int n,const double &a,bool diaonly=false) {
		this->m = m;
		this->n = n;
		s = m*n;
		x = new double[s];
		if (diaonly) {
			int i;
			//for(i=0;i<s;i++) x[i] = 0;
			memset(x,0,s*sizeof(double));
			if (n>=m)
				for(i=0;i<m;i++) x[i*n+i] = a;
			else for(i=0;i<n;i++) x[i*n+i] = a;
		} else {
			int i;
			if (a==0.0) memset(x,0,s*sizeof(double));
			else for(i=0;i<s;i++) x[i] = a;
		}
	}
	// put v on the diagonal
	inline cmatrix(int m, int n,const cvector &v) {
		this->m = m;
		this->n = n;
		s = m*n;
		x = new double[s];
		//for(int i=0;i<s;i++) x[i] = 0;
		memset(x,0,s*sizeof(double));
		int l = m;
		if (n<l) l = n;
		if (v.m<l) l = v.m;
		for(int i=0,c=0;i<l;i++,c+=n+1) x[c] = v.x[i]; 
	}
	inline cmatrix(const cvector &v) {
		m = v.m;
		n = 1;
		s = m;
		x = new double[s];
		//for(int i=0;i<s;i++) x[i] = v.x[i];
		memcpy(x,v.x,s*sizeof(double));
	}
		
	inline cmatrix(double *v,int m, int n) {
		this->m = m;
		this->n = n;
		s = m*n;
		//int i;
		x = new double[s];
		//for(i=0;i<s;i++) x[i] = v[i];
		memcpy(x,v,s*sizeof(double));
	}
	// forms a cmatrix of the outer product (ie v1*v2') -- v2 is
	// "transposed" temporarily for this operation
	inline cmatrix(const cmatrix &v1, const cmatrix &v2) {
		if (v1.n!=v2.n) {
			s = 1;
			m=1; n=1; x = new double[1];
			//x[0] = NaN;
			//x[0] = 0.0/0.0;
			x[0] = 0;
		} else {
			n = v2.m; m = v1.m;
			s = n*m;
			x = new double[s];
			int i,j,k,c=0;
			for(i=0;i<m;i++) for(j=0;j<n;j++,c++) {
				x[c] = 0;
				for(k=0;k<v1.n;k++)
					x[c] += v1.x[i*v1.n+k]*
						v2.x[j*v1.n+k];
			}
		}
	}
	inline cmatrix(const cvector &v1, const cvector &v2) {
		n = v2.m; m = v1.m;
		s = n*m;
		x = new double[s];
		int i,j,c=0;
		for(i=0;i<m;i++) for(j=0;j<n;j++,c++)
			x[c] = v1.x[i]*v2.x[j];
	}

	inline cmatrix operator-() const {
		cmatrix ret(m,n);
		for(int i=0;i<s;i++) ret.x[i] = -x[i];
		return ret;
	}
	inline cmatrix& operator=(double a) {
		if (a==0) memset(x,0,s*sizeof(double));
		else for(int i=0;i<s;i++) x[i] = a;
		return *this;
	}
	inline cmatrix& operator=(const cmatrix &ma) {
		if (&ma==this) return *this;
		if (ma.n != n || ma.m != m) {
			s = ma.s; m = ma.m; n = ma.n;
			delete []x;
			x = new double[s];
		}
		//for(int i=0;i<s;i++) x[i] = ma.x[i];
		memcpy(x,ma.x,s*sizeof(double));
		return *this;
	}

	inline bool isvalid() const {
		for(int i=0;i<s;i++) if(!finite(x[i])) return false;
		return true;
	}

	inline cmatrix operator*(const cmatrix &ma) const {
		if (n!=ma.m) {
			cerr << "invalid cmatrix multiply" << endl;
			//assert(0);
		}
		cmatrix ret(m,ma.n);
		int c=0;
		for(int i=0;i<m;i++) for(int j=0;j<ma.n;j++,c++) {
			ret.x[c] = 0;
			for(int k=0;k<n;k++)
				ret.x[c] += x[i*n+k] * ma.x[k*ma.n+j];
		}
		return ret;
	}
	inline cvector operator*(const cvector &v) const {
		if (n!=v.m) {
			cerr << "invalid cvector-cmatrix multiply" << endl;
			//assert(0);
		}
		cvector ret(m);
		int c = 0;
		for(int i=0;i<m;i++,c+=n) {
			ret.x[i] = 0;
			for(int j=0;j<n;j++)
				ret.x[i] += x[c+j] * v.x[j];
		}
		return ret;
	}

	inline double dot(const cmatrix &ma) const {
		if (n!=ma.n || m!=ma.m) {
			cerr << "invalid cmatrix dot-product" << endl;
			//assert(0);
		}
		int c = 0;
		double ret = 0.0;
		for(int i=0;i<m;i++,c+=n)
			for(int j=0;j<n;j++)
				ret += x[c+j]*ma.x[c+j];
		return ret;
	}
		
	inline void outer(const cmatrix &ma, cmatrix &ret) const {
		if (n!=ma.n || ret.m!=m || ret.n!=ma.m) {
			cerr << "invalid cmatrix outer multiply" << endl;
			//assert(0);
		}
		int c=0;
		for(int i=0;i<m;i++) for(int j=0;j<ma.m;j++,c++) {
			ret.x[c] = 0;
			for(int k=0;k<n;k++)
				ret.x[c] += x[i*n+k] * ma.x[j*ma.m+k];
		}
	}
	inline void inner(const cmatrix &ma, cmatrix &ret) const {
		if (m!=ma.m || ret.m!=n || ret.n!=ma.n) {
			cerr << "invalid cmatrix inner multiply" << endl;
			//assert(0);
		}
		int c=0;
		for(int i=0;i<n;i++) for(int j=0;j<ma.n;j++,c++) {
			ret.x[c] = 0;
			for(int k=0;k<m;k++)
				ret.x[c] += x[k*m+i] * ma.x[k*m+j];
		}
	}

	inline double rowmult(int r, const cvector &v, int exclude) const {
		if (n!=v.m) {
			cerr << "invalid matrix-vector multiply" << endl;
			//assert(0);
		}
		double ret = 0.0;
		int c=n*r;
		for(int j=0;j<n;j++,c++)
			if (j!=exclude) ret += x[c]*v[j];
		return ret;
	}
	inline double rowmult(int r, const cvector &v) const {
		if (n!=v.m) {
			cerr << "invalid matrix-vector multiply" << endl;
			//assert(0);
		}
		double ret = 0.0;
		int c=n*r;
		for(int j=0;j<n;j++,c++)
			ret += x[c]*v[j];
		return ret;
	}

	inline cmatrix &multbyrow(const double *v) {
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] *= v[j];
		return *this;
	}
	inline cmatrix &multbycol(const double *v) {
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] *= v[i];
		return *this;
	}
	inline cmatrix &multbyrow(const cvector &v) {
		if (n!=v.m) {
			cerr << "invalid multbycol" << endl;
			//assert(0);
		}
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] *= v[j];
		return *this;
	}
	inline cmatrix &multbycol(const cvector &v) {
		if (m!=v.m) {
			cerr << "invalid multbycol" << endl;
			//assert(0);
		}
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] *= v[i];
		return *this;
	}
	inline cmatrix &dividebyrow(const double *v) {
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] /= v[j];
		return *this;
	}
	inline cmatrix &dividebycol(const double *v) {
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] /= v[i];
		return *this;
	}
	inline cmatrix &dividebyrow(const cvector &v) {
		if (n!=v.m) {
			cerr << "invalid dividebycol" << endl;
			//assert(0);
		}
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] /= v[j];
		return *this;
	}
	inline cmatrix &dividebycol(const cvector &v) {
		if (m!=v.m) {
			cerr << "invalid dividebycol" << endl;
			//assert(0);
		}
		int c = 0;
		for(int i=0;i<m;i++)
			for(int j=0;j<n;j++,c++)
				x[c] /= v[i];
		return *this;
	}
	
	inline double operator()(int i, int j) const {
		return x[i*n+j];
	}

	inline const double *operator[](int i) const {
		return x+(i*n);
	}
	inline double *operator[](int i) {
		return x+(i*n);
	}
	inline cmatrix t() const {
		return cmatrix(*this,true);
	}

	inline cmatrix &operator+=(const cmatrix &ma) {
		if (m!=ma.m||n!=ma.n) {
			cerr << "invalid cmatrix addition" << endl;
			//assert(0);
		}
		for(int i=0;i<s;i++) x[i] += ma.x[i];
		return *this;
	}

	inline cmatrix &operator-=(const cmatrix &ma) {
		if (m!=ma.m||n!=ma.n) {
			cerr << "invalid cmatrix addition" << endl;
			//assert(0);
		}
		for(int i=0;i<s;i++) x[i] -= ma.x[i];
		return *this;
	}

	inline cmatrix &operator*=(const cmatrix &ma) {
		if (n!=ma.m || n != ma.n) {
			cerr << "invalid cmatrix multiply" << endl;
			//assert(0);
		}
		int i,j,k,c=0;
		std::vector<double> newrow(n);
		for(i=0;i<m;i++) {
		  for(j=0;j<n;j++) {
		    newrow[j] = 0;
		    for(k=0;k<n;k++)
		      newrow[j] += x[c+k] * ma.x[k*n+j];
		  }
		  for(j=0; j < n; j++,c++)
		    x[c] = newrow[j];
		}
		return *this;
	}

	inline cmatrix &operator+=(const double &a) {
		for(int i=0;i<s;i++) x[i] += a;
		return *this;
	}

	inline cmatrix &operator-=(const double &a) {
		for(int i=0;i<s;i++) x[i] -= a;
		return *this;
	}

	inline cmatrix &operator*=(const double &a) {
		for(int i=0;i<s;i++) x[i] *= a;
		return *this;
	}

	inline cmatrix &operator/=(const double &a) {
		for(int i=0;i<s;i++) x[i] /= a;
		return *this;
	}

	inline double max() const {
		double t,ma = x[0];
		for(int i=1;i<s;i++) 
			if ((t=x[i])>ma) ma=t;
		return ma;
	}

	inline double min() const {
		double t,mi = x[0];
		for(int i=1;i<s;i++) 
			if ((t=x[i])<mi) mi=t;
		return mi;
	}

	inline double absmin() const {
		double t,mi = x[0]>0?x[0]:-x[0];
		for(int i=1;i<s;i++)
			if ((t=(x[i]>0?x[i]:-x[i]))<mi) mi=t;
		return mi;
	}

	inline double absmax() const {
		double t,ma = x[0]>0?x[0]:-x[0];
		for(int i=1;i<s;i++)
			if ((t=(x[i]>0?x[i]:-x[i]))>ma) ma=t;
		return ma;
	}

	inline bool operator==(const cmatrix &ma) const {
		if (ma.n!=n||ma.m!=m) return false;
#if !defined(HAVE_BCMP)
		for(int i=0;i<s;i++) if (ma.x[i]!=x[i]) return false;
		return true;
#else
		return bcmp(ma.x,x,s*sizeof(double))==0.0;
#endif // HAVE_BCMP
	}
	inline bool operator==(const double &a) const {
		for(int i=0;i<s;i++) if (x[i]!=a) return false;
		return true;
	}
	inline bool operator!=(const cmatrix &ma) const {
		if (ma.n!=n||ma.m!=m) return true;
#if !defined(HAVE_BCMP)
		for(int i=0;i<s;i++) if (ma.x[i]!=x[i]) return true;
		return false;
#else
		return bcmp(ma.x,x,s*sizeof(double))!=0.0;
#endif // HAVE_BCMP
	}
	inline bool operator!=(const double &a) const {
		for(int i=0;i<s;i++) if (x[i]!=a) return true;
		return false;
	}

	inline double norm2() const { // returns the square of the frobenius norm
		double ret=x[0]*x[0];
		for(int i=1;i<s;i++) ret += x[i]*x[i];
		return ret;
	}
	inline double norm() const { // returns the frobenius norm
		return sqrt(norm2());
	}

	friend ostream& operator<<(ostream& s, const cmatrix& ma);
	friend istream& operator>>(istream& s, cmatrix& ma);

	// LU decomposition -- ix is the row permutations
	int LUdecomp(cmatrix &LU, int *ix) const;
	// LU back substitution --
	//    ix from above fn call (this should be an LU combination)
	void LUbacksub(int *ix, double *col) const;

	// solves equation Ax=b (A is this, x is the returned value)
bool solve(cvector &b, cvector &dest);
	double *solve(const double *b, bool &worked) const;
	inline double *solve(const double *b) const { bool w; return solve(b,w); }
	
	inline void negate() { for(int i = 0; i < s; i++) x[i] = -x[i]; }
	cmatrix inv(bool &worked) const;
	inline cmatrix inv() const { bool w; return inv(w); }
	double adjoint();
	inline double trace();
	double testAdjoint();
	inline void multiply(const cvector &source, cvector &dest) {
	  //assert(n == source.m && m == dest.m);
	  int i,j,c=0;
	  for(i = 0; i < m; i++) {
	    dest[i] = 0;
	    for(j = 0; j < n; j++,c++)
	      dest[i] += x[c] * source[j];
	  }
	}

	inline double *values() { return x; }



	// w needs to points to an array of n fp numbers
	void svd(cmatrix &u, cmatrix &v, double *w);

	 // makes the matrix tri-diagonal (if symmetric)
	// The matrix is replaced by Q, and the vectors d and e hold
	// the diagonal and off-diagonal elements respectively
	void tridiag(cvector &d, cvector &e);

	// converts the Q matrix from tridiag into the eigenvectors
	// d and e are the diagonal and off-diagonal elements from
	// tridiag and the eigenvalues are left in d
	void tridiagev(cvector &d, cvector &e);

	// returns the inverse of a real, symmetric matrix
	// via eigenvalue decomposition.  If any of the e.v. are
	// very close to zero, it uses the psuedo inverse
	// (ie 1/ev = 0)
	cmatrix syminv(double tol=1e-10) const;

	// uses the above two to compute eigen decomp of symmetric
	// matrix (destroys matrix and replaces with eigenvectors)
	cvector eigensym() { cvector d,e; tridiag(d,e); tridiagev(d,e); return d; }


	inline ostream &niceprint(ostream& s) {
		s << m << ' ' << n << endl;
		for(int i=0;i<m;i++) {
			for(int j=0;j<n;j++)
				s << x[i*n+j] << ' ';
			s << endl;
		}
		return s;
	}

	inline int getm() const { return m; }
	inline int getn() const { return n; }

	inline void compact() { }

private:
	static double pythag(double a, double b);

	int m,n,s;
	double *x;
};

inline cmatrix operator+(const cmatrix &a, const cmatrix &b) {
	return cmatrix(a)+=b;
}
inline cmatrix operator-(const cmatrix &a, const cmatrix &b) {
	return cmatrix(a)-=b;
}
inline cmatrix operator+(const cmatrix &a, const double &b) {
	return cmatrix(a)+=b;
}
inline cmatrix operator-(const cmatrix &a, const double &b) {
	return cmatrix(a)-=b;
}
inline cmatrix operator+(const double &a, const cmatrix &b) {
	return cmatrix(b)+=a;
}
inline cmatrix operator-(const double &a, const cmatrix &b) {
	return cmatrix(b.getn(),b.getm(),a)-=b;
}
inline cmatrix operator*(const cmatrix &a, const double &b) {
	return cmatrix(a)*=b;
}
inline cmatrix operator*(const double &b, const cmatrix &a) {
	return cmatrix(a)*=b;
}
inline cmatrix operator/(const cmatrix &a, const double &b) {
	return cmatrix(a)/=b;
}

inline ostream& operator<<(ostream& s, const cmatrix& ma) {
//  	s << ma.m << ' ' << ma.n << ' ';
	for(int i=0;i<ma.s;i++) { 
if (i%ma.n==0) s << endl; s << ma.x[i]; if (i!=ma.s) s << ' '; }
	return s;
}

inline istream& operator>>(istream& s, cmatrix& ma) {
	int tn,tm;
	s >> tm >> tn;
	if (tm!=ma.m || tn!=ma.n) {
		delete []ma.x;
		ma.s = tm*tn;
		ma.x = new double[ma.s];
		ma.m = tm; ma.n = tn;
	}
	for(int i=0;i<ma.s;i++) { s >> ma.x[i]; }
	return s;
}
#endif
