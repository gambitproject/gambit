"""
A set of utilities for computing Nash equilibria
"""

import os, sys
import libgambit

def launch_external_program(prog, game):
    """
    Helper function for launching calls to external programs.
    Calls the specified program 'prog', passing the game to standard
    input in .efg format (if a tree) or .nfg format (if a table).
    Returns the object referencing standard output of the external program.
    """
    child_stdin, child_stdout = os.popen2("%s -q" % prog)
    if game.IsTree():
        child_stdin.write(game.efg_file())
    else:
        child_stdin.write(game.nfg_file())
    # Need to close, or at least flush, stdin of the child, or else
    # processing won't begin...
    child_stdin.close()

    return child_stdout



def enummixed(game, rational=False):
    """
    Compute all equilibria of a two-player game by enumeration of extreme
    points of the equilibrium set.
    """
    profiles = [ ]
    game.BuildComputedValues()

    commandLine = "gambit-enummixed"
    if not rational:
        commandLine += " -d 10"
    
    for line in launch_external_program(commandLine, game):
        entries = line.split(",")
        
        profile = game.mixed_strategy(rational=rational)
        for (i, p) in enumerate(entries[1:]):
            if rational:
                profile[i] = libgambit.Rational(p)
            else:
                profile[i] = float(p)
                
        profiles.append(profile)

    return profiles

def simpdiv(game):
    """
    Compute one equilibrium of an N-player game using simplicial subdivision.
    """
    profiles = [ ]
    game.BuildComputedValues()

    commandLine = "gambit-simpdiv"
    
    for line in launch_external_program(commandLine, game):
        entries = line.split(",")
        
        profile = game.mixed_strategy(rational=True)
        for (i, p) in enumerate(entries[1:]):
            profile[i] = libgambit.Rational(p)
        profiles.append(profile)

    return profiles

