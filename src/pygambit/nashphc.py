"""Enumerate Nash equilibria by solving systems of polynomial equations using PHCpack.
"""

from __future__ import annotations

import contextlib
import itertools
import pathlib
import string
import subprocess
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
_playerletters = [c for c in string.ascii_lowercase if c not in ("e", "i", "j")]


def _strategy_index(game: gbt.Game, player: str, label: str) -> int:
    """The index of the strategy labeled `label` within `player`'s full strategy list.

    This is the basis of the PHC variable-naming scheme (player letter + this index),
    which must stay stable across supports, so it is always computed against the full
    list of the player's strategies, never a support-restricted subset.
    """
    return game.get_strategies(player).index(label)


def _contingencies(
        support: gbt.StrategySupportProfile,
        skip_player: str
) -> typing.Generator[list[str | None], None, None]:
    """Generate all contingencies of strategy labels in `support` for all players
    except player `skip_player`, whose entry is `None`.
    """
    game = support.game
    for profile in itertools.product(
        *[[strategy for strategy in game.get_strategies(player) if strategy in support[player]]
          if player != skip_player else [None]
          for player in game.players]
    ):
        yield list(profile)


def _equilibrium_equations(support: gbt.StrategySupportProfile, player: str) -> list:
    """Generate the equations that the strategy of `player` must satisfy in any
    totally-mixed equilibrium on `support`.
    """
    game = support.game
    players = list(game.players)
    player_index = {p: i for i, p in enumerate(players)}
    player_support = support[player]
    payoffs = {
        strategy: [] for strategy in game.get_strategies(player) if strategy in player_support
    }

    strategies = list(player_support)
    for profile in _contingencies(support, player):
        contingency = "*".join(
            f"{_playerletters[player_index[p]]}{_strategy_index(game, p, strat)}"
            for p, strat in zip(players, profile, strict=True) if strat is not None
        )
        for strategy in strategies:
            profile[player_index[player]] = strategy
            payoff_vec = game.get_payoffs(
                {p: strat for p, strat in zip(players, profile, strict=True)}
            )
            if payoff_vec[player] != 0:
                payoffs[strategy].append(f"({payoff_vec[player]}*{contingency})")

    payoffs = {s: "+".join(v) for s, v in payoffs.items()}
    equations = [f"({payoffs[strategies[0]]})-({payoffs[s]})" for s in strategies[1:]]
    equations.append(
        "+".join(_playerletters[player_index[player]] + str(_strategy_index(game, player, s))
                 for s in strategies) + "-1"
    )
    return equations


def _is_nash(profile: gbt.MixedStrategyProfile, maxregret: float, negtol: float) -> bool:
    """Check if the profile is an (approximate) Nash equilibrium, allowing a maximum
    regret of `maxregret` and a tolerance of (small) negative probabilities of `negtol`."""
    for player in profile.game.players:
        for strategy in profile.game.get_strategies(player):
            if profile[player][strategy] < -negtol:
                return False
    return profile.max_regret() < maxregret


def _solution_to_profile(game: gbt.Game, entry: dict) -> gbt.MixedStrategyProfileDouble:
    profile = game.mixed_strategy_profile()
    for i, player in enumerate(game.players):
        playerchar = _playerletters[i]
        distribution = {}
        for j, strategy in enumerate(game.get_strategies(player)):
            try:
                distribution[strategy] = entry["vars"][playerchar + str(j)].real
            except KeyError:
                distribution[strategy] = 0.0
        profile[player] = distribution
    return profile


def _format_support(support, label: str) -> str:
    game = support.game
    strings = [
        "".join(str(int(strategy in support[player])) for strategy in game.get_strategies(player))
        for player in game.players
    ]
    return label + "," + ",".join(strings)


def _format_profile(profile: gbt.MixedStrategyProfileDouble, label: str,
                    decimals: int = 6) -> str:
    """Render the mixed strategy profile `profile` to a one-line string with the given
    `label`.
    """
    game = profile.game
    return (f"{label}," +
            ",".join(["{p:.{decimals}f}".format(p=profile[player][s], decimals=decimals)
                      for player in game.players for s in game.get_strategies(player)]))


def _profile_from_support(support: gbt.StrategySupportProfile) -> gbt.MixedStrategyProfileDouble:
    """Construct a mixed strategy profile corresponding to the (pure strategy) equilibrium
    on `support`.
    """
    game = support.game
    profile = game.mixed_strategy_profile()
    for player in game.players:
        player_support = support[player]
        profile[player] = {
            strategy: (1.0 if strategy in player_support else 0.0)
            for strategy in game.get_strategies(player)
        }
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
