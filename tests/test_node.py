import dataclasses
import functools
import itertools
import typing

import pytest

import pygambit as gbt

from . import games


def test_get_infoset():
    """Test to ensure that we can retrieve an infoset for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.infoset
    assert game.root.children["U1"].infoset
    assert not game.root.children["U1"].children["D2"].children["U3"].infoset


def test_infoset_equality_is_symmetric():
    """A node-anchored infoset proxy and the resolved Infoset compare equal from either side."""
    game = games.read_from_file("basic_extensive_game.efg")
    proxy = game.root.infoset
    infoset = next(iter(game.infosets))
    assert proxy == infoset
    assert infoset == proxy


def test_node_infoset_truthiness():
    """A node-anchored infoset view is truthy iff the node currently has an
    infoset, and tracks mutation across the terminal boundary."""
    game = games.read_from_file("basic_extensive_game.efg")
    terminal = game.root.children["U1"].children["D2"].children["U3"]
    proxy = terminal.infoset
    assert not proxy
    game.append_move(terminal, game.players["Player 1"], ["a", "b"])
    assert proxy


def test_get_outcome():
    """Test to ensure that we can retrieve an outcome for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert (
        game.root.children["U1"].children["D2"].children["U3"].outcome
        == game.outcomes["Outcome 1"]
    )
    assert not game.root.outcome


def test_set_outcome_null():
    """Setting an outcome to null leaves the node's outcome view falsy."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"].children["U2"].children["U3"]
    game.set_outcome(node, None)
    assert not node.outcome


def test_node_outcome_subscript_tracks_mutation():
    """Indexing the outcome view reads/writes the outcome's payoffs, reflecting mutation."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"].children["D2"].children["U3"]
    proxy = node.outcome
    player = game.players["Player 1"]
    proxy[player] = 7
    assert node.outcome[player] == 7


def test_outcome_equality_is_symmetric():
    """A node-anchored outcome view and the resolved Outcome compare equal from either side."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"].children["D2"].children["U3"]
    proxy = node.outcome
    outcome = game.outcomes["Outcome 1"]
    assert proxy == outcome
    assert outcome == proxy


def test_get_player():
    """Test to ensure that we can retrieve a player for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.player == game.players["Player 1"]
    assert not game.root.children["U1"].children["D2"].children["U3"].player


def test_player_equality_is_symmetric():
    """A node-anchored player view and the resolved Player compare equal from either side."""
    game = games.read_from_file("basic_extensive_game.efg")
    proxy = game.root.player
    player = game.players["Player 1"]
    assert proxy == player
    assert player == proxy


def test_node_player_resolves_chance():
    """At a chance node the player view resolves to the chance player."""
    game = games.read_from_file("stripped_down_poker.efg")
    chance_node = game.root
    assert chance_node.player.is_chance
    assert chance_node.player == game.players.chance


def test_get_game():
    """Test to ensure that we can retrieve the game object from a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game == game.root.game


def test_get_parent():
    """Test to ensure that we can retrieve a parent node for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].parent == game.root
    assert game.root.parent is None


def test_get_prior_action():
    """Test to ensure that we can retrieve the prior action for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].prior_action == game.root.infoset.actions["U1"]
    assert game.root.prior_action is None


def test_get_prior_sibling():
    """Test to ensure that we can retrieve a prior sibling of a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["D1"].prior_sibling == game.root.children["U1"]
    assert game.root.children["U1"].prior_sibling is None


def test_get_next_sibling():
    """Test to ensure that we can retrieve a next sibling of a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].next_sibling == game.root.children["D1"]
    assert game.root.children["D1"].next_sibling is None


def test_is_terminal():
    """Test to ensure that we can check if a given node is a terminal node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.is_terminal is False
    assert game.root.children["U1"].children["U2"].children["U3"].is_terminal is True


def test_is_successor_of():
    """Test to ensure that we can check if a given node is a
    successor of a supplied node
    """
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].is_successor_of(game.root)
    assert not game.root.is_successor_of(game.root.children["U1"])
    with pytest.raises(TypeError):
        game.root.is_successor_of(9)
    with pytest.raises(TypeError):
        game.root.is_successor_of("Test")
    with pytest.raises(TypeError):
        game.root.is_successor_of(game.players["Player 1"])


def _get_path_of_action_labels(node: gbt.Node) -> list[str]:
    """
    Computes the path of action labels from a given node to the root.
    Returns a list of strings.
    """
    if not isinstance(node, gbt.Node):
        raise TypeError(f"Input must be a pygambit.Node, but got {type(node).__name__}")

    path = []
    current_node = node
    while current_node.parent:
        path.append(current_node.prior_action.label)
        current_node = current_node.parent

    return path


@dataclasses.dataclass
class SubgameRootsTestCase:
    """TestCase for testing subgame root detection."""
    factory: typing.Callable[[], gbt.Game]
    expected_paths: list[list[str]]


SUBGAME_ROOTS_CASES = [
    # ------------------------------------------------------------------------
    #                              Empty Game
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameRootsTestCase(factory=gbt.Game.new_tree, expected_paths=[[]]),
        id="empty_tree"
    ),
    # ------------------------------------------------------------------------
    #                      Perfect Information Games
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/ijgt/selten1975/fig2"),
            expected_paths=[[], ["L"], ["L", "L"]]
        ),
        id="centipede_3_rounds"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=lambda: games.Centipede.get_test_data(N=5, m0=2, m1=7)[0],
            expected_paths=[[], ["Push"], ["Push", "Push"], ["Push", "Push", "Push"],
                            ["Push", "Push", "Push", "Push"]]
        ),
        id="centipede_5_rounds"
    ),
    # ------------------------------------------------------------------------
    #              Imperfect Information (No Absent-Mindedness)
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/geb/wichardt2008"),
            expected_paths=[[]]
        ),
        id="wichardt_no_nontrivial_subgames"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(games.read_from_file, "binary_3_levels_generic_payoffs.efg"),
            expected_paths=[[]]
        ),
        id="binary_3_levels_no_nontrivial_subgames"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(
                games.read_from_file,
                "subgame_roots_finder_small_subgames_and_overplapping_infosets.efg"),
            expected_paths=[[], ["1"], ["2"], ["1", "2", "2"], ["2", "1", "2"],
                            ["1", "1", "1", "2", "2"], ["2", "2", "2"]]
        ),
        id="small_subgames_and_overlapping_infosets_inside_subgames_no_Nature_moves"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(
                games.read_from_file,
                "subgame_roots_finder_overplapping_infosets_with_Nature.efg"),
            expected_paths=[[], ["1_2"], ["1_2", "1_3", "1_2"], ["1_3", "1_2"]]
        ),
        id="overlapping_infosets_inside_subgames_and_Nature_move"
    ),
    # ------------------------------------------------------------------------
    #                           Absent-Minded Games
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(games.read_from_file, "AM-subgames.efg"),
            expected_paths=[[], ["2"], ["1", "1"], ["2", "1"]]
        ),
        id="Absent-minded-game-with-paths-intersecting-infoset-two-times"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(games.read_from_file, "noPR-action-AM-two-hops.efg"),
            expected_paths=[[], ["2", "1", "1"]]
        ),
        id="Absent-minded-game-with-paths-intersecting-infoset-three-times"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(games.read_from_file, "AM-unary-hops.efg"),
            expected_paths=[[], ["1", "1"], ["T", "1", "1", "1", "1", "1"]]
        ),
        id="Absent-minded-game-with-paths-intersecting-infoset-two-times"
    ),
    pytest.param(
        SubgameRootsTestCase(
            factory=functools.partial(games.read_from_file, "AM-unary-branches.efg"),
            expected_paths=[[], ["1", "1", "1", "T"]]
        ),
        id="Absent-minded-game-with-paths-intersecting-infoset-two-times"
    ),
]


@pytest.mark.parametrize("test_case", SUBGAME_ROOTS_CASES)
def test_subgame_roots(test_case: SubgameRootsTestCase):
    """
    Tests that the set of nodes marked as subgame roots matches the expected
    set of paths (Action Labels from Root -> Node).
    """
    game = test_case.factory()

    actual_roots = [node for node in game.nodes if node.is_subgame_root]
    actual_paths = [_get_path_of_action_labels(node) for node in actual_roots]

    assert sorted(actual_paths) == sorted(test_case.expected_paths)


# ============================================================================
#                          Subgame tree / GameSubgame
# ============================================================================
@dataclasses.dataclass
class SubgameStructureTestCase:
    """Expected subgame structure of a game.

    `roots` lists each subgame root as a node->root action-label path, in the
    postorder `game.subgames` is expected to produce (children before parents).

    `parents` maps each subgame-root path to its expected parent path
    (or None for the root subgame).

    `children` maps each subgame-root path to the set of its child subgame paths.

    `differences` maps each subgame-root path to the set of
    (player_label, infoset_number) keys in that subgame's difference ---
    the information sets belonging to the subgame but not to any child subgame.
    """
    factory: typing.Callable[[], gbt.Game]
    roots: list[list[str]]
    parents: dict[tuple[str, ...], tuple[str, ...] | None]
    children: dict[tuple[str, ...], set[tuple[str, ...]]]
    differences: dict[tuple[str, ...], set[tuple[str, int]]]


SUBGAME_STRUCTURE_CASES = [
    # ------------------------------------------------------------------------
    #                    EF game with the only subgame
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameStructureTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/geb/wichardt2008"),
            roots=[[]],
            parents={(): None},
            children={(): set()},
            differences={(): {("Player 1", 0), ("Player 1", 1), ("Player 2", 0)}},
        ),
        id="wichardt_no_nontrivial_subgames",
    ),
    # ------------------------------------------------------------------------
    #                       Tree with eight subgames
    # ------------------------------------------------------------------------
    pytest.param(
        SubgameStructureTestCase(
            factory=functools.partial(games.read_from_file, "subgame-8-roots.efg"),
            roots=[
                ["L", "L", "L", "L", "L"],
                ["R", "L", "L", "L", "L"],
                ["L", "L", "L", "L"],
                ["L", "L"],
                ["R", "L"],
                ["L"],
                ["R"],
                [],
            ],
            parents={
                ("L", "L", "L", "L", "L"): ("L", "L", "L", "L"),
                ("R", "L", "L", "L", "L"): ("L", "L", "L", "L"),
                ("L", "L", "L", "L"): ("L", "L"),
                ("L", "L"): ("L",),
                ("R", "L"): ("L",),
                ("L",): (),
                ("R",): (),
                (): None,
            },
            children={
                ("L", "L", "L", "L", "L"): set(),
                ("R", "L", "L", "L", "L"): set(),
                ("L", "L", "L", "L"): {("L", "L", "L", "L", "L"),
                                       ("R", "L", "L", "L", "L")},
                ("L", "L"): {("L", "L", "L", "L")},
                ("R", "L"): set(),
                ("L",): {("L", "L"), ("R", "L")},
                ("R",): set(),
                (): {("L",), ("R",)},
            },
            differences={
                ("L", "L", "L", "L", "L"): {
                    ("Player 1", 3), ("Player 2", 2), ("Player 2", 3),
                },
                ("R", "L", "L", "L", "L"): {("Player 1", 4), ("Player 1", 5)},
                ("L", "L", "L", "L"): {("Player 2", 1)},
                ("L", "L"): {("Player 1", 1), ("Player 1", 2)},
                ("R", "L"): {("Player 1", 6)},
                ("L",): {("Player 2", 0)},
                ("R",): {
                    ("Player 1", 7), ("Player 1", 8), ("Player 1", 9),
                    ("Player 2", 4), ("Player 2", 5), ("Player 2", 6),
                },
                (): {("Player 1", 0)},
            },
        ),
        id="eight_subgames",
    ),
]


@pytest.mark.parametrize("test_case", SUBGAME_STRUCTURE_CASES)
def test_subgames_postorder_sequence(test_case: SubgameStructureTestCase):
    """`game.subgames` produces the expected postorder sequence of roots."""
    game = test_case.factory()
    actual = [_get_path_of_action_labels(sg.root) for sg in game.subgames]
    assert actual == test_case.roots


@pytest.mark.parametrize("test_case", SUBGAME_STRUCTURE_CASES)
def test_subgame_parent_links(test_case: SubgameStructureTestCase):
    """Each subgame's `parent` matches the expected parent path."""
    game = test_case.factory()
    for sg in game.subgames:
        path = tuple(_get_path_of_action_labels(sg.root))
        parent_path = (
            None if sg.parent is None
            else tuple(_get_path_of_action_labels(sg.parent.root))
        )
        assert parent_path == test_case.parents[path]


@pytest.mark.parametrize("test_case", SUBGAME_STRUCTURE_CASES)
def test_subgame_children(test_case: SubgameStructureTestCase):
    """Each subgame's `children` match the expected set of child paths."""
    game = test_case.factory()
    actual = {
        tuple(_get_path_of_action_labels(sg.root)):
            {tuple(_get_path_of_action_labels(c.root)) for c in sg.children}
        for sg in game.subgames
    }
    assert actual == test_case.children


@pytest.mark.parametrize("test_case", SUBGAME_STRUCTURE_CASES)
def test_minimal_subgame_for_each_infoset(test_case: SubgameStructureTestCase):
    """`game.minimal_subgame(infoset)` returns the smallest subgame containing the infoset."""
    game = test_case.factory()
    expected_path_for_key = {
        key: path
        for path, keys in test_case.differences.items()
        for key in keys
    }
    for infoset in game.infosets:
        key = (infoset.player.label, infoset.number)
        actual_path = tuple(_get_path_of_action_labels(game.minimal_subgame(infoset).root))
        assert actual_path == expected_path_for_key[key]


@pytest.mark.parametrize("game_file, expected_node_data", [
    (
        "binary_3_levels_generic_payoffs.efg",
        [
            # Format: (Path in Node->Root order, (Player Label, Infoset Num, Action Label) or None)
            ([], None),
            (["Left"], None),
            (["Left", "Left"], ("Player 1", 0, "Left")),
            (["Right", "Left"], ("Player 1", 0, "Left")),
            (["Right"], None),
            (["Left", "Right"], ("Player 1", 0, "Right")),
            (["Right", "Right"], ("Player 1", 0, "Right")),
        ]
    ),
    (
        gbt.catalog.load("journals/geb/wichardt2008"),
        [
            ([], None),
            (["R"], ("Player 1", 0, "R")),
            (["r", "R"], None),
            (["l", "R"], None),
            (["L"], ("Player 1", 0, "L")),
            (["r", "L"], None),
            (["l", "L"], None),
        ]
    ),
    (
        "subgames.efg",
        [
            ([], None),
            (["1"], None),
            (["2"], None),
            (["1", "2"], ("Player 2", 0, "2")),
            (["2", "1", "2"], ("Player 1", 1, "1")),
            (["2", "2"], ("Player 2", 0, "2")),
            (["1", "2", "2"], ("Player 2", 1, "1")),
            (["1", "1", "2", "2"], ("Player 1", 1, "2")),
            (["1", "1", "1", "2", "2"], ("Player 2", 2, "1")),
            (["2", "1", "2", "2"], ("Player 1", 1, "2")),
            (["1", "2", "1", "2", "2"], ("Player 2", 2, "2")),
            (["2", "2", "1", "2", "2"], ("Player 2", 2, "2")),
            (["1", "2", "2", "1", "2", "2"], ("Player 1", 4, "2")),
            (["1", "1", "2", "2", "1", "2", "2"], ("Player 2", 4, "1")),
            (["1", "1", "1", "2", "2", "1", "2", "2"], ("Player 1", 5, "1")),
            (["2", "1", "1", "2", "2", "1", "2", "2"], ("Player 1", 5, "1")),
            (["2", "2", "2", "1", "2", "2"], ("Player 1", 4, "2")),
            (["2", "2", "2"], ("Player 1", 1, "2")),
        ]
    ),
    (
        "AM-driver-subgame.efg",
        [
            ([], None),
            (["S"], ("Player 1", 0, "S")),
            (["T", "S"], None),
        ]
    ),
])
def test_node_own_prior_action_non_terminal(game_file, expected_node_data):
    """
    Tests `node.own_prior_action` for non-terminal nodes.
    Also verifies that all terminal nodes return None.
    """
    game = game_file if isinstance(game_file, gbt.Game) else games.read_from_file(game_file)

    actual_node_data = []

    for node in game.nodes:
        if node.is_terminal:
            assert node.own_prior_action is None, (
                f"Terminal node at {_get_path_of_action_labels(node)} must be None"
            )
        else:
            # Only collect data for non-terminal nodes
            opa = node.own_prior_action
            details = (
                (opa.infoset.player.label, opa.infoset.number, opa.label)
                if opa is not None else None
            )
            actual_node_data.append((_get_path_of_action_labels(node), details))

    assert actual_node_data == expected_node_data


@pytest.mark.parametrize("game_file, expected_unreachable_paths", [
    # Games without absent-mindedness, where all nodes are reachable
    (gbt.catalog.load("journals/geb/wichardt2008"), []),
    ("subgames.efg", []),

    # An absent-minded driver game with an unreachable terminal node
    (
        "AM-driver-one-infoset.efg",
        [["T", "S"]]
    ),

    # An absent-minded driver game with an unreachable subtree
    (
        "AM-driver-subgame.efg",
        [["T", "S"], ["r", "T", "S"], ["l", "T", "S"]]
    ),
])
def test_is_strategy_reachable(game_file: str, expected_unreachable_paths: list[list[str]]):
    """
    Tests `node.is_strategy_reachable` by collecting all unreachable nodes,
    converting them to their action-label paths, and comparing the resulting
    list of paths against a known-correct list.
    """
    game = game_file if isinstance(game_file, gbt.Game) else games.read_from_file(game_file)
    nodes = game.nodes

    actual_unreachable_paths = [
        _get_path_of_action_labels(node) for node in nodes if not node.is_strategy_reachable
    ]

    assert actual_unreachable_paths == expected_unreachable_paths


def test_append_move_error_player_actions():
    """Test to ensure there are actions when appending with a player"""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_move(game.root, game.players["Player 1"], [])


def test_append_move_error_player_mismatch():
    """Test to ensure the node and the player are from the same game"""
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1.append_move(game1.root, game2.players["Player 1"], ["a"])


def test_append_move_error_infoset_mismatch():
    """Test to ensure the node and the player are from the same game"""
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1.append_infoset(game1.root, game2.root.infoset)


def test_insert_move_error_player_actions():
    """Test to ensure there are actions when inserting with a player"""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.insert_move(game.root, game.players["Player 1"], 0)


def test_insert_move_error_player_mismatch():
    """Test to ensure the node and the player are from the same game"""
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1.insert_move(game1.root, game2.players["Player 1"], 1)


def test_node_leave_infoset():
    """A node-anchored infoset proxy is lazy: it re-resolves after the node leaves its infoset."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    proxy = node.infoset
    assert len(proxy.members) == 2
    game.leave_infoset(node)
    assert list(proxy.members) == [node]


def test_leave_infoset_sole_member_is_noop():
    """Leaving a sinleton infoset is a no-op: infoset identity and label retained."""
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.infoset.label = "solo"            # root is its infoset's only member
    iset = game.root.infoset
    game.leave_infoset(game.root)
    assert game.root.infoset == iset
    assert game.root.infoset.label == "solo"


def test_leave_infoset_from_shared_infoset_relabels():
    """Leaving a multi-node infoset places the node in a new unlabeled singleton,
    while the original infoset keeps its label with the members left behind."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    sibling = game.root.children["D1"]
    node.infoset.label = "multi"
    assert len(node.infoset.members) == 2       # precondition: a multi-node infoset
    game.leave_infoset(node)
    assert node.infoset.label == ""             # departing node: fresh unlabeled singleton
    assert sibling.infoset.label == "multi"     # label stays with the rump
    assert list(sibling.infoset.members) == [sibling]


def test_node_infoset_becomes_null_when_truncated():
    """A captured infoset proxy re-resolves to null after the node is truncated to a leaf."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    proxy = node.infoset
    assert proxy
    game.delete_tree(node)
    assert not proxy


def test_node_delete_parent():
    """Test to ensure deleting a parent node works"""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    game.delete_parent(node)
    assert game.root == node


def test_node_delete_tree():
    """Test to ensure deleting every child of a node works"""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"]
    game.delete_tree(node)
    assert len(node.children) == 0


def test_node_copy_nonterminal():
    """Test on copying to a nonterminal node."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.copy_tree(game.root, game.root)


def test_node_copy_across_games():
    """Test to ensure a gbt.MismatchError is raised when trying to copy a tree
    from a different game.
    """
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1.copy_tree(game1.root, game2.root)
    with pytest.raises(gbt.MismatchError):
        game1.copy_tree(game2.root, game1.root)


def _subtrees_equal(
        n1: gbt.Node,
        n2: gbt.Node,
        recursion_stop_node: gbt.Node | None = None
) -> bool:
    if n1 == recursion_stop_node:
        return n2.is_terminal
    if n1.is_terminal and n2.is_terminal:
        return n1.outcome == n2.outcome
    if n1.is_terminal is not n2.is_terminal:
        return False
    # now, both n1 and n2 are non-terminal
    # check that they are in the same infosets
    if n1.infoset != n2.infoset:
        return False
    # check that they have the same number of children
    if len(n1.children) != len(n2.children):
        return False

    return all(
        _subtrees_equal(c1, c2, recursion_stop_node) for (c1, c2) in zip(
            n1.children, n2.children, strict=True
            )
    )


def test_copy_tree_onto_nondescendent_terminal_node():
    """Test copying a subtree to a non-descendent node."""
    g = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    src_node = g.root.children["R"].children["L"]
    dest_node = g.root.children["R"].children["R"]

    g.copy_tree(src_node, dest_node)

    assert _subtrees_equal(src_node, dest_node)


def test_copy_tree_onto_descendent_terminal_node():
    """Test copying a subtree to a node that's a descendent of the original."""
    g = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    src_node = g.root.children["R"]
    dest_node = g.root.children["R"].children["L"].children["R"]

    g.copy_tree(src_node, dest_node)

    assert _subtrees_equal(src_node, dest_node, dest_node)


def test_node_move_nonterminal():
    """Test on moving to a nonterminal node."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.move_tree(game.root, game.root)


def test_node_move_successor():
    """Test on moving a node to one of its successors."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.move_tree(game.root, game.root.children["U1"].children["U2"].children["U3"])


def test_node_move_across_games():
    """Test to ensure a gbt.MismatchError is raised when trying to move a tree
    between different games.
    """
    game1 = gbt.Game.new_tree()
    game2 = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game1.move_tree(game1.root, game2.root)
    with pytest.raises(gbt.MismatchError):
        game1.move_tree(game2.root, game1.root)


def test_append_move_creates_single_infoset_list_of_nodes():
    """Test that appending a list of nodes creates a single infoset."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    nodes = [game.root.children["2"].children["1"],
             game.root.children["1"].children["1"],
             game.root.children["1"].children["2"]]
    game.append_move(nodes, "Player 3", ["B", "F"])
    assert len(game.players["Player 3"].infosets) == 1


def test_append_move_same_infoset_list_of_nodes():
    """Test that nodes from a list of nodes are resolved in the same infoset."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    node1 = game.root.children["2"].children["1"]
    node2 = game.root.children["1"].children["1"]
    game.append_move([node1, node2], "Player 3", ["B", "F"])
    assert node1.infoset == node2.infoset


def test_append_move_actions_list_of_nodes():
    """Test that nodes from a list of nodes that resolved in the same infoset
    have the same actions.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    node1 = game.root.children["2"].children["1"]
    node2 = game.root.children["1"].children["1"]
    game.append_move([node1, node2], "Player 3", ["B", "F", "S"])
    assert list(node1.infoset.actions) == list(node2.infoset.actions)


def test_append_move_actions_list_of_node_labels():
    """Test that nodes from a list of node labels are resolved correctly."""
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    node1 = game.root.children["2"].children["1"]
    node2 = game.root.children["1"].children["1"]
    node1.label = "0"
    node2.label = "00"
    game.append_move(["0", "00"], "Player 3", ["B", "F", "S"])

    assert node1.children["B"].parent.label == "0"
    assert node2.children["B"].parent.label == "00"
    assert len(node1.children) == 3
    assert len(node2.children) == 3


def test_append_move_actions_list_of_mixed_node_references():
    """Test that nodes from a list of nodes with either 'node' or str references
    are resolved correctly.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")

    node1 = game.root.children["2"].children["1"]
    node2 = game.root.children["1"].children["1"]
    node1.label = "000"
    node_references = ["000", node2]
    game.append_move(node_references, "Player 3", ["B", "F", "S"])

    assert node1.children["B"].parent.label == "000"
    assert len(node1.children) == 3
    assert len(node2.children) == 3


def test_append_move_labels_list_of_nodes():
    """Test that nodes from a list of nodes that resolved in the same infoset
    have the same labels per action.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    node1 = game.root.children["2"].children["1"]
    node2 = game.root.children["1"].children["1"]
    game.append_move([node1, node2], "Player 3", ["B", "F", "S"])

    for action1, action2 in zip(node1.infoset.actions, node2.infoset.actions, strict=True):
        assert action1.label == action2.label


def test_append_move_node_list_with_non_terminal_node():
    """Test that we get an UndefinedOperationError when we import in append_move a list
    of nodes that has a non-terminal node.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_move(
            [game.root.children["2"], game.root.children["1"].children["2"]],
            "Player 3",
            ["B", "F"]
        )


def test_append_move_node_list_with_duplicate_node_references():
    """Test that we get a ValueError when we import in append_move a list
    nodes with non-unique node references.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    node = game.root.children["1"].children["2"]
    node.label = "00"
    with pytest.raises(ValueError):
        game.append_move(
            ["00", game.root.children["2"].children["1"], node],
            "Player 3",
            ["B", "F"]
        )


def test_append_move_node_list_is_empty():
    """Test that we get a ValueError when we import in append_move an
    empty list of nodes.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    with pytest.raises(ValueError):
        game.append_move([], "Player 3", ["B", "F"])


def test_append_infoset_node_list_with_non_terminal_node():
    """Test that we get an UndefinedOperationError when we import in append_infoset
    a list of nodes that has a non-terminal node.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    seed_node = game.root.children["1"].children["1"]
    game.append_move(seed_node, "Player 3", ["B", "F"])
    with pytest.raises(gbt.UndefinedOperationError):
        game.append_infoset(
            [game.root.children["2"], game.root.children["1"].children["2"]],
            seed_node.infoset
        )


def test_append_infoset_node_list_with_duplicate_node():
    """Test that we get a ValueError when we import in append_infoset a list
    with non-unique elements.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    seed_node = game.root.children["1"].children["1"]
    game.append_move(seed_node, "Player 3", ["B", "F"])
    with pytest.raises(ValueError):
        game.append_infoset(
            [game.root.children["1"].children["2"],
             game.root.children["2"].children["1"],
             game.root.children["1"].children["2"]],
            seed_node.infoset
        )


def test_append_infoset_node_list_is_empty():
    """Test that we get a ValueError when we import in append_infoset an
    empty list of nodes.
    """
    game = games.read_from_file("sample_extensive_game.efg")
    game.add_player("Player 3")
    seed_node = game.root.children["1"].children["1"]
    game.append_move(seed_node, "Player 3", ["B", "F"])
    with pytest.raises(ValueError):
        game.append_infoset([], seed_node.infoset)


def _count_subtree_nodes(start_node: gbt.Node, count_terminal: bool) -> int:
    """Counts nodes in the subtree rooted at `start_node` (including `start_node`).

    Parameters
    ----------
    start_node: Node
        The root of the subtree
    count_terminal: bool
        Include or exclude terminal nodes from count
    """
    count = 1 if count_terminal or not start_node.is_terminal else 0

    for child in start_node.children:
        count += _count_subtree_nodes(child, count_terminal)
    return count


def test_len_matches_expected_node_count():
    """Verify `len(game.nodes)` matches expected node count
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    expected_node_count = 9

    direct_len = len(game.nodes)
    assert direct_len == expected_node_count

    assert direct_len == _count_subtree_nodes(game.root, True)


def test_len_after_delete_tree():
    """Verify `len(game.nodes)` is correct after `delete_tree`.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)

    root_of_the_deleted_subtree = game.root.children["R"].children["L"]
    number_of_deleted_nodes = _count_subtree_nodes(root_of_the_deleted_subtree, True) - 1

    game.delete_tree(root_of_the_deleted_subtree)

    assert len(game.nodes) == initial_number_of_nodes - number_of_deleted_nodes


def test_len_after_delete_parent():
    """Verify `len(game.nodes)` is correct after `delete_parent`.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = len(game.nodes)

    node_parent_to_delete = game.root.children["L"].children["L"]

    number_of_node_ancestors = _count_subtree_nodes(node_parent_to_delete, True)
    number_of_parent_ancestors = _count_subtree_nodes(node_parent_to_delete.parent, True)
    diff = number_of_parent_ancestors - number_of_node_ancestors

    game.delete_parent(node_parent_to_delete)

    assert len(game.nodes) == initial_number_of_nodes - diff


def test_len_after_append_move():
    """Verify `len(game.nodes)` is correct after `append_move`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)

    terminal_node = game.root.children["R"].children["L"].children["L"]  # the [1,1,0] terminal
    player = game.players["Player 1"]
    actions_to_add = ["T", "M", "B"]

    game.append_move(terminal_node, player, actions_to_add)

    assert len(game.nodes) == initial_number_of_nodes + len(actions_to_add)


def test_len_after_append_infoset():
    """Verify `len(game.nodes)` is correct after `append_infoset`.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = len(game.nodes)

    member_node = game.root.children["L"]
    infoset_to_modify = member_node.infoset
    number_of_infoset_actions = len(infoset_to_modify.actions)
    terminal_node_to_add = game.root.children["L"].children["L"].children["l"]

    game.append_infoset(terminal_node_to_add, infoset_to_modify)

    assert len(game.nodes) == initial_number_of_nodes + number_of_infoset_actions


def test_len_after_add_action():
    """Verify `len(game.nodes)` is correct after `add_action`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)

    infoset_to_modify = game.root.children["L"].infoset   # Player 2's infoset
    num_nodes_in_infoset = len(infoset_to_modify.members)

    game.add_action(infoset_to_modify)

    assert len(game.nodes) == initial_number_of_nodes + num_nodes_in_infoset


def test_len_after_delete_action():
    """Verify `len(game.nodes)` is correct after `delete_action`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    initial_number_of_nodes = len(game.nodes)

    action_to_delete = game.root.infoset.actions["L"]

    # Deleting an action removes the subtree reached by that action at each
    # member node of its information set.
    nodes_to_delete = sum(
        _count_subtree_nodes(member.children[action_to_delete.label], True)
        for member in action_to_delete.infoset.members
    )

    game.delete_action(action_to_delete)

    assert len(game.nodes) == initial_number_of_nodes - nodes_to_delete


def test_len_after_insert_move():
    """Verify `len(game.nodes)` is correct after `insert_move`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)

    node_to_insert_above = game.root.children["L"].children["R"]  # the [1, 0] node
    player = game.players["Player 2"]
    num_actions_to_add = 3

    game.insert_move(node_to_insert_above, player, num_actions_to_add)

    assert len(game.nodes) == initial_number_of_nodes + num_actions_to_add


def test_len_after_insert_infoset():
    """Verify `len(game.nodes)` is correct after `insert_infoset`.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)

    infoset_to_modify = game.root.children["L"].infoset
    node_to_insert_above = game.root.children["L"].children["R"]
    number_of_infoset_actions = len(infoset_to_modify.actions)

    game.insert_infoset(node_to_insert_above, infoset_to_modify)

    assert len(game.nodes) == initial_number_of_nodes + number_of_infoset_actions


def test_len_after_copy_tree():
    """Verify `len(game.nodes)` is correct after `copy_tree`.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    initial_number_of_nodes = len(game.nodes)
    src_node = game.root.children["R"].children["L"]
    dest_node = game.root.children["R"].children["R"]
    number_of_src_ancestors = _count_subtree_nodes(src_node, True)

    game.copy_tree(src_node, dest_node)

    assert len(game.nodes) == initial_number_of_nodes + number_of_src_ancestors - 1


def test_node_plays():
    """Verify `node.plays` returns plays reachable from a given node.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")

    test_node = game.root.children["L"]

    expected_set_of_plays = {
        game.root.children["L"].children["R"],
        game.root.children["L"].children["L"].children["r"],
        game.root.children["L"].children["L"].children["l"],
    }

    assert set(test_node.plays) == expected_set_of_plays


def test_node_children_action_label():
    """Label lookup returns the correct child.

    The RHS reaches the child positionally (independent of ``__getitem__``); a label
    on both sides would make the assertion circular.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    root_children = list(game.root.children)
    assert game.root.children["King"] == root_children[0]
    assert game.root.children["Queen"].children["Fold"] == list(root_children[1].children)[1]


def test_node_children_action():
    """Action lookup returns the correct child.

    The RHS reaches the child positionally -- cf. `test_node_children_action_label()`.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.root.children[game.root.infoset.actions["King"]] == list(game.root.children)[0]


def test_node_children_empty_label():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError, match="empty or all whitespace"):
        _ = game.root.children["   "]


def test_node_children_terminal_node():
    game = games.read_from_file("stripped_down_poker.efg")
    terminal = next(n for n in game.nodes if n.is_terminal)
    with pytest.raises(KeyError, match="No action with label"):
        _ = terminal.children["Bet"]


def test_node_children_nonexistent_action():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(KeyError, match="No action with label 'Jack'"):
        _ = game.root.children["Jack"]


def test_node_children_rejects_int():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(TypeError, match="16.7.0"):
        _ = game.root.children[0]


def test_node_children_other_infoset_action():
    game = games.read_from_file("stripped_down_poker.efg")
    with pytest.raises(ValueError):
        _ = game.root.children[game.root.children["King"].infoset.actions["Bet"]]


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_node_label_valid(label):
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.label = label
    assert game.root.label == label


def test_node_label_duplicate_raises_valueerror():
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.label = "shared"
    with pytest.raises(ValueError):
        game.root.children["U1"].label = "shared"


def test_node_label_empty_is_allowed():
    """Node labels may be empty (unlike outcomes/players); multiple empties coexist."""
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.label = ""
    game.root.children["U1"].label = ""
    assert game.root.label == ""
    assert game.root.children["U1"].label == ""


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_node_label_invalid_raises_valueerror(label):
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.root.label = label


@pytest.mark.parametrize("label", games.NON_ASCII_LABELS)
def test_node_label_non_ascii_rejected(label):
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(UnicodeEncodeError):
        game.root.label = label


@pytest.mark.parametrize(
    "game_obj",
    [
        pytest.param(games.read_from_file("basic_extensive_game.efg")),
        pytest.param(games.read_from_file("binary_3_levels_generic_payoffs.efg")),
        pytest.param(games.read_from_file("cent3.efg")),
        pytest.param(gbt.catalog.load("journals/ijgt/selten1975/fig1")),
        pytest.param(gbt.catalog.load("journals/ijgt/selten1975/fig2")),
        pytest.param(games.read_from_file("stripped_down_poker.efg")),
        pytest.param(gbt.Game.new_tree()),
    ],
)
def test_nodes_iteration_order(game_obj: gbt.Game):
    """Verify that the C++ `game.nodes` iterator produces the DFS traversal.
    """
    def dfs(node: gbt.Node) -> typing.Iterator[gbt.Node]:
        yield node
        for child in node.children:
            yield from dfs(child)

    assert all(a == b for a, b in itertools.zip_longest(game_obj.nodes, dfs(game_obj.root)))
