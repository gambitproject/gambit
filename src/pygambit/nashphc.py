"""Enumerate Nash equilibria by solving systems of polynomial equations using PHCpack.
"""

from __future__ import annotations

import contextlib
import itertools
import pathlib
import string
import subprocess
import sys
import typing

import pygambit as gbt
import pygambit.util as util


def _process_phc_output(output: str) -> list[dict]:
    """Parse the output file from a run of PHC pack and produce a list of dictionaries,
    with each element in the list corresponding to one of the solutions found.
    """
    startsol = output.find("THE SOLUTIONS :\n\n")
    if startsol == -1:
        startsol = output.find("THE SOLUTIONS :\n")

    solns = output[startsol:]
    firstequals = solns.find("solution")
    firstcut = solns[firstequals:]

    secondequals = firstcut.find("=====")
    if secondequals >= 0:
        secondcut = firstcut[:secondequals]
    else:
        secondequals = firstcut.find("TIMING")
        secondcut = firstcut[:secondequals]

    solutions = []
    for line in secondcut.split("\n"):
        tokens = [x.strip() for x in line.split() if x and not x.isspace()]

        if not tokens:
            continue

        if tokens[0] == "solution":
            if len(tokens) == 3:
                # This is a solution that didn't involve iteration
                solutions.append({"vars": {}})
            else:
                solutions.append({
                    "startresidual": float(tokens[6]),
                    "iterations":    int(tokens[9]),
                    "result":        tokens[10],
                    "vars":          {}
                })
        elif tokens[0] == "t":
            solutions[-1]["t"] = complex(float(tokens[2]),
                                         float(tokens[3]))
        elif tokens[0] == "m":
            solutions[-1]["m"] = int(tokens[2])
        elif tokens[0] == "the":
            pass
        elif tokens[0] == "==":
            solutions[-1]["err"] = float(tokens[3])
            solutions[-1]["rco"] = float(tokens[7])
            solutions[-1]["res"] = float(tokens[11])
            with contextlib.suppress(IndexError):
                # Some solutions don't have type information
                solutions[-1]["type"] = " ".join([tokens[13], tokens[14]])
        else:
            # This is a solution line
            solutions[-1]["vars"][tokens[0]] = complex(float(tokens[2]),
                                                       float(tokens[3]))
    return solutions


def _run_phc(phcpack_path: pathlib.Path | str, equations: list[str]) -> list[dict]:
    """Call PHCpack via an external binary to solve a set of equations, and return
    the details on solutions found.

    Parameters
    ----------
    phcpack_path : pathlib.Path or str
        The path to the PHC program binary

    equations : list[str]
        The set of equations to solve, expressed as string representations of the
        polynomials.

    Returns
    -------
    A list of dictionaries, each representing one solution.  Each dictionary
    may contain the following keys:
        - `vars': A dictionary whose keys are the variable names,
                  and whose values are the solution values, represented
                  using the Python `complex` type.
        - `type': The text string PHCpack emits describing the output
                  (e.g., "no solution", "real regular", etc.)
        - `startresidual'
        - `iterations'
        - `result'
        - `t'
        - `m'
        - `err'
        - `rco'
        - `res'
    """
    with (
            util.make_temporary(f"{len(equations)}\n" +
                                ";\n".join(equations) + ";\n\n\n") as infn,
            util.make_temporary() as outfn
    ):
        result = subprocess.run([phcpack_path, "-b", infn, outfn])
        if result.returncode != 0:
            raise ValueError(f"PHC run failed with return code {result.returncode}")
        with outfn.open() as outfile:
            return _process_phc_output(outfile.read())


# Use this table to assign letters to player strategy variables
# Skip 'e', 'i', and 'j', because PHC doesn't allow these in variable names.
_playerletters = [c for c in string.ascii_lowercase if c != ["e", "i", "j"]]


def _contingencies(
        support: gbt.StrategySupportProfile,
        skip_player: gbt.Player
) -> typing.Generator[list[gbt.Strategy], None, None]:
    """Generate all contingencies of strategies in `support` for all players
    except player `skip_player`.
    """
    for profile in itertools.product(
        *[[strategy for strategy in player.strategies if strategy in support]
          if player != skip_player else [None]
          for player in support.game.players]
    ):
        yield list(profile)


def _equilibrium_equations(support: gbt.StrategySupportProfile, player: gbt.Player) -> list:
    """Generate the equations that the strategy of `player` must satisfy in any
    totally-mixed equilibrium on `support`.
    """
    payoffs = {strategy: [] for strategy in player.strategies if strategy in support}

    strategies = list(support[player])
    for profile in _contingencies(support, player):
        contingency = "*".join(f"{_playerletters[strat.player.number]}{strat.number}"
                               for strat in profile if strat is not None)
        for strategy in strategies:
            profile[player.number] = strategy
            if support.game[profile][player] != 0:
                payoffs[strategy].append(f"({support.game[profile][player]}*{contingency})")

    payoffs = {s: "+".join(v) for s, v in payoffs.items()}
    equations = [f"({payoffs[strategies[0]]})-({payoffs[s]})" for s in strategies[1:]]
    equations.append("+".join(_playerletters[player.number] + str(strat.number)
                              for strat in strategies) + "-1")
    return equations


def _is_nash(profile: gbt.MixedStrategyProfile, maxregret: float, negtol: float) -> bool:
    """Check if the profile is an (approximate) Nash equilibrium, allowing a maximum
    regret of `maxregret` and a tolerance of (small) negative probabilities of `negtol`."""
    for player in profile.game.players:
        for strategy in player.strategies:
            if profile[strategy] < -negtol:
                return False
    return profile.max_regret() < maxregret


def _solution_to_profile(game: gbt.Game, entry: dict) -> gbt.MixedStrategyProfileDouble:
    profile = game.mixed_strategy_profile()
    for player in game.players:
        playerchar = _playerletters[player.number]
        for strategy in player.strategies:
            try:
                profile[strategy] = entry["vars"][playerchar + str(strategy.number)].real
            except KeyError:
                profile[strategy] = 0.0
    return profile


def _format_support(support, label: str) -> str:
    strings = ["".join(str(int(strategy in support)) for strategy in player.strategies)
               for player in support.game.players]
    return label + "," + ",".join(strings)


def _format_profile(profile: gbt.MixedStrategyProfileDouble, label: str,
                    decimals: int = 6) -> str:
    """Render the mixed strategy profile `profile` to a one-line string with the given
    `label`.
    """
    return (f"{label}," +
            ",".join(["{p:.{decimals}f}".format(p=profile[s], decimals=decimals)
                      for player in profile.game.players for s in player.strategies]))


def _profile_from_support(support: gbt.StrategySupportProfile) -> gbt.MixedStrategyProfileDouble:
    """Construct a mixed strategy profile corresponding to the (pure strategy) equilibrium
    on `support`.
    """
    profile = support.game.mixed_strategy_profile()
    for player in support.game.players:
        for strategy in player.strategies:
            profile[strategy] = 1.0 if strategy in support else 0.0
    return profile


def _solve_support(support: gbt.StrategySupportProfile,
                   phcpack_path: pathlib.Path | str,
                   maxregret: float,
                   negtol: float,
                   onsupport=lambda x, label: None,
                   onequilibrium=lambda x, label: None) -> list[gbt.MixedStrategyProfileDouble]:
    onsupport(support, "candidate")
    if len(support) == len(support.game.players):
        profiles = [_profile_from_support(support)]
    else:
        eqns = [eqn
                for player in support.game.players
                for eqn in _equilibrium_equations(support, player)]
        try:
            profiles = [
                _solution_to_profile(support.game, entry)
                for entry in _run_phc(phcpack_path, eqns)
            ]
        except ValueError:
            onsupport(support, "singular")
            profiles = []
        except Exception:
            onsupport(support, "singular")
            raise
    profiles = [p for p in profiles if _is_nash(p, maxregret, negtol)]
    for profile in profiles:
        onequilibrium(profile, "NE")
    return profiles


def phcpack_solve(game: gbt.Game, phcpack_path: pathlib.Path | str,
                  maxregret: float) -> list[gbt.MixedStrategyProfileDouble]:
    negtol = 1.0e-6
    return [
        eqm
        for support in gbt.nash.possible_nash_supports(game)
        for eqm in _solve_support(support, phcpack_path, maxregret, negtol)
    ]


def main():
    game = gbt.Game.parse_game(sys.stdin.read())
    phcpack_solve(game, "./phc", maxregret=1.0e-6)


if __name__ == "__main__":
    main()
