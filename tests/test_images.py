import pygambit as gbt
from pygambit.image import build_node_images, build_set_image


def test_build_images_of_nodes_in_outcomes():
    """Generate images in outcomes of the nodes of a simple centipede game with 4 terminal nodes
    """
    g = gbt.read_efg("tests/test_games/e02.efg")
    z1, z2, z3, z4 = g.outcomes
    expected_images = {
        g.root: {z1, z2, z3, z4},
        g.root.children[0]: {z1},
        g.root.children[1]: {z2, z3, z4},
        g.root.children[1].children[0]: {z2},
        g.root.children[1].children[1]: {z3, z4},
        g.root.children[1].children[1].children[0]: {z3},
        g.root.children[1].children[1].children[1]: {z4}
    }
    assert expected_images == build_node_images(g)


def test_build_images_of_infosets_in_outcomes():
    """Generate images in outcomes of the infosets of a simple centipede game with 4 terminal nodes
    """
    g = gbt.read_efg("tests/test_games/e02.efg")
    node_images = build_node_images(g)
    z1, z2, z3, z4 = g.outcomes
    infosets = g.infosets
    expected_images = {
        infosets[0]: {z1, z2, z3, z4},
        infosets[1]: {z3, z4},
        infosets[2]: {z2, z3, z4}
    }
    built_images = {infoset: build_set_image(infoset, node_images) for infoset in infosets}
    assert expected_images == built_images


def test_build_images_of_actions_in_outcomes():
    """Generate images in outcomes of the actions of a simple centipede game with 4 terminal nodes
    """
    g = gbt.read_efg("tests/test_games/e02.efg")
    node_images = build_node_images(g)
    z1, z2, z3, z4 = g.outcomes
    actions = g.actions
    expected_images = {
        actions[0]: {z1},
        actions[1]: {z2, z3, z4},
        actions[2]: {z3},
        actions[3]: {z4},
        actions[4]: {z2},
        actions[5]: {z3, z4}
    }
    built_images = {action: build_set_image(action, node_images) for action in g.actions}
    assert expected_images == built_images
