#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/logit.py
# Command-line driver program for quantal response equilibrium tracing
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
"""Command-line driver program for quantal response equilibrium tracing."""

from __future__ import annotations

import pathlib

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

DESCRIPTION = "Compute a branch of the logit equilibrium correspondence"
PROG_NAME = "gambit-logit"


def _render_qre_row(qre, decimals: int) -> str:
    """Render a QRE point (anything exposing `lam`, `profile`, and `log_like`, as
    `LogitQREMixedStrategyProfile`/`LogitQREMixedBehaviorProfile` and
    `LogitQREMixedStrategyFitResult`/`LogitQREMixedBehaviorFitResult` all do) as a
    single comma-separated line, matching `gambit-logit`'s own `PrintProfile`: the
    lambda label is fixed-point, the profile values use `decimals` significant
    digits, and the log-likelihood is appended, fixed-point, when it is meaningful
    (non-positive).
    """
    label = f"{qre.lam:.{decimals}f}"
    text = render_profile_csv(qre.profile, label, decimals, fixed=False)
    if qre.log_like <= 0.0:
        text += f",{qre.log_like:.{decimals}f}"
    return text


def _read_frequencies(path: str, game: gbt.Game) -> gbt.MixedStrategyProfileDouble:
    """Read observed strategy frequencies for maximum-likelihood estimation: a flat,
    comma-separated list of counts, one per strategy, in the same order as a profile's
    CSV row, matching the C++ tool's `ReadProfile`.
    """
    count = sum(len(list(player.strategies)) for player in game.players)
    try:
        fields = pathlib.Path(path).read_text().split(",")
        values = [float(fields[i]) for i in range(count)]
    except (OSError, ValueError, IndexError):
        raise ValueError(f"Error reading strategy frequencies from '{path}'.") from None
    frequencies = game.mixed_strategy_profile(rational=False)
    it = iter(values)
    for player in game.players:
        frequencies[player.label] = {s: next(it) for s in player.strategies}
    return frequencies


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
@click.option("-S", "--strategic", is_flag=True, help="use strategic game")
@click.option(
    "-s",
    "first_step",
    default=0.03,
    show_default=True,
    type=float,
    help="initial stepsize for the tracing procedure",
)
@click.option(
    "-a",
    "max_accel",
    default=1.1,
    show_default=True,
    type=float,
    help="maximum acceleration of the step size during the tracing procedure",
)
@click.option(
    "-m",
    "maxregret",
    default=1.0e-8,
    show_default=True,
    type=float,
    help="maximum regret acceptable as a proportion of the range of payoffs in the game",
)
@click.option(
    "-l",
    "target_lambda",
    multiple=True,
    type=float,
    help="compute the QRE at LAMBDA accurately; may be specified multiple times",
)
@click.option(
    "-L",
    "mle_file",
    type=str,
    default=None,
    help="compute maximum likelihood estimate; read strategy frequencies from FILE",
)
@click.option(
    "-e",
    "--terminal-only",
    is_flag=True,
    help="print only the terminal equilibrium (default is to print the entire branch)",
)
@click.option("-q", "--quiet", is_flag=True, help="quiet mode (suppresses banner)")
@version_option(DESCRIPTION)
@handle_errors
def main(
    file: str | None,
    decimals: int,
    strategic: bool,
    first_step: float,
    max_accel: float,
    maxregret: float,
    target_lambda: tuple[float, ...],
    mle_file: str | None,
    terminal_only: bool,
    quiet: bool,
) -> None:
    if not quiet:
        print_banner(DESCRIPTION)
    game = read_game(open_game_file(file, PROG_NAME))
    if not game.is_perfect_recall:
        raise ValueError("Computing equilibria of games with imperfect recall is not supported.")

    def stream(qre) -> None:
        click.echo(_render_qre_row(qre, decimals))

    event_callback = None if terminal_only else stream

    # Maximum-likelihood estimation, like the C++ tool, is only defined over the
    # strategic representation, since the observed frequencies are read as a flat
    # list of strategy counts.
    if mle_file is not None and (strategic or not game.is_tree):
        frequencies = _read_frequencies(mle_file, game)
        result = gbt.qre.logit_estimate(
            frequencies,
            first_step=first_step,
            max_accel=max_accel,
            event_callback=event_callback,
        )
        click.echo(_render_qre_row(result, decimals))
        return

    if target_lambda:
        for qre in gbt.qre.logit_solve_lambda(
            game,
            list(target_lambda),
            use_strategic=strategic,
            first_step=first_step,
            max_accel=max_accel,
            event_callback=event_callback,
        ):
            click.echo(_render_qre_row(qre, decimals))
        return

    if terminal_only:
        result = gbt.nash.logit_solve(
            game,
            use_strategic=strategic,
            maxregret=maxregret,
            first_step=first_step,
            max_accel=max_accel,
        )
        click.echo(render_profile_csv(result.equilibria[-1], "NE", decimals, fixed=False))
        return

    result = gbt.nash.logit_solve(
        game,
        use_strategic=strategic,
        maxregret=maxregret,
        first_step=first_step,
        max_accel=max_accel,
        event_callback=stream,
    )
    click.echo(render_profile_csv(result.equilibria[-1], "NE", decimals, fixed=False))


if __name__ == "__main__":
    main()
