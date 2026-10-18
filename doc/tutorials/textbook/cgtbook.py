"""Small helpers shared by the notebooks for
*An Introduction to Computational Game Theory*, Chapters 4-6.

Everything game-theoretic (building games, solving them) is done with
pygambit; this module only

* builds games with the book's strategy names (``bimatrix``, ``table_game``),
* prints mixed strategy profiles in the book's notation (``show``,
  ``eq_table``), and
* draws the two kinds of picture the chapters use again and again
  (``plot_upper_envelope``, ``plot_br_2x2``).

Teaching implementations of algorithms (best-response search, support
enumeration, Lemke-Howson, ...) are deliberately *not* here: they live in
the notebooks, next to the text they illustrate.
"""

from __future__ import annotations

from fractions import Fraction

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import pygambit as gbt

# Colours of the payoff macros in example-games.tex: player 1 blue, player 2 red.
P1_COLOUR = "#1f4fbf"
P2_COLOUR = "#c0262d"


# ---------------------------------------------------------------------------
# Building games
# ---------------------------------------------------------------------------

def table_game(payoffs, strategies, players=None, title="Untitled"):
    """An n-player strategic form game with the book's labels.

    payoffs    : list of n arrays, payoffs[i][s_1, ..., s_n] is player i+1's payoff
    strategies : list of n lists of strategy names
    players    : optional list of n player names (default "1", "2", ...)
    """
    payoffs = [np.asarray(a, dtype=object) for a in payoffs]
    g = gbt.StrategicGame.from_arrays(*payoffs, title=title)
    for player, names in zip(list(g.players), strategies, strict=True):
        g.relabel_strategies(player, {str(k + 1): s for k, s in enumerate(names)})
    if players is not None:
        g.relabel_players(dict(zip(list(g.players), players, strict=True)))
    return g


def bimatrix(A, B, rows, cols, title="Untitled"):
    """A bimatrix game (A, B); rows are player 1's strategies, as in the book."""
    return table_game([A, B], [rows, cols], title=title)


def frac(v):
    """Convert a pygambit Rational / float / int to a Fraction (or float)."""
    if isinstance(v, float):
        return v
    try:
        return Fraction(str(v))
    except ValueError:
        return float(v)


def _fmt(v, digits=4):
    v = frac(v)
    if isinstance(v, Fraction):
        return str(v)
    return f"{v:.{digits}f}".rstrip("0").rstrip(".") if abs(v) > 1e-12 else "0"


def nice(v, digits=4):
    """Readable version of pygambit's labelled vectors (payoffs, strategy values,
    regrets, profiles): nested dicts of strings such as '2/3'."""
    if isinstance(v, (int, float, Fraction)) or type(v).__name__ in ("Rational", "Decimal"):
        return _fmt(v, digits)
    if isinstance(v, dict):
        return {k: nice(x, digits) for k, x in v.items()}
    return {k: nice(x, digits) for k, x in v}


def payoff_arrays(g):
    """The payoff tables of g as numpy object arrays of Fractions, one per player."""
    return [np.vectorize(frac, otypes=[object])(a) for a in g.to_arrays()]


def strategy_names(g):
    return {p: list(g.get_strategies(p)) for p in g.players}


def probs(profile, player):
    g = profile.game
    return [profile[player][s] for s in g.get_strategies(player)]


def show(profile, names=("x", "y", "z"), digits=4):
    """Print a mixed strategy profile as  x = (2/3, 1/3),  y = (1/2, 1/2, 0)."""
    parts = []
    for name, player in zip(names, profile.game.players, strict=False):
        values = ", ".join(_fmt(p, digits) for p in probs(profile, player))
        parts.append(f"{name} = ({values})")
    return ",  ".join(parts)


def eq_table(equilibria, digits=4):
    """A DataFrame with one row per equilibrium: probabilities and payoffs."""
    rows = []
    for eq in equilibria:
        g = eq.game
        row = {}
        for player in g.players:
            for s in g.get_strategies(player):
                row[(f"player {player}", s)] = _fmt(eq[player][s], digits)
        for player in g.players:
            row[("payoff", player)] = _fmt(eq.payoffs[player], digits)
        rows.append(row)
    df = pd.DataFrame(rows)
    df.columns = pd.MultiIndex.from_tuples(df.columns)
    df.index = [f"eq {i + 1}" for i in range(len(rows))]
    return df


def pure_profile_names(eq):
    """For a pure equilibrium, the tuple of strategy names played."""
    g = eq.game
    return tuple(
        next(s for s in g.get_strategies(p) if eq[p][s] == 1) for p in g.players
    )


# ---------------------------------------------------------------------------
# Pictures
# ---------------------------------------------------------------------------

def plot_upper_envelope(lines, xlabel, ylabel, names, colour, ax=None, marks=()):
    """Plot payoff lines  a_k + (b_k - a_k) t  for t in [0, 1] and their upper envelope.

    lines : list of (value at t=0, value at t=1)
    marks : t-values to mark with a dotted vertical line (e.g. equilibria)
    """
    if ax is None:
        _, ax = plt.subplots(figsize=(5, 3.5))
    t = np.linspace(0, 1, 401)
    vals = np.array([a + (b - a) * t for a, b in lines], dtype=float)
    for (a, b), name in zip(lines, names, strict=True):
        ax.plot([0, 1], [float(a), float(b)], color="0.6", lw=1)
        ax.annotate(name, (1.02, float(b)), color="0.3", va="center", annotation_clip=False)
    ax.plot(t, vals.max(axis=0), color=colour, lw=2.5, label="upper envelope")
    for m in marks:
        ax.axvline(float(m), color="0.3", ls=":", lw=1)
    ax.set_xlim(0, 1)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.spines[["top", "right"]].set_visible(False)
    return ax


def plot_br_2x2(A, B, ax=None, equilibria=(), segments=(), title=None):
    """Best response correspondences of a 2x2 game in the book's Chapter 5 axes:
    horizontal axis prob(right) = y_2, vertical axis prob(Top) = x_1.

    equilibria : list of (prob(right), prob(Top)) points to mark.
    segments   : list of pairs of such points; drawn as thick black segments
                 (sets of equilibria in degenerate games).
    """
    A = np.asarray(A, dtype=float)
    B = np.asarray(B, dtype=float)
    if ax is None:
        _, ax = plt.subplots(figsize=(3.6, 3.6))
    # player 1: EU(Top) - EU(Bottom) as a function of r = prob(right)
    r = np.linspace(0, 1, 2001)
    d1 = (A[0, 0] - A[1, 0]) * (1 - r) + (A[0, 1] - A[1, 1]) * r
    # player 2: EU(right) - EU(left) as a function of p = prob(Top)
    p = np.linspace(0, 1, 2001)
    d2 = (B[0, 1] - B[0, 0]) * p + (B[1, 1] - B[1, 0]) * (1 - p)

    def corr(xs, d):
        pts = []
        for x, dv in zip(xs, d, strict=True):
            if abs(dv) < 1e-9:
                pts += [(x, 0.0), (x, 1.0)]
            else:
                pts.append((x, 1.0 if dv > 0 else 0.0))
        # insert vertical jumps at sign changes
        out = [pts[0]]
        for a, b in zip(pts, pts[1:], strict=False):
            if a[1] != b[1] and a[0] != b[0]:
                out.append((b[0], a[1]))
            out.append(b)
        return np.array(out)

    br1 = corr(r, d1)  # (r, prob(Top))
    br2 = corr(p, d2)  # (p, prob(right))
    ax.plot(br1[:, 0], br1[:, 1], color=P1_COLOUR, lw=2.5, label="BR$_1$")
    ax.plot(br2[:, 1], br2[:, 0], color=P2_COLOUR, lw=1.5, ls="--", label="BR$_2$")
    for a, b in segments:
        ax.plot([float(a[0]), float(b[0])], [float(a[1]), float(b[1])], color="black",
                lw=5, solid_capstyle="round", zorder=4)
    for e in equilibria:
        ax.plot(*map(float, e), "o", color="black", ms=7, zorder=5)
    ax.set_xlim(-0.04, 1.04)
    ax.set_ylim(-0.04, 1.04)
    ax.set_aspect("equal")
    ax.set_xlabel("prob(right)")
    ax.set_ylabel("prob(Top)")
    if title:
        ax.set_title(title, fontsize=10)
    ax.spines[["top", "right"]].set_visible(False)
    return ax
