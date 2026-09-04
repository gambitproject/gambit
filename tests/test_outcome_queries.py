import pytest

import pygambit as gbt

from . import games


def test_get_outcome():
    """Test to ensure that we can retrieve an outcome for a given node"""
    game = games.read_from_file("basic_extensive_game.efg")
    assert (
        game.root.children["U1"].children["D2"].children["U3"].outcome
        == game.outcomes["Outcome 1"]
    )
    assert not game.root.outcome


def test_make_outcome_null():
    """Resetting a node's outcome to null leaves the node's outcome view falsy."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"].children["U2"].children["U3"]
    game.make_outcome_null(gbt.H.path("U1", "U2", "U3"))
    assert not node.outcome


def test_node_outcome_subscript_tracks_mutation():
    """Indexing the outcome view reads/writes the outcome's payoffs, reflecting mutation."""
    game = games.read_from_file("basic_extensive_game.efg")
    node = game.root.children["U1"].children["D2"].children["U3"]
    proxy = node.outcome
    player = "Player 1"
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


def test_null_outcome_label_is_none():
    """The blessed nullity idiom: a node with no outcome has `outcome.label is None`."""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.outcome.label is None


def test_null_outcome_compares_unequal_to_itself():
    """Null outcomes are unequal to everything, including another view of the same
    node's outcome; equality must not short-circuit on node identity."""
    game = games.read_from_file("basic_extensive_game.efg")
    assert (game.root.outcome == game.root.outcome) is False


def test_null_outcome_reads_zero_payoffs():
    """Reading a payoff through an unset node reports zero to every player of the game."""
    game = games.read_from_file("basic_extensive_game.efg")
    for player in game.players:
        assert game.root.outcome[player] == 0


def test_null_outcome_payoff_write_raises():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.UndefinedOperationError):
        game.root.outcome["Player 1"] = 1


def test_null_outcome_label_write_raises():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.root.outcome.label = "Outcome 4"


def test_null_outcome_number_is_none():
    """The null outcome is not a member of the game's outcomes, so it has no number.
    -1 would be a valid index and would silently resolve to the last real outcome."""
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.root.outcome.number is None


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_outcome_label(label: str):
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_outcome_label_invalid_raises_valueerror(label: str):
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    with pytest.raises(ValueError):
        outcome.label = label


@pytest.mark.parametrize("label", games.UNICODE_LABELS)
def test_outcome_label_unicode_accepted(label: str):
    """Non-ASCII UTF-8 labels are accepted as of #862 (17.0)."""
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome.label == label


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]]), "outcome label")]
)
def test_outcome_index_label(game: gbt.Game, label: str):
    outcome = next(iter(game.outcomes))
    outcome.label = label
    assert outcome == game.outcomes[label]
    assert game.outcomes[label].label == label


@pytest.mark.parametrize(
    "game", [gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])]
)
def test_outcome_index_unmatched_label(game: gbt.Game):
    with pytest.raises(KeyError):
        _ = game.outcomes["not an outcome"]


@pytest.mark.parametrize(
    "game", [gbt.Game.new_table([2, 2])]
)
def test_outcome_index_invalid_type(game: gbt.Game):
    with pytest.raises(TypeError):
        _ = game.outcomes[1.3]


def test_outcome_payoff_by_player_label():
    game = gbt.Game.from_arrays([[0, 0], [0, 0]], [[0, 0], [0, 0]])
    pl1, pl2 = list(game.players)
    game.relabel_players({pl1: "joe", pl2: "dan"})
    out1, out2, *_ = list(game.outcomes)
    out1["joe"] = 1
    out1["dan"] = 2
    out2["joe"] = 3
    out2["dan"] = 4
    assert out1["joe"] == 1
    assert out1["dan"] == 2
    assert out2["joe"] == 3
    assert out2["dan"] == 4
