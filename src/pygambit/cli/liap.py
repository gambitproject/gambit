#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/liap.py
# Command-line driver program for Nash equilibrium computation via
# Lyapunov function minimization
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
"""Command-line driver program for Nash equilibrium computation via Lyapunov
function minimization.
"""

from __future__ import annotations

import click
import numpy as np

import pygambit as gbt

from .common import (
    handle_errors,
    open_game_file,
    print_banner,
    read_behavior_profiles_csv,
    read_game,
    read_strategy_profiles_csv,
    render_profile_csv,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria by minimizing the Lyapunov function"
PROG_NAME = "gambit-liap"

_DEFAULT_TRIES = 10


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "With no options, attempts to compute an equilibrium from randomly "
        "generated starting points."
    ),
)
@click.argument("file", required=False, default=None)
@click.option("-A", "--agent", is_flag=True, help="compute agent form equilibria")
@click.option(
    "-d",
    "decimals",
    default=6,
    show_default=True,
    type=int,
    help="print probabilities with DECIMALS digits",
)
@click.option(
    "-n",
    "n_tries",
    type=int,
    default=None,
    help="number of starting points to generate randomly (mutually exclusive with -s)",
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
    "-i",
    "maxiter",
    default=1000,
    show_default=True,
    type=int,
    help="maximum number of iterations per point",
)
@click.option(
    "-m",
    "maxregret",
    default=1.0e-4,
    show_default=True,
    type=float,
    help="maximum regret acceptable as a proportion of the range of payoffs in the game",
)
@click.option(
    "-s",
    "start_file",
    type=str,
    default=None,
    help="file containing starting points (mutually exclusive with -n)",
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
    agent: bool,
    decimals: int,
    n_tries: int | None,
    seed: int | None,
    maxiter: int,
    maxregret: float,
    start_file: str | None,
    quiet: bool,
    verbose: bool,
) -> None:
    if not quiet:
        print_banner(DESCRIPTION)
    if n_tries is not None and start_file is not None:
        raise ValueError("The -n and -s options are mutually exclusive.")
    if seed is not None and n_tries is None:
        raise ValueError("The -R option requires -n.")
    game = read_game(open_game_file(file, PROG_NAME))
    use_agent = agent and game.is_tree

    def render(profile, label: str = "NE") -> None:
        click.echo(render_profile_csv(profile, label, decimals))

    def render_event(event) -> None:
        if not verbose:
            return
        if isinstance(event, gbt.LiapStartEvent):
            render(event.profile, "start")
        elif isinstance(event, gbt.LiapEndEvent):
            render(event.profile, "end")

    if use_agent:
        starts = (
            read_behavior_profiles_csv(start_file, game)
            if start_file is not None
            else [
                game.random_behavior_profile(gen=np.random.default_rng(seed))
                for _ in range(n_tries if n_tries is not None else _DEFAULT_TRIES)
            ]
        )
        for start in starts:
            gbt.nash.liap_agent_solve(
                start,
                maxregret=maxregret,
                maxiter=maxiter,
                nash_callback=render,
                event_callback=render_event,
            )
    else:
        starts = (
            read_strategy_profiles_csv(start_file, game)
            if start_file is not None
            else [
                game.random_strategy_profile(gen=np.random.default_rng(seed))
                for _ in range(n_tries if n_tries is not None else _DEFAULT_TRIES)
            ]
        )
        for start in starts:
            gbt.nash.liap_solve(
                start,
                maxregret=maxregret,
                maxiter=maxiter,
                nash_callback=render,
                event_callback=render_event,
            )


if __name__ == "__main__":
    main()
