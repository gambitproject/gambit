from gbt import *

#
# Wrappers to convert from internal profile list to Python list of profiles
#
def ProfilesToList(list):
    ret = [ ]
    for i in xrange(0, len(list)):  ret.append(list[i])
    return ret

def NashEnumPolyNfg(game):
    return ProfilesToList(EnumPolyNfgHelper(game))

def NashEnumPureNfg(game, stopAfter=0):
    return ProfilesToList(EnumPureNfgHelper(game, stopAfter))

def NashEnumMixedNfg(game, useRational=True, stopAfter=0):
    if useRational:
        return ProfilesToList(EnumMixedNfgRationalHelper(game, stopAfter))
    else:
        return ProfilesToList(EnumMixedNfgDoubleHelper(game, stopAfter))

def NashLcpNfg(game, useRational=True, stopAfter=0, maxDepth=0):
    if useRational:
        return ProfilesToList(LcpNfgRationalHelper(game, stopAfter, maxDepth))
    else:
        return ProfilesToList(LcpNfgDoubleHelper(game, stopAfter, maxDepth))

def NashLpNfg(game, useRational=True):
    if useRational:
        return ProfilesToList(LpNfgRationalHelper(game))
    else:
        return ProfilesToList(LpNfgDoubleHelper(game))

def NashLiapNfg(profile, maxitsN=100, tolN=1.0e8, maxits1=100, tol1=.0001):
    return ProfilesToList(LiapNfgHelper(profile, maxitsN, tolN, maxits1, tol1))

def NashLogitNfg(game):
    return ProfilesToList(LogitNfgHelper(game))

def NashSimpdivNfg(start, useRational=True, leashLength=0):
    if start.IsRational():
        return ProfilesToList(SimpdivNfgRationalHelper(start, leashLength))
    else:
        return ProfilesToList(SimpdivNfgDoubleHelper(start, leashLength))

def NashYamamotoNfg(game):
    return ProfilesToList(YamamotoNfgHelper(game))


def NashEnumPolyEfg(game):
    return ProfilesToList(EnumPolyEfgHelper(game))

def NashEnumPureEfg(game, stopAfter=0):
    return ProfilesToList(EnumPureEfgHelper(game, stopAfter))

def NashLcpEfg(game, useRational=True, stopAfter=0, maxDepth=0):
    if useRational:
        return ProfilesToList(LcpEfgRationalHelper(game, stopAfter, maxDepth))
    else:
        return ProfilesToList(LcpEfgDoubleHelper(game, stopAfter, maxDepth))

def NashLpEfg(game, useRational=True):
    if useRational:
        return ProfilesToList(LpEfgRationalHelper(game))
    else:
        return ProfilesToList(LpEfgDoubleHelper(game))

def NashLiapEfg(profile, maxitsN=100, tolN=1.0e-8,
                maxits1=100, tol1=.0001):
    return ProfilesToList(LiapEfgHelper(profile, maxitsN, tolN, maxits1, tol1))

def NashLogitEfg(game):
    return ProfilesToList(LogitEfgHelper(game))





