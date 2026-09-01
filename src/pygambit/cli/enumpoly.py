#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/enumpoly.py
# Command-line driver program for Nash equilibrium enumeration via support
# enumeration and polynomial system solving
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
"""Command-line driver program for Nash equilibrium enumeration via support
enumeration and polynomial system solving.

.. note::

   Solving via PHCpack (the C++ tool's ``phcpack_path`` equivalent has no CLI
   option) is not available here; see :func:`pygambit.nash.enumpoly_solve`'s
   `phcpack_path` parameter for the equivalent computation from Python.
"""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    load_game,
    render_profile_csv,
    render_support_csv,
    validate_stop_after,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria by solving polynomial systems"
PROG_NAME = "gambit-enumpoly"


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "With no options, reports all Nash equilibria found."
    ),
)
@click.argument("file", required=False, default=None)
@click.option(
    "-d",
    "decimals",
    default=6,
    show_default=True,
    type=int,
    help="show equilibrium probabilities with DECIMALS digits",
)
@click.option("-S", "--strategic", is_flag=True, help="use strategic game")
@click.option(
    "-m",
    "maxregret",
    default=1.0e-8,
    show_default=True,
    type=float,
    help="maximum regret acceptable as a proportion of the range of payoffs in the game",
)
@click.option(
    "-e",
    "stop_after",
    type=str,
    default=None,
    callback=validate_stop_after,
    help="terminate after finding EQA equilibria (default is to search in all supports)",
)
@click.option(
    "-r",
    "max_rectangles",
    default=20_000,
    show_default=True,
    type=int,
    help="maximum number of rectangles to examine when searching for roots on a "
    "single support, before giving up on that support",
)
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@click.option(
    "-V",
    "--verbose",
    is_flag=True,
    help="verbose mode (shows supports investigated); default is only to show equilibria",
)
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    decimals: int,
    strategic: bool,
    maxregret: float,
    stop_after: int | None,
    max_rectangles: int,
    quiet: bool,
    verbose: bool,
) -> None:
    game = load_game(quiet, DESCRIPTION, file, PROG_NAME)
    if not game.is_perfect_recall:
        raise ValueError("Computing equilibria of games with imperfect recall is not supported.")

    def render(profile) -> None:
        click.echo(render_profile_csv(profile, "NE", decimals))

    def render_event(event) -> None:
        if not verbose:
            return
        if isinstance(event, gbt.EnumPolyCandidateSupportEvent):
            click.echo(render_support_csv(event.support, "candidate"))
        elif isinstance(event, gbt.EnumPolySingularSupportEvent):
            click.echo(render_support_csv(event.support, "singular"))
        elif isinstance(event, gbt.EnumPolyBudgetExceededSupportEvent):
            click.echo(render_support_csv(event.support, "budget-exceeded"))

    gbt.nash.enumpoly_solve(
        game,
        use_strategic=strategic,
        stop_after=stop_after,
        maxregret=maxregret,
        max_rectangles=max_rectangles,
        nash_callback=render,
        event_callback=render_event,
    )


if __name__ == "__main__":
    main()
