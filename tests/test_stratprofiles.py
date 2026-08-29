import pytest

import pygambit as gbt

from . import games


def test_getitem_labels():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    support = profile["Player 1"]
    assert set(support) == {"1", "2", "3"}
    assert support.player == game.players["Player 1"]
    assert "1" in support
    assert "not-a-label" not in support


def test_getitem_unknown_player():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(KeyError):
        profile["not-a-player"]


def test_getitem_rejects_non_str():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(TypeError):
        profile[game.players["Player 1"]]


def test_predicate_construction():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile(lambda player, label: label != "3")
    assert set(profile["Player 1"]) == {"1", "2"}
    assert set(profile["Player 2"]) == {"1", "2"}


def test_predicate_construction_error():
    game = games.read_from_file("mixed_strategy.nfg")
    with pytest.raises(ValueError):
        game.strategy_support_profile(lambda player, label: player.label != "Player 1")


def test_iter_yields_one_support_per_player():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    supports = list(profile)
    assert len(supports) == len(game.players)
    assert {s.player.label for s in supports} == {p.label for p in game.players}


def test_setitem_replaces_support():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    profile["Player 1"] = ["1", "2"]
    assert set(profile["Player 1"]) == {"1", "2"}
    assert set(profile["Player 2"]) == {"1", "2"}


def test_setitem_unknown_label():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(ValueError):
        profile["Player 1"] = ["not-a-label"]


def test_setitem_empty():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(ValueError):
        profile["Player 1"] = []


def test_setitem_unknown_player():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(KeyError):
        profile["not-a-player"] = ["1"]


def test_copy_is_independent():
    game = games.read_from_file("mixed_strategy.nfg")
    original = game.strategy_support_profile()
    copy = original.copy()
    copy["Player 1"] = ["1"]
    assert set(copy["Player 1"]) == {"1"}
    assert set(original["Player 1"]) == {"1", "2", "3"}


def test_strategysupport_is_snapshot():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    snapshot = profile["Player 1"]
    profile["Player 1"] = ["1"]
    assert set(snapshot) == {"1", "2", "3"}


def test_is_dominated():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    assert profile.is_dominated("Player 1", "1", strict=False) is False


def test_is_dominated_unknown_player():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(KeyError):
        profile.is_dominated("not-a-player", "1", strict=False)


def test_is_dominated_unknown_strategy():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile()
    with pytest.raises(KeyError):
        profile.is_dominated("Player 1", "not-a-label", strict=False)


def test_restrict():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = game.strategy_support_profile(lambda player, label: label != "3")
    restricted = profile.restrict()
    assert len(restricted.players["Player 1"].strategies) == 2
    assert len(restricted.players["Player 2"].strategies) == 2


def test_undominated():
    game = games.read_from_file("mixed_strategy.nfg")
    profile = gbt.supports.undominated_strategies_solve(game)
    while True:
        new_profile = gbt.supports.undominated_strategies_solve(profile)
        if new_profile == profile:
            break
        profile = new_profile
    assert profile == game.strategy_support_profile(lambda player, label: label == "1")
