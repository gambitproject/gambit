import pygambit as gbt


def test_build_images_of_nodes_in_outcomes():
    """Generate images in outcome of the nodes of a simple centipede game with 4 terminal nodes
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
    assert expected_images == g.compute_images()
