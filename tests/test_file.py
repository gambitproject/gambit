import pytest

import pygambit as gbt


def test_string_empty():
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game("")
    assert "Parse error in game file: Empty file or string provided" in str(excinfo.value)


def test_efg_no_newline_end():
    gbt.Game.parse_game(
        'NFG 1 R "prisoners dilemma"\n {"Player 1" "Player 2"} {2 2}\n'
        ' -6 -6 -10 0 0 -10 -1 -1.0'
    )


def test_string_wrong_magic():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG", "")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 1:3: Expecting file type" in str(excinfo.value)


def test_efg_unsupported_version():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG 2", "EFG 1")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 1:6: Accepting only EFG version 2" in str(excinfo.value)


def test_efg_unsupported_precision():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG 2 R", "EFG 2 X")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert (
        "Parse error in game file: line 1:9: Accepting only EFG R or D data type"
        in str(excinfo.value)
    )


def test_efg_invalid_node_type():
    with open("tests/test_games/e02.efg") as f:
        file_text = f.read().replace('p "" 1 1', 'x "" 1 1')
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 4:3: Invalid type of node" in str(excinfo)


def test_efg_payoffs_too_many():
    with open("tests/test_games/e02.efg") as f:
        file_text = f.read().replace("1, 1", "1, 2, 3")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 5:29: Expected '}'" in str(excinfo)


def test_nfg_title_missing():
    with open("tests/test_games/e02.nfg") as f:
        file_text = f.read().replace('"Selten (IJGT, 75), Figure 2, normal form"', "")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 1:11: Game title missing" in str(excinfo)


def test_nfg_player_missing():
    with open("tests/test_games/e02.nfg") as f:
        file_text = f.read().replace('"Player 2"', "")
    with pytest.raises(ValueError) as excinfo:
        gbt.Game.parse_game(file_text)
    assert "Parse error in game file: line 1:73: Expected '}'" in str(excinfo)


def test_nfg_payoffs_not_enough():
    data = """
NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" } { 3 2 }
1 1 0 2 0 2 1 1 0 3
"""
    with pytest.raises(ValueError, match="Expected numerical payoff"):
        gbt.Game.parse_game(data)


def test_nfg_payoffs_too_many():
    data = """
NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" } { 3 2 }
1 1 0 2 0 2 1 1 0 3 2 0 5 1
"""
    with pytest.raises(ValueError, match="end-of-file"):
        gbt.Game.parse_game(data)


def test_nfg_outcomes_not_enough():
    data = """
NFG 1 R "Two person 2 x 2 game with unique mixed equilibrium" { "Player 1" "Player 2" }

{ { "1" "2" }
{ "1" "2" }
}
""

{
{ "" 2, 0 }
{ "" 0, 1 }
{ "" 0, 1 }
{ "" 1, 0 }
}
1 2 3
"""
    with pytest.raises(ValueError, match="Expected outcome index"):
        gbt.Game.parse_game(data)


def test_nfg_outcomes_too_many():
    data = """
NFG 1 R "Two person 2 x 2 game with unique mixed equilibrium" { "Player 1" "Player 2" }

{ { "1" "2" }
{ "1" "2" }
}
""

{
{ "" 2, 0 }
{ "" 0, 1 }
{ "" 0, 1 }
{ "" 1, 0 }
}
1 2 3 4 2
"""
    with pytest.raises(ValueError, match="end-of-file"):
        gbt.Game.parse_game(data)
