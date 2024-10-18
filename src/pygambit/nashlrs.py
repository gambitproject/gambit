"""Interface to external standalone tool lrsnash.
"""

from __future__ import annotations

import itertools
import pathlib
import subprocess
import sys

import pygambit as gbt
import pygambit.util as util


def _generate_lrs_input(game: gbt.Game) -> str:
    s = f"{len(game.players[0].strategies)} {len(game.players[1].strategies)}\n\n"
    for st1 in game.players[0].strategies:
        s += " ".join(str(gbt.Rational(game[st1, st2][game.players[0]]))
                      for st2 in game.players[1].strategies) + "\n"
    s += "\n"
    for st1 in game.players[0].strategies:
        s += " ".join(str(gbt.Rational(game[st1, st2][game.players[1]]))
                      for st2 in game.players[1].strategies) + "\n"
    return s


def _parse_lrs_output(game: gbt.Game, txt: str) -> list[gbt.MixedStrategyProfileRational]:
    data = "\n".join([x for x in txt.splitlines() if not x.startswith("*")]).strip()
    eqa = []
    for component in data.split("\n\n"):
        profiles = {"1": [], "2": []}
        for line in component.strip().splitlines():
            profiles[line[0]].append([gbt.Rational(x) for x in line[1:].strip().split()[:-1]])
        for p1, p2 in itertools.product(profiles["1"], profiles["2"]):
            eqa.append(game.mixed_strategy_profile([p1, p2], rational=True))
    return eqa


def lrsnash_solve(game: gbt.Game,
                  lrsnash_path: pathlib.Path | str) -> list[gbt.MixedStrategyProfileRational]:
    if len(game.players) != 2:
        raise RuntimeError("Method only valid for two-player games.")
    with util.make_temporary(_generate_lrs_input(game)) as infn:
        result = subprocess.run([lrsnash_path, infn], encoding="utf-8",
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            raise ValueError(f"PHC run failed with return code {result.returncode}")
        return _parse_lrs_output(game, result.stdout)


def main():
    game = gbt.Game.parse_game(sys.stdin.read())
    eqa = lrsnash_solve(game, "./lrsnash")
    for eqm in eqa:
        print("NE," +
              ",".join(str(eqm[strat]) for player in game.players for strat in player.strategies))


if __name__ == "__main__":
    main()
