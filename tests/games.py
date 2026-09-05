"""A utility module to create/load games for the test suite."""

import itertools
import pathlib
from abc import ABC, abstractmethod

import numpy as np

import pygambit as gbt


def all_infosets(game: gbt.Game) -> list[tuple]:
    """The History of a representative member of every information set belonging
    to a personal player, across every player, in the canonical order
    `Game.infosets` used to yield before its removal (17.0.0). One History per
    information set, matching `Game.get_infosets`."""
    return [h for p in game.players for h in game.get_infosets(p)]


def all_nodes(game: gbt.Game) -> list[gbt.Node]:
    """Every node in the game, in depth-first traversal order, matching
    `Game.nodes` before its removal (17.0.0)."""
    return game._all_nodes()


def children_histories(game: gbt.Game, history: tuple) -> list[tuple]:
    """The Histories of the children of the node at `history` -- the standard way
    to enumerate a node's children now that `Node.children` is removed (17.0.0)."""
    return [(*history, action) for action in game.get_actions(gbt.H.path(*history))]


def children_of(game: gbt.Game, history: tuple) -> list[gbt.Node]:
    """The children of the node at `history`, as real Nodes -- for callers that
    need actual `Node` objects (e.g. `.members`/`.player`), not just Histories."""
    return [node_at_history(game, h) for h in children_histories(game, history)]


def player_infosets(game: gbt.Game, player: str) -> list[tuple]:
    """The History of a representative member of every information set belonging
    to `player`, in canonical order, matching `Player.infosets` before its
    removal (17.0.0)."""
    return list(game.get_infosets(player))


# (game.title, infoset label) -> the History of a member node, hand-verified against
# each fixture file before `Infoset.label` (and thus by-label lookup) was removed
# (17.0.0). `find_infoset`/`find_infoset_in_game` are a fixed stand-in for that
# removed lookup, recognizing only the fixture games this test suite happens to use
# it with; add an entry here if a new fixture/label combination is needed.
_INFOSET_LABEL_HISTORIES = {
    ("Test Extensive Form Game", "Infoset 1:1"): (),
    ("Test Extensive Form Game", "Infoset 2:1"): ("U1",),
    ("Test Extensive Form Game", "Infoset 3:1"): ("U1", "U2"),
    ("A simple Poker game", "Alice has King"): ("King",),
    ("A simple Poker game", "Alice has Queen"): ("Queen",),
    ("A simple Poker game", "Bob's response"): ("King", "Bet"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,1)"):
        ("1=rational", "2=rational"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,3)"):
        ("1=rational", "2=rational", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,5)"):
        ("1=rational", "2=rational", "p", "p", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,2)"):
        ("1=altruist", "2=rational"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,4)"):
        ("1=altruist", "2=rational", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(1,6)"):
        ("1=altruist", "2=rational", "p", "p", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,1)"):
        ("1=rational", "2=rational", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,4)"):
        ("1=rational", "2=rational", "p", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,5)"):
        ("1=rational", "2=rational", "p", "p", "p", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,2)"):
        ("1=rational", "2=altruist", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,3)"):
        ("1=rational", "2=altruist", "p", "p", "p"),
    ("Centipede game. Three inning with probability of altruism.  ", "(2,6)"):
        ("1=rational", "2=altruist", "p", "p", "p", "p", "p"),
    ("AM-driver variation", "Absent-minded"): (),
    ("AM-driver variation", "Second"): ("1",),
    ("AM-driver variation", "Third"): ("1", "1", "2"),
    ("AM-game with two players", "Absent-minded"): (),
    ("AM-game with two players", "Response 1"): ("1", "1"),
    ("AM-game with two players", "Response 2"): ("1", "2"),
    ("AM-game with two players", "Response 3"): ("2", "1"),
    ("AM-game with two players", "Response 4"): ("2", "2"),
    ("Untitled Extensive Game", "Absent-minded"): (),
    ("Untitled Extensive Game", "Second"): ("1",),
    ("Untitled Extensive Game", "Player 2"): ("1", "1", "2", "2"),
}


def node_at_history(game: gbt.Game, history: tuple) -> gbt.Node:
    """The Node reached by following `history` (a tuple of action labels) from
    the root -- the standard way to obtain a `Node` now that `Game.root` is
    removed (17.0.0): `H.path(*history)` always resolves to exactly one node."""
    return game._get_nodes(gbt.H.path(*history))[0]


def find_infoset(game: gbt.Game, player: str, label: str) -> gbt.Node:
    """The representative node of `player`'s information set historically identified
    by `label`, matching `Player.infosets[label]` before its removal (17.0.0)."""
    node = node_at_history(game, _INFOSET_LABEL_HISTORIES[(game.title, label)])
    assert node.player == player
    return node


def find_infoset_in_game(game: gbt.Game, label: str) -> gbt.Node:
    """The representative node of the information set historically identified by
    `label`, searching across all (personal) players, matching
    `Game.infosets[label]` before its removal (17.0.0)."""
    return node_at_history(game, _INFOSET_LABEL_HISTORIES[(game.title, label)])


def _node_history(node: gbt.Node) -> tuple:
    """The plain-tuple history of `node`, walked via the private
    `Node._parent`/`._prior_action`."""
    labels = []
    current = node
    while current._parent() is not None:
        labels.append(current._prior_action().label)
        current = current._parent()
    labels.reverse()
    return tuple(labels)


def selector_for_nodes(nodes: list[gbt.Node]) -> gbt.Selector:
    """A `Selector` matching exactly the given (possibly scattered, mixed-depth)
    nodes -- for adapting fixtures that compute a `Node` list dynamically to the
    `H`-only mutation methods."""
    histories = frozenset(_node_history(n) for n in nodes)
    return gbt.H.after().filter(lambda h: h[:] in histories)


def selector_for_histories(histories: list[tuple]) -> gbt.Selector:
    """A `Selector` matching exactly the given (possibly scattered, mixed-depth)
    Histories -- the History-only counterpart to `selector_for_nodes`, for
    fixtures that already work in terms of `History` rather than `Node`."""
    histories = frozenset(histories)
    return gbt.H.after().filter(lambda h: h[:] in histories)


def selector_for_node(node: gbt.Node) -> gbt.Selector:
    """A root-anchored `Selector` matching exactly `node`'s own path of action
    labels -- for adapting a fixture's `Node` (e.g. one from `Game.get_infosets`)
    to profile indexing, which is `Selector`-only."""
    return gbt.H.path(*_node_history(node))


# Label-validation fixtures.
# VALID: accepted by the C++ validator (IsValidLabel in src/games/game.h), including
#        well-formed UTF-8 text (#862, 17.0.0). A single Unicode whitespace character
#        (not just ASCII space) between two printables is valid, e.g. a no-break space.
# INVALID: rejected by the validator -> ValueError.  Includes structural violations
#          (leading/trailing/double whitespace) and control characters.  "Whitespace"
#          is generalized to any Unicode space separator (category Zs), not just the
#          literal ASCII space -- so a no-break space (U+00A0) at the start/end, or
#          doubled with an ordinary space, is invalid the same way a plain space is.
#          Also includes control characters, both as literal ASCII bytes and as a
#          control code point reached via a multi-byte UTF-8 encoding (U+0085 NEL,
#          U+2028 LINE SEPARATOR).
# UNICODE_LABELS: non-ASCII labels, also included in VALID_LABELS; kept separate so
#                 tests can specifically exercise multi-byte UTF-8 decoding.
UNICODE_LABELS = ["é", "naïve", "日本語", "😀"]
VALID_LABELS = ["x", "a b", "a b", "a b c", *UNICODE_LABELS]
INVALID_LABELS = [
    " x",
    "x ",
    " ",
    "a  b",
    "a\tb",
    "a\nb",
    "a\x01b",
    "a\x7fb",
    "ab",
    " x",
    "x ",
    " ",
    "a  b",
    "a b",
]


def read_from_file(fn: str) -> gbt.Game:
    if fn.endswith(".efg"):
        return gbt.read_efg(pathlib.Path("tests/test_games") / fn)
    elif fn.endswith(".nfg"):
        return gbt.read_nfg(pathlib.Path("tests/test_games") / fn)
    elif fn.endswith(".agg"):
        return gbt.read_agg(pathlib.Path("tests/test_games") / fn)
    elif fn.endswith(".bagg"):
        return gbt.read_bagg(pathlib.Path("tests/test_games") / fn)
    else:
        raise ValueError(f"Unknown file extension in {fn}")


def create_efg_corresponding_to_bimatrix_game_arrays(
    A: np.ndarray, B: np.ndarray, title: str
) -> gbt.Game:
    """
    There is no direct pygambit method to create an EFG from a stategic-form game.
    Here we create an EFG corresponding to a bimatrix game, given by two numpy arrays.
    Player 1 moves first.
    """
    assert A.shape == B.shape
    m, n = A.shape
    g = gbt.Game.new_tree(players=["1", "2"], title=title)
    actions1 = [str(i) for i in range(m)]
    actions2 = [str(i) for i in range(n)]
    g.append_move(gbt.H.path(), "1", actions1)
    g.append_move(gbt.H.path(...), "2", actions2)
    for i, j in itertools.product(range(m), range(n)):
        g.make_outcome(gbt.H.path(str(i), str(j)), {"1": A[i, j], "2": B[i, j]}, f"({i},{j})")
    return g


def create_efg_corresponding_to_bimatrix_game(g: gbt.Game) -> gbt.Game:
    """
    There is no direct pygambit method to create an EFG from a stategic-form game.
    Here we create an EFG corresponding to a bimatrix pygambit.Game.
    Player 1 moves first.
    """
    assert len(g.players) == 2
    A, B = g.to_arrays()
    return create_efg_corresponding_to_bimatrix_game_arrays(A, B, g.title)


################################################################################################
# Extensive-form games (efg)


def create_2x2_zero_sum_efg(variant: None | str = None) -> gbt.Game:
    """
    EFG corresponding to 2x2 zero-sum game (I,-I).

    If variant is:
        - "missing term outcome", terminal node after action 0 then 1 does not have an outcome.
        - "with neutral outcome", there is a (0,0) payoff outcomes at a non-terminal node.
    """
    title = "EFG for 2x2 zero-sum game (I,-I)"

    if variant:
        title += " " + variant

    A = np.eye(2)
    B = -A
    g = create_efg_corresponding_to_bimatrix_game_arrays(A, B, title)

    if variant == "missing term outcome":
        g.make_outcome_null(gbt.H.path("0", "1"))
    elif variant == "with neutral outcome":
        g.make_outcome(gbt.H.path("0"), {"1": 0, "2": 0}, "neutral")

    return g


def create_stripped_down_poker_efg(nonterm_outcomes: bool = False) -> gbt.Game:
    """
    Returns
    -------
    Game
        Stripped-Down Poker: A Classroom Game with Signaling and Bluﬃng
        Reiley et al (2008)

        Two-player extensive-form poker game between Alice and Bob
        Chance deals King or Queen to Fred
        Alice can then Bet or Fold; after raising Bob is in an infoset with two nodes
        and can choose to Call or Fold
    """
    if not nonterm_outcomes:
        return read_from_file("stripped_down_poker.efg")

    g = gbt.Game.new_tree(
        players=["Alice", "Bob"],
        title="Stripped-Down Poker: a simple game of one-card\
                                            poker from Reiley et al (2008).",
    )
    deals = ["King", "Queen"]
    g.append_event(gbt.H.path(), dict.fromkeys(deals, gbt.Rational(1, 2)))

    for card in deals:
        g.append_move(
            gbt.H.path(...).filter(lambda h, card=card: h[0] == card),
            player="Alice", actions=["Bet", "Fold"]
        )

    g.append_move(gbt.H.path(..., "Bet"), player="Bob", actions=["Call", "Fold"])

    g.make_outcome(gbt.H.path(), {"Alice": -1, "Bob": -1}, "Ante")
    g.make_outcome(gbt.H.path(..., "Fold"), {"Alice": 0, "Bob": 2}, "Alice Folds")
    g.make_outcome(gbt.H.path(..., "Bet"), {"Alice": -1, "Bob": 0}, "Alice Bets")
    g.make_outcome(gbt.H.path(..., "Bet", "Fold"), {"Alice": 3, "Bob": 0}, "Bob Folds")
    g.make_outcome(
        gbt.H.path("King", "Bet", "Call"), {"Alice": 4, "Bob": -1}, "Bob Calls and Loses"
    )
    g.make_outcome(
        gbt.H.path("Queen", "Bet", "Call"), {"Alice": 0, "Bob": 3}, "Bob Calls and Wins"
    )
    return g


def _create_kuhn_poker_efg_without_outcomes():
    """
    Used in create_kuhn_poker_efg()
    """
    g = gbt.Game.new_tree(players=["Alice", "Bob"], title="Three-card poker (J, Q, K), two-player")
    cards = ["J", "Q", "K"]
    deals = ["JQ", "JK", "QJ", "QK", "KJ", "KQ"]

    g.append_event(gbt.H.path(), dict.fromkeys(deals, gbt.Rational(1, 6)))
    for alice_card in cards:
        # Alice's first move
        g.append_move(
            gbt.H.path(...).filter(lambda h, card=alice_card: h[0][0] == card),
            "Alice", ["Check", "Bet"]
        )
    for bob_card in cards:
        # Bob's move after Alice checks
        g.append_move(
            gbt.H.path(..., "Check").filter(lambda h, card=bob_card: h[0][1] == card),
            "Bob", ["Check", "Bet"]
        )
    for alice_card in cards:
        # Alice's move if Bob's second action is bet
        g.append_move(
            gbt.H.path(..., "Check", "Bet").filter(lambda h, card=alice_card: h[0][0] == card),
            "Alice", ["Fold", "Call"]
        )
    for bob_card in cards:
        # Bob's move after Alice bets initially
        g.append_move(
            gbt.H.path(..., "Bet").filter(lambda h, card=bob_card: h[0][1] == card),
            "Bob", ["Fold", "Call"]
        )
    return g


def _kuhn_showdown_winner(deal: str):
    """
    Used in:
    _create_kuhn_poker_efg_only_term_outcomes();
    _create_kuhn_poker_efg_nonterm_outcomes()
    """
    # deal is an element of deals = ["JQ", "JK", "QJ", "QK", "KJ", "KQ"]
    card_values = dict(J=0, Q=1, K=2)
    a, b = deal
    return "Alice" if card_values[a] > card_values[b] else "Bob"


def _create_kuhn_poker_efg_only_term_outcomes() -> gbt.Game:
    """
    Used in create_kuhn_poker_efg()
    """
    g = _create_kuhn_poker_efg_without_outcomes()

    def calculate_payoffs(term_node):
        def get_path(node):
            path = []
            while node._parent():
                path.append(node._prior_action().label)
                node = node._parent()
            return path

        def showdown(deal, payoffs, pot):
            payoffs[_kuhn_showdown_winner(deal)] += pot
            return payoffs

        def bet(player, payoffs, pot):
            payoffs[player] += -1
            pot += 1
            return payoffs, pot

        path = get_path(term_node)
        deal = path.pop()  # needed if there is a showdown
        payoffs = dict(Alice=-1, Bob=-1)  # ante of 1 for both players
        pot = 2
        if path.pop() == "Check":  # Alice checks
            if path.pop() == "Check":  # Bob checks
                payoffs = showdown(deal, payoffs, pot)
            else:  # Bob bets
                payoffs, pot = bet("Bob", payoffs, pot)
                if path.pop() == "Fold":  # Alice folds
                    payoffs["Bob"] += pot
                else:  # Alice calls
                    payoffs, pot = bet("Alice", payoffs, pot)
                    payoffs = showdown(deal, payoffs, pot)
        else:  # Alice bets
            payoffs, pot = bet("Alice", payoffs, pot)
            if path.pop() == "Fold":  # Bob
                payoffs["Alice"] += pot
            else:  # Bob calls
                payoffs, pot = bet("Bob", payoffs, pot)
                payoffs = showdown(deal, payoffs, pot)

        return tuple(payoffs.values())

    # group terminal nodes by their payoffs, so each of the 4 possible outcomes is created once
    payoff_labels = {
        (1, -1): "Alice wins 1",
        (2, -2): "Alice wins 2",
        (-1, 1): "Bob wins 1",
        (-2, 2): "BOb wins 2",
    }
    nodes_by_payoff = {payoffs: [] for payoffs in payoff_labels}
    for term_node in [n for n in all_nodes(g) if not g.get_actions(selector_for_node(n))]:
        nodes_by_payoff[calculate_payoffs(term_node)].append(term_node)

    for payoffs, nodes in nodes_by_payoff.items():
        g.make_outcome(
            selector_for_nodes(nodes), {"Alice": payoffs[0], "Bob": payoffs[1]},
            payoff_labels[payoffs]
        )

    return g


def _create_kuhn_poker_efg_nonterm_outcomes() -> gbt.Game:
    """
    Used in create_kuhn_poker_efg()
    """
    g = _create_kuhn_poker_efg_without_outcomes()

    # each outcome's payoffs, keyed by the same labels used below; collected up front so each
    # outcome can be created once, attached to every node (terminal or not) that shares it.
    payoffs_by_key = {"Ante": (-1, -1)}
    for player in ["Alice", "Bob"]:
        payoffs_by_key[f"{player} bets"] = (-1, 0) if player == "Alice" else (0, -1)
        payoffs_by_key[f"{player} wins showdown for pot of 2"] = (
            (2, 0) if player == "Alice" else (0, 2)
        )
        payoffs_by_key[f"{player} folds"] = (0, 3) if player == "Alice" else (3, 0)
        payoffs_by_key[f"{player} calls and wins"] = (3, 0) if player == "Alice" else (0, 3)
        payoffs_by_key[f"{player} calls and loses"] = (-1, 4) if player == "Alice" else (4, -1)

    nodes_by_key = {key: [] for key in payoffs_by_key}
    nodes_by_key["Ante"].append(node_at_history(g, ()))

    def collect_nodes(term_node):
        def get_path(node):
            path = []
            while node._parent():
                path.append((node, node._prior_action().label))
                node = node._parent()
            return path

        path = get_path(term_node)
        _, deal = path.pop()
        winner = _kuhn_showdown_winner(deal)  # needed if there is a showdown

        n, label = path.pop()
        if label == "Check":  # Alice checks
            n, label = path.pop()
            if label == "Check":  # Bob checks
                nodes_by_key[f"{winner} wins showdown for pot of 2"].append(n)
            else:  # Bob bets
                nodes_by_key["Bob bets"].append(n)
                n, label = path.pop()
                if label == "Fold":  # Alice folds
                    nodes_by_key["Alice folds"].append(n)
                else:  # Alice calls
                    tmp = "wins" if winner == "Alice" else "loses"
                    nodes_by_key[f"Alice calls and {tmp}"].append(n)
        else:  # Alice bets
            nodes_by_key["Alice bets"].append(n)
            n, label = path.pop()
            if label == "Fold":  # Bob
                nodes_by_key["Bob folds"].append(n)
            else:  # Bob calls
                tmp = "wins" if winner == "Bob" else "loses"
                nodes_by_key[f"Bob calls and {tmp}"].append(n)

    for term_node in [n for n in all_nodes(g) if not g.get_actions(selector_for_node(n))]:
        collect_nodes(term_node)

    for key, nodes in nodes_by_key.items():
        # the same non-terminal node is revisited once per terminal descendant walked above
        deduped_nodes = list(dict.fromkeys(nodes))
        alice_payoff, bob_payoff = payoffs_by_key[key]
        g.make_outcome(
            selector_for_nodes(deduped_nodes), {"Alice": alice_payoff, "Bob": bob_payoff}, key
        )

    return g


def create_kuhn_poker_efg(nonterm_outcomes: bool = False) -> gbt.Game:
    """
    Returns
    -------
    Game
        Kuhn poker with 3 cards and 2 players

        If nonterm_outcomes is True then the ante and bets are captured with nonterminal
        outcomes; else the only outcomes are at terminal nodes.
        In both cases, all terminal nodes have outcomes.
    """
    if nonterm_outcomes:
        g = _create_kuhn_poker_efg_nonterm_outcomes()
    else:
        g = _create_kuhn_poker_efg_only_term_outcomes()
    return g


def kuhn_poker_lp_mixed_strategy_prof():
    """
    Returns
    -------
    Data for the extreme equilibrium in mixed stategies for Kuhn poker found by lp_solve
    """
    alice = [0] * 27
    alice[2] = "6/15"
    alice[4] = "7/15"
    alice[19] = "2/15"

    bob = [0] * 64
    bob[12] = "1/3"
    bob[14] = "1/3"
    bob[28] = "1/3"
    return [alice, bob]


def kuhn_poker_lcp_first_mixed_strategy_prof():
    """
    Returns
    -------
    Data for the first extreme equilibrium in mixed stategies for Kuhn poker found by lcp_solve
    """
    alice = [0] * 27
    alice[1] = "2/3"
    alice[4] = "1/3"
    bob = [0] * 64
    bob[12] = "2/3"
    bob[30] = "1/3"
    return [alice, bob]


def create_one_shot_trust_efg(unique_NE_variant: bool = False) -> gbt.Game:
    """
    TODO: this could be replaced with two .efg files

    One-shot trust game, after Kreps (1990)

    The unique_NE_variant makes Trust a dominant strategy, replacing the
    non-singleton equilibrium component from the standard version of the game
    where the Buyer plays "Not Trust" and the seller can play any mixture with
    < 0.5 probability on Honor with a unique NE where the Buyer plays Trust and
    the Seller plays Abuse.
    """
    g = gbt.Game.new_tree(
        players=["Buyer", "Seller"], title="One-shot trust game, after Kreps (1990)"
    )
    g.append_move(gbt.H.path(), "Buyer", ["Trust", "Not trust"])
    g.append_move(gbt.H.path("Trust"), "Seller", ["Honor", "Abuse"])
    g.make_outcome(
        gbt.H.path("Trust", "Honor"), {"Buyer": 1, "Seller": 1}, "Trustworthy"
    )
    if unique_NE_variant:
        g.make_outcome(
            gbt.H.path("Trust", "Abuse"),
            {"Buyer": "1/2", "Seller": 2},
            "Untrustworthy",
        )
    else:
        g.make_outcome(
            gbt.H.path("Trust", "Abuse"),
            {"Buyer": -1, "Seller": 2},
            "Untrustworthy",
        )
    g.make_outcome(gbt.H.path("Not trust"), {"Buyer": 0, "Seller": 0}, "Opt-out")
    return g


def create_EFG_for_nxn_bimatrix_coordination_game(n: int) -> gbt.Game:
    A = np.eye(n, dtype=int)
    B = A
    title = f"{n}x{n} coordination game, {2**n - 1} equilibria"
    return create_efg_corresponding_to_bimatrix_game_arrays(A, B, title)


def create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq() -> gbt.Game:
    # 6 x 6 Payoff matrix A:
    A = [
        [-180, 72, -333, 297, -153, 270],
        [-30, 17, -33, 42, -3, 20],
        [-81, 36, -126, 126, -36, 90],
        [90, -36, 126, -126, 36, -81],
        [20, -3, 42, -33, 17, -30],
        [270, -153, 297, -333, 72, -180],
    ]
    # 6 x 6 Payoff matrix B:
    B = [
        [72, 36, 17, -3, -36, -153],
        [-180, -81, -30, 20, 90, 270],
        [297, 126, 42, -33, -126, -333],
        [-333, -126, -33, 42, 126, 297],
        [270, 90, 20, -30, -81, -180],
        [-153, -36, -3, 17, 36, 72],
    ]
    A = np.array(A)
    B = np.array(B)
    title = "6x6 Long Lemke-Howson Paths, unique eq"
    return create_efg_corresponding_to_bimatrix_game_arrays(A, B, title)


class EfgFamilyForReducedStrategicFormTests(ABC):
    """ """

    @abstractmethod
    def __init__(params):
        pass

    @abstractmethod
    def gbt_game(self):
        pass

    @abstractmethod
    def reduced_strategies(self):
        pass

    @abstractmethod
    def reduced_strategic_form(self):
        pass

    def set_size_of_rsf(self, reduced_strategies):
        self.size_of_rsf = [len(r) for r in reduced_strategies]

    @classmethod
    def get_test_data(cls, **params):
        """
        given the provided parameters, return a tuple with:
            - the game as a gbt.Game object
            - the expected list of players reduced strategies in this game
            - the expected reduced strategic form (i.e. payoff tensors) for this game
        the tuple is used directly in test_reduced_strategic_form in test_extensive.py
        """

        game = cls(params)

        return (
            game.gbt_game(),
            [[str(i) for i in range(1, len(r) + 1)] for r in game.reduced_strategies()],
            game.reduced_strategic_form(),
        )

    @classmethod
    def get_map_test_data(cls, **params):
        """
        given the provided parameters, return a tuple with:
            - the game as a gbt.Game object
            - the expected infoset-to-action map of each reduced strategy, per player:
              the pre-17.0 signature split per information set ("*" = no action
              prescribed), or the empty tuple for the single trivial strategy of a
              player who has no information sets
        the tuple is used directly in test_reduced_strategy_maps in test_extensive.py
        """
        game = cls(params)
        gbt_game = game.gbt_game()
        maps = [
            [tuple(sig) if len(gbt_game.get_infosets(player)) > 0 else () for sig in sigs]
            for player, sigs in zip(gbt_game.players, game.reduced_strategies(), strict=True)
        ]
        return (gbt_game, maps)


class Centipede(EfgFamilyForReducedStrategicFormTests):
    """
    Two-player Centipede game

    Params: number of rounds (N); two payoff parameters (m0, m1)
    """

    def __init__(self, params):
        self.N = params["N"]
        self.m0 = params["m0"]
        self.m1 = params["m1"]

    def gbt_game(self):
        g = gbt.Game.new_tree(players=["1", "2"], title=f"Centipede Game with {self.N} rounds")
        current_player = "1"
        for t in range(self.N):
            g.append_move(gbt.H.path(*(["Push"] * t)), current_player, ["Take", "Push"])
            payoffs = [2**t * self.m0, 2**t * self.m1]  # take payoffs
            if current_player == "2":
                payoffs.reverse()
            g.make_outcome(
                gbt.H.path(*(["Push"] * t), "Take"), {"1": payoffs[0], "2": payoffs[1]},
                f"take_{t}"
            )
            if t == self.N - 1:  # for last round, push payoffs
                payoffs = [2 ** (t + 1) * self.m1, 2 ** (t + 1) * self.m0]
                if current_player == "2":
                    payoffs.reverse()
                g.make_outcome(
                    gbt.H.path(*(["Push"] * (t + 1))), {"1": payoffs[0], "2": payoffs[1]},
                    f"push_{t}"
                )
            current_player = "2" if current_player == "1" else "1"
        return g

    def reduced_strategies(self):
        if self.N % 2 == 0:
            n_moves = [int(self.N / 2)] * 2
        else:
            n_moves = [int((self.N + 1) / 2), int((self.N - 1) / 2)]

        def get_rss(n):
            # Given n number of times a player moves, their reduced strategies are such that
            # they Have n positions; have all *s after any 1; have prefixes 1, 21, 221, 2221, etc.
            # and finally the last strategyt is all 2s
            ret = ["2" * (i) + "1" * 1 + "*" * (n - i - 1) for i in range(n)]
            ret.append("2" * n)
            return ret

        rs = [get_rss(n) for n in n_moves]
        self.set_size_of_rsf(rs)
        return rs

    def reduced_strategic_form(self):
        m, n = self.size_of_rsf
        p1_payoffs = np.zeros((m, n), dtype=int)
        p2_payoffs = np.zeros((m, n), dtype=int)
        row1_1 = [self.m0] * n
        row1_2 = [self.m1] * n
        p1_payoffs[0, :] = row1_1
        p2_payoffs[0, :] = row1_2

        for j in range(n - 1 if self.N % 2 == 0 else n):
            max_in_col_p1 = 2 ** (2 * j + 1) * self.m1
            max_in_col_p2 = 2 ** (2 * j + 1) * self.m0
            base1 = [max_in_col_p1] * (m - 1)
            base2 = [max_in_col_p2] * (m - 1)
            for i in range(1, (j + 1)):
                base1[i - 1] = 2 ** (2 * i) * self.m0
                base2[i - 1] = 2 ** (2 * i) * self.m1
            p1_payoffs[1:, j] = base1
            p2_payoffs[1:, j] = base2
        if self.N % 2 == 0:
            # final col
            p1_payoffs[:, n - 1] = p1_payoffs[:, n - 2]
            p2_payoffs[:, n - 1] = p2_payoffs[:, n - 2]
            p1_extra_pay = 2 ** (2 * (n - 1)) * self.m0
            p2_extra_pay = 2 ** (2 * (n - 1)) * self.m1
        else:
            # final row
            p1_payoffs[m - 1, :] = p1_payoffs[m - 2, :]
            p2_payoffs[m - 1, :] = p2_payoffs[m - 2, :]
            p1_extra_pay = 2 ** (2 * (n) - 1) * self.m1
            p2_extra_pay = 2 ** (2 * (n) - 1) * self.m0
        p1_payoffs[m - 1, n - 1] = p1_extra_pay
        p2_payoffs[m - 1, n - 1] = p2_extra_pay
        return p1_payoffs, p2_payoffs


class BinaryTreeGames(EfgFamilyForReducedStrategicFormTests):
    """
    Params:
        - single positive integer, namely the number of "level"s
        - number of players (currently the 1, 2, and 3-player versions are used in tests)

    These games:
        - are all binary trees with imperfect information
        - after every L/R choice the subsequent, have the two subsequent nodes (unless terminal)
            together and alone in a single infoset (so all infosets except the root are pairs)
        - the purpose of these games is to test the reduced strategy lists
        - payoff matrices are all zero for simplicity

    These games have rougly 2^root(level) many reduced strategies

    The 2-player versions appear in:

    B. von Stengel, A. van den Elzen, and A. J. J. Talman (2002)
    Computing normal form perfect equilibria for extensive two-person games
    Econometrica 70(2), 693-715

    The 1-player versions have Imperfect Recall
    """

    def __init__(self, n_players, params):
        self.level = params["level"]
        self.players = list(range(1, n_players + 1))
        self.n_players = n_players

    def get_n_infosets(self, level):
        if self.n_players == 1:
            return {1: 2 ** (level - 1)}

        players = list(range(1, self.n_players + 1))
        n_isets = [1] + [0] * (self.n_players - 1)
        whose_turn = 1  # start from player 2 and level 2
        for lev in range(2, level + 1):
            n_isets[whose_turn] += 2 ** (lev - 2)
            whose_turn = (whose_turn + 1) % self.n_players
        return {p: n_isets[p - 1] for p in players}

    def _redu_strategies_level_1(self, player):
        return ["1", "2"] if player == 1 else ["*"]

    def player_with_changes(self, level):
        return ((level - 1) % self.n_players) + 1

    def last_player_with_changes(self, level):
        return ((level - 2) % self.n_players) + 1

    @abstractmethod
    def _redu_strats(self, player, level):
        pass

    def reduced_strategies(self):
        rs = [self._redu_strats(player, self.level) for player in self.players]
        self.set_size_of_rsf(rs)
        return rs

    def create_binary_tree(self, g, path, whose_turn, depth, max_depth):
        # whose_turn cycles through 0,1,n_players-1; current player is str(whose_turn + 1)
        if depth == max_depth:
            g.make_outcome(
                gbt.H.path(*path), {str(p): 0 for p in self.players},
                f"leaf_{len(list(g.outcomes))}"
            )
        else:
            current_player = str(whose_turn + 1)
            g.append_move(gbt.H.path(*path), current_player, ["L", "R"])

            whose_turn = (whose_turn + 1) % self.n_players
            for label in ["L", "R"]:
                self.create_binary_tree(g, (*path, label), whose_turn, depth + 1, max_depth)

    def gbt_game(self):
        g = gbt.Game.new_tree(
            players=[str(p) for p in self.players],
            title=f"Binary Tree Game (L={self.level})",
        )
        self.create_binary_tree(g, (), 0, 0, self.level)
        for n in all_nodes(g):
            history = _node_history(n)
            if g.get_actions(gbt.H.path(*history)) and g.get_actions(gbt.H.path(*history, "L")):
                left, right = children_of(g, history)
                g.make_infoset(
                    selector_for_nodes(list(left.members) + [right]),
                    left.player,
                )
        return g

    def reduced_strategic_form(self):
        # special case for 1 player
        dims = (self.size_of_rsf[0], 1) if len(self.size_of_rsf) == 1 else self.size_of_rsf

        zeros = np.zeros(dims, dtype=int)
        return [zeros] * len(self.players)


class BinEfgOnePlayerIR(BinaryTreeGames):
    def __init__(self, params):
        super().__init__(n_players=1, params=params)

    def _redu_strats(self, player, level):
        if level == 1:
            return self._redu_strategies_level_1(player)
        else:
            tmp = self._redu_strats(1, level - 1)
            tmp = [t[1:] for t in tmp]  # remove first action (1 from 1st half; 2 from 2nd half)
            n_half = int(len(tmp) / 2)
            first_half = tmp[:n_half]
            second_half = tmp[n_half:]
            n_stars = self.get_n_infosets(level)[1] - self.get_n_infosets(level - 1)[1] - 1
            stars = "*" * n_stars
            return (
                ["11" + t + stars for t in first_half]
                + ["12" + t + stars for t in second_half]
                + ["21" + stars + t for t in first_half]
                + ["22" + stars + t for t in second_half]
            )


class BinEfgTwoOrThreePlayers(BinaryTreeGames):
    def _redu_strats(self, player, level):
        if level == 1:
            return self._redu_strategies_level_1(player)
        elif player == self.player_with_changes(level):
            if player == 1:
                last_player = self.last_player_with_changes(level)
                tmp1 = self.get_n_infosets(level)
                tmp2 = self.get_n_infosets(level - 1)
                n_stars = tmp1[player] - tmp2[last_player] - 1
                stars = "*" * n_stars
                return [
                    "1" + t + stars for t in self._redu_strats(player=last_player, level=level - 1)
                ] + [
                    "2" + stars + t for t in self._redu_strats(player=last_player, level=level - 1)
                ]
            elif player == 2:
                tmp = self._redu_strats(player=1, level=level - 1)
                tmp = [
                    t[1:] for t in tmp
                ]  # remove first action (1 from 1st half; 2 from 2nd half)
                # split into two halves
                n_half = int(len(tmp) / 2)
                first_half = tmp[:n_half]
                second_half = tmp[n_half:]
                # create first half suffix
                first_half = itertools.product(first_half, first_half)
                first_half = ["".join(t) for t in first_half]
                first_half = ["1" + t for t in first_half]  # add 1 to front
                # create second half suffix
                second_half = itertools.product(second_half, second_half)
                second_half = ["".join(t) for t in second_half]
                second_half = ["2" + t for t in second_half]  # add 2 to front
                return first_half + second_half  # glue halves together
            else:  # player == 3:
                tmp = self._redu_strats(player=2, level=level - 1)
                tmp = itertools.product(tmp, tmp)
                tmp = ["".join(t) for t in tmp]
                return tmp
        else:
            return self._redu_strats(player, level - 1)


class BinEfgTwoPlayer(BinEfgTwoOrThreePlayers):
    def __init__(self, params):
        super().__init__(n_players=2, params=params)


class BinEfgThreePlayer(BinEfgTwoOrThreePlayers):
    def __init__(self, params):
        super().__init__(n_players=3, params=params)


def make_rational(input: str):
    return gbt.Rational(input)


vectorized_make_rational = np.vectorize(make_rational)
