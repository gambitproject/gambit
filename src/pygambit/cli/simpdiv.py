#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/simpdiv.py
# Command-line driver program for Nash equilibrium computation via
# simplicial subdivision
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
"""Command-line driver program for Nash equilibrium computation via simplicial
subdivision.
"""

from __future__ import annotations

import click
import numpy as np

import pygambit as gbt

from .common import (
    handle_errors,
    open_game_file,
    print_banner,
    read_game,
    read_strategy_profiles_csv,
    render_profile_csv,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria using simplicial subdivision"
PROG_NAME = "gambit-simpdiv"


def _default_start(game: gbt.Game) -> gbt.MixedStrategyProfileRational:
    """Each player's first strategy, matching the C++ library's `SimpdivDefaultStart`."""
    start = game.mixed_strategy_profile(rational=True)
    for player in game.players:
        first_strategy = next(iter(player.strategies))
        start[player.label] = {first_strategy.label: 1}
    return start


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "With no options, computes one approximate Nash equilibrium."
    ),
)
@click.argument("file", required=False, default=None)
@click.option(
    "-g",
    "refine",
    default=2,
    show_default=True,
    type=int,
    help="granularity of grid refinement at each step",
)
@click.option(
    "-r",
    "rand_denom",
    type=int,
    default=None,
    help="generate random starting points with denominator DENOM (mutually exclusive with -s)",
)
@click.option(
    "-n",
    "n_tries",
    type=int,
    default=None,
    help="number of starting points to generate (requires -r)",
)
@click.option(
    "-R",
    "seed",
    type=int,
    default=None,
    help="seed the random number generator used to generate starting points "
    "(default is to seed from system entropy); requires -n",
)
@click.option(
    "-s",
    "start_file",
    type=str,
    default=None,
    help="file containing starting points (mutually exclusive with -r)",
)
@click.option(
    "-d",
    "decimals",
    default=0,
    show_default=True,
    type=int,
    help="show profiles as floating point with DECIMALS digits "
    "(default is to display rational numbers)",
)
@click.option(
    "-m",
    "maxregret",
    type=str,
    default=None,
    help="maximum regret acceptable as a proportion of range of payoffs in the game "
    "(default is 1/10000000)",
)
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@click.option(
    "-V",
    "--verbose",
    is_flag=True,
    help="verbose mode (shows intermediate output); default is to only show equilibria",
)
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    refine: int,
    rand_denom: int | None,
    n_tries: int | None,
    seed: int | None,
    start_file: str | None,
    decimals: int,
    maxregret: str | None,
    quiet: bool,
    verbose: bool,
) -> None:
    if not quiet:
        print_banner(DESCRIPTION)
    if rand_denom is not None and start_file is not None:
        raise ValueError("The -r and -s options are mutually exclusive.")
    if n_tries is not None and rand_denom is None:
        raise ValueError("The -n option requires -r.")
    if seed is not None and n_tries is None:
        raise ValueError("The -R option requires -n.")
    game = read_game(open_game_file(file, PROG_NAME))
    maxregret_value = gbt.Rational(maxregret) if maxregret is not None else None
    as_float = decimals > 0

    if start_file is not None:
        starts = read_strategy_profiles_csv(start_file, game, rational=True)
    elif rand_denom is not None:
        starts = [
            game.random_strategy_profile(denom=rand_denom, gen=np.random.default_rng(seed))
            for _ in range(n_tries if n_tries is not None else 1)
        ]
    else:
        starts = [_default_start(game)]

    def render(profile, label: str = "NE") -> None:
        click.echo(render_profile_csv(profile, label, decimals, as_float=as_float))

    def render_event(event) -> None:
        if not verbose:
            return
        if isinstance(event, gbt.SimpdivStartEvent):
            render(event.profile, "start")
        elif isinstance(event, gbt.SimpdivRefinementEvent):
            render(event.profile, f"{float(event.grid_size):.6f}")

    for start in starts:
        gbt.nash.simpdiv_solve(
            start,
            maxregret=maxregret_value,
            refine=refine,
            nash_callback=render,
            event_callback=render_event,
        )


if __name__ == "__main__":
    main()
