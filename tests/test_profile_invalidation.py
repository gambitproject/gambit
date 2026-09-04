import pytest

import pygambit as gbt

from . import games


def test_mixed_strategy_profile_game_structure_changed_no_tree():
    game = gbt.Game.from_arrays([[2, 2], [0, 0]], [[0, 0], [1, 1]])
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    distribution = {s: 0 for s in game.get_strategies(player)}
    next(iter(game.outcomes))[player] = 3
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.player_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_values
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(player, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_strategy(player, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(player)


def test_mixed_strategy_profile_game_structure_changed_tree():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    game.set_move_actions(gbt.H.path(), ["D1"], drop=True)
    distribution = {s: 0 for s in game.get_strategies(player)}
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_behavior()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.player_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_values
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(player, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_strategy(player, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(player)


def test_mixed_behavior_profile_game_structure_changed():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_behavior_profile(rational=b) for b in [False, True]]
    game.set_move_actions(gbt.H.path(), ["D1"], drop=True)
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.action_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.action_values
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_strategy()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.beliefs
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_probs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_values
        with pytest.raises(gbt.GameStructureChangedError):
            profile.agent_liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.agent_max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.node_values
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.realiz_probs
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(game.root, {})
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_action(game.root, {})
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(game.root)
