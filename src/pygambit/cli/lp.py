#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/lp.py
# Command-line driver program for Nash equilibrium computation via LP
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
programming, for two-player constant-sum games.
"""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    load_game,
    render_profile_csv,
    render_profile_detail,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria by solving a linear program"
PROG_NAME = "gambit-lp"


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "With no options, computes one Nash equilibrium."
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
@click.option("-D", "--detail", is_flag=True, help="print detailed information about equilibria")
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    decimals: int | None,
    strategic: bool,
    detail: bool,
    quiet: bool,
) -> None:
    game = load_game(quiet, DESCRIPTION, file, PROG_NAME)
    rational = decimals is None
    render_decimals = decimals or 0

    def render(profile) -> None:
        if detail:
            click.echo(render_profile_detail(profile, render_decimals))
        else:
            click.echo(render_profile_csv(profile, "NE", render_decimals))

    gbt.nash.lp_solve(
        game,
        rational=rational,
        use_strategic=strategic,
        nash_callback=render,
    )


if __name__ == "__main__":
    main()
