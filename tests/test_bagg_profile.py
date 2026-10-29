"""Tests of the mixed behavior profile of a Bayesian action-graph game (BAGG).

Ex-ante payoffs and player-level regrets are checked against the explicit extensive game of
``bagg_oracle``; conditional (agent-level) regrets are checked against a direct computation
from the parsed BAGG description.
"""

import itertools
from fractions import Fraction

import pytest

import pygambit as gbt

from . import bagg_oracle, games
from .test_bagg_oracle import FIRST_ACTIONS, MIXED, PROFILES, PURE_NASH, SAME_NODES, UNIFORM

R = gbt.Rational
GAME = "bayesian_2p2t_shared_node.bagg"
PLAYERS = ["1", "2"]
TYPES = ["1", "2"]


@pytest.fixture(scope="module")
def oracle() -> gbt.Game:
    return bagg_oracle.bagg_to_efg(GAME)


@pytest.fixture(scope="module")
def desc() -> bagg_oracle.BAGGDescription:
    return bagg_oracle.read_description(GAME)


@pytest.fixture
def game() -> gbt.Game:
    return games.read_from_file(GAME)


def _set(profile, strategies):
    for (player, type_), weights in strategies.items():
        total = sum(weights)
        profile[str(player), str(type_)] = {
            str(k + 1): R(str(Fraction(w, total))) for k, w in enumerate(weights)
        }
    return profile


def _conditional_regrets(desc, strategies) -> dict:
    """Conditional regret of each (player, type) computed from the BAGG description."""
    probs = {}
    for (i, t), weights in strategies.items():
        total = sum(Fraction(w) for w in weights)
        probs[(i - 1, t - 1)] = [Fraction(w) / total for w in weights]
    regrets = {}
    for i in range(desc.num_players):
        others = [j for j in range(desc.num_players) if j != i]
        for t in range(len(desc.priors[i])):
            values = []
            for node in desc.action_sets[i][t]:
                value = Fraction(0)
                for combo in itertools.product(
                    *(
                        [
                            (tj, aj)
                            for tj in range(len(desc.priors[j]))
                            for aj in range(len(desc.action_sets[j][tj]))
                        ]
                        for j in others
                    )
                ):
                    weight = Fraction(1)
                    nodes = [0] * desc.num_players
                    nodes[i] = node
                    for j, (tj, aj) in zip(others, combo, strict=True):
                        weight *= desc.priors[j][tj] * probs[(j, tj)][aj]
                        nodes[j] = desc.action_sets[j][tj][aj]
                    value += weight * desc.payoffs(tuple(nodes))[i]
                values.append(value)
            current = sum(p * v for p, v in zip(probs[(i, t)], values, strict=True))
            regrets[(i + 1, t + 1)] = max(values) - current
    return regrets


@pytest.mark.parametrize("rational", [False, True])
@pytest.mark.parametrize("game_path", [GAME, "Bayesian-Coffee-3-2-2-3.bagg", "2x2.bagg"])
def test_construction_is_uniform(game_path, rational):
    game = games.read_from_file(game_path)
    profile = game.mixed_behavior_profile(rational=rational)
    expected = gbt.Rational if rational else float
    assert isinstance(profile, gbt.BAGGMixedBehaviorProfile)
    assert len(profile) == len(game.players)
    for player in game.players:
        for infoset in game.get_infosets(player):
            action = profile[player, infoset]
            assert len(action) > 0
            for _, probability in action:
                assert isinstance(probability, expected)
                assert probability == pytest.approx(1 / len(action))


@pytest.mark.parametrize("game_path", [GAME, "Bayesian-Coffee-3-2-2-3.bagg"])
def test_random_profiles(game_path):
    game = games.read_from_file(game_path)
    double = game.random_behavior_profile()
    assert isinstance(double, gbt.BAGGMixedBehaviorProfileDouble)
    exact = game.random_behavior_profile(denom=7)
    assert isinstance(exact, gbt.BAGGMixedBehaviorProfileRational)
    for player in game.players:
        for infoset in game.get_infosets(player):
            assert sum(p for _, p in double[player, infoset]) == pytest.approx(1.0)
            assert sum(p for _, p in exact[player, infoset]) == 1
            assert all(p * 7 == int(p * 7) for _, p in exact[player, infoset])


def test_random_profile_rejects_bad_denominator(game):
    with pytest.raises(ValueError):
        game.random_behavior_profile(denom=0)


def test_construction_from_data(game):
    data = [[[1, 0], [0, 1]], [[R(1, 3), R(2, 3)], [1, 0]]]
    profile = game.mixed_behavior_profile(data=data, rational=True)
    assert profile["1", "2"] == {"1": 0, "2": 1}
    assert profile["2", "1"] == {"1": R(1, 3), "2": R(2, 3)}
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(data=[[[1, 0]]])


def test_not_a_tree_or_strategy_profile(game):
    profile = game.mixed_behavior_profile()
    assert not isinstance(profile, gbt.MixedBehaviorProfile)
    assert not isinstance(profile, gbt.MixedStrategyProfile)
    assert profile.game == game


@pytest.mark.parametrize(
    "attr",
    ["realiz_probs", "beliefs", "node_values", "event_probs", "as_strategy", "history_values"],
)
def test_tree_only_members_absent(game, attr):
    assert not hasattr(game.mixed_behavior_profile(), attr)


def test_indexing_forms_round_trip(game):
    profile = game.mixed_behavior_profile(rational=True)
    profile["1", "2"] = {"1": R(1, 4), "2": R(3, 4)}
    expected = {"1": R(1, 4), "2": R(3, 4)}
    assert profile["1", "2"] == expected
    snapshot = profile["1"]
    assert isinstance(snapshot, gbt.BAGGMixedBehavior)
    assert snapshot.player == "1"
    assert len(snapshot) == 2
    assert snapshot["2"] == expected
    assert [label for label, _ in snapshot] == ["1", "2"]
    assert snapshot == profile["1"]
    assert [list(profile[p]) for p in PLAYERS] == [list(b) for b in profile]


def test_assignment_omitted_actions_are_zero(game):
    profile = game.mixed_behavior_profile(rational=True)
    profile["2", "1"] = {"2": 1}
    assert profile["2", "1"] == {"1": 0, "2": 1}


def test_snapshots_do_not_follow_later_changes(game):
    profile = game.mixed_behavior_profile(rational=True)
    snapshot = profile["1"]
    action = profile["1", "1"]
    profile["1", "1"] = {"1": 1}
    assert snapshot["1"] == {"1": R(1, 2), "2": R(1, 2)}
    assert action == {"1": R(1, 2), "2": R(1, 2)}
    with pytest.raises(TypeError):
        snapshot["1"] = {"1": 1}


@pytest.mark.parametrize("rational", [False, True])
def test_lookup_errors(game, rational):
    profile = game.mixed_behavior_profile(rational=rational)
    with pytest.raises(KeyError):
        profile["3"]
    with pytest.raises(KeyError):
        profile["3", "1"]
    with pytest.raises(KeyError):
        profile["1", "3"]
    with pytest.raises(KeyError):
        profile["1"]["3"]
    with pytest.raises(KeyError):
        profile["1", "1"]["3"]
    with pytest.raises(TypeError):
        profile[1]
    with pytest.raises(TypeError):
        profile["1", 1]
    with pytest.raises(TypeError):
        profile["1", "1", "1"]
    with pytest.raises(KeyError):
        profile["3", "1"] = {"1": 1}
    with pytest.raises(KeyError):
        profile["1", "3"] = {"1": 1}


@pytest.mark.parametrize("rational", [False, True])
def test_assignment_errors(game, rational):
    profile = game.mixed_behavior_profile(rational=rational)
    with pytest.raises(ValueError):
        profile["1", "1"] = {"3": 1}
    with pytest.raises(ValueError):
        profile["1", "1"] = {"1": -1}
    with pytest.raises(ValueError):
        profile["1", "1"] = {"1": 0, "2": 0}
    with pytest.raises(TypeError):
        profile["1", "1"] = [1, 0]
    with pytest.raises(TypeError):
        profile["1"] = {"1": {"1": 1}}
    assert profile["1", "1"] == {"1": pytest.approx(0.5), "2": pytest.approx(0.5)}


@pytest.mark.parametrize("strategies,expected_payoffs,expected_regret", PROFILES)
def test_exact_ex_ante_quantities_match_oracle(
    game, oracle, strategies, expected_payoffs, expected_regret
):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    reference = bagg_oracle.behavior_profile(oracle, strategies)
    assert profile.payoffs == reference.payoffs
    assert profile.payoffs["1"] == expected_payoffs[0]
    assert profile.payoffs["2"] == expected_payoffs[1]
    assert profile.max_regret() == reference.max_regret() == expected_regret


@pytest.mark.parametrize("strategies,expected_payoffs,expected_regret", PROFILES)
def test_double_ex_ante_quantities_match_oracle(
    game, oracle, strategies, expected_payoffs, expected_regret
):
    exact = bagg_oracle.behavior_profile(oracle, strategies)
    profile = game.mixed_behavior_profile()
    for (player, type_), weights in strategies.items():
        total = sum(weights)
        profile[str(player), str(type_)] = {str(k + 1): w / total for k, w in enumerate(weights)}
    assert isinstance(profile.payoffs["1"], float)
    for player in PLAYERS:
        assert profile.payoffs[player] == pytest.approx(float(exact.payoffs[player]))
    assert profile.max_regret() == pytest.approx(float(exact.max_regret()))


def test_random_profiles_match_oracle(game, oracle):
    for _ in range(5):
        profile = game.random_behavior_profile(denom=6)
        strategies = {
            (int(p), int(t)): [prob for _, prob in profile[p, t]] for p in PLAYERS for t in TYPES
        }
        reference = bagg_oracle.behavior_profile(oracle, strategies)
        assert profile.payoffs == reference.payoffs
        assert profile.max_regret() == reference.max_regret()


@pytest.mark.parametrize(
    "strategies",
    [PURE_NASH, FIRST_ACTIONS, SAME_NODES, UNIFORM, MIXED],
    ids=["pure-nash", "first-actions", "common-nodes", "uniform", "mixed"],
)
def test_agent_max_regret_is_largest_conditional_regret(game, desc, strategies):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    regrets = _conditional_regrets(desc, strategies)
    assert profile.agent_max_regret() == max(regrets.values())


def test_agent_max_regret_zero_at_equilibrium(game):
    profile = _set(game.mixed_behavior_profile(rational=True), PURE_NASH)
    assert profile.agent_max_regret() == 0
    assert profile.max_regret() == 0


def test_player_and_agent_regrets_are_on_different_scales(game, desc):
    profile = _set(game.mixed_behavior_profile(rational=True), FIRST_ACTIONS)
    assert profile.max_regret() == R(1, 2)
    assert profile.agent_max_regret() > profile.max_regret()


@pytest.mark.parametrize("rational", [False, True])
def test_infoset_probs_are_priors(game, rational):
    probs = game.mixed_behavior_profile(rational=rational).infoset_probs
    expected = {
        "1": {"1": Fraction(1, 3), "2": Fraction(2, 3)},
        "2": {"1": Fraction(1, 4), "2": Fraction(3, 4)},
    }
    for player in PLAYERS:
        for infoset in TYPES:
            assert probs[player][infoset] == pytest.approx(float(expected[player][infoset]))
            if rational:
                assert probs[player][infoset] == expected[player][infoset]
    with pytest.raises(KeyError):
        probs["3"]


def test_copy_is_independent(game):
    profile = game.mixed_behavior_profile(rational=True)
    duplicate = profile.copy()
    assert duplicate == profile
    duplicate["1", "1"] = {"1": 1}
    assert duplicate != profile
    assert profile["1", "1"] == {"1": R(1, 2), "2": R(1, 2)}
    profile["1", "1"] = {"1": 1}
    assert duplicate == profile


def test_equality_across_numeric_types(game):
    assert game.mixed_behavior_profile() != game.mixed_behavior_profile(rational=True)
    assert game.mixed_behavior_profile() == game.mixed_behavior_profile()


def test_as_float(game):
    profile = _set(game.mixed_behavior_profile(rational=True), MIXED)
    as_float = profile.as_float()
    assert isinstance(as_float, gbt.BAGGMixedBehaviorProfileDouble)
    assert as_float["1", "1"]["2"] == pytest.approx(0.75)
    assert as_float.payoffs["1"] == pytest.approx(float(profile.payoffs["1"]))
    double = game.mixed_behavior_profile()
    assert double.as_float() == double


def test_normalize(game):
    profile = game.mixed_behavior_profile(rational=True)
    profile["1", "1"] = {"1": 1, "2": 3}
    normalized = profile.normalize()
    assert normalized["1", "1"] == {"1": R(1, 4), "2": R(3, 4)}
    assert normalized["1", "2"] == {"1": R(1, 2), "2": R(1, 2)}
    assert profile["1", "1"] == {"1": 1, "2": 3}
    double = game.mixed_behavior_profile()
    double["2", "2"] = {"1": 1, "2": 1}
    assert double.normalize()["2", "2"] == {"1": 0.5, "2": 0.5}


def test_coffee_game_ex_ante_payoffs():
    game = games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg")
    assert list(game.players) == ["1", "2", "3"]
    exact = game.mixed_behavior_profile(rational=True)
    double = game.mixed_behavior_profile()
    for player in game.players:
        assert double.payoffs[player] == pytest.approx(float(exact.payoffs[player]))
    assert double.max_regret() == pytest.approx(float(exact.max_regret()))


STRATEGY_SETS = [PURE_NASH, FIRST_ACTIONS, SAME_NODES, UNIFORM, MIXED]
STRATEGY_IDS = ["pure-nash", "first-actions", "common-nodes", "uniform", "mixed"]


@pytest.mark.parametrize("strategies", STRATEGY_SETS, ids=STRATEGY_IDS)
def test_conditional_values_match_oracle_exact(game, oracle, strategies):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    reference = bagg_oracle.behavior_profile(oracle, strategies)
    for player in PLAYERS:
        for type_ in TYPES:
            selector = bagg_oracle.infoset_selector(oracle, int(player), int(type_))
            assert profile.infoset_values[player][type_] == reference.infoset_values[selector]
            assert profile.infoset_regrets[player][type_] == reference.infoset_regrets[selector]
            assert dict(profile.action_values[player][type_]) == dict(
                reference.action_values[selector]
            )
            assert dict(profile.action_regrets[player][type_]) == dict(
                reference.action_regrets[selector]
            )


@pytest.mark.parametrize("strategies", STRATEGY_SETS, ids=STRATEGY_IDS)
def test_conditional_values_match_oracle_double(game, oracle, strategies):
    profile = game.mixed_behavior_profile()
    for (player, type_), weights in strategies.items():
        total = sum(weights)
        profile[str(player), str(type_)] = {str(k + 1): w / total for k, w in enumerate(weights)}
    reference = bagg_oracle.behavior_profile(oracle, strategies)
    for player in PLAYERS:
        for type_ in TYPES:
            selector = bagg_oracle.infoset_selector(oracle, int(player), int(type_))
            assert profile.infoset_values[player][type_] == pytest.approx(
                float(reference.infoset_values[selector])
            )
            for action, value in profile.action_values[player][type_]:
                assert value == pytest.approx(float(reference.action_values[selector][action]))
            for action, regret in profile.action_regrets[player][type_]:
                assert regret == pytest.approx(float(reference.action_regrets[selector][action]))


@pytest.mark.parametrize("strategies", STRATEGY_SETS, ids=STRATEGY_IDS)
def test_regrets_are_best_action_value_minus_type_value(game, strategies):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    for player in PLAYERS:
        for type_ in TYPES:
            values = dict(profile.action_values[player][type_])
            assert profile.infoset_regrets[player][type_] == (
                max(values.values()) - profile.infoset_values[player][type_]
            )
            for action, value in values.items():
                others = [v for k, v in values.items() if k != action]
                expected = max(max(others) - value, 0) if others else 0
                assert profile.action_regrets[player][type_][action] == expected


@pytest.mark.parametrize("strategies", STRATEGY_SETS, ids=STRATEGY_IDS)
def test_largest_infoset_regret_is_agent_max_regret(game, strategies):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    largest = max(
        regret for regrets in profile.infoset_regrets for _, regret in regrets[1].items()
    )
    assert largest == profile.agent_max_regret()


def test_conditional_quantities_are_indexed_by_player_and_infoset(game):
    profile = game.mixed_behavior_profile()
    assert [player for player, _ in profile.infoset_values] == PLAYERS
    assert list(profile.infoset_values["1"].keys()) == TYPES
    assert len(profile.action_values["1"]["1"]) == len(profile["1", "1"])


@pytest.mark.parametrize("rational", [False, True])
def test_liap_values_zero_at_equilibrium(game, rational):
    profile = game.mixed_behavior_profile(rational=rational)
    for (player, type_), weights in PURE_NASH.items():
        profile[str(player), str(type_)] = {str(k + 1): w for k, w in enumerate(weights)}
    assert profile.liap_value() == 0
    assert profile.agent_liap_value() == 0


@pytest.mark.parametrize("strategies", STRATEGY_SETS[1:], ids=STRATEGY_IDS[1:])
def test_liap_values_match_definitions(game, strategies):
    profile = _set(game.mixed_behavior_profile(rational=True), strategies)
    expected_agent = sum(
        r**2
        for player in PLAYERS
        for type_ in TYPES
        for _, r in profile.action_values[player][type_]
        for r in [max(r - profile.infoset_values[player][type_], 0)]
    )
    expected = sum(
        (profile.infoset_probs[player][type_] * max(v - profile.infoset_values[player][type_], 0))
        ** 2
        for player in PLAYERS
        for type_ in TYPES
        for _, v in profile.action_values[player][type_]
    )
    assert profile.agent_liap_value() == expected_agent
    assert profile.liap_value() == expected
    assert (profile.liap_value() == 0) == (profile.max_regret() == 0)
