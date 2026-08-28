import pathlib

import pytest

import pygambit as gbt
from pygambit.nashphc import (
    _contingencies,
    _equilibrium_equations,
    _format_profile,
    _format_support,
    _is_nash,
    _playerletters,
    _process_phc_output,
    _profile_from_support,
    _solution_to_profile,
    _strategy_index,
    phcpack_solve,
)


@pytest.fixture
def matching_pennies():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 1, "2": -1}, "a")
    game.make_outcome({"1": "1", "2": "2"}, {"1": -1, "2": 1}, "b")
    game.make_outcome({"1": "2", "2": "1"}, {"1": -1, "2": 1}, "c")
    game.make_outcome({"1": "2", "2": "2"}, {"1": 1, "2": -1}, "d")
    return game


def test_playerletters_excludes_disallowed_variable_letters():
    """PHC does not allow 'e', 'i', or 'j' in variable names."""
    assert "e" not in _playerletters
    assert "i" not in _playerletters
    assert "j" not in _playerletters
    assert len(_playerletters) == 23


def test_strategy_index(matching_pennies):
    player = matching_pennies.players["1"]
    assert _strategy_index(player, "1") == 0
    assert _strategy_index(player, "2") == 1


def test_contingencies_skips_given_player(matching_pennies):
    p1, p2 = matching_pennies.players
    support = matching_pennies.strategy_support_profile()
    conts = list(_contingencies(support, p1))
    assert all(cont[p1.number] is None for cont in conts)
    assert {cont[p2.number] for cont in conts} == {"1", "2"}


def test_equilibrium_equations(matching_pennies):
    p1, p2 = matching_pennies.players
    support = matching_pennies.strategy_support_profile()
    equations = _equilibrium_equations(support, p1)
    assert equations == [
        "((1*b0)+(-1*b1))-((-1*b0)+(1*b1))",
        "a0+a1-1",
    ]


def test_is_nash_true_for_mixed_equilibrium(matching_pennies):
    profile = matching_pennies.mixed_strategy_profile()
    profile["1"] = {"1": 0.5, "2": 0.5}
    profile["2"] = {"1": 0.5, "2": 0.5}
    assert _is_nash(profile, 1e-6, 1e-6)


def test_is_nash_false_for_pure_profile(matching_pennies):
    profile = matching_pennies.mixed_strategy_profile()
    profile["1"] = {"1": 1.0, "2": 0.0}
    profile["2"] = {"1": 1.0, "2": 0.0}
    assert not _is_nash(profile, 1e-6, 1e-6)


def test_solution_to_profile(matching_pennies):
    entry = {"vars": {"a0": complex(0.5, 0), "a1": complex(0.5, 0),
                      "b0": complex(0.5, 0), "b1": complex(0.5, 0)}}
    profile = _solution_to_profile(matching_pennies, entry)
    assert profile["1"] == {"1": 0.5, "2": 0.5}
    assert profile["2"] == {"1": 0.5, "2": 0.5}


def test_solution_to_profile_missing_variable_defaults_to_zero(matching_pennies):
    entry = {"vars": {"a0": complex(1.0, 0), "b0": complex(1.0, 0)}}
    profile = _solution_to_profile(matching_pennies, entry)
    assert profile["1"] == {"1": 1.0, "2": 0.0}
    assert profile["2"] == {"1": 1.0, "2": 0.0}


def test_format_support(matching_pennies):
    support = matching_pennies.strategy_support_profile()
    assert _format_support(support, "candidate") == "candidate,11,11"


def test_profile_from_support_pure_strategy_support(matching_pennies):
    support = matching_pennies.strategy_support_profile(lambda player, label: label == "1")
    profile = _profile_from_support(support)
    assert profile["1"] == {"1": 1.0, "2": 0.0}
    assert profile["2"] == {"1": 1.0, "2": 0.0}


def test_format_profile(matching_pennies):
    profile = matching_pennies.mixed_strategy_profile()
    profile["1"] = {"1": 0.5, "2": 0.5}
    profile["2"] = {"1": 0.5, "2": 0.5}
    assert _format_profile(profile, "test", decimals=2) == "test,0.50,0.50,0.50,0.50"


def test_process_phc_output_single_solution_with_diagnostics():
    output = (
        "\nTHE SOLUTIONS :\n\n"
        "1 2\n"
        "===========================================================================\n"
        "solution 1 :\n"
        "t :  0.0 0.0\n"
        "m :  1\n"
        "the solution for t :\n"
        " a0 :  0.5 0.0\n"
        " a1 :  0.5 0.0\n"
        "== err :  1e-16 = rco :  1.0 = res :  0.0 =\n"
        "===========================================================================\n"
    )
    solutions = _process_phc_output(output)
    assert len(solutions) == 1
    assert solutions[0]["vars"] == {"a0": complex(0.5, 0.0), "a1": complex(0.5, 0.0)}
    assert solutions[0]["t"] == complex(0.0, 0.0)
    assert solutions[0]["m"] == 1
    assert solutions[0]["err"] == 1e-16
    assert solutions[0]["rco"] == 1.0
    assert solutions[0]["res"] == 0.0


def test_process_phc_output_multiple_solutions_terminated_by_timing():
    output = (
        "THE SOLUTIONS :\n\n"
        "solution 1 :\n"
        " a0 :  1.0 0.0\n"
        " a1 :  0.0 0.0\n"
        "solution 2 :\n"
        " a0 :  0.0 0.0\n"
        " a1 :  1.0 0.0\n"
        "TIMING INFORMATION\n"
    )
    solutions = _process_phc_output(output)
    assert len(solutions) == 2
    assert solutions[0]["vars"] == {"a0": complex(1.0, 0.0), "a1": complex(0.0, 0.0)}
    assert solutions[1]["vars"] == {"a0": complex(0.0, 0.0), "a1": complex(1.0, 0.0)}


def test_phcpack_solve_mocked_subprocess(monkeypatch, matching_pennies):
    """`phcpack_solve` writes equations to a temp file, invokes the external PHC binary,
    and parses its output file -- verified here without requiring the real `phc` binary.
    """
    phc_output = (
        "THE SOLUTIONS :\n\n"
        "solution 1 :\n"
        " a0 :  5.0E-01 0.0E+00\n"
        " a1 :  5.0E-01 0.0E+00\n"
        " b0 :  5.0E-01 0.0E+00\n"
        " b1 :  5.0E-01 0.0E+00\n"
        "TIMING INFORMATION\n"
    )
    captured = {}

    def _fake_run(cmd, **kwargs):
        captured["cmd"] = cmd
        pathlib.Path(cmd[3]).write_text(phc_output)
        result = type("Result", (), {"returncode": 0})()
        return result

    monkeypatch.setattr("pygambit.nashphc.subprocess.run", _fake_run)
    profiles = phcpack_solve(matching_pennies, "./phc", maxregret=1e-6)
    assert len(profiles) == 1
    assert profiles[0]["1"] == {"1": 0.5, "2": 0.5}
    assert profiles[0]["2"] == {"1": 0.5, "2": 0.5}
    assert captured["cmd"][0] == "./phc"


def test_phcpack_solve_nonzero_returncode_raises_and_reports_singular(monkeypatch,
                                                                      matching_pennies):
    reported = []

    def _fake_run(cmd, **kwargs):
        result = type("Result", (), {"returncode": 1})()
        return result

    monkeypatch.setattr("pygambit.nashphc.subprocess.run", _fake_run)
    import pygambit.nash as nash

    for support in nash.possible_nash_supports(matching_pennies):
        from pygambit.nashphc import _solve_support
        profiles = _solve_support(
            support, "./phc", maxregret=1e-6, negtol=1e-6,
            onsupport=lambda x, label: reported.append(label),
        )
        assert profiles == []
    assert "singular" in reported
