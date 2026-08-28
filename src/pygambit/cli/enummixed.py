#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/enummixed.py
# Command-line driver program for mixed-strategy Nash equilibrium enumeration
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
"""Command-line driver program for mixed-strategy Nash equilibrium enumeration."""

from __future__ import annotations

import click

import pygambit as gbt

from .common import (
    handle_errors,
    open_game_file,
    print_banner,
    read_game,
    render_profile_csv,
    version_option,
)

DESCRIPTION = "Compute Nash equilibria by enumerating extreme points"
PROG_NAME = "gambit-enummixed"


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
    type=int,
    default=None,
    help="compute using floating-point arithmetic; display results with DECIMALS digits",
)
@click.option("-c", "--cliques", is_flag=True, help="output connectedness information")
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@version_option(DESCRIPTION)
@handle_errors
def main(file: str | None, decimals: int | None, cliques: bool, quiet: bool) -> None:
    if not quiet:
        print_banner(DESCRIPTION)
    game = read_game(open_game_file(file, PROG_NAME))
    rational = decimals is None

    def render(profile) -> None:
        click.echo(render_profile_csv(profile, "NE", decimals or 0))

    result = gbt.nash.enummixed_solve(
        game,
        rational=rational,
        nash_callback=render,
        cliques=cliques,
    )
    if cliques:
        for index, clique in enumerate(result.parameters["cliques"], start=1):
            label = f"convex-{index}"
            for profile in clique:
                click.echo(render_profile_csv(profile, label, decimals or 0))


if __name__ == "__main__":
    main()
