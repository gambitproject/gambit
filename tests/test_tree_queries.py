import dataclasses
import functools
import typing

import pytest

import pygambit as gbt

from . import games


def test_get_actions():
    """A personal or chance node's actions, in order; a terminal node has none --
    a node is terminal exactly when this is empty."""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.get_actions(gbt.H.path()) == ["U1", "D1"]
    assert game.get_actions(gbt.H.path("U1")) == ["U2", "D2"]
    assert game.get_actions(gbt.H.path("U1", "D2", "U3")) == []


def test_get_actions_requires_selector():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(TypeError):
        game.get_actions(())
    with pytest.raises(TypeError):
        game.get_actions("U1")


def test_get_actions_requires_single_match():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.get_actions(gbt.H.path(...))
    with pytest.raises(ValueError):
        game.get_actions(gbt.H.path(...).filter(lambda h: False))


def test_get_player():
    """A personal node's player; a terminal node has none."""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.get_player(gbt.H.path()) == "Player 1"
    assert game.get_player(gbt.H.path("U1", "D2", "U3")) is None


def test_get_player_resolves_chance():
    """At a chance node, the player label is the chance player's."""
    game = games.read_from_file("stripped_down_poker.efg")
    assert game.get_player(gbt.H.path()) == "Chance"


def test_get_player_requires_selector():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(TypeError):
        game.get_player(())
    with pytest.raises(TypeError):
        game.get_player("U1")


def test_get_player_requires_single_match():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.get_player(gbt.H.path(...))
    with pytest.raises(ValueError):
        game.get_player(gbt.H.path(...).filter(lambda h: False))


def test_get_parent():
    """Test to ensure that we can retrieve a parent node for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].parent == game.root
    assert game.root.parent is None


def test_get_prior_action():
    """Test to ensure that we can retrieve the prior action for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.children["U1"].prior_action == gbt.Branch(game.root, "U1")
    assert game.root.prior_action is None


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
        game.root.is_successor_of("Player 1")


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

    actual_roots = [node for node in games.all_nodes(game) if node.is_subgame_root]
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
    """`game.get_minimal_subgame(node)` returns the smallest subgame containing
    the information set `node` belongs to."""
    game = test_case.factory()
    expected_path_for_key = {
        key: path
        for path, keys in test_case.differences.items()
        for key in keys
    }
    for player in game.players:
        for node in game.get_infosets(player):
            key = (node.player, games.infoset_number(node))
            selector = games.selector_for_nodes([node])
            actual_path = tuple(
                _get_path_of_action_labels(game.get_minimal_subgame(selector).root)
            )
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

    for node in games.all_nodes(game):
        if not node.children:
            assert node.own_prior_action is None, (
                f"Terminal node at {_get_path_of_action_labels(node)} must be None"
            )
        else:
            # Only collect data for non-terminal nodes
            opa = node.own_prior_action
            if opa is not None:
                details = (opa.node.player, games.infoset_number(opa.node), opa.label)
            else:
                details = None
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
    nodes = games.all_nodes(game)

    actual_unreachable_paths = [
        _get_path_of_action_labels(node) for node in nodes if not node.is_strategy_reachable
    ]

    assert actual_unreachable_paths == expected_unreachable_paths


@pytest.mark.parametrize(
    "game, player_label, strategy_label, infoset_path, expected_action_label",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", "1", [], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", "2", [], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", "1", ["R"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", "2", ["R"], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 3", "1", ["R", "L"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 3", "2", ["R", "L"], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "1", [], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "2", [], "L"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", "1", ["L"], "R"),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", "2", ["L"], "L"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", "1", [], "U1"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", "2", [], "D1"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", "1", ["U1"], "U2"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", "2", ["U1"], "D2"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", "1", ["U1", "U2"], "U3"),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", "2", ["U1", "U2"], "D3"),
    ],
)
def test_get_behavior_prescribed_action_defined(
    game, player_label, strategy_label, infoset_path, expected_action_label
):
    """Verify `Game.get_behavior` retrieves the correct action for defined actions."""
    selector = gbt.H.path(*infoset_path)

    prescribed_action = game.get_behavior(player_label, strategy_label).get(selector)

    assert prescribed_action == expected_action_label


@pytest.mark.parametrize(
    "game, player_label, strategy_label, infoset_label, infoset_path",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", "1", None, ["L", "L"]),
        (games.read_from_file("cent3.efg"), "Player 1", "1", "(1,3)", None),
        (games.read_from_file("cent3.efg"), "Player 1", "1", "(1,5)", None),
        (games.read_from_file("cent3.efg"), "Player 1", "2", "(1,5)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "1", "(2,4)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "1", "(2,4)", None),
        (games.read_from_file("cent3.efg"), "Player 2", "2", "(2,5)", None),
    ],
)
def test_get_behavior_prescribed_action_undefined_returns_none(
    game, player_label, strategy_label, infoset_label, infoset_path
):
    """Verify `Game.get_behavior` returns None when called on an unreached player's infoset"""
    if infoset_label is not None:
        node = next(iter(games.find_infoset_in_game(game, infoset_label).members))
        selector = games.selector_for_node(node)
    else:
        selector = gbt.H.path(*infoset_path)

    prescribed_action = game.get_behavior(player_label, strategy_label).get(selector)

    assert prescribed_action is None


@pytest.mark.parametrize(
    "game, player_label, other_infoset_path",
    [
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 1", ["R"]),
        (gbt.catalog.load("journals/ijgt/selten1975/fig1"), "Player 2", []),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 1", ["L"]),
        (gbt.catalog.load("journals/ijgt/selten1975/fig2"), "Player 2", []),
        (games.read_from_file("basic_extensive_game.efg"), "Player 1", ["U1"]),
        (games.read_from_file("basic_extensive_game.efg"), "Player 2", ["U1", "U2"]),
        (games.read_from_file("basic_extensive_game.efg"), "Player 3", []),
    ],
)
def test_get_behavior_raises_value_error_for_wrong_player(
    game, player_label, other_infoset_path
):
    """
    Verify `Game.get_behavior`'s result raises ValueError when the infoset belongs
    to a different player than the strategy.
    """
    behavior = game.get_behavior(player_label, next(iter(game.get_strategies(player_label))))
    other_selector = gbt.H.path(*other_infoset_path)

    with pytest.raises(ValueError):
        behavior.get(other_selector)


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
def test_get_histories_after_iteration_order(game_obj: gbt.Game):
    """`Game.get_histories(H.after())` -- the public replacement for the removed
    `Game.nodes` -- produces nodes in depth-first traversal order.
    """
    def dfs(node: gbt.Node) -> typing.Iterator[gbt.Node]:
        yield node
        for child in node.children:
            yield from dfs(child)

    expected = [games._node_history(node) for node in dfs(game_obj.root)]
    assert game_obj.get_histories(gbt.H.after()) == expected
