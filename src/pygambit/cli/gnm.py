#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/gnm.py
# Command-line driver program for Nash equilibrium computation via a global
# Newton method
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
"""Command-line driver program for Nash equilibrium computation via a global
Newton method.
"""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    open_game_file,
    print_banner,
    read_game,
    render_profile_csv,
    resolve_strategy_starts,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria using a global Newton method"
PROG_NAME = "gambit-gnm"
_EXTRA_BANNER = ("Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton",)


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified."
    ),
)
@click.argument("file", required=False, default=None)
@click.option(
    "-d",
    "decimals",
    default=6,
    show_default=True,
    type=int,
    help="show equilibria as floating point with DECIMALS digits",
)
@click.option(
    "-n",
    "n_vectors",
    type=int,
    default=None,
    help="number of perturbation vectors to generate randomly (mutually exclusive with -s)",
)
@click.option(
    "-R",
    "seed",
    type=int,
    default=None,
    help="seed the random number generator used to generate perturbation vectors "
    "(default is to seed from system entropy); requires -n",
)
@click.option(
    "-s",
    "start_file",
    type=str,
    default=None,
    help="file containing perturbation vectors (mutually exclusive with -n)",
)
@click.option(
    "-m",
    "end_lambda",
    default=-10.0,
    show_default=True,
    type=float,
    help="lambda to end tracing (must be negative)",
)
@click.option(
    "-f",
    "local_newton_interval",
    default=3,
    show_default=True,
    type=int,
    help="frequency to run local Newton method",
)
@click.option(
    "-i",
    "local_newton_maxits",
    default=10,
    show_default=True,
    type=int,
    help="maximum iterations in local Newton",
)
@click.option(
    "-c",
    "steps",
    default=100,
    show_default=True,
    type=int,
    help="number of steps in each support cell",
)
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@click.option(
    "-V",
    "--verbose",
    is_flag=True,
    help="verbose mode (shows intermediate output)",
)
@version_option(DESCRIPTION, _EXTRA_BANNER)
@handle_errors
def main(
    file: str | None,
    decimals: int,
    n_vectors: int | None,
    seed: int | None,
    start_file: str | None,
    end_lambda: float,
    local_newton_interval: int,
    local_newton_maxits: int,
    steps: int,
    quiet: bool,
    verbose: bool,
) -> None:
    if not quiet:
        print_banner(DESCRIPTION, _EXTRA_BANNER)
    if end_lambda >= 0.0:
        raise ValueError("Value for -m (end lambda) must be negative")
    if local_newton_interval <= 0:
        raise ValueError("Value for -f (local Newton frequency) must be at least 1")
    if local_newton_maxits <= 0:
        raise ValueError("Value for -i (local Newton iterations) must be at least 1")
    if steps <= 0:
        raise ValueError("Value for -c (steps in support cell) must be at least 1")
    game = read_game(open_game_file(file, PROG_NAME))
    perturbations = resolve_strategy_starts(game, n_vectors, seed, start_file)

    def render(profile, label: str = "NE") -> None:
        click.echo(render_profile_csv(profile, label, decimals))

    def render_event(event) -> None:
        if not verbose:
            return
        if isinstance(event, gbt.GNMPerturbationEvent):
            render(event.profile, "pert")
        elif isinstance(event, gbt.GNMStartEvent):
            render(event.profile, "start")
        elif isinstance(event, gbt.GNMStepEvent):
            render(event.profile, f"{event.lam:.6g}")

    for perturbation in perturbations:
        gbt.nash.gnm_solve(
            perturbation,
            end_lambda=end_lambda,
            steps=steps,
            local_newton_interval=local_newton_interval,
            local_newton_maxits=local_newton_maxits,
            nash_callback=render,
            event_callback=render_event,
        )


if __name__ == "__main__":
    main()
