"""
Trace a smooth parameterized curve using a predictor-corrector method.
"""

import math
import numpy
import scipy.linalg.decomp    # for QR decomposition (TODO: use it!)


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
        
def qr_decomp(b):
    q = numpy.identity(b.shape[0], numpy.float)
    for m in xrange(b.shape[1]):
        for k in xrange(m+1, b.shape[0]):
            b[m,m], b[k,m] = givens(b, q, b[m,m], b[k,m], m, k, m+1)
    return q

def newton_step(q, b, u, y):
    # Expects q and b to be 2-D arrays, and u and y to be 1-D arrays
    # Returns steplength
    for k in xrange(b.shape[1]):
        for ell in xrange(k):
            y[k] -= b[ell,k] * y[ell]
        y[k] /= b[k,k]

    d = 0.0
    for k in xrange(b.shape[0]):
        s = 0.0
        for ell in xrange(b.shape[1]):
            s += q[ell,k] * y[ell]
        u[k] -= s
        d += s*s
    return math.sqrt(d)


def trace_path(start, startLam, maxLam, compute_lhs, compute_jac,
               omega=1.0, hStart=0.03, maxDecel=1.1, maxIter=1000,
               crit=None, printer=None):
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
    if printer is not None:
        printer(x)

    y = numpy.zeros(len(start))
    q = numpy.zeros((len(start)+1, len(start)+1)) 
    b = compute_jac(x)
    q = qr_decomp(b)
    t = q[-1]                # last column is tangent
    
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

        if printer is not None:
            printer(x)

        newT = q[-1]    # new tangent
        if sum(t * newT) < 0.0:
            # Bifurcation detected
            print "Detected bifurcation near %f" % x[-1]

            omega = -omega
        t = newT[:]
        
    return x
    

