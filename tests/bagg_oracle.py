"""Independent oracle for Bayesian action-graph games (BAGGs).

A small BAGG is expanded into an explicit extensive game in which chance draws each player's
type from its prior, each type is an information set of its player, and the actions at a type
follow the type's action set.  Players, types and actions are labelled "1", "2", ... by position.

The BAGG file is parsed here directly, so nothing in this module touches pygambit's BAGG class.
"""

import dataclasses
import itertools
import pathlib
import re
from fractions import Fraction

import pygambit as gbt

TEST_GAMES = pathlib.Path(__file__).parent / "test_games"


@dataclasses.dataclass
class BAGGDescription:
    """A BAGG without function nodes: type priors, type-action sets and node payoffs."""

    priors: list[list[Fraction]]
    action_sets: list[list[list[int]]]
    neighbors: list[list[int]]
    node_payoff: list[dict[tuple[int, ...], Fraction]]

    @property
    def num_players(self) -> int:
        return len(self.priors)

    def payoffs(self, nodes: tuple[int, ...]) -> tuple[Fraction, ...]:
        """The payoff to each player when player i chooses action node `nodes[i]`."""
        counts = [0] * len(self.neighbors)
        for node in nodes:
            counts[node] += 1
        return tuple(
            self.node_payoff[node][tuple(counts[nb] for nb in self.neighbors[node])]
            for node in nodes
        )


def _tokens(path: pathlib.Path) -> list[str]:
    lines = [ln for ln in path.read_text().splitlines() if not ln.lstrip().startswith("#")]
    return re.sub(r"[\[\]]", " ", "\n".join(lines)).split()


def read_description(path: str | pathlib.Path) -> BAGGDescription:
    """Parse a `.bagg` file that has no function nodes and uses payoff types 0 or 1."""
    path = pathlib.Path(path)
    if not path.is_absolute():
        path = TEST_GAMES / path
    tokens = iter(_tokens(path))

    def take() -> str:
        return next(tokens)

    num_players, num_nodes, num_functions = int(take()), int(take()), int(take())
    assert num_functions == 0, "the oracle supports BAGGs without function nodes only"
    num_types = [int(take()) for _ in range(num_players)]
    priors = [[Fraction(take()) for _ in range(t)] for t in num_types]
    sizes = [[int(take()) for _ in range(t)] for t in num_types]
    action_sets = [
        [[int(take()) for _ in range(size)] for size in player_sizes] for player_sizes in sizes
    ]
    neighbors = [[int(take()) for _ in range(int(take()))] for _ in range(num_nodes)]

    player_nodes = [sorted({n for tset in sets for n in tset}) for sets in action_sets]
    configurations = [set() for _ in range(num_nodes)]
    for nodes in itertools.product(*player_nodes):
        counts = [0] * num_nodes
        for node in nodes:
            counts[node] += 1
        for node in nodes:
            configurations[node].add(tuple(counts[nb] for nb in neighbors[node]))

    node_payoff = []
    for node in range(num_nodes):
        kind = int(take())
        if kind == 0:
            table = {c: Fraction(take()) for c in sorted(configurations[node])}
        elif kind == 1:
            table = {}
            for _ in range(int(take())):
                config = tuple(int(take()) for _ in neighbors[node])
                table[config] = Fraction(take())
        else:
            raise ValueError(f"unsupported payoff function type {kind}")
        node_payoff.append(table)
    return BAGGDescription(priors, action_sets, neighbors, node_payoff)


def _rational(value: Fraction) -> gbt.Rational:
    return gbt.Rational(str(value))


def bagg_to_efg(source: str | pathlib.Path | BAGGDescription) -> gbt.Game:
    """The extensive game equivalent to a BAGG, with exact (rational) probabilities and payoffs.

    Chance draws the types of players 1, 2, ... in turn; then each player in turn moves at
    the information set of its realised type.  Since a BAGG is simultaneous-move, no player
    observes another's action.  Types are labelled "1", "2", ... and actions "1", "2", ...
    by position within the type's action set.
    """
    desc = source if isinstance(source, BAGGDescription) else read_description(source)
    players = [str(i + 1) for i in range(desc.num_players)]
    game = gbt.ExtensiveGame(players=players, title="Oracle extensive game for a BAGG")

    for priors in desc.priors:
        game.append_event(
            gbt.H.plays,
            {str(t + 1): _rational(p) for t, p in enumerate(priors)},
        )
    for i, player in enumerate(players):
        for t, nodes in enumerate(desc.action_sets[i]):
            game.append_move(
                gbt.H.plays.filter(lambda h, i=i, t=t: h[i].action == str(t + 1)),
                player,
                [str(k + 1) for k in range(len(nodes))],
            )

    n = desc.num_players
    type_choices = [range(len(p)) for p in desc.priors]
    for types in itertools.product(*type_choices):
        action_choices = [range(len(desc.action_sets[i][t])) for i, t in enumerate(types)]
        for actions in itertools.product(*action_choices):
            nodes = tuple(desc.action_sets[i][types[i]][actions[i]] for i in range(n))
            path = [str(t + 1) for t in types] + [str(a + 1) for a in actions]
            label = "".join(path)
            game.make_outcome(
                gbt.H.path(*path),
                {players[i]: _rational(v) for i, v in enumerate(desc.payoffs(nodes))},
                label,
            )
    return game


def infoset_selector(game: gbt.Game, player: int, type_: int) -> gbt.Selector:
    """A selector for the information set of 1-based `player` at 1-based `type_`."""
    n = len(game.players)
    chance_prefix = ["1"] * n
    chance_prefix[player - 1] = str(type_)
    return gbt.H.path(*chance_prefix, *["1"] * (player - 1))


def behavior_profile(
    game: gbt.Game, strategies: dict[tuple[int, int], list], rational: bool = True
):
    """A mixed behavior profile of the oracle game.

    `strategies` maps (player, type), both 1-based, to action weights by position, which are
    normalised to probabilities.
    Information sets not mentioned are left at the uniform distribution.
    """
    profile = game.mixed_behavior_profile(rational=rational)
    for (player, type_), weights in strategies.items():
        total = sum(Fraction(w) for w in weights)
        profile[infoset_selector(game, player, type_)] = {
            str(k + 1): _rational(Fraction(w) / total) if rational else float(Fraction(w) / total)
            for k, w in enumerate(weights)
        }
    return profile
