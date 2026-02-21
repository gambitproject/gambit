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


def _run_phc(phcpack_path: pathlib.Path | str, phc_input_string: str) -> list[dict]:
    """Call PHCpack via an external binary to solve a set of equations, and return
    the details on solutions found.

    Parameters
    ----------
    phcpack_path : pathlib.Path or str
        The path to the PHC program binary

    phc_input_string : str
        The exact formatted string representing the polynomial equations for PHCpack.

    Returns
    -------
    A list of dictionaries, each representing one solution.
    """
    with (
            util.make_temporary(phc_input_string) as infn,
            util.make_temporary() as outfn
    ):
        result = subprocess.run([phcpack_path, "-b", infn, outfn])
        if result.returncode != 0:
            raise ValueError(f"PHC run failed with return code {result.returncode}")
        with outfn.open() as outfile:
            return _process_phc_output(outfile.read())


# Use this table to assign letters to player strategy variables
# Skip 'e', 'i', and 'j', because PHC doesn't allow these in variable names.
_playerletters = [c for c in string.ascii_lowercase if c not in ["e", "i", "j"]]


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

def generate_phc_input(support: gbt.StrategySupportProfile) -> tuple[str, dict]:
    """
    Generates the PHCpack input string and the strategy-to-variable mapping.
    """
    # 1. Generate the polynomial equations
    eqns = [eqn
            for player in support.game.players
            for eqn in _equilibrium_equations(support, player)]

    # 2. Format the exact string PHCpack expects
    phc_input_string = f"{len(eqns)}\n" + ";\n".join(eqns) + ";\n\n\n"

    # 3. Create the mapping:
    mapping = {}
    for player in support.game.players:
        playerchar = _playerletters[player.number]
        for strategy in player.strategies:
            var_name = playerchar + str(strategy.number)
            mapping[var_name] = strategy

    return phc_input_string, mapping

def _is_nash(profile: gbt.MixedStrategyProfile, maxregret: float, negtol: float) -> bool:
    """Check if the profile is an (approximate) Nash equilibrium, allowing a maximum
    regret of `maxregret` and a tolerance of (small) negative probabilities of `negtol`."""
    for player in profile.game.players:
        for strategy in player.strategies:
            if profile[strategy] < -negtol:
                return False
    return profile.max_regret() < maxregret


def _solution_to_profile(game: gbt.Game, entry: dict, strategy_mapping: dict) -> gbt.MixedStrategyProfileDouble:
    """Converts a parsed PHCpack solution dictionary into a Gambit MixedStrategyProfile
    using the provided strategy mapping.
    """
    profile = game.mixed_strategy_profile()

    # Initialize all strategy probabilities to 0.0 first
    for player in game.players:
        for strategy in player.strategies:
            profile[strategy] = 0.0

    # Apply the parsed probabilities using the mapping
    for var_name, strategy in strategy_mapping.items():
        if var_name in entry["vars"]:
            profile[strategy] = entry["vars"][var_name].real

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
        # 1. Generate the formatted input string and the strategy mapping
        phc_input_string, mapping = generate_phc_input(support)

        try:
            # 2. Run PHCpack using the pre-formatted string
            parsed_solutions = _run_phc(phcpack_path, phc_input_string)

            # 3. Convert parsed solutions to Gambit profiles using the mapping
            profiles = [
                _solution_to_profile(support.game, entry, mapping)
                for entry in parsed_solutions
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


def _read_game(fn: str) -> gbt.Game:
    for reader in [gbt.read_efg, gbt.read_nfg, gbt.read_agg]:
        try:
            return reader(fn)
        except Exception:
            pass
    raise OSError(f"Unable to read or parse {fn}")


def main():
    game = _read_game(sys.argv[1])
    phcpack_solve(game, "./phc", maxregret=1.0e-6)


if __name__ == "__main__":
    main()
