import typing

from pygambit import Action, Game, Infoset, Node, Outcome

ImageDict = dict[Node, set[Outcome]]


def build_node_images(game: Game) -> ImageDict:
    """Recursively compute images in outcomes for each node in the game tree.

    For each node in the tree, calculates the set of outcomes that can be reached from it.

    Returns
    -------
    dict[Node, set[Outcome]]
        A dictionary mapping each node to the set of outcomes that can be reached from it:
        - For terminal nodes, this is either {node.outcome} or an empty set
        - For non-terminal nodes, this is the union of all images in outcomes of children

    Notes
    -----
    This traverses the game tree using depth-first search, building the sets of outcomes
    from the bottom up.
    """
    node_images = {}

    def dfs(node) -> set[Outcome]:
        if node.is_terminal:
            if node.outcome is None:
                node_images[node] = set()
            else:
                node_images[node] = {node.outcome}
        else:
            union = set()
            for child in node.children:
                union |= dfs(child)
            node_images[node] = union
        return node_images[node]

    dfs(game.root)
    return node_images


def build_set_image(
        infoset_or_action: typing.Union[Infoset, Action],
        node_images: ImageDict) -> set[Outcome]:
    # Get the set of nodes constituting a given infoset or action
    image_in_nodes = infoset_or_action.members

    # Calculate the union of images of the member nodes
    union = set()
    for node in image_in_nodes:
        union |= node_images[node]
    return union
