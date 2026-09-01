#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/enumpure.py
# Command-line driver program for pure-strategy Nash equilibrium enumeration
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
"""Command-line driver program for pure-strategy Nash equilibrium enumeration."""

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

DESCRIPTION = "Search for Nash equilibria in pure strategies"
PROG_NAME = "gambit-enumpure"


@click.command(
    context_settings={"help_option_names": ["-h", "--help"]},
    help=(
        f"{DESCRIPTION}.\n\n"
        "Reads a game from FILE, or from standard input if FILE is not specified. "
        "With no options, locates all Nash equilibria in pure strategies."
    ),
)
@click.argument("file", required=False, default=None)
@click.option(
    "-S",
    "--strategic",
    is_flag=True,
    help="report equilibria in strategies even for extensive games",
)
@click.option("-A", "--agent", is_flag=True, help="compute agent form equilibria")
@click.option("-D", "--detail", is_flag=True, help="print detailed information about equilibria")
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@version_option(DESCRIPTION)
@handle_errors
def main(file: str | None, strategic: bool, agent: bool, detail: bool, quiet: bool) -> None:
    game = load_game(quiet, DESCRIPTION, file, PROG_NAME)

    def render(profile) -> None:
        is_behavior = hasattr(profile, "as_strategy")
        if strategic and is_behavior:
            profile = profile.as_strategy()
        elif not strategic and game.is_tree and not is_behavior:
            profile = profile.as_behavior()
        if detail:
            click.echo(render_profile_detail(profile, 0))
        else:
            click.echo(render_profile_csv(profile, "NE", 0))

    if agent and game.is_tree:
        gbt.nash.enumpure_agent_solve(game, nash_callback=render)
    else:
        gbt.nash.enumpure_solve(game, nash_callback=render)


if __name__ == "__main__":
    main()
