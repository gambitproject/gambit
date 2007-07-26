#
# Compute all equilibria using support enumeration with PHCPACK as a backend.
#

import gambit
import phc
import time

###########################################################################
# Enumeration of supports: The most naive approach
###########################################################################

# We implement a very simple support enumeration.  The idea is we can
# identify the strategies a player has in a support by an integer: if
# a player has J strategies, then we can identify the strategies by
# a number between 1 and 2**J.  In this case, bitwise operations tell
# us which strategies are in the support; the least-significant bit is
# 1 if the first strategy is in the support and 0 if not; the next
# least-significant is 1 if the second strategy is in the support, and 0
# if not; and so forth.

# IndexToSupport maps a list of these indices into the corresponding
# support object on a game
def IndexToSupport(game, strats):
    support = gambit.StrategySupport(game)

    for pl in xrange(game.NumPlayers()):
        player = game.GetPlayer(pl+1)
        for st in xrange(player.NumStrategies()):
            if not (strats[pl] & (1 << st)):
                support.RemoveStrategy(player.GetStrategy(st+1))

    return support

# EnumerateSupports is a recursive Python generator which creates all
# possible supports.  Start the recursion by calling it with
# player == 1 and sofar == [].
def EnumerateSupports(game, player, sofar):
    for supportId in xrange(1, 2**(game.GetPlayer(player).NumStrategies())):
        strats = sofar + [ supportId ]
        if player == game.NumPlayers():
            yield IndexToSupport(game, strats)
        else:
            for support in EnumerateSupports(game, player+1, strats):
                yield support
    raise StopIteration


###########################################################################
# Enumeration of supports: A more sophisticated approach
###########################################################################


# setin: Set of strategies assumed "in" the support
# setout: Set of strategies assumed "outside" the support
# setfree: Set of strategies not yet allocated
# These form a partition of the set of all strategies
def UndominatedSubsupports(game, setin, setout, setfree, skipdom = False):
    #print time.time(), len(setin), len(setout)
    support = gambit.StrategySupport(game)
    for strategy in setout:
        if not support.RemoveStrategy(strategy):
            # If the removal fails, it means we have no strategies
            # for this player; we can stop
            raise StopIteration
    if setfree == []:
        # We have assigned all strategies.  Now check for dominance.
        # Note that we check these "by hand" because when eliminating
        # by "external" dominance, the last strategy for a player
        # won't be eliminated, even if it should, because RemoveStrategy()
        # will not allow the last strategy for a player to be removed.
        # Need to consider whether the API should be modified for this.
        for player in game.Players():
            for strategy in support.Strategies(player):
                if support.IsDominated(strategy, True, True):
                    raise StopIteration
        yield support
    else:
        #print "Starting iterated dominance"
        if not skipdom:
            while True:
                newsupport = support.Undominated(True, True)
                if newsupport == support: break
                support = newsupport
            #print "Done with iterated dominance"

            for strategy in setin:
                if not support.Contains(strategy):
                    # We dropped a strategy already assigned as "in":
                    # we can terminate this branch of the search
                    raise StopIteration

            subsetout = setout[:]
            subsetfree = setfree[:]

            for strategy in setfree:
                if not newsupport.Contains(strategy):
                    subsetout.append(strategy)
                    subsetfree.remove(strategy)

        else:
            subsetout = setout[:]
            subsetfree = setfree[:]
            
        # Switching the order of the following two calls (roughly)
        # switches whether supports are tried largest first, or
        # smallest first
        # Currently: smallest first

        # When we add a strategy to 'setin', we can skip the dominance
        # check at the next iteration, because it will return the same
        # result as here
            
        for subsupport in UndominatedSubsupports(game,
                                                 setin,
                                                 subsetout + [subsetfree[0]],
                                                 subsetfree[1:]):
            yield subsupport

        for subsupport in UndominatedSubsupports(game,
                                                 setin + [subsetfree[0]],
                                                 subsetout, subsetfree[1:]):
            yield subsupport


    raise StopIteration

# PHC doesn't allow some playerletters in variable names; we use this table
# to assign playerletters to player strategy variables
playerletters = [ 'a', 'b', 'c', 'd', 'f', 'g', 'h', 'k', 'l', 'm',
                  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
                  'x', 'y', 'z' ]


def Equations(support, player):
    payoffs = [ [] for strategy in support.Strategies(player) ]

    players = [ (pl, oplayer)
                for (pl, oplayer) in enumerate(support.GetGame().Players())
                if player != oplayer ]
    strategies = [ (st, strategy) 
                   for (st, strategy) in enumerate(support.Strategies(player)) ]
    for profile in gambit.StrategyIterator(support,
                                           support.GetStrategy(player.GetNumber(),
                                                               1)):
        contingency = "*".join([playerletters[pl] +
                                "%d" %
                                (profile.GetStrategy(oplayer).GetNumber()-1)
                                for (pl, oplayer) in players])
        for (st, strategy) in strategies:
            payoffs[st].append("(%s*%s)" %
                               (str(float(profile.GetStrategyValue(strategy))),
                                contingency))

    equations = [ "(%s)-(%s);\n" % ("+".join(payoffs[0]), "+".join(s2))
                  for s2 in payoffs[1:] ]
        
    # Substitute in sum-to-one constraints
    for (pl, oplayer) in enumerate(support.GetGame().Players()):
        if player == oplayer: continue
        playerchar = playerletters[pl]
        if support.NumStrategies(pl+1) == 1:
            for (i, equation) in enumerate(equations):
                equations[i] = equation.replace("*%s%d" % (playerchar,
                                                           support.GetStrategy(pl+1, 1).GetNumber()-1),
                                                "")
        else:
            subvar = "1-" + "-".join( playerchar+"%d" % (strategy.GetNumber() - 1)
                                      for (st, strategy) in enumerate(support.Strategies(oplayer))
                                      if st != 0 )
            for (i, equation) in enumerate(equations):
                equations[i] = equation.replace("%s%d" % (playerchar,
                                                          support.GetStrategy(pl+1, 1).GetNumber()-1),
                                                "("+subvar+")")

    return equations

try:
    #import scipy

    def LiapValue(support, x):
        profile = support.NewMixedStrategyDouble()
        for i in xrange(1, len(x)+1):
            profile[i] = x[i-1]
        return profile.GetLiapValue()

    def CheckEquilibrium(game, support, entry, logger):
        profile = support.NewMixedStrategyDouble()

        for (pl, player) in enumerate(game.Players()):
            playerchar = playerletters[pl]
            total = 0.0
            for (st, strategy) in enumerate(support.Strategies(player)):
                try:
                    profile[strategy] = entry["vars"][playerchar + str(strategy.GetNumber()-1)].real
                except KeyError:
                    profile[strategy] = 0.0
                total += profile[strategy]

            profile[support.GetStrategy(pl+1, 1)] = 1.0 - total


        if profile.ToFullSupport().GetLiapValue() < 1.0e-10:
            logger.OnNashSolution(profile.ToFullSupport())
            return
        else:
            logger.OnNonNashSolution(profile.ToFullSupport())
            return

        #print "Got profile with liap value %f, proceeding to polish" % profile.GetLiapValue()
        x0 = scipy.array([ profile[i] for i in xrange(1, profile.Length()+1) ])
        #print x0
        x1 = scipy.optimize.optimize.fmin_bfgs(lambda x: LiapValue(support, x), x0, disp=0, gtol=1.0e-6)
        for i in xrange(1, profile.MixedProfileLength()+1):
            profile[i] = float(x1[i-1])
        if profile.ToFullSupport().GetLiapValue() < 1.0e-10:
            logger.OnNashSolution(profile.ToFullSupport())
        else:
            logger.OnNonNashSolution(profile.ToFullSupport())

except ImportError:
    def IsNash(profile, vars):
        for i in xrange(1, profile.MixedProfileLength()+1):
            if profile[i] < -negTolerance: return False

        for (pl, player) in enumerate(profile.GetGame().Players()):
            playerchar = playerletters[pl]
            payoff = profile.GetPayoff(player)
            total = 0.0
            for (st, strategy) in enumerate(player.Strategies()):
                if "%s%d" % (playerchar, st) not in vars.keys():
                    if profile.GetStrategyValue(strategy) - payoff > payoffTolerance:
                        return False
                else:
                    total += vars["%s%d" % (playerchar, st)].real
        
        return True

    def CheckEquilibrium(game, support, entry, logger):
        profile = game.NewMixedStrategyDouble()

        index = 1

        for (pl, player) in enumerate(game.Players()):
            playerchar = playerletters[pl]
            total = 0.0
            for (st, strategy) in enumerate(player.Strategies()):
                try:
                    profile[index] = entry["vars"][playerchar + str(st)].real
                except KeyError:
                    profile[index] = 0.0
                total += profile[index]
                index += 1

            profile[support.GetStrategy(pl+1, 1).GetId()] = 1.0 - total
            entry["vars"][playerchar + str(support.GetStrategy(pl+1,1).GetNumber()-1)] = complex(1.0 - total)

        x = ",".join([ "%f" % profile[i]
                       for i in xrange(1, game.MixedProfileLength() + 1)])
        if IsNash(profile, entry["vars"]):
            logger.OnNonNashSolution(profile)
        else:
            logger.OnNonNashSolution(profile)


def IsPureSupport(support):
    return reduce(lambda x, y: x and y,
                  [ support.NumStrategies(pl+1) == 1
                    for (pl, player) in enumerate(support.Players()) ])
                 
def ProfileFromPureSupport(game, support):
    profile = game.NewMixedStrategyDouble()

    for index in xrange(1, game.MixedProfileLength() + 1):
        profile[index] = 0.0

    for (pl, player) in enumerate(support.Players()):
        profile[support.GetStrategy(pl+1, 1).GetId()] = 1.0

    return profile

def EnumViaPHC(game, prefix, logger, solve=True):
    game.BuildComputedValues()

    for support in UndominatedSubsupports(game, [], [],
                                          [ strategy
                                            for player in game.Players()
                                            for strategy in player.Strategies() ]):
        logger.OnCandidateSupport(support)

        if not solve: continue

        if IsPureSupport(support):
            # By definition, if this is a pure-strategy support, it
            # must be an equilibrium (because of the dominance
            # elimination process)
            logger.OnNashSolution(ProfileFromPureSupport(game, support))
            continue

        eqns = reduce(lambda x, y: x + y,
                      [ Equations(support, player)
                        for player in game.Players() ])
    
        phcinput = ("%d\n" % len(eqns)) + "".join(eqns)
        #print phcinput
        
        try:
            phcoutput = phc.RunPHC("./phc", prefix, phcinput)
            #print "%d solutions found; checking for equilibria now" % len(phcoutput)
            for entry in phcoutput:
                CheckEquilibrium(game, support, entry, logger)

            #print "Equilibrium checking complete"
        except ValueError:
            logger.OnSingularSupport(support)
        except:
            logger.OnSingularSupport(support)


class NullLogger:
    def OnCandidateSupport(self, support): pass
    def OnSingularSupport(self, support): pass
    def OnNashSolution(self, profile): pass
    def OnNonNashSolution(self, profile): pass

class StandardLogger(NullLogger):
    def OnNashSolution(self, profile):
        print "NE," + ",".join(["%f" % max(profile[i], 0.0)
                                for i in xrange(1, profile.MixedProfileLength() + 1)])

class VerboseLogger(StandardLogger):
    def PrintSupport(self, support, label):
        strings = [ reduce(lambda x, y: x + y,
                           [ str(int(support.Contains(strategy)))
                             for strategy in player.Strategies() ])
                    for player in support.Players() ]
        print label + "," + ",".join(strings)

    def OnCandidateSupport(self, support):
        self.PrintSupport(support, "candidate")
    def OnSingularSupport(self, support):
        self.PrintSupport(support, "singular")

    def OnNonNashSolution(self, profile):
        print ("noneqm," +
               ",".join([str(profile[i])
                         for i in xrange(1, profile.MixedProfileLength() + 1)]) +
               "," + str(profile.GetLiapValue()))
                                
        
class CountLogger:
    def __init__(self):
        self.candidates = 0
        self.singulars = 0
        self.nash = 0
        self.nonnash = 0

    def OnCandidateSupport(self, support): self.candidates += 1
    def OnSingularSupport(self, support):  self.singulars += 1
    def OnNashSolution(self, profile):     self.nash += 1
    def OnNonNashSolution(self, profile):  self.nonnash += 1



########################################################################
# Instrumentation for standalone use
########################################################################

def PrintBanner(f):
    f.write("Compute Nash equilibria by solving polynomial systems\n")
    f.write("(solved using Jan Verschelde's PHCPACK solver)\n")
    f.write("Gambit version 0.2007.03.12, Copyright (C) 2007, The Gambit Project\n")
    f.write("This is free software, distributed under the GNU GPL\n\n")

def PrintHelp(progname):
    PrintBanner(sys.stderr)
    sys.stderr.write("Usage: %s [OPTIONS]\n" % progname)
    sys.stderr.write("Accepts game on standard input.\n")
    sys.stderr.write("With no options, reports all Nash equilibria found.\n\n")
    sys.stderr.write("Options:\n")
    sys.stderr.write("  -h               print this help message\n")
    sys.stderr.write("  -e               enumerate supports only\n")
    sys.stderr.write("  -n               tolerance for negative probabilities (default 0)\n")
    sys.stderr.write("  -p               prefix to use for filename in calling PHC\n")
    sys.stderr.write("  -q               quiet mode (suppresses banner)\n")
    sys.stderr.write("  -t               tolerance for non-best-response payoff (default 1.0e-6)\n")
    sys.stderr.write("  -v               verbose mode (shows supports investigated)\n")
    sys.stderr.write("                   (default is only to show equilibria)\n")
    sys.exit(1)

payoffTolerance = 1.0e-6
negTolerance = 0.0

if __name__ == '__main__':
    import getopt, sys

    verbose = False
    quiet = False
    prefix = "blek"
    solve = True

    try:
        opts, args = getopt.getopt(sys.argv[1:], "p:n:t:hqvd:e")
    except getopt.GetoptError:
        PrintHelp(sys.argv[0])

    for (o, a) in opts:
        if o == "-h":
            PrintHelp(sys.argv[0])
        elif o == "-d":
            pass
        elif o == "-v":
            verbose = True
        elif o == "-q":
            quiet = True
        elif o == "-p":
            prefix = a
        elif o == '-e':
            # Enumerate only
            solve = False
        elif o == "-n":
            try:
                negTolerance = float(a)
            except ValueError:
                sys.stderr.write("%s: `-n' option expects a floating-point number\n" % sys.argv[0])
                sys.exit(1)
        elif o == "-t":
            try:
                payoffTolerance = float(a)
            except ValueError:
                sys.stderr.write("%s: `-t' option expects a floating-point number\n" % sys.argv[0])
        else:
            sys.stderr.write("%s: Unknown option `-%s'.\n" %
                             (sys.argv[0], o))
            sys.exit(1)

    if not quiet:
        PrintBanner(sys.stderr)

    game = gambit.ReadGame(sys.stdin.read())
    game.BuildComputedValues()
    if verbose:
        logger = VerboseLogger()
    else:
        logger = StandardLogger()
    EnumViaPHC(game, prefix, logger, solve=solve)

    
