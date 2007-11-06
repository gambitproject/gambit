#
# Compute all equilibria of a strategic game using support enumeration.
#
# This program enumerates all "admissible" supports in a strategic game.
# The main function, Enumerate(), does the enumeration, and hands off
# each nontrivial admissible support to the Solve() method of the
# solver object provided it.
#
# This program currently offers three solver objects:
# (1) A "null" solver object (so, for instance, one can just count supports)
# (2) A "PHCSolve" solver object, which solves the implied system using
#     PHCpack (external binary required)
# (3) A "Bertini" solver object, which solves the implied system using
#     Bertini (external binary required)
#

import gambit
import phc
import time, os


#############################################################################
# Generate polynomial equations for a support
#############################################################################

# Use this table to assign letters to player strategy variables
# We skip 'e' and 'i', because PHC doesn't allow these in variable names.
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


#############################################################################
# Utility routines
#############################################################################

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
        logger.OnNashSolution(profile)
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


#############################################################################
# Solver classes
#############################################################################

class SupportSolver:
    def __init__(self, logger):
        self.logger = logger

    def GetLogger(self):  return self.logger

class NullSolver(SupportSolver):
    def __init__(self, logger):
        SupportSolver.__init__(self, logger)

    def Solve(support):
        pass

#############################################################################
# Solver for solving support via PHCpack
#############################################################################

class PHCSolver(SupportSolver):
    def __init__(self, prefix, logger):
        SupportSolver.__init__(self, logger)
        self.prefix = prefix

    def GetFilePrefix(self):  return self.prefix

    def Solve(self, support):
        game = support.GetGame()
        
        eqns = reduce(lambda x, y: x + y,
                      [ Equations(support, player)
                        for player in game.Players() ])
    
        phcinput = ("%d\n" % len(eqns)) + "".join(eqns)
        #print phcinput
        
        try:
            phcoutput = phc.RunPHC("./phc", self.prefix, phcinput)
            #print "%d solutions found; checking for equilibria now" % len(phcoutput)
            for entry in phcoutput:
                CheckEquilibrium(game, support, entry, self.logger)

            #print "Equilibrium checking complete"
        except ValueError:
            self.logger.OnSingularSupport(support)
        except:
            self.logger.OnSingularSupport(support)


#############################################################################
# Solver for solving support via Bertini
#############################################################################

class BertiniSolver(SupportSolver):
    def __init__(self, logger, bertiniPath="./bertini"):
        SupportSolver.__init__(self, logger)
        self.bertiniPath = bertiniPath

    def GetBertiniPath(self):   return self.bertiniPath
    def SetBertiniPath(self, path):  self.bertiniPath = path

    def Solve(self, support):
        game = support.GetGame()
        
        eqns = reduce(lambda x, y: x + y,
                      [ Equations(support, player)
                        for player in game.Players() ])

        variables = [ ]
        for (pl, player) in enumerate(game.Players()):
            for st in xrange(support.NumStrategies(pl+1) - 1):
                variables.append("%c%d" % (playerletters[pl], st+1))
                           
        bertiniInput = "CONFIG\n\nEND;\n\nINPUT\n\n"
        bertiniInput += "variable_group %s;\n" % ", ".join(variables)
        bertiniInput += "function %s;\n\n" % ", ".join([ "e%d" % i
                                                         for i in xrange(len(eqns)) ])

        for (eq, eqn) in enumerate(eqns):
            bertiniInput += "e%d = %s\n" % (eq, eqn)

        bertiniInput += "\nEND;\n"

        #print bertiniInput

        infile = file("input", "w")
        infile.write(bertiniInput)
        infile.close()

        if os.system("%s > /dev/null" % self.bertiniPath) == 0:
            try:
                solutions = [ ]
                outfile = file("real_finite_solutions")
                lines = iter(outfile)

                lines.next()
                lines.next()
                while lines.next().strip() != "-1":
                    solution = { }
                    solution["vars"] = { }
                    for var in variables:
                        value = lines.next().split()
                        solution["vars"][var] = complex(float(value[0]),
                                                        float(value[1]))
                    #print solution
                    solutions.append(solution)
                outfile.close()
                os.remove("real_finite_solutions")
                for entry in solutions:
                    CheckEquilibrium(game, support, entry, self.logger)

            except IOError:
                #print "couldn't open real_finite_solutions"
                self.logger.OnSingularSupport(support)



#############################################################################
# Logger classes for storing and reporting output
#############################################################################

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


#############################################################################
# The main program: enumerate supports and pass them to a solver class
#############################################################################

#
# Compute the admissible supports such that all strategies in 'setin'
# are in the support, all the strategies in 'setout' are not in the
# support.
#
# setin: Set of strategies assumed "in" the support
# setout: Set of strategies assumed "outside" the support
# setfree: Set of strategies not yet allocated
# These form a partition of the set of all strategies
#
def AdmissibleSubsupports(game, setin, setout, setfree, skipdom = False):
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
            
        for subsupport in AdmissibleSubsupports(game,
                                                 setin,
                                                 subsetout + [subsetfree[0]],
                                                 subsetfree[1:]):
            yield subsupport

        for subsupport in AdmissibleSubsupports(game,
                                                 setin + [subsetfree[0]],
                                                 subsetout, subsetfree[1:]):
            yield subsupport


    raise StopIteration

def Enumerate(game, solver):
    game.BuildComputedValues()

    for support in AdmissibleSubsupports(game, [], [],
                                         [ strategy
                                           for player in game.Players()
                                           for strategy in player.Strategies() ]):
        solver.GetLogger().OnCandidateSupport(support)
        
        if IsPureSupport(support):
            # By definition, if this is a pure-strategy support, it
            # must be an equilibrium (because of the dominance
            # elimination process)
            solver.GetLogger().OnNashSolution(ProfileFromPureSupport(game, support))
        else:
            solver.Solve(support)

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
    sys.stderr.write("  -m               backend to use (null, phc, bertini)\n")
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
    backend = "phc"

    try:
        opts, args = getopt.getopt(sys.argv[1:], "p:n:t:hqvd:m:")
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
        elif o == "-m":
            if a in [ "null", "phc", "bertini" ]:
                backend = a
            else:
                sys.stderr.write("%s: unknown backend `%s' passed to `-m'\n" %
                                 (sys.argv[0], a))
                sys.exit(1)
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

    if backend == "bertini":
        Enumerate(game, solver=BertiniSolver(logger,
                                             bertiniPath="./bertini"))
    elif backend == "phc":
        Enumerate(game, solver=PHCSolver(prefix, logger))
    else:
        Enumerate(game, solver=NullSolver(logger))

    
