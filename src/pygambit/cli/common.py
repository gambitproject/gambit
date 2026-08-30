#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/cli/common.py
# Infrastructure shared by Gambit's command-line tools
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
"""Infrastructure shared by Gambit's command-line tools."""

from __future__ import annotations

import functools
import io
import pathlib
import sys

import click

import pygambit as gbt

_GAME_FORMATS = (
    ("NFG", gbt.read_nfg),
    ("EFG", gbt.read_efg),
    ("#AGG", gbt.read_agg),
    ("#BAGG", gbt.read_bagg),
)


def _gambit_version() -> str:
    """The Gambit version string, in the same (not necessarily PEP 440-normalized)
    form as the C++ tools' ``VERSION`` macro, which is taken verbatim from
    `build_support/GAMBIT_VERSION`.
    """
    version_file = pathlib.Path(__file__).parents[3] / "build_support/GAMBIT_VERSION"
    if version_file.exists():
        return version_file.read_text().strip()
    return gbt.__version__


def open_game_file(path: str | None, prog_name: str) -> io.IOBase:
    """Open `path` for reading, or return standard input if `path` is `None` or
    `"-"`.  A failure to open the file is reported the way the C++ tools' call to
    `perror()` does, and exits with status 1.
    """
    if path is None or path == "-":
        return sys.stdin.buffer
    try:
        return open(path, "rb")
    except OSError as exc:
        click.echo(f"{prog_name}: {path}: {exc.strerror}", err=True)
        sys.exit(1)


def read_game(source: io.IOBase) -> gbt.Game:
    """Read a game from an open file (or standard input), auto-detecting its file
    format the way the underlying C++ tools' ``ReadGame()`` does: first as a
    (legacy) ``.gbt`` workspace, and, failing that, by the leading token of the
    file, which identifies it as an NFG, EFG, AGG, or BAGG representation.
    """
    data = source.read()
    if isinstance(data, str):
        data = data.encode("utf-8")
    if not data.strip():
        raise ValueError("Empty file or string provided")
    try:
        return gbt.read_gbt(io.BytesIO(data))
    except Exception:
        pass
    text = data.decode("utf-8", errors="replace").lstrip()
    for tag, reader in _GAME_FORMATS:
        if text.startswith(tag):
            try:
                return reader(io.BytesIO(data))
            except Exception as exc:
                raise ValueError(f"Parse error in game file: {exc}") from None
    raise ValueError("Unrecognized file format")


def print_banner(description: str, extra_lines: tuple[str, ...] = ()) -> None:
    """Print the standard Gambit command-line tool banner to standard error.  Some
    tools (`gambit-gnm`, `gambit-ipa`) insert additional lines, such as a Gametracer
    credit, between the description and the Gambit version line; pass those as
    `extra_lines`.
    """
    click.echo(description, err=True)
    for line in extra_lines:
        click.echo(line, err=True)
    click.echo(
        f"Gambit version {_gambit_version()}, Copyright (C) 1994-2026, The Gambit Project",
        err=True,
    )
    click.echo("This is free software, distributed under the GNU GPL", err=True)
    click.echo(err=True)


def version_option(description: str, extra_lines: tuple[str, ...] = ()) -> callable:
    """A ``-v``/``--version`` option which prints the tool's banner and exits,
    matching the behavior of the C++ command-line tools.  See `print_banner` for
    the meaning of `extra_lines`.
    """

    def callback(ctx: click.Context, _param: click.Parameter, value: bool) -> None:
        if not value or ctx.resilient_parsing:
            return
        print_banner(description, extra_lines)
        ctx.exit(0)

    return click.option(
        "-v",
        "--version",
        is_flag=True,
        expose_value=False,
        is_eager=True,
        callback=callback,
        help="print version information",
    )


def handle_errors(f: callable) -> callable:
    """Decorator which reports errors the way the C++ command-line tools do:
    a message on standard error and exit code 1, instead of a Python traceback.
    """

    @functools.wraps(f)
    def wrapper(*args, **kwargs):
        try:
            f(*args, **kwargs)
        except (ValueError, RuntimeError, OSError) as exc:
            click.echo(f"Error: {exc}", err=True)
            sys.exit(1)

    return wrapper


def validate_stop_after(
    ctx: click.Context, param: click.Parameter, value: str | None
) -> int | None:
    """A click callback for a ``-e``/``stop_after``-style option: validates that `value`
    is a positive integer, matching the C++ tools' `std::from_chars`-based validation,
    reporting the same error text and exiting immediately (before the tool's banner)
    on failure.
    """
    if value is None:
        return None
    if not value.isdigit() or int(value) == 0:
        click.echo(f"Error: -e argument must be a positive integer; got '{value}'.", err=True)
        sys.exit(1)
    return int(value)


def format_value(
    value: gbt.Rational | float,
    decimals: int,
    fixed: bool = True,
    as_float: bool = False,
) -> str:
    """Format a probability or payoff value the way the C++ tools do: exactly, for a
    `Rational`, or, for a float, fixed-point with `decimals` digits after the point
    (`fixed=True`, matching `MixedStrategyProfileCSVRenderer`/`MixedBehaviorProfileCSVRenderer`),
    or with `decimals` significant digits, trimmed of trailing zeroes and switching to
    scientific notation as needed (`fixed=False`, matching `gambit-logit`'s own
    `PrintProfile`, which explicitly unsets `std::ios::fixed` before printing profile
    values).  If `as_float` is True, a `Rational` is first converted to a float and
    formatted the same way as a float would be, matching `gambit-simpdiv`'s
    `MixedStrategyCSVAsFloatRenderer`.
    """
    if isinstance(value, gbt.Rational) and not as_float:
        return str(value)
    value = float(value)
    return f"{value:.{decimals}f}" if fixed else f"{value:.{decimals}g}"


def _is_behavior_profile(profile) -> bool:
    return "Behavior" in type(profile).__name__


def render_profile_csv(
    profile: gbt.MixedStrategyProfile | gbt.MixedBehaviorProfile,
    label: str,
    decimals: int,
    fixed: bool = True,
    as_float: bool = False,
) -> str:
    """Render a strategy or behavior profile as a single comma-separated line,
    tagged with `label`, matching `MixedStrategyProfileCSVRenderer` and
    `MixedBehaviorProfileCSVRenderer` in the C++ tools.  See `format_value` for
    the meaning of `fixed` and `as_float`.
    """
    if _is_behavior_profile(profile):
        values = [
            prob
            for player in profile.game.players
            for _infoset, action in profile[player]
            for _label, prob in action
        ]
    else:
        values = [
            prob for player in profile.game.players for _label, prob in profile[player]
        ]
    return ",".join([label, *(format_value(v, decimals, fixed, as_float) for v in values)])


def render_support_csv(
    support: gbt.StrategySupportProfile | gbt.BehaviorSupportProfile,
    label: str,
) -> str:
    """Render a strategy or behavior support profile as a single comma-separated line,
    tagged with `label`: one field per player (strategy support) or per information set
    (behavior support), each field a string of `1`/`0` digits marking membership in the
    support, matching `gambit-enumpoly`'s own `PrintSupport`.
    """
    if isinstance(support, gbt.BehaviorSupportProfile):
        fields = [
            "".join(
                "1" if action in action_support else "0"
                for action in action_support.infoset.actions
            )
            for player in support.game.players
            for action_support in support[player]
        ]
    else:
        fields = [
            "".join(
                "1" if strategy in support[player] else "0"
                for strategy in support.game.get_strategies(player)
            )
            for player in support.game.players
        ]
    return ",".join([label, *fields])


def render_profile_detail(
    profile: gbt.MixedStrategyProfile | gbt.MixedBehaviorProfile,
    decimals: int,
) -> str:
    """Render a strategy or behavior profile as a human-readable description,
    matching `MixedStrategyProfileDetailRenderer` and
    `MixedBehaviorProfileDetailRenderer` in the C++ tools.
    """
    if _is_behavior_profile(profile):
        return _render_behavior_detail(profile, decimals)
    return _render_strategy_detail(profile, decimals)


def _name_or_number(obj) -> str:
    # Gambit's Python API numbers players/strategies/infosets/actions from 0;
    # the C++ tools display the underlying (1-based) engine numbering.
    return obj.label if obj.label else str(obj.number + 1)


def _render_strategy_detail(profile: gbt.MixedStrategyProfile, decimals: int) -> str:
    lines = []
    for number, player in enumerate(profile.game.players, start=1):
        lines.append(f"Strategy profile for player {number}:")
        lines.append("Strategy   Prob          Value")
        lines.append("--------   -----------   -----------")
        probs = profile[player]
        values = profile.strategy_values[player]
        for strategy in profile.game.get_strategies(player):
            prob = format_value(probs[strategy], decimals)
            value = format_value(values[strategy], decimals)
            lines.append(f"{strategy:>8}    {prob:>10}   {value:>11}")
    return "\n".join(lines)


def _render_behavior_detail(profile: gbt.MixedBehaviorProfile, decimals: int) -> str:
    lines = []
    action_values = profile.action_values
    beliefs = profile.beliefs
    realiz_probs = profile.realiz_probs
    for number, player in enumerate(profile.game.players, start=1):
        lines.append(f"Behavior profile for player {number}:")
        lines.append("Infoset    Action     Prob          Value")
        lines.append("-------    -------    -----------   -----------")
        for infoset, mixed_action in profile[player]:
            infoset_name = _name_or_number(infoset)
            values = action_values[next(iter(infoset.members))]
            for action in infoset.actions:
                prob = mixed_action[action]
                value = values[action]
                value_text = format_value(value, decimals) if value is not None else ""
                lines.append(
                    f"{infoset_name:>7}    {action:>7}   "
                    f"{format_value(prob, decimals):>11}   {value_text:>11}"
                )
        lines.append("")
        lines.append("Infoset    Node       Belief        Prob")
        lines.append("-------    -------    -----------   -----------")
        for infoset, _mixed_action in profile[player]:
            infoset_name = _name_or_number(infoset)
            for node in infoset.members:
                node_name = _name_or_number(node)
                belief = beliefs[node]
                belief_text = format_value(belief, decimals) if belief is not None else ""
                realiz_text = format_value(realiz_probs[node], decimals)
                lines.append(
                    f"{infoset_name:>7}    {node_name:>7}   {belief_text:>11}   {realiz_text:>11}"
                )
        lines.append("")
    return "\n".join(lines)


def read_strategy_profiles_csv(
    path: str,
    game: gbt.Game,
) -> list[gbt.MixedStrategyProfileRational]:
    """Read one mixed strategy profile per line from `path`, each a flat comma-separated
    list of probabilities in the same order as a profile's CSV row (excluding any
    leading label), matching the C++ tools' `ReadStrategyProfiles`/`ReadProfiles`.
    Values are parsed as exact rationals; a method which requires floating-point
    starting points converts the result via `~MixedStrategyProfile.as_float`.
    """
    strategies = [
        strategy for player in game.players for strategy in game.get_strategies(player)
    ]
    profiles = []
    for line in pathlib.Path(path).read_text().splitlines():
        line = line.strip()
        if not line:
            continue
        fields = line.split(",")
        try:
            values = iter([gbt.Rational(fields[i]) for i in range(len(strategies))])
        except (ValueError, IndexError) as exc:
            raise ValueError(f"Error reading strategy profile from '{path}': {exc}") from None
        profile = game.mixed_strategy_profile(rational=True)
        for player in game.players:
            profile[player] = {s: next(values) for s in game.get_strategies(player)}
        profiles.append(profile)
    return profiles


def read_behavior_profiles_csv(
    path: str, game: gbt.Game
) -> list[gbt.MixedBehaviorProfileRational]:
    """Read one mixed behavior profile per line from `path`, each a flat comma-separated
    list of probabilities in the same order as a profile's CSV row (excluding any
    leading label), matching the C++ tools' `ReadBehaviorProfiles`.  Values are parsed
    as exact rationals; a method which requires floating-point starting points converts
    the result via `~MixedBehaviorProfile.as_float`.
    """
    count = sum(
        len(node.infoset.actions)
        for player in game.players
        for node in game.get_infosets(player)
    )
    profiles = []
    for line in pathlib.Path(path).read_text().splitlines():
        line = line.strip()
        if not line:
            continue
        fields = line.split(",")
        try:
            values = iter([gbt.Rational(fields[i]) for i in range(count)])
        except (ValueError, IndexError) as exc:
            raise ValueError(f"Error reading behavior profile from '{path}': {exc}") from None
        profile = game.mixed_behavior_profile(rational=True)
        for player in game.players:
            for node in game.get_infosets(player):
                profile[node] = {a: next(values) for a in node.infoset.actions}
        profiles.append(profile)
    return profiles
