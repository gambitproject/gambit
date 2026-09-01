#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/lcp.py
# Command-line driver program for Nash equilibrium computation via LCP
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
"""Command-line driver program for Nash equilibrium computation via linear
complementarity programming.
"""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    load_game,
    render_profile_csv,
    render_profile_detail,
    validate_stop_after,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria by solving a linear complementarity program"
PROG_NAME = "gambit-lcp"


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "For an extensive game, computes one equilibrium via the sequence form. "
        "For a strategic game (or with -S), with no options, reports all accessible "
        "Nash equilibria."
    ),
)
@click.argument("file", required=False, default=None)
@click.option(
    "-d",
    "decimals",
    type=int,
    default=None,
    help="compute using floating-point arithmetic; display results with DECIMALS digits",
)
@click.option("-S", "--strategic", is_flag=True, help="use strategic game")
@click.option(
    "-e",
    "stop_after",
    type=str,
    default=None,
    callback=validate_stop_after,
    help="terminate after finding EQA equilibria "
    "(strategic games only; default is to find all accessible equilibria)",
)
@click.option(
    "-r",
    "max_depth",
    type=int,
    default=0,
    help="terminate recursion at DEPTH "
    "(strategic games only; only if number of equilibria sought is not 1)",
)
@click.option("-D", "--detail", is_flag=True, help="print detailed information about equilibria")
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    decimals: int | None,
    strategic: bool,
    stop_after: int | None,
    max_depth: int,
    detail: bool,
    quiet: bool,
) -> None:
    game = load_game(quiet, DESCRIPTION, file, PROG_NAME)
    rational = decimals is None
    render_decimals = decimals or 0
    use_strategic = strategic or not game.is_tree

    def render(profile) -> None:
        if detail:
            click.echo(render_profile_detail(profile, render_decimals))
        else:
            click.echo(render_profile_csv(profile, "NE", render_decimals))

    gbt.nash.lcp_solve(
        game,
        rational=rational,
        use_strategic=strategic,
        stop_after=stop_after if use_strategic else None,
        max_depth=max_depth if use_strategic else None,
        nash_callback=render,
    )


if __name__ == "__main__":
    main()
