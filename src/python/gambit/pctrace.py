#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/pctrace.py
# Trace a smooth parameterized curve using a predictor-corrector method
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
"""Trace a smooth parameterized curve using a predictor-corrector method.
"""

from __future__ import print_function

import math
import numpy
import scipy.linalg        

def qr_decomp(b):
    qq, b[:,:] = scipy.linalg.qr(b, overwrite_a=True)
    return qq.transpose()

def newton_step(q, b, u, y):
    # Expects q and b to be 2-D arrays, and u and y to be 1-D arrays
    # Returns steplength
    for k in xrange(b.shape[1]):
        y[k] -= numpy.dot(b[:k,k], y[:k])
        y[k] /= b[k,k]

    d = 0.0
    for k in xrange(b.shape[0]):
        s = numpy.dot(q[:-1,k], y)
        u[k] -= s
        d += s*s
    return math.sqrt(d)

def trace_path(start, startLam, maxLam, compute_lhs, compute_jac,
               omega=1.0, hStart=0.03, maxDecel=1.1, maxIter=1000,
               crit=None, callback=None):
    """
    Trace a differentiable path starting at the vector 'start' with
    parameter 'startLam', until 'maxLam' is reached.  lhs() returns the
    value of the LHS at any point (x, lam), and jac() returns the value
    of the Jacobian at any point (x, lam).  omega determines the orientation
    to trace the curve.  Optionally, 'crit' is a function to search for a
    zero of along the path.
    """

    tol = 1.0e-4              # tolerance for corrector iteration
    maxDist = 0.4             # maximal distance to curve
    maxContr = 0.6            # maximal contraction rate in corrector
    eta = 0.1                 # perturbation to avoid cancellation
                              # in calculating contraction rate
    h = hStart                # initial stepsize
    hmin = 1.0e-8             # minimal stepsize

    newton = False            # using Newton steplength (for zero-finding)

    x = numpy.array([x for x in start] + [startLam])
    if callback is not None:
        callback(x)

    y = numpy.zeros(len(start))
    q = numpy.zeros((len(start)+1, len(start)+1)) 
    b = compute_jac(x)
    q = qr_decomp(b)
    t = q[-1]                # last column is tangent

    if startLam == 0.0 and omega*t[-1] < 0.0:
        # Reverse orientation of curve to trace away from boundary
        omega = -omega
    
    while x[-1] >= 0.0 and x[-1] < maxLam:
        accept = True

        if abs(h) <= hmin:
            # Stop.
            return x

        # Predictor step
        u = x + h*omega*t

        decel = 1.0 / maxDecel      # initialize deceleration factor
        b = compute_jac(u)
        #q, b = scipy.linalg.decomp.qr(b, mode='qr')
        q = qr_decomp(b)

        it = 1
        disto = 0.0
        while True:
            if it == maxIter:
                # Give up!
                return x
            
            y = compute_lhs(u)
            dist = newton_step(q, b, u, y)

            if dist >= maxDist:
                accept = False
                break

            decel = max(decel, math.sqrt(dist / maxDist) * maxDecel)

            if it >= 2:
                contr = dist / (disto + tol * eta)
                if contr > maxContr:
                    accept = False
                    break
                decel = max(decel, math.sqrt(contr / maxContr) * maxDecel)

            if dist < tol:
                # Success; break out of iteration
                break

            disto = dist
            it += 1

        if not accept:
            h /= maxDecel       # PC not accepted; change stepsize and retry
            if abs(h) < hmin:
                # Stop.
                return x
            continue    # back out to main loop to try again

        # Determine new stepsize
        if decel > maxDecel:
            decel = maxDecel

        if not newton and crit is not None:
            # Currently, 't' is the tangent at 'x'.  We also need the
            # tangent at 'u'.
            newT = q[-1]

            if crit(x, t) * crit(u, newT) < 0.0:
                # Enter Newton mode, since a critical point has been bracketed
                newton = True

        if newton:
            # Newton-type steplength adaptation, secant method
            newT = q[-1]
            h *= -crit(u, newT) / (crit(u, newT) - crit(x, t))
        else:
            # Standard steplength adaptaiton
            h = abs(h / decel)

        # PC step was successful; update and iterate
        x = u[:]

        if callback is not None:
            callback(x)

        newT = q[-1]    # new tangent
        if sum(t * newT) < 0.0:
            # Bifurcation detected
            print("Detected bifurcation near %f" % x[-1])

            omega = -omega
        t = newT[:]
        
    return x


def upd(q, b, x, u, y, w, t, h, angmax):
    n = len(w)
    n1 = n+1
    
    for k in xrange(n):
        b[n1-1,k] = (w[k] - y[k]) / h

    for k in xrange(n):
        givens(b, q, b[k,k], b[n1-1,k], k, n1-1, k)

    ang = 0.0
    for k in xrange(n1):
        ang = ang + t[k] * q[n1-1,k]

    if ang > 1.0:  ang = 1.0
    if ang < -1.0: ang = -1.0

    ang = math.acos(ang)
    if ang > angmax:
        return False
    else:
        return True

def givens(b, q, c1, c2, ell1, ell2, ell3):
    if math.fabs(c1) + math.fabs(c2) == 0.0:
        return c1, c2

    if math.fabs(c2) >= math.fabs(c1):
        sn = math.sqrt(1.0 + (c1/c2)*(c1/c2)) * math.fabs(c2)
    else:
        sn = math.sqrt(1.0 + (c2/c1)*(c2/c1)) * math.fabs(c1)
    s1 = c1/sn
    s2 = c2/sn
    
    for k in xrange(q.shape[1]):
        sv1 = q[ell1, k]
        sv2 = q[ell2, k]
        q[ell1, k] = s1*sv1 + s2*sv2
        q[ell2, k] = -s2*sv1 + s1*sv2

    for k in xrange(ell3, b.shape[1]):
        sv1 = b[ell1, k]
        sv2 = b[ell2, k]
        b[ell1, k] = s1*sv1 + s2*sv2
        b[ell2, k] = -s2*sv1 + s1*sv2

    return sn, 0.0

def ynorm(y):
    s = 0.0
    for i in xrange(len(y)):
        s += y[i]**2
    return math.sqrt(s)

def newt(q, b, u, v, w, p, pv, r, pert, dmax, dmin, ctmax, cdmax,
         compute_lhs):
    # input: q, b, u, w=H(u)
    # output: v, r=H(v)
    # w is changed
    # one Newton step is performed
    # q, b = QR decomposition of A
    # q, b are updated
    # perturbations are used for stabilization
    # residual and contraction tests are performed

    test = True
    n = len(w)
    n1 = n+1
    
    for k in xrange(n):
        if abs(w[k]) > pert:
            pv[k] = 0.0
        elif w[k] > 0.0:
            pv[k] = w[k] - pert
        else:
            pv[k] = w[k] + pert
        w[k] = w[k] - pv[k]

    d1 = ynorm(w)

    if d1 > dmax:
        print("newt: fail on LHS norm test")
        return False, r, v

    for k in xrange(n):
        for ell in xrange(k-1):
            w[k] = w[k] - b[ell, k] * w[ell]
        w[k] = w[k] / b[k,k]

    d2 = ynorm(w)

    for k in xrange(n1):
        s = 0.0
        for ell in xrange(n):
            s = s + q[ell,k] * w[ell]
        v[k] = u[k] - s

    r = compute_lhs(v)
        
    for k in xrange(n):
        p[k] = r[k] - pv[k]

    d3 = ynorm(p)
    contr = d3 / (d1 + dmin)
    if contr > ctmax:
        print("newt: fail on contraction test")
        test = False
    
    for k in reversed(xrange(n-1)):
        givens(b, q, w[k], w[k+1], k, k+1, k)

    for k in xrange(n):
        b[0,k] = b[0,k] - p[k] / d2

    for k in xrange(n-1):
        givens(b, q, b[k,k], b[k+1,k], k, k+1, k)

    if b[n-1,n-1] < 0.0:
        print("newt: fail on diagonal sign test")
        test = False
        b[n-1,n-1] = -b[n-1,n-1]
        for k in xrange(n1):
            q[n-1,k] = -q[n-1,k]
            q[n1-1,k] = -q[n1-1,k]

    for i in xrange(1,n):
        for k in xrange(i-1):
            if abs(b[k,i]) > cdmax * abs(b[i,i]):
                print("... reconditioning ...")
                if b[i,i] > 0.0:
                    b[i,i] = abs(b[k,i]) / cdmax
                else:
                    b[i,i] = -abs(b[k,i]) / cdmax

    for k in xrange(n-1):
        b[k+1,k] = 0.0

    return test, r, v

def estimate_jac(x, compute_lhs):
    n = len(x)-1
    n1 = len(x)
    b = numpy.zeros((n1,n))
    h = 0.32

    for i in xrange(n1):
        x[i] = x[i] + h
        y = compute_lhs(x)
        x[i] = x[i] - h
        for k in xrange(n):
            b[i,k] = y[k]

    y = compute_lhs(x)

    for i in xrange(n1):
        for k in xrange(n):
            b[i,k] = (b[i,k] - y[k]) / h

    return b

def trace_path_nojac(start, startLam, maxLam, compute_lhs, 
                     omega=1.0, hStart=0.03, maxDecel=1.1, maxIter=1000,
                     crit=None, callback=None):
    """
    Trace a differentiable path starting at the vector 'start' with
    parameter 'startLam', until 'maxLam' is reached.  lhs() returns the
    value of the LHS at any point (x, lam), and jac() returns the value
    of the Jacobian at any point (x, lam).  omega determines the orientation
    to trace the curve.  Optionally, 'crit' is a function to search for a
    zero of along the path.
    """

    tol = 1.0e-4              # tolerance for corrector iteration
    maxDist = 0.4             # maximal distance to curve
    maxContr = 0.6            # maximal contraction rate in corrector
    eta = 0.1                 # perturbation to avoid cancellation
                              # in calculating contraction rate
    h = hStart                # initial stepsize
    hmin = 1.0e-8             # minimal stepsize

    ctmax = 0.8
    dmax = 0.01
    dmin = 0.0001
    pert = 0.00001
    hmax = 1.28
    hmin = 0.000001
    h = 0.03
    cdmax = 1000.0
    angmax = 3.141592654 / 3
    maxstp = 9000
    acfac = 1.1

    newton = False            # using Newton steplength (for zero-finding)

    x = numpy.array([x for x in start] + [startLam])
    if callback is not None:
        callback(x)

    y = numpy.zeros(len(start))
    q = numpy.zeros((len(start)+1, len(start)+1)) 
    v = numpy.zeros(len(start)+1)
    p = numpy.zeros(len(start))
    pv = numpy.zeros(len(start))
    r = numpy.zeros(len(start))
    
    b = estimate_jac(x, compute_lhs)
    q = qr_decomp(b)
    
    while x[-1] >= 0.0 and x[-1] < maxLam:
        accept = True
        qq = q[:,:]
        bb = b[:,:]
        
        t = q[-1][:]               # last column is tangent

        if abs(h) <= hmin:
            # Stop.
            return x

        # Predictor step
        u = x + h*omega*t
        print("PREDICTOR")
        callback(u)
        w = compute_lhs(u)
        print(w)
        print(ynorm(w))

        test = upd(q, b, x, u, y, w, t, h, angmax)
        if not test:
            print("Fail angle test...")
            h = h / acfac
            q = qq
            b = bb
            continue

        test, r, v = newt(q, b, u, v, w, p, pv, r, pert, dmax, dmin, ctmax, cdmax,
                       compute_lhs)
        if not test:
            print("Fail Newton test...")
            h = h / acfac
            q = qq
            b = bb
            continue

        if newton:
            # Newton-type steplength adaptation
            h = -(v[-1]-1.0) / q[-1,-1]
            if abs(h) <= hmin:
                # Stop
                return x
        else:
            # Standard steplength adaptaiton
            h = abs(h) * acfac
            #if h > hmax:
            #    h = hmax

        # PC step was successful; update and iterate
        x = v[:]
        y = r[:]

        if callback is not None:
            print("ACCEPTED")
            callback(x)
        print(y)
        print(ynorm(y))
        print()
        
    return x


