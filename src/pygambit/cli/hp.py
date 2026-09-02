#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/hp.py
# Command-line driver program for Nash equilibrium computation via the
# Herings-Peeters (2001) homotopy method
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
"""Command-line driver program for Nash equilibrium computation via the
Herings-Peeters (2001) homotopy method.
"""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    load_game,
    render_profile_csv,
    resolve_strategy_starts,
    version_option,
)

DESCRIPTION = "Compute a Nash equilibrium using the Herings-Peeters (2001) homotopy method"
PROG_NAME = "gambit-hp"


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
    "n_priors",
    type=int,
    default=None,
    help="number of prior distributions to generate randomly (mutually exclusive with -s)",
)
@click.option(
    "-R",
    "seed",
    type=int,
    default=None,
    help="seed the random number generator used to generate prior distributions "
    "(default is to seed from system entropy); requires -n",
)
@click.option(
    "-s",
    "start_file",
    type=str,
    default=None,
    help="file containing prior distributions (mutually exclusive with -n)",
)
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@click.option(
    "-V",
    "--verbose",
    is_flag=True,
    help="verbose mode (shows each point traced along the homotopy path)",
)
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    decimals: int,
    n_priors: int | None,
    seed: int | None,
    start_file: str | None,
    quiet: bool,
    verbose: bool,
) -> None:
    game = load_game(quiet, DESCRIPTION, file, PROG_NAME)
    priors = resolve_strategy_starts(game, n_priors, seed, start_file)

    def render_event(event: gbt.HPStepEvent) -> None:
        if verbose:
            click.echo(render_profile_csv(event.profile, f"{event.t:.6g}", decimals))

    for prior in priors:
        prior = prior.as_float()
        if verbose:
            click.echo(render_profile_csv(prior, "prior", decimals))
        result = gbt.nash.hp_solve(prior, event_callback=render_event)
        for eq in result.equilibria:
            click.echo(render_profile_csv(eq, "NE", decimals))


if __name__ == "__main__":
    main()
