import io

import pytest

import pygambit as gbt


def _parse_efg(text: str) -> gbt.Game:
    with io.StringIO(text) as f:
        return gbt.read_efg(f)


def _parse_nfg(text: str) -> gbt.Game:
    with io.StringIO(text) as f:
        return gbt.read_nfg(f)


def test_string_empty():
    with pytest.raises(ValueError) as excinfo:
        _parse_efg("")
    assert (
        "Parse error in game file: line 1:2: Expecting EFG file type indicator"
        in str(excinfo.value)
    )


def test_efg_no_newline_end():
    _parse_nfg(
        'NFG 1 R "prisoners dilemma"\n {"Player 1" "Player 2"} {2 2}\n'
        ' -6 -6 -10 0 0 -10 -1 -1.0'
    )


def test_string_wrong_magic():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG", "")
    with pytest.raises(ValueError) as excinfo:
        _parse_efg(file_text)
    assert (
        "Parse error in game file: line 1:3: Expecting EFG file type indicator"
        in str(excinfo.value)
    )


def test_efg_unsupported_version():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG 2", "EFG 1")
    with pytest.raises(ValueError) as excinfo:
        _parse_efg(file_text)
    assert "Parse error in game file: line 1:6: Accepting only EFG version 2" in str(excinfo.value)


def test_efg_unsupported_precision():
    with open("tests/test_games/e01.efg") as f:
        file_text = f.read().replace("EFG 2 R", "EFG 2 X")
    with pytest.raises(ValueError) as excinfo:
        _parse_efg(file_text)
    assert (
        "Parse error in game file: line 1:9: Accepting only EFG R or D data type"
        in str(excinfo.value)
    )


def test_efg_invalid_node_type():
    with open("tests/test_games/e02.efg") as f:
        file_text = f.read().replace('p "" 1 1', 'x "" 1 1')
    with pytest.raises(ValueError) as excinfo:
        _parse_efg(file_text)
    assert "Parse error in game file: line 4:3: Invalid type of node" in str(excinfo)


def test_efg_payoffs_too_many():
    with open("tests/test_games/e02.efg") as f:
        file_text = f.read().replace("1, 1", "1, 2, 3")
    with pytest.raises(ValueError) as excinfo:
        _parse_efg(file_text)
    assert "Parse error in game file: line 5:29: Expected '}'" in str(excinfo)


def test_nfg_title_missing():
    with open("tests/test_games/e02.nfg") as f:
        file_text = f.read().replace('"Selten (IJGT, 75), Figure 2, normal form"', "")
    with pytest.raises(ValueError) as excinfo:
        _parse_nfg(file_text)
    assert "Parse error in game file: line 1:11: Game title missing" in str(excinfo)


def test_nfg_player_missing():
    with open("tests/test_games/e02.nfg") as f:
        file_text = f.read().replace('"Player 2"', "")
    with pytest.raises(ValueError) as excinfo:
        _parse_nfg(file_text)
    assert "Parse error in game file: line 1:73: Expected '}'" in str(excinfo)


def test_nfg_payoffs_not_enough():
    data = """
NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" } { 3 2 }
1 1 0 2 0 2 1 1 0 3
"""
    with pytest.raises(ValueError, match="Expected numerical payoff"):
        _parse_nfg(data)


def test_nfg_payoffs_too_many():
    data = """
NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" } { 3 2 }
1 1 0 2 0 2 1 1 0 3 2 0 5 1
"""
    with pytest.raises(ValueError, match="end-of-file"):
        _parse_nfg(data)


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
        _parse_nfg(data)


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
        _parse_nfg(data)
