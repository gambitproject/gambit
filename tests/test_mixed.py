import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        (games.create_coord_4x4_nfg(),
         [["0", "0", "0", "0"], ["1/3", "1/3", "1/3", "0"]], True),
        (games.create_coord_4x4_nfg(),
         [["1", "0", "0", "0"], ["0", "0", "0", "0"]], True),
        (games.create_coord_4x4_nfg(),
         [[0, 0, 0, 0], [1.0, 1.0, 1.0, 1.0]], False),
        (games.create_coord_4x4_nfg(),
         [[1.0, 1.0, 1.0, 1.0], [0, 0, 0, 0]], False),
    ],
)
def test_normalize_zero_value_error(game, profile_data, rational_flag):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    with pytest.raises(ValueError, match="zero"):
        profile.normalize()


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        (games.create_coord_4x4_nfg(),
         [["1", "1", "0", "-1"], ["1/3", "1/3", "1/3", "0"]], True),
        (games.create_zero_matrix_nfg(),
         [["1", "0"], ["0", "-1"]], True),
        (games.create_coord_4x4_nfg(),
         [[0, 0, 0, -1.0], [1.0, 1.0, 1.0, 1.0]], False),
        (games.create_zero_matrix_nfg(),
         [[1.0, 1.0], [0, -1.0]], False),
    ],
)
def test_normalize_neg_entry_value_error(game, profile_data, rational_flag):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    with pytest.raises(ValueError, match="negative"):
        profile.normalize()


@pytest.mark.parametrize(
    "game,profile_data,expected_data,rational_flag",
    [
        (games.create_coord_4x4_nfg(), [["1", "2", "3", "14"], ["1", "1", "1", "1"]],
            [["1/20", "2/20", "3/20", "14/20"], ["1/4", "1/4", "1/4", "1/4"]], True),
        (games.create_coord_4x4_nfg(), [[1.0, 2.0, 3.0, 14.0], [1, 1, 1, 1]],
            [[1 / 20, 2 / 20, 3 / 20, 14 / 20], [0.25, 0.25, 0.25, 0.25]], False),
    ],
)
def test_normalize(game, profile_data, expected_data, rational_flag):
    """Test the normalize function of a mixed strategy profile"""
    assert (
        game.mixed_strategy_profile(data=profile_data, rational=rational_flag).normalize() ==
        game.mixed_strategy_profile(data=expected_data, rational=rational_flag)
    )


def test_get_probabilities_strategy():
    game = games.create_strategic_game()
    strategy = game.players[0].strategies[0]
    assert game.mixed_strategy_profile(rational=False)[strategy] == 0.5
    assert game.mixed_strategy_profile(rational=True)[strategy] == gbt.Rational("1/2")


def test_get_probabilities_player():
    game = games.create_strategic_game()
    assert game.mixed_strategy_profile(rational=False)[game.players[0]] == [0.5, 0.5]
    assert (
        game.mixed_strategy_profile(rational=True)[game.players[0]] ==
        [gbt.Rational("1/2"), gbt.Rational("1/2")]
    )


def test_set_probability_strategy():
    game = games.create_strategic_game()
    profile = game.mixed_strategy_profile(rational=False)
    profile[game.players[0].strategies[0]] = 0.72
    assert profile[game.players[0].strategies[0]] == 0.72

    profile = game.mixed_strategy_profile(rational=True)
    profile[game.players[0].strategies[0]] = gbt.Rational("2/9")
    assert profile[game.players[0].strategies[0]] == gbt.Rational("2/9")


def test_set_probability_player():
    game = games.create_strategic_game()
    profile = game.mixed_strategy_profile(rational=False)
    profile[game.players[0]] = [0.72, 0.28]
    assert profile[game.players[0]] == [0.72, 0.28]

    profile = game.mixed_strategy_profile(rational=True)
    profile[game.players[0]] = [gbt.Rational("7/9"), gbt.Rational("2/9")]
    assert profile[game.players[0]] == [gbt.Rational("7/9"), gbt.Rational("2/9")]


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,payoffs",
    [
        (games.create_strategic_game(), [["1/2", "1/2"], ["1/2", "1/2"]], True, (0, 0)),
        (games.create_zero_matrix_nfg(), [["1/2", "1/2"], ["1/2", "1/2"]], True, (0, 0)),
        (games.create_coord_4x4_nfg(),
            [[0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25]],
            False, (0.25, 0.25)),
        (games.create_coord_4x4_nfg(),
            [["1/4", "1/4", "1/4", "1/4"], ["1/4", "1/4", "1/4", "1/4"]],
            True, (0.25, 0.25)),
        (games.create_coord_4x4_nfg(),
            [["1/3", "1/3", "1/3", "0"], ["1/3", "1/3", "1/3", "0"]],
            True, (gbt.Rational(1, 3), gbt.Rational(1, 3))),
        (games.create_coord_4x4_nfg(),
            [["1/3", "1/3", "0", "1/3"], ["1/3", "1/3", "1/3", "0"]],
            True, (gbt.Rational(2, 9), gbt.Rational(2, 9))),
    ],
)
def test_payoffs(game, profile_data, rational_flag, payoffs):
    """Reference data tests for MixedStrategyProfile.strategy_values"""
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for payoff, player in zip(payoffs, profile.game.players):
        assert profile.payoff(player) == payoff


def test_payoffs_by_label():
    game = games.create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).payoff("Joe") == 0
    assert game.mixed_strategy_profile(rational=True).payoff("Joe") == 0


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,strategy_values",
    [
        (games.create_strategic_game(), [[0.5, 0.5], [0.5, 0.5]], False, ([0, 0], [0, 0])),
        (games.create_strategic_game(), [["1/2", "1/2"], ["1/2", "1/2"]], True, ([0, 0], [0, 0])),
        (games.create_coord_4x4_nfg(),
            [[0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25]],
            False, ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25])),
        (games.create_coord_4x4_nfg(),
            [["1/4", "1/4", "1/4", "1/4"], ["1/4", "1/4", "1/4", "1/4"]],
            True, ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25])),
        (games.create_coord_4x4_nfg(),
            [["1", "0", "0", "0"], ["1", "0", "0", "0"]], True,
            ([gbt.Rational(1), gbt.Rational(0), gbt.Rational(0), gbt.Rational(0)],
             [gbt.Rational(1), gbt.Rational(0), gbt.Rational(0), gbt.Rational(0)])),
        (games.create_coord_4x4_nfg(),
            [["3/7", "0", "0", "4/7"], ["1/3", "1/3", "1/3", "0"]], True,
            ([gbt.Rational(1, 3), gbt.Rational(1, 3), gbt.Rational(1, 3), gbt.Rational(0)],
             [gbt.Rational(3, 7), gbt.Rational(0), gbt.Rational(0), gbt.Rational(4, 7)])),
    ]
)
def test_strategy_value(game, profile_data, rational_flag, strategy_values):
    """Reference data tests for MixedStrategyProfile.strategy_values"""
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for strategy_values_for_player, player in zip(
        strategy_values, profile.game.players
    ):
        for i, s in enumerate(player.strategies):
            assert profile.strategy_value(s) == strategy_values_for_player[i]


@pytest.mark.parametrize(
    "game",
    [
        (games.create_coord_4x4_nfg()),
        (games.create_zero_matrix_nfg()),
    ]
)
def test_strategy_regret(game):
    profile = game.mixed_strategy_profile(rational=False)
    for player in game.players:
        for strategy in player.strategies:
            assert (
                profile.regret(strategy) ==
                (
                    max(profile.strategy_value(s) for s in player.strategies)
                    - profile.strategy_value(strategy)
                )
            )


def test_strategy_value_by_label():
    game = games.create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).strategy_value("defect") == 0
    assert game.mixed_strategy_profile(rational=True).strategy_value("defect") == 0


def test_liap_value():
    game = games.create_strategic_game()
    assert game.mixed_strategy_profile(rational=False).liap_value() == 0
    assert game.mixed_strategy_profile(rational=True).liap_value() == 0


def test_as_behavior_roundtrip():
    game = games.read_from_file("mixed_behavior_game.efg")
    assert (
        game.mixed_strategy_profile(rational=False).as_behavior().as_strategy() ==
        game.mixed_strategy_profile(rational=False)
    )
    assert (
        game.mixed_strategy_profile(rational=True).as_behavior().as_strategy() ==
        game.mixed_strategy_profile(rational=True)
    )


def test_as_behavior_error():
    game = games.create_strategic_game()
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=False).as_behavior()
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=True).as_behavior()


@pytest.mark.parametrize(
    "game,profile_data,liap_expected",
    [
        (games.create_zero_matrix_nfg(),
         [["3/4", "1/4"], ["2/5", "3/5"]],
         0),
        (games.create_zero_matrix_nfg(),
         [["1/2", "1/2"], ["1/2", "1/2"]],
         0),
        (games.create_zero_matrix_nfg(),
         [[1, 0], [1, 0]],
         0),
        (games.create_coord_4x4_nfg(),
         [["1/3", "1/2", "1/12", "1/12"], ["3/8", "1/8", "1/4", "1/4"]],
         gbt.Rational("245/2304")),
        (games.create_coord_4x4_nfg(),
         [["1/4", "1/4", "1/4", "1/4"], ["1/4", "1/4", "1/4", "1/4"]],
         0),
        (games.create_coord_4x4_nfg(),
         [[1, 0, 0, 0], [1, 0, 0, 0]],
         0),
        (games.create_el_farol_bar_game(),
         [["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"]],
         0),
        (games.create_el_farol_bar_game(),
         [["1/1", "0/1"], ["1/1", "0/1"], ["0/1", "1/1"], ["0/1", "1/1"], ["0/1", "1/1"]],
         0),
    ]
)
def test_liapunov_value_reference(game, profile_data, liap_expected):
    assert (
        game.mixed_strategy_profile(rational=True, data=profile_data).liap_value() == liap_expected
    )


@pytest.mark.parametrize(
    "game,profile_data,liap_expected,tol",
    [
        (games.create_coord_4x4_nfg(),
         [[1/3, 1/2, 1/12, 1/12], [3/8, 1/8, 1/4, 1/4]],
         gbt.Rational("245/2304"), 1e-9),
        (games.create_zero_matrix_nfg(),
         [[1/4, 3/4], [2/5, 3/5]],
         0, 1e-9),
    ]
)
def test_liapunov_value_reference_double(game, profile_data, liap_expected, tol):
    assert (
        abs(game.mixed_strategy_profile(rational=False, data=profile_data).liap_value()
            - liap_expected) < tol
    )


@pytest.mark.parametrize(
    "game,profile_data",
    [
        (games.create_coord_4x4_nfg(),
         [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]]),
        (games.create_centipede_game_with_chance(),
         [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/1"]]),
        (games.create_el_farol_bar_game(),
         [["1/1", "0/1"], ["1/2", "1/2"], ["1/3", "2/3"], ["1/5", "4/5"], ["1/8", "7/8"]]),
    ]
)
def test_liapunov_value_consistency(game, profile_data):
    profile = game.mixed_strategy_profile(rational=True, data=profile_data)
    assert (
        profile.liap_value() ==
        sum([max(profile.strategy_value(strategy) - profile.payoff(player), 0)**2
             for player in game.players for strategy in player.strategies])
    )


@pytest.mark.parametrize(
    "game,profile_data,tol",
    [
        (games.create_coord_4x4_nfg(),
         [[1/3, 1/3, 0/3, 1/3], [1/4, 1/4, 3/8, 1/8]], 1e-5),
        (games.create_centipede_game_with_chance(),
         [[1/3, 1/3, 1/3, 0], [.10, 3/5, .3, 0]], 1e-5),
    ]
)
def test_liapunov_value_consistency_double(game, profile_data, tol):
    profile = game.mixed_strategy_profile(rational=False, data=profile_data)

    assert (
        abs(profile.liap_value() -
            sum([max(profile.strategy_value(strategy) - profile.payoff(player), 0)**2
                for player in game.players for strategy in player.strategies])) < tol
    )


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha",
    [
     (games.create_coord_4x4_nfg(),
      [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]],
      [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]],
      gbt.Rational("3/5")),
     (games.create_zero_matrix_nfg(),
      [["1/4", "3/4"], ["3/5", "2/5"]],
      [["1/2", "1/2"], ["3/5", "2/5"]],
      gbt.Rational("5/6")),
     (games.create_centipede_game_with_chance(),
      [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/1"]],
      [["1/3", "1/3", "1/3", "0/1"], ["1/5", "2/5", "1/5", "1/5"]],
      gbt.Rational("1/12")),
     (games.create_selten_horse_game(),
      [["4/9", "5/9"], ["1/11", "10/11"], ["8/9", "1/9"]],
      [["4/9", "5/9"], ["10/11", "1/11"], ["8/9", "1/9"]],
      gbt.Rational("4/9")),
     (games.create_el_farol_bar_game(),
      [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["11/12", "1/12"], ["1/2", "1/2"]],
      [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["1/12", "11/12"], ["1/2", "1/2"]],
      gbt.Rational("1/2")),
    ]
)
def test_linearity_payoff(game, profile1, profile2, alpha):
    profile1 = game.mixed_strategy_profile(rational=True, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=True, data=profile2)

    profile_data = [[alpha*profile1[strategy] + (1-alpha)*profile2[strategy]
                    for strategy in player.strategies] for player in game.players]
    profile3 = game.mixed_strategy_profile(rational=True, data=profile_data)

    for player in game.players:
        assert (
           alpha*profile1.payoff(player) + (1 - alpha)*profile2.payoff(player) ==
           profile3.payoff(player)
        )


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha,tol",
    [
     (games.create_coord_4x4_nfg(),
      [[1/5, 2/5, 0/5, 2/5], [3/8, 1/4, 3/8, 0/4]],
      [[1/5, 2/5, 0/5, 2/5], [1/4, 3/8, 0/4, 3/8]],
      3/5, 1e-5),
     (games.create_centipede_game_with_chance(),
      [[1/3, 1/3, 1/3, 0/1], [1/10, 3/5, 3/10, 0/1]],
      [[1/3, 1/3, 1/3, 0/1], [1/5, 2/5, 1/5, 1/5]],
      1/12, 1e-5)
    ]
)
def test_linearity_payoff_double(game, profile1, profile2, alpha, tol):
    profile1 = game.mixed_strategy_profile(rational=False, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=False, data=profile2)

    profile_data = [[alpha*profile1[strategy] + (1-alpha)*profile2[strategy]
                    for strategy in player.strategies] for player in game.players]
    profile3 = game.mixed_strategy_profile(rational=False, data=profile_data)

    for player in game.players:
        assert (
           abs(alpha*profile1.payoff(player) + (1 - alpha)*profile2.payoff(player) -
               profile3.payoff(player)) < tol
        )


@pytest.mark.parametrize(
    "game,profile_data",
    [
     (games.create_coord_4x4_nfg(),
      [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]]),
     (games.create_zero_matrix_nfg(),
      [["4/5", "1/5"], ["4/7", "3/7"]]),
     (games.create_centipede_game_with_chance(),
      [["1/5", "2/5", "1/5", "1/5"], ["1/10", "3/5", "3/10", "0/1"]]),
     (games.create_selten_horse_game(),
      [["4/9", "5/9"], ["6/11", "5/11"], ["4/7", "3/7"]]),
     (games.create_el_farol_bar_game(),
      [["4/9", "5/9"], ["1/3", "2/3"], ["0/1", "1/1"], ["11/12", "1/12"], ["1/3", "2/3"]]),
    ]
)
def test_payoff_and_strategy_value_consistency(game, profile_data):
    profile = game.mixed_strategy_profile(rational=True, data=profile_data)

    for player in game.players:
        assert (
           sum([profile[player][strategy] * profile.strategy_value(strategy)
                for strategy in player.strategies]) ==
           profile.payoff(player)
        )


@pytest.mark.parametrize(
    "game,profile_data,tol",
    [
     (games.create_coord_4x4_nfg(),
      [[.2, .4, 0, .4], [1/4, 3/8, 0, 3/8]], 1e-5),
     (gbt.Game.from_arrays([[1, 2], [-3, 4]], [[-4, 3], [2, 1]]),
      [[1/2, 1/2], [3/5, 2/5]], 1e-5),
     (games.create_selten_horse_game(),
      [[4/9, 5/9], [6/11, 5/11], [4/7, 3/7]], 1e-5),
     (games.create_centipede_game_with_chance(),
      [[1/3, 1/3, 1/3, 0/1], [1/10, 3/5, 3/10, 0/1]], 1e-5)
    ]
)
def test_payoff_and_strategy_value_consistency_double(game, profile_data, tol):
    profile = game.mixed_strategy_profile(rational=False, data=profile_data)

    for player in game.players:
        assert (
           abs(sum([profile[player][strategy] * profile.strategy_value(strategy)
                    for strategy in player.strategies]) -
               profile.payoff(player)) < tol
        )


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha",
    [
     (games.create_coord_4x4_nfg(),
      [["1/1111", "10/1111", "100/1111", "1000/1111"], ["1/4", "1/8", "3/8", "1/4"]],
      [["1/1111", "10/1111", "99/1111", "1001/1111"], ["1/4", "1/8", "3/8", "1/4"]],
      gbt.Rational("1/2")),
     (games.create_centipede_game_with_chance(),
      [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/10"]],
      [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/10"]],
      gbt.Rational("82943/62500")),
    ]
)
def test_linearity_strategy_value(game, profile1, profile2, alpha):
    profile1 = game.mixed_strategy_profile(rational=True, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=True, data=profile2)

    profile_data = [[alpha*profile1[strategy] + (1-alpha)*profile2[strategy]
                    for strategy in player.strategies] for player in game.players]
    profile3 = game.mixed_strategy_profile(rational=True, data=profile_data)

    for player in game.players:
        for strategy in player.strategies:
            assert (
               profile3.strategy_value(strategy) ==
               alpha*profile1.strategy_value(strategy) +
               (1-alpha)*profile2.strategy_value(strategy)
            )
