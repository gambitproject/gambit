//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/gtracer/cmatrix.cc
// Implementation of matrix classes for Gametracer
// This file is based on GameTracer v0.2, which is
// Copyright (c) 2002, Ben Blum and Christian Shelton
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

#include <cmath>
#include <cfloat>
#include "cmatrix.h"
#include "core/matrix.h"

namespace Gambit {
namespace gametracer {

cvector::~cvector() { delete []x; }

cmatrix::~cmatrix()
 { delete []x; }

cmatrix cmatrix::inv(bool &worked) const {
	if (m!=n) {
		std::cerr << "invalid cmatrix inverse" << std::endl;
		exit(1);
	}
	cmatrix temp(n,n);
	int *ix = new int[n];
	
	if (!LUdecomp(temp,ix)) {
		worked = false;
		delete []ix;
		return cmatrix(n,n,0,false);
	}
	worked = true;

	cmatrix ret(n,n);
	int i,j;
	auto *col = new double[n];
	for(j=0;j<n;j++) {
		for(i=0;i<n;i++) col[i] = 0;
		col[j] = 1;
		temp.LUbacksub(ix,col);
		for(i=0;i<n;i++) ret.x[i*n+j] = col[i];
	}
	delete []col;
	delete []ix;
	return ret;
}

int cmatrix::LUdecomp(cmatrix &LU, int *ix) const {
	if (m!=n||LU.m!=LU.n||LU.n!=n) {
		std::cerr << "invalid cmatrix in LUdecomp" << std::endl;
		exit(1);
	}
	int d=1,i,j,k;
	LU = *this;
	auto *vv = new double[n];
	double dum;

	k = 0;
	for(i=0;i<n;i++) {
		vv[i] = fabs(x[k]); k++;
		for(j=1;j<n;j++,k++) if(vv[i]<(dum=fabs(x[k]))) vv[i]=dum;
		if (vv[i]==(double)0.0) {
			delete []vv;
			return 0;
		}
		vv[i] = 1/vv[i];
	}
	double sum,big;
	int imax;
	for(j=0;j<n;j++) {
		for(i=0;i<j;i++) {
			sum = LU.x[i*n+j];
			for(k=0;k<i;k++) sum -= LU.x[i*n+k]*LU.x[k*n+j];
			LU.x[i*n+j] = sum;
		}
		big = 0;
		for(i=j;i<n;i++) {
			sum = LU.x[i*n+j];
			for(k=0;k<j;k++) sum -= LU.x[i*n+k]*LU.x[k*n+j];
			LU.x[i*n+j] = sum;
			if ((dum=vv[i]*fabs(sum))>=big) {
				big = dum;
				imax = i;
			}
		}
		if (j!=imax) {
			for(k=0;k<n;k++) {
				dum = LU.x[imax*n+k];
				LU.x[imax*n+k] = LU.x[j*n+k];
				LU.x[j*n+k] = dum;
			}
			d = -d;
			vv[imax] = vv[j];
		}
		ix[j] = imax;
		if (LU.x[j*n+j] == 0) {
			LU.x[j*n+j] = (double)1.0e-20;
		}
		if (j!=n-1) {
			dum = 1/LU.x[j*n+j];
			for(i=j+1;i<n;i++) LU.x[i*n+j] *= dum;
		}
	}
	delete []vv;
	return d;
}

void cmatrix::LUbacksub(int *ix, double *b) const {
	if (n!=m) {
		std::cerr << "invalid cmatrix in LUbacksub" << std::endl;
		exit(1);
	}
	int ip,ii=-1,i;
	double sum;

	for(i=0;i<n;i++) {
		ip = ix[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii!=-1)
			for(int j=ii;j<=i-1;j++) sum -= x[i*n+j]*b[j];
		else if (sum!=0) ii=i;
		b[i] = sum;
	}
	for(i=n-1;i>=0;i--) {
		sum = b[i];
		for(int j=i+1;j<=n-1;j++) sum -= x[i*n+j]*b[j];
		b[i] = sum/x[i*n+i];
	}
}

bool cmatrix::solve(cvector &b, cvector &ret) {
	if (m!=n) {
		std::cerr << "invalid cmatrix in solve" << std::endl;
		exit(1);
	}
	for(int i=0;i<n;i++) ret[i] = b[i];
	int *ix = new int[n];
	cmatrix a(n,n);
	
	if (!LUdecomp(a,ix)) {
		delete []ix;
		return false;
	}
	a.LUbacksub(ix,ret.values());
	delete []ix;
	return true;
}
double *cmatrix::solve(const double *b, bool &worked) const {
	if (m!=n) {
		std::cerr << "invalid cmatrix in solve" << std::endl;
		exit(1);
	}
	auto *ret = new double[n];
	for(int i=0;i<n;i++) ret[i] = b[i];
	int *ix = new int[n];
	cmatrix a(n,n);
	
	if (!LUdecomp(a,ix)) {
		worked = false;
		delete []ix;
		return ret;
	}
	worked=true;
	a.LUbacksub(ix,ret);
	delete []ix;
	return ret;
}

double cmatrix::pythag(double a, double b) {
	double absa,absb;
	absa = fabs(a);
	absb = fabs(b);
	if (absa>absb) {
		double sqr = absb/absa;
		return absa*std::sqrt(1.0+sqr*sqr);
	} else {
		if (absb==0.0) return 0;
		double sqr = absa/absb;	
		return absb*std::sqrt(1.0+sqr*sqr);
	}
}

#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))

void cmatrix::svd(cmatrix &u, cmatrix &v, double *w) {

	u = *this;
	if (v.n!=n || v.m!=n) {
		delete []v.x;
		v.n = n; v.m = n;
		v.s = n*n;
		v.x = new double[v.s];
	}

	int flag,i,its,j,jj,k,l,nm;
	double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

	rv1 = new double[n];
	g=scale=anorm=(double)0;
	for(i=0;i<n;i++) {
		l = i+1;
		rv1[i] = scale*g;
		g=s=scale=(double)0.0;
		if (i<m) {
			for(k=i;k<m;k++) scale += fabs(u[k][i]);
			if (scale) {
				for(k=i;k<m;k++) {
					u[k][i] /= scale;
					s += u[k][i]*u[k][i];
				}
				f = u[i][i];
				g = -SIGN(std::sqrt(s),f);
				h = f*g-s;
				u[i][i] = f-g;
				for(j=l;j<n;j++) {
					for(s=0.0,k=i;k<m;k++)
						s += u[k][i]*u[k][j];
					f=s/h;
					for(k=i;k<m;k++) u[k][j] += f*u[k][i];
				}
				for(k=i;k<m;k++) u[k][i] *= scale;
			}
		}
		w[i] = scale *g;
		g=s=scale=(double)0;
		if (i<m && i!=n-1) {
			for(k=l;k<n;k++) scale += fabs(u[i][k]);
			if (scale) {
				for(k=l;k<n;k++) {
					u[i][k] /= scale;
					s += u[i][k]*u[i][k];
				}
				f = u[i][l];
				g = -SIGN(std::sqrt(s),f);
				h = f*g-s;
				u[i][l] = f-g;
				for(k=l;k<n;k++) rv1[k] = u[i][k]/h;
				for(j=l;j<m;j++) {
					for(s=(double)0,k=l;k<n;k++) 
						s+=u[j][k]*u[i][k];
					for(k=l;k<n;k++) u[j][k] += s*rv1[k];
				}
				for(k=l;k<n;k++) u[i][k] *= scale;
			}
		}
		double temp = fabs(w[i])+fabs(rv1[i]);
		anorm = anorm>temp ? anorm : temp;
	}
	for(i=n-1;i>=0;i--) {
		if (i<n-1) {
			if (g) {
				for(j=l;j<n;j++)
					v[j][i] = (u[i][j]/u[i][l])/g;
				for(j=l;j<n;j++) {
					for(s=(double)0,k=l;k<n;k++)
						s += u[i][k]*v[k][j];
					for(k=l;k<n;k++) v[k][j] += s*v[k][i];
				}
			}
			for(j=l;j<n;j++) v[i][j]=v[j][i]=(double)0;
		}
		v[i][i] = (double)1;
		g=rv1[i];
		l=i;
	}
	for(i=m>n?n-1:m-1;i>=0;i--) {
		l=i+1;
		g=w[i];
		for(j=l;j<n;j++) u[i][j] = (double)0;
		if (g) {
			g = 1/g;
			for(j=l;j<n;j++) {
				for(s=(double)0,k=l;k<m;k++)
					s += u[k][i]*u[k][j];
				f = (s/u[i][i])*g;
				for(k=i;k<m;k++) u[k][j] += f*u[k][i];
			}
			for(j=i;j<m;j++) u[j][i] *= g;
		} else for (j=i;j<m;j++) u[j][i] = (double)0;
		++u[i][i];
	}
	for(k=n-1;k>=0;k--) {
		for(its=1;its<=30;its++) {
			flag = 1;
			for(l=k;l>=0;l--) {
				nm = l-1;
				if ((double)(fabs(rv1[l])+anorm)==anorm) {
					flag = 0;
					break;
				}
				if ((double)(fabs(w[nm])+anorm)==anorm) break;
			}
			if (flag) {
				c = (double)0;
				s = (double)1;
				for(i=l;i<=k;i++) {
					f = s*rv1[i];
					rv1[i] = c*rv1[i];
					if ((double)(fabs(f)+anorm)==anorm) break;
					g = w[i];
					h = pythag(f,g);
					w[i] = h;
					h = 1/h;
					c = g*h;
					s = -f*h;
					for(j=0;j<m;j++) {
						y = u[j][nm];
						z = u[j][i];
						u[j][nm] = y*c+z*s;
						u[j][i] = z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l==k) {
				if (z<0.0) {
					w[k] = -z;
					for(j=0;j<n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its==30) {
				// some other method (like an error return
				// value should be put here)
				std::cerr << "no convergence after 30 svdcmp "
				  "iterations" << std::endl;
				exit(1);
			}
			x = w[l];
			nm = k-1;
			y = w[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y-z)*(y+z)+(g-h)*(g+h))/(2*h*y);
			g = pythag(f,1.0);
			f = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1;
			for(j=l;j<=nm;j++) {
				i = j+1;
				g = rv1[i];
				y = w[i];
				h = s*g;
				g = c*g;
				z = pythag(f,h);
				rv1[j] = z;
				c = f/z;
				s = h/z;
				f = x*c+g*s;
				g = g*c-x*s;
				h = y*s;
				y *= c;
				for(jj=0;jj<n;jj++) {
					x = v[jj][j];
					z = v[jj][i];
					v[jj][j] = x*c+z*s;
					v[jj][i] = z*c-x*s;
				}
				z = pythag(f,h);
				w[j] = z;
				if (z) {
					z = 1/z;
					c = f*z;
					s = h*z;
				}
				f = c*g+s*y;
				x = c*y-s*g;
				for(jj=0;jj<m;jj++) {
					y=u[jj][j];
					z=u[jj][i];
					u[jj][j]=y*c+z*s;
					u[jj][i]=z*c-y*s;
				}
			}
			rv1[l] = 0;
			rv1[k] = f;
			w[k] = x;
		}
	}
	delete []rv1;
}

double cmatrix::adjoint() {
  int i, j, i0, j0, maxi, lastj = -1;
  double max, pivot;
  std::vector<int> r(m);
  std::vector<int> r2(m);
  std::vector<int> c(m);
  double D = 1.0;
  Gambit::Matrix<double> retval(0, m-1, 0, m-1);
  for(i = 0; i < m; i++)
    for(j = 0; j < m; j++)
      retval(i, j) = x[i*n+j];

  for(i= 0; i < m; i++) {
    r[i] = -1;
  }
  for(j = 0; j < m; j++) {
    if(D == 0.0)
      return DBL_MAX;
    max = -1.0;
    maxi = -1;
    for(i = 0; i < m; i++) {
      if(r[i] < 0 && fabs(retval(i,j)) > max) {
	max = fabs(retval(i,j));
	maxi = i;
      }
    }
    if(j != lastj && max == 0.0) {
      if(lastj >= 0)
        return DBL_MAX;
      lastj = j;
      if(j != m-1)
	continue;
    }
    if(maxi == -1) {
      std::cout << "oops";
      return DBL_MAX;
    }

    i = maxi;
    pivot = retval(i,j);
    for(i0 = 0; i0 < m; i0++) {
      if(i0 != i) {
	for(j0 = 0; j0 < m; j0++) {
	  if(j0 != j) {
	    retval(i0,j0) *= pivot;
	    retval(i0,j0) -= retval(i0,j) * retval(i,j0);
	    retval(i0,j0) /= D;
	  }
	}
      }
    }
    for(i0 = 0; i0 < m; i0++) {
      retval(i0,j) = -retval(i0,j);
    }
    retval(i,j) = D;
    D = pivot;
    r[i] = j;
    c[j] = i;
    if(j == lastj)
      break;
    if(j == m-1 && lastj >= 0)
      j = lastj - 1;
  }
  //  std::cout << retval << std::endl << std::endl;
  int s=0;
  i = 0;
  r2 = r;
  while(i < m-1) {
    j = r2[i];
    if(i != j) {
      s++;
      r2[i] = r2[j];
      r2[j] = j;
    } else
      i++;
  }
  for(i = 0; i < m; i++)
    for(j = 0; j < m; j++)
      x[i*n+j] = retval(c[i],r[j]);
  if(s%2 == 1) {
    negate();
    D = -D;
  }
  // std::cout << *this << std::endl << std::endl;
  return D;
}

double cmatrix::testAdjoint()
//returns the characteristic polynomial and adjoint cmatrix
{
  cmatrix c(n,n,1.0,true), p(n,n);
  int i = 0, j;
  double det,b;

  for (;;) {
    i++;
    if (i == n) break;
    p = *this * c;
    c = p;
    b = - c.trace() / i;
    for (j = 0; j < n; j++) c[j][j] += b;
  }
  p = m * c;
  det = - p.trace() / n;
  b = 1 - 2*((n - 1) % 2);
  *this = c * b;
  return det;
}

double cmatrix::trace() {
  //assert(n == m);
  double sum = 0.0;
  for(int i = 0; i < n; i++) {
    sum += x[i*n+i];
  }
  return sum;
}

} // end namespace Gambit::gametracer
} // end namespace Gambit
