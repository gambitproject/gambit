import dataclasses
import functools
import itertools
import typing

import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_infoset_set_label(label):
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.infoset.label = label
    assert game.root.infoset.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_infoset_label_invalid_raises_valueerror(label):
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.root.infoset.label = label


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_infoset_label_unicode_accepted(label):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.infoset.label = label
    assert game.root.infoset.label == label


def test_infoset_label_duplicate_within_player_raises_valueerror():
    game = games.read_from_file("subgames.efg")
    player = next(p for p in game.players if len(game.get_infosets(p)) >= 2)
    first, second = (n.infoset for n in itertools.islice(game.get_infosets(player), 2))
    first.label = "shared"
    with pytest.raises(ValueError):
        second.label = "shared"


def test_infoset_player_retrieval():
    game = games.read_from_file("basic_extensive_game.efg")
    p1, *_ = game.players
    assert p1 == game.root.infoset.player


def test_infoset_node_precedes():
    game = games.read_from_file("basic_extensive_game.efg")
    assert not game.root.infoset.precedes(game.root)
    assert game.root.children["U1"].infoset.precedes(game.root.children["U1"])


def test_make_infoset_change_player_keeps_label():
    """Re-forming an information set under a different player retains an
    explicitly specified label and its membership."""
    game = games.read_from_file("basic_extensive_game.efg")
    _, p2, *_ = game.players
    members = list(game.root.infoset.members)
    game.make_infoset(games.selector_for_nodes(members), p2, "moved")
    assert game.root.infoset.player == p2
    assert game.root.infoset.label == "moved"
    assert list(game.root.infoset.members) == members


def test_make_infoset_terminal_node_raises():
    """All nodes must be decision nodes."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path("U1", "U2", "U3"), game.root.player)


def test_make_infoset_converts_chance_node():
    """A chance node becomes a personal decision node, discarding its probabilities."""
    game = games.read_from_file("stripped_down_poker.efg")
    chance_node = game.root            # the deal is a chance move
    personal = next(n for n in game.nodes if not n.is_terminal and n.infoset)
    game.make_infoset(gbt.H.path(), personal.infoset.player)
    assert not chance_node.event
    assert chance_node.infoset
    assert chance_node.infoset.player == personal.infoset.player


@pytest.mark.parametrize("node_actions", [["c", "d"], ["b", "a"]])
def test_make_infoset_requires_matching_action_labels(node_actions):
    """Nodes must have the same actions, with the same labels in the same order;
    a matching count is not sufficient."""
    game = gbt.Game.new_tree(players=["1"])
    game.append_move(gbt.H.path(), "1", ["a", "b"])
    game.append_move(gbt.H.path("a"), "1", node_actions)
    with pytest.raises(ValueError):
        game.make_infoset(gbt.H.after().filter(lambda h: h[:] in ((), ("a",))), "1")


def test_make_infoset_empty_nodes_raises():
    """`nodes` must be nonempty."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.make_infoset(gbt.H.path(...).filter(lambda h: False), game.root.player)


def test_make_infoset_strategic_game_raises():
    """`make_infoset` is only defined for games with a tree representation."""
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_infoset(gbt.H.path(), "1")


def test_set_move_actions_add_preserves_existing_action_order():
    """New actions may be declared at any position; the existing actions' relative
    order is preserved."""
    game = games.read_from_file("basic_extensive_game.efg")
    labels = list(game.root.actions)
    game.set_move_actions(game.root, labels + ["end"])
    assert list(game.root.actions)[:-1] == labels
    game.set_move_actions(game.root, ["front"] + labels + ["end"])
    assert list(game.root.actions)[1:-1] == labels


@pytest.mark.parametrize(
    "inprobs,outprobs",
    [
        ({"King": "1/4", "Queen": "3/4"}, [gbt.Rational("1/4"), gbt.Rational("3/4")]),
        ({"King": 0.75, "Queen": 0.25}, [0.75, 0.25]),
        ({"King": 1}, [1, 0]),
    ],
)
def test_make_event_sets_probabilities(inprobs, outprobs):
    """Probabilities are given as a mapping from action label to probability,
    which may be sparse: an omitted action is assigned probability zero.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    game.make_event(gbt.H.path(), inprobs, "Deal")
    probs = game.root.action_probs
    for action, prob in zip(game.root.actions, outprobs, strict=True):
        assert probs[action] == prob


@pytest.mark.parametrize("probs", [["1/4", "3/4"], [0.75, 0.25]])
def test_make_event_probs_not_a_mapping_raises_typeerror(probs):
    """A positional sequence of probabilities is no longer accepted."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(TypeError):
        game.make_event(gbt.H.path(), probs, "Deal")


def test_make_event_pools_nodes_from_different_infosets():
    """Nodes in distinct information sets are formed into a single event."""
    game = games.read_from_file("stripped_down_poker.efg")
    nodes = [game.root.children["King"], game.root.children["Queen"]]
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert nodes[0].event == nodes[1].event
    assert nodes[0].event
    assert list(nodes[0].action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]
    assert not game.get_infosets("Alice")


def test_make_event_requires_matching_action_labels():
    """Nodes must have the same actions, with the same labels in the same order."""
    game = games.read_from_file("stripped_down_poker.efg")
    # King node has actions Bet, Fold; its own Bet-child has actions Call, Fold.
    with pytest.raises(ValueError):
        game.make_event(
            gbt.H.after().filter(lambda h: h[:] in (("King",), ("King", "Bet"))),
            {"Bet": "1/2", "Fold": "1/2"}
        )


def test_make_event_converts_personal_node():
    """A personal decision node becomes a chance node carrying the probabilities given."""
    game = games.read_from_file("stripped_down_poker.efg")
    node = next(
        n for n in game.get_infosets("Alice") if n.infoset.label == "Alice has King"
    )
    game.make_event(gbt.H.path("King"), {"Bet": "1/4", "Fold": "3/4"})
    assert node.event
    assert list(node.action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]


def test_make_event_terminal_node_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_event(gbt.H.path("King", "Fold"), {"a": "1/2", "b": "1/2"})


def test_make_event_strategic_game_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        game.make_event(gbt.H.path(), {"a": 1})


def test_make_event_empty_nodes_raises():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path(...).filter(lambda h: False), {"a": "1/2", "b": "1/2"})


def test_make_event_label_held_by_rump_raises():
    """A label may be reused only if all members of the event holding it are absorbed."""
    game = games.read_from_file("stripped_down_poker.efg")
    game.make_event(gbt.H.path(...), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    before = game.to_efg()
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path("King"), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    assert game.to_efg() == before


def test_make_event_label_reused_when_fully_absorbed():
    """A label held by an existing event may be reused once all of that
    event's members are absorbed into the new one; the old event is not left behind.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    nodes = [game.root.children["King"], game.root.children["Queen"]]
    game.make_event(gbt.H.path(...), {"Bet": "1/2", "Fold": "1/2"}, "Coin")
    game.make_event(gbt.H.path(...), {"Bet": "1/4", "Fold": "3/4"}, "Coin")
    assert nodes[0].event == nodes[1].event
    assert nodes[0].event.label == "Coin"
    assert list(nodes[0].action_probs.values()) == [gbt.Rational("1/4"), gbt.Rational("3/4")]
    assert [
        n.event.label for n in game.get_events()
    ].count("Coin") == 1


@pytest.mark.parametrize(
    "probs", [{"King": "3/4", "Queen": "-1/2"}, {"King": 0.75, "Queen": 0.40},
              {"King": "foo", "Queen": "bar"}]
)
def test_make_event_invalid_probs_raises(probs):
    """Values must be numbers, non-negative, and sum to exactly one."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError):
        game.make_event(gbt.H.path(), probs)


def test_make_event_malformed_probs_raises():
    """An unknown action label as a mapping key raises KeyError."""
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(KeyError):
        game.make_event(gbt.H.path(), {"Jack": 1})


@dataclasses.dataclass
class AbsentMindednessTestCase:
    """TestCase for testing is_absent_minded."""
    factory: typing.Callable[[], gbt.Game]
    expected_am_paths: list[list[str]]


ABSENT_MINDEDNESS_CASES = [
    # Games without absent-mindedness
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/ijgt/selten1975/fig2"),
            expected_am_paths=[]
        ),
        id="short_centipede_perfect_info"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            expected_am_paths=[]
        ),
        id="poker_stripped"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "basic_extensive_game.efg"),
            expected_am_paths=[]
        ),
        id="basic_extensive"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "gilboa_two_am_agents.efg"),
            expected_am_paths=[]
        ),
        id="gilboa_forgetting_info"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/geb/wichardt2008"),
            expected_am_paths=[]
        ),
        id="wichardt_forgetting_action"
    ),
    # Games with absent-mindedness
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-AM-driver-two-players.efg"),
            expected_am_paths=[[]]
        ),
        id="AM_driver_two_players"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-action-AM.efg"),
            expected_am_paths=[[]]
        ),
        id="AM_forgetting_action"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-action-AM-two-hops.efg"),
            expected_am_paths=[["2", "1", "1", "1", "1"], ["1", "1", "1"]]
        ),
        id="AM_infoset_takes_two_hops"
    ),
]


def _get_node_by_path(game, path: list[str]) -> gbt.Node:
    """
    Helper to find a node by following a sequence of action labels.
    """
    node = game.root
    for action_label in reversed(path):
        node = node.children[action_label]
    return node


@pytest.mark.parametrize("test_case", ABSENT_MINDEDNESS_CASES)
def test_infoset_is_absent_minded(test_case: AbsentMindednessTestCase):
    """
    Test `infoset.is_absent_minded`.

    Verifies that the set of infosets marked as absent-minded matches the
    expected set derived from action paths.
    """
    game = test_case.factory()

    expected_infosets = {
        _get_node_by_path(game, path).infoset
        for path in test_case.expected_am_paths
        }
    actual_infosets = {
        n.infoset for p in game.players for n in game.get_infosets(p)
        if n.infoset.is_absent_minded
    }

    assert actual_infosets == expected_infosets


def _bagwell_p2_nodes(game: gbt.Game) -> tuple[gbt.Node, gbt.Node, gbt.Node, gbt.Node]:
    """Player 2's four decision nodes in Bagwell (1995).

    Player 2 has two information sets -- one for each signal the chance move
    can produce -- each with two members and actions ("S", "C").  Returns
    (A, B, C, D) with {A, B} the members of one and {C, D} of the other.
    """
    return (game.root.children["S"].children["s"],
            game.root.children["C"].children["s"],
            game.root.children["S"].children["c"],
            game.root.children["C"].children["c"])


def test_make_infoset_cherry_pick_leaves_rumps():
    """Partial consumption leaves the remainders behind, labels retained."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    A.infoset.label = "X"
    C.infoset.label = "Y"
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2")
    assert B.infoset == C.infoset
    assert list(A.infoset.members) == [A]
    assert list(D.infoset.members) == [D]
    assert A.infoset.label == "X"
    assert D.infoset.label == "Y"


def test_make_infoset_label_held_by_rump_raises():
    """Reusing a label whose infoset is only partly consumed is rejected."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    A.infoset.label = "X"
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "X")     # A remains in "X"


def test_make_infoset_failure_leaves_game_unchanged():
    """A rejected call must not modify the partition."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    A.infoset.label = "X"
    C.infoset.label = "Y"
    with pytest.raises(ValueError):
        game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "X")
    assert A.infoset == B.infoset
    assert C.infoset == D.infoset
    assert A.infoset.label == "X"
    assert C.infoset.label == "Y"


def test_make_infoset_idempotent():
    """Repeating a call is a no-op: label reuse permits equality of membership."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "Z")
    game.make_infoset(games.selector_for_nodes([B, C]), "Player 2", "Z")
    assert B.infoset == C.infoset
    assert B.infoset.label == "Z"


def test_make_infoset_split_leaves_new_infoset_unlabeled():
    """A node split out gets a fresh unlabeled infoset; the rump keeps the label."""
    game = gbt.catalog.load("journals/geb/bagwell1995")
    A, B, C, D = _bagwell_p2_nodes(game)
    A.infoset.label = "X"
    game.make_infoset(gbt.H.path("S", "s"), "Player 2")
    assert A.infoset.label == ""
    assert B.infoset.label == "X"


def test_make_infoset_across_different_source_players():
    """Nodes drawn from different players all land under the target player."""
    game = gbt.Game.new_tree(players=["1", "2", "3"])
    game.append_move(gbt.H.path(), "1", ["a", "b"])
    game.append_move(gbt.H.path("a"), "2", ["a", "b"])   # player 2
    game.append_move(gbt.H.path("b"), "3", ["a", "b"])   # player 3
    n2 = game.root.children["a"]
    n3 = game.root.children["b"]
    assert n2.infoset.player == "2"
    assert n3.infoset.player == "3"
    game.make_infoset(gbt.H.path(...), "1")
    assert n2.infoset == n3.infoset
    assert n2.infoset.player == "1"
    assert n3.infoset.player == "1"


def test_infoset_proxy_reresolves_after_split():
    """A node-anchored infoset proxy is lazy: it re-resolves after the node is
    placed in a new information set."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    proxy = node.infoset
    assert len(proxy.members) == 2
    game.make_infoset(gbt.H.path("U1"), node.player)
    assert list(proxy.members) == [node]


def test_infoset_members_is_a_plain_snapshot_list():
    """`members` returns a plain `list`, not a lazily-resolved view: it supports
    integer indexing, and a list obtained before a mutation keeps reflecting the
    information set as it was at the time, rather than tracking its owner."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    members = node.infoset.members
    assert isinstance(members, list)
    assert node in (members[0], members[1])
    game.make_infoset(gbt.H.path("U1"), node.player)
    assert len(members) == 2
    assert list(node.infoset.members) == [node]


def test_reveal_splits_infoset_by_action():
    """Revealing the deal to Bob separates his single infoset into per-card
    singletons; the other player's structure is untouched."""
    game = games.create_stripped_down_poker_efg(nonterm_outcomes=True)
    n_alice = len(game.get_infosets("Alice"))
    assert len(game.get_infosets("Bob")) == 1
    game.reveal(game.root, "Bob")
    bob = game.get_infosets("Bob")
    assert len(bob) == 2
    assert all(len(list(n.infoset.members)) == 1 for n in bob)
    assert len(game.get_infosets("Alice")) == n_alice


def test_reveal_absent_minded_infoset_raises():
    """Revealing the move at an absent-minded infoset is rejected (17.0)."""
    game = gbt.Game.new_tree(players=["Driver", "2"])
    game.append_move(gbt.H.path(), "Driver", ["Continue", "Exit"])
    game.append_move(gbt.H.path("Continue"), "Driver", ["Continue", "Exit"])
    game.make_infoset(gbt.H.after().filter(lambda h: h[:] in ((), ("Continue",))), "Driver")
    game.append_move(gbt.H.path("Continue", "Continue"), "2", ["l", "r"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.reveal(game.root, "2")
