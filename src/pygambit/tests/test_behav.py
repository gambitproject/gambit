import pytest

import pygambit as gbt


@pytest.mark.parametrize(
    "game,dpayoffs,rpayoffs",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [3.0, 3.0, 3.25],
     [gbt.Rational(3, 1), gbt.Rational(3, 1), gbt.Rational(13, 4)])]
)
def test_payoff(game: gbt.Game, dpayoffs: list, rpayoffs: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(game.players)):
        assert profile_double.payoff(game.players[i]) == dpayoffs[i]
        assert profile_rational.payoff(game.players[i]) == rpayoffs[i]


@pytest.mark.parametrize(
    "game,label,dpayoffs,rpayoffs",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Player 1", "Player 2", "Player 3"],
     [3.0, 3.0, 3.25],
     [gbt.Rational(3, 1), gbt.Rational(3, 1), gbt.Rational(13, 4)])]
)
def test_payoff_label(game: gbt.Game, label: list, dpayoffs: list, rpayoffs: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(label)):
        assert profile_double.payoff(label[i]) == dpayoffs[i]
        assert profile_rational.payoff(label[i]) == rpayoffs[i]


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_is_defined_at(game: gbt.Game):
    infoset = game.players[0].infosets[0]
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    assert profile_double.is_defined_at(infoset)
    assert profile_rational.is_defined_at(infoset)


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), "Infoset 1:1"),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), "Infoset 2:1"),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), "Infoset 3:1")]
)
def test_is_defined_at_label(game: gbt.Game, label: str):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    assert profile_double.is_defined_at(label)
    assert profile_rational.is_defined_at(label)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_get_probabilities_action(game: gbt.Game):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    action = game.players[0].infosets[0].actions[0]
    assert profile_double[action] == 0.5
    assert profile_rational[action] == gbt.Rational("1/2")


@pytest.mark.parametrize(
    "game,label",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["U1", "D1", "U2", "D2", "D3", "U3"])]
)
def test_get_probabilities_action_label(game: gbt.Game, label: str):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in label:
        assert profile_double[i] == 0.5
        assert profile_rational[i] == gbt.Rational("1/2")


@pytest.mark.parametrize(
    "game,infprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [0.5, 0.5]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [gbt.Rational("1/2"), gbt.Rational("1/2")])]
)
def test_get_probabilities_infoset(game: gbt.Game, infprob: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(game.players)):
        infs = game.players[i].infosets[0]
        assert profile_double[infs] == infprob
        assert profile_rational[infs] == infprob


@pytest.mark.parametrize(
    "game,label,infprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"], [0.5, 0.5]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"],
     [gbt.Rational("1/2"), gbt.Rational("1/2")])]
)
def test_get_probabilities_infoset_label(game: gbt.Game, label: list, infprob: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(game.players)):
        for j in range(len(label)):
            infs = game.players[i]
            assert profile_double[infs][label[j]] == infprob
            assert profile_rational[infs][label[j]] == infprob


@pytest.mark.parametrize(
    "game,plprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [0.5, 0.5]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [gbt.Rational("1/2"), gbt.Rational("1/2")])]
)
def test_get_probabilities_player(game: gbt.Game, plprob: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(game.players)):
        player = game.players[i]
        assert profile_double[player] == [plprob]
        assert profile_rational[player] == [plprob]


@pytest.mark.parametrize(
    "game,label,plprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Player 1", "Player 2", "Player 3"], [0.5, 0.5]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Player 1", "Player 2", "Player 3"],
     [gbt.Rational("1/2"), gbt.Rational("1/2")])]
)
def test_get_probabilities_player_label(game: gbt.Game, label: list, plprob: list):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(game.players)):
        for j in range(len(label)):
            player = game.players[i]
            assert profile_double[player][label[j]] == [plprob]
            assert profile_rational[player][label[j]] == [plprob]


@pytest.mark.parametrize(
    "game,actprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), 0.72),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     gbt.Rational("2/9"))]
)
def test_set_probabilities_actions(game: gbt.Game, actprob: float):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    profile_double[game.actions[0]] = 0.72
    profile_rational[game.actions[0]] = gbt.Rational("2/9")
    assert profile_double[game.actions[0]] == 0.72
    assert profile_rational[game.actions[0]] == gbt.Rational("2/9")


@pytest.mark.parametrize(
    "game,label,actprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["U1"], 0.72),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["D1"],
     gbt.Rational("2/9"))]
)
def test_set_probabilities_actions_label(game: gbt.Game, label: list, actprob: float):
    profile_double = game.mixed_behavior_profile(False)
    profile_rational = game.mixed_behavior_profile(True)
    for i in range(len(label)):
        profile_double[game.actions[label[i]]] == 0.72
        profile_rational[game.actions[label[i]]] == gbt.Rational("2/9")


@pytest.mark.parametrize(
    "game,infdprob,infrprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [0.72, 0.28],
     [gbt.Rational("2/9"), gbt.Rational("7/9")]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [0.42, 0.58],
     [gbt.Rational("4/13"), gbt.Rational("9/13")]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [0.02, 0.98],
     [gbt.Rational("1/98"), gbt.Rational("97/98")])]
)
def test_set_probabilities_infoset(game: gbt.Game, infdprob: list, infrprob: list):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(game.players)):
        infs = game.players[i].infosets[0]
        profile_double[infs] = infdprob
        assert profile_double[infs] == infdprob

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        infs = game.players[i].infosets[0]
        profile_rational[infs] = infrprob
        assert profile_rational[infs] == infrprob


@pytest.mark.parametrize(
    "game,label,idprob,irprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"],
     [[0.72, 0.28], [0.42, 0.58], [0.02, 0.98]],
     [[gbt.Rational("2/9"), gbt.Rational("7/9")],
     [gbt.Rational("4/13"), gbt.Rational("9/13")],
     [gbt.Rational("1/98"), gbt.Rational("97/98")]])]
)
def test_set_probabilities_infoset_label(game: gbt.Game, label: list, idprob: list, irprob: list):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(label)):
        profile_double[label[i]] = idprob[i]
        assert profile_double[label[i]] == idprob[i]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(label)):
        profile_rational[label[i]] = irprob[i]
        assert profile_rational[label[i]] == irprob[i]


@pytest.mark.parametrize(
    "game,pldprob,plrprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [[0.72, 0.28]],
     [[gbt.Rational("2/9"), gbt.Rational("7/9")]]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [[0.42, 0.58]],
     [[gbt.Rational("4/13"), gbt.Rational("9/13")]]),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [[0.02, 0.98]],
     [[gbt.Rational("1/98"), gbt.Rational("97/98")]])]
)
def test_set_probabilities_player(game: gbt.Game, pldprob: list, plrprob: list):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(game.players)):
        profile_double[game.players[i]] = pldprob
        assert profile_double[game.players[i]] == pldprob

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        profile_rational[game.players[i]] = plrprob
        assert profile_rational[game.players[i]] == plrprob


@pytest.mark.parametrize(
    "game,label,pdprob,prprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Player 1", "Player 2", "Player 3"],
     [[[0.72, 0.28]], [[0.42, 0.58]], [[0.02, 0.98]]],
     [[[gbt.Rational("2/9"), gbt.Rational("7/9")]],
     [[gbt.Rational("4/13"), gbt.Rational("9/13")]],
     [[gbt.Rational("1/98"), gbt.Rational("97/98")]]])]
)
def test_set_probabilities_player_label(game: gbt.Game, label: list, pdprob: list, prprob: list):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(label)):
        profile_double[label[i]] = pdprob[i]
        assert profile_double[label[i]] == pdprob[i]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        profile_rational[label[i]] = prprob[i]
        assert profile_rational[label[i]] == prprob[i]


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_realiz_prob(game):
    assert game.mixed_behavior_profile(rational=False).realiz_prob(game.root) == 1
    assert game.mixed_behavior_profile(rational=True).realiz_prob(game.root) == 1


@pytest.mark.parametrize(
    "game,infprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), 1.0),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     gbt.Rational("1/1"))]
)
def test_infoset_prob(game: gbt.Game, infprob: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(game.players)):
        infs = profile_double.infoset_prob(game.players[i].infosets[0])
        assert infs == infprob

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        infs = profile_rational.infoset_prob(game.players[i].infosets[0])
        assert infs == infprob


@pytest.mark.parametrize(
    "game,label,infprob",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"], 1.0),
     (gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"],
     gbt.Rational("1/1"))]
)
def test_infoset_prob_label(game: gbt.Game, label: list, infprob: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(label)):
        assert profile_double.infoset_prob(label[i]) == infprob

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(label)):
        assert profile_rational.infoset_prob(label[i]) == infprob


@pytest.mark.parametrize(
    "game,dpayoff,rpayoff",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"), [3.0, 3.0, 3.25],
     [gbt.Rational("3/1"), gbt.Rational("3/1"), gbt.Rational("13/4")])]
)
def test_infoset_payoff(game: gbt.Game, dpayoff: list, rpayoff: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(game.players)):
        pl_inf = game.players[i].infosets[0]
        assert profile_double.infoset_value(pl_inf) == dpayoff[i]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        pl_inf = game.players[i].infosets[0]
        assert profile_rational.infoset_value(pl_inf) == rpayoff[i]


@pytest.mark.parametrize(
    "game,label,dpayoff,rpayoff",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["Infoset 1:1", "Infoset 2:1", "Infoset 3:1"], [3.0, 3.0, 3.25],
     [gbt.Rational("3/1"), gbt.Rational("3/1"), gbt.Rational("13/4")])]
)
def test_infoset_payoff_label(game: gbt.Game, label: list, dpayoff: list, rpayoff: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(label)):
        assert profile_double.infoset_value(label[i]) == dpayoff[i]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(label)):
        assert profile_rational.infoset_value(label[i]) == rpayoff[i]


@pytest.mark.parametrize(
    "game,dpayoff,rpayoff",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [3.0, 3.0, 3.0, 3.0, 3.5, 3.0],
     [gbt.Rational("3/1"), gbt.Rational("3/1"), gbt.Rational("3/1"),
     gbt.Rational("3/1"), gbt.Rational("7/2"), gbt.Rational("3/1")])]
)
def test_action_payoff(game: gbt.Game, dpayoff: float, rpayoff: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(game.players)):
        tmp = game.players[i].infosets[0]
        for j in range(len(tmp.actions)):
            assert profile_double.action_value(tmp.actions[j]) == dpayoff[2*i+j]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(game.players)):
        tmp = game.players[i].infosets[0]
        for j in range(len(tmp.actions)):
            assert profile_rational.action_value(tmp.actions[j]) == rpayoff[2*i+j]


@pytest.mark.parametrize(
    "game,label,dpayoff,rpayoff",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     ["U1", "D1", "U2", "D2", "U3", "D3"],
     [3.0, 3.0, 3.0, 3.0, 3.5, 3.0],
     [gbt.Rational("3/1"), gbt.Rational("3/1"), gbt.Rational("3/1"),
     gbt.Rational("3/1"), gbt.Rational("7/2"), gbt.Rational("3/1")])]
)
def test_action_payoff_label(game: gbt.Game, label: list, dpayoff: float, rpayoff: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(label)):
        assert profile_double.action_value(label[i]) == dpayoff[i]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(label)):
        assert profile_rational.action_value(label[i]) == dpayoff[i]


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_regret(game: gbt.Game):
    profile_double = game.mixed_behavior_profile(rational=False)
    profile_rational = game.mixed_behavior_profile(rational=True)
    for profile in [profile_double, profile_rational]:
        for player in game.players:
            for infoset in player.infosets:
                for action in infoset.actions:
                    assert (
                        profile.regret(action) ==
                        max(profile.action_value(a) for a in infoset.actions) -
                        profile.action_value(action)
                    )


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_node_value(game: gbt.Game):
    profile_double = game.mixed_behavior_profile(rational=False)
    profile_rational = game.mixed_behavior_profile(rational=True)
    for profile in [profile_double, profile_rational]:
        for player in game.players:
            assert (
                profile.node_value(player, game.root) == profile.payoff(player)
            )


@pytest.mark.parametrize(
    "game,probs",
    [(gbt.Game.read_game("test_games/complicated_extensive_game.efg"),
     [0.5, 0.5])]
)
def test_martingale_property(game: gbt.Game, probs: list):
    profile_double = game.mixed_behavior_profile(rational=False)
    parent = game.root.children[0]
    childs = parent.children
    tsum = 0
    for i in range(len(childs)):
        tsum += probs[i]*profile_double.node_value(game.players[1], childs[i])
    assert (
       profile_double.node_value(game.players[1], parent) == tsum
    )


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_liap_value(game: gbt.Game):
    profile_double = game.mixed_behavior_profile(rational=False)
    profile_rational = game.mixed_behavior_profile(rational=True)
    assert profile_double.liap_value() == 0.0625
    assert profile_rational.liap_value() == gbt.Rational("1/16")


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/mixed_behavior_game.efg")]
)
def test_as_strategy(game: gbt.Game):
    profile_double = game.mixed_behavior_profile(rational=False)
    profile_rational = game.mixed_behavior_profile(rational=True)
    mixed_double = profile_double.as_strategy()
    mixed_rational = profile_rational.as_strategy()
    assert (
        [mixed_double[strategy] for strategy in game.strategies] ==
        [profile_double[action] for action in game.actions]
    )
    assert (
        [mixed_rational[strategy] for strategy in game.strategies] ==
        [profile_rational[action] for action in game.actions]
    )


@pytest.mark.parametrize(
    "game,dvalues,rvalues,tdvalues,trvalues,threshold",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [0.8, 0.2, 1.0, 1.5, 0.0, 0.4],
     [gbt.Rational(4, 5), gbt.Rational(1, 5), gbt.Rational(1, 1),
     gbt.Rational(3, 2), gbt.Rational(0, 1), gbt.Rational(2, 5)],
     [0.8, 0.2, 0.32, 0.48, 0.08, 0.12],
     [gbt.Rational(4, 5), gbt.Rational(1, 5), gbt.Rational(8, 25),
     gbt.Rational(12, 25), gbt.Rational(2, 25), gbt.Rational(3, 25)],
     1e-13)]
)
def test_node_belief(game: gbt.Game, dvalues: float, rvalues: float,
                     tdvalues: float, trvalues: float, threshold: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(dvalues)):
        profile_double[game.actions[i]] = dvalues[i]
    assert profile_double.belief(game.root) == 1
    for i in range(1, 3):
        tmp = game.infosets[i]
        for j in range(len(tmp.members)):
            assert (
               abs(profile_double.belief(tmp.members[j]) - tdvalues[2*(i-1) + j])
            ) < threshold

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(rvalues)):
        profile_rational[game.actions[i]] = rvalues[i]
    assert profile_rational.belief(game.root) == gbt.Rational(1, 1)
    for i in range(1, 3):
        tmp = game.infosets[i]
        for j in range(len(tmp.members)):
            assert (
               profile_rational.belief(tmp.members[j]) == trvalues[2*(i-1) + j]
            )


@pytest.mark.parametrize(
    "game,dvalues,rvalues,tdvalues,trvalues",
    [(gbt.Game.read_game("test_games/mixed_behavior_game.efg"),
     [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
     [gbt.Rational(4, 5), gbt.Rational(1, 5), gbt.Rational(2, 5),
     gbt.Rational(3, 5), gbt.Rational(0, 1), gbt.Rational(1, 1)],
     [1.0], [gbt.Rational(1, 1)])]
)
def test_infoset_belief(game: gbt.Game, dvalues: float, rvalues: float,
                        tdvalues: float, trvalues: float):
    profile_double = game.mixed_behavior_profile(rational=False)
    for i in range(len(dvalues)):
        profile_double[game.actions[i]] = dvalues[i]
    assert profile_double.belief(game.infosets[0].members[0]) == tdvalues[0]

    profile_rational = game.mixed_behavior_profile(rational=True)
    for i in range(len(rvalues)):
        profile_rational[game.actions[i]] = rvalues[i]
    assert profile_rational.belief(game.infosets[0].members[0]) == trvalues[0]


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_payoff_with_chance_player(game: gbt.Game):
    profile_double_with_chance = game.mixed_behavior_profile(False)
    profile_rational_with_chance = game.mixed_behavior_profile(True)
    chance_player = game.players.chance
    with pytest.raises(ValueError):
        profile_double_with_chance.payoff(chance_player)
        profile_rational_with_chance.payoff(chance_player)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_payoff_with_chance_player_infoset(game: gbt.Game):
    profile_double_with_chance = game.mixed_behavior_profile(False)
    profile_rational_with_chance = game.mixed_behavior_profile(True)
    chance_infoset = game.players.chance.infosets[0]
    with pytest.raises(ValueError):
        profile_double_with_chance.infoset_value(chance_infoset)
        profile_rational_with_chance.infoset_value(chance_infoset)


@pytest.mark.parametrize(
    "game",
    [gbt.Game.read_game("test_games/complicated_extensive_game.efg")]
)
def test_payoff_with_chance_player_action(game: gbt.Game):
    profile_double_with_chance = game.mixed_behavior_profile(False)
    profile_rational_with_chance = game.mixed_behavior_profile(True)
    chance_action = game.players.chance.infosets[0].actions[0]
    with pytest.raises(ValueError):
        profile_double_with_chance.action_value(chance_action)
        profile_rational_with_chance.action_value(chance_action)
