import io
import os.path
from glob import glob

import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.efg")))
def test_read_efg(game_path):
    game = gbt.read_efg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_efg_invalid():
    game_path = os.path.join(
        "tests", "test_games", "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
    )
    with pytest.raises(ValueError):
        gbt.read_efg(game_path)


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.nfg")))
def test_read_nfg(game_path):
    game = gbt.read_nfg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_nfg_invalid():
    game_path = os.path.join("tests", "test_games", "cent3.efg")
    with pytest.raises(ValueError):
        gbt.read_nfg(game_path)


@pytest.mark.parametrize("game_path", glob(os.path.join("tests", "test_games", "*.agg")))
def test_read_agg(game_path):
    game = gbt.read_agg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_agg_invalid():
    game_path = os.path.join(
        "tests", "test_games", "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
    )
    with pytest.raises(ValueError):
        gbt.read_agg(game_path)


def test_read_agg_zero_header():
    with pytest.raises(ValueError):
        gbt.read_agg(io.StringIO("0 0 0\n"))


def test_read_bagg():
    game_path = os.path.join("contrib", "games", "Bayesian-Coffee-3-2-2-3.bagg")
    game = gbt.read_bagg(game_path)
    assert isinstance(game, gbt.Game)


def test_read_bagg_invalid():
    game_path = os.path.join(
        "tests", "test_games", "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
    )
    with pytest.raises(ValueError):
        gbt.read_bagg(game_path)


def test_read_bagg_zero_header():
    with pytest.raises(ValueError):
        gbt.read_bagg(io.StringIO("0 0 0\n"))


def _two_type_bagg(p1_dist, p2_dist):
    """2x2_fraction_types.bagg (player 1 has two types, player 2 one) with the given type
    distributions."""
    return (
        f"#BAGG\n2\n2\n0\n2 1\n{p1_dist}\n{p2_dist}\n2 2 2\n0 1\n0 1\n0 1\n"
        "2 0 1\n2 1 0\n\n0\t35 -4\n0\t-10 95\n"
    )


@pytest.mark.parametrize(
    "p1_dist,p2_dist",
    [
        pytest.param("1/3 2/3", "1", id="fractions"),
        pytest.param("0.25 0.75", "1", id="decimals"),
        pytest.param("0.25 3/4", "1.0", id="mixed_notation"),
    ],
)
def test_read_bagg_valid_type_distribution(p1_dist, p2_dist):
    game = gbt.read_bagg(io.StringIO(_two_type_bagg(p1_dist, p2_dist)))
    assert isinstance(game, gbt.Game)


@pytest.mark.parametrize(
    "p1_dist,p2_dist",
    [
        pytest.param("0.3333 0.6666", "1", id="sum_not_exactly_one"),
        pytest.param("1/3 1/3", "1", id="sum_below_one"),
        pytest.param("1/2 2/3", "1", id="sum_above_one"),
        pytest.param("0 1", "1", id="zero_probability_type"),
        pytest.param("-1/2 3/2", "1", id="negative_probability"),
        pytest.param("1/3 2/3", "1/2", id="second_player_sum"),
    ],
)
def test_read_bagg_invalid_type_distribution(p1_dist, p2_dist):
    with pytest.raises(ValueError, match="type probabilities"):
        gbt.read_bagg(io.StringIO(_two_type_bagg(p1_dist, p2_dist)))


def test_read_bagg_player_without_types():
    with pytest.raises(ValueError, match="number of types"):
        gbt.read_bagg(io.StringIO("#BAGG\n2\n2\n0\n0 1\n1\n2\n0 1\n2 0 1\n2 1 0\n"))


def test_read_gbt_invalid():
    game_path = os.path.join(
        "tests", "test_games", "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
    )
    with pytest.raises(ValueError):
        gbt.read_gbt(game_path)


def test_read_gbt_workspace(tmp_path):
    game_path = tmp_path / "game.gbt"
    game_path.write_text(
        """<?xml version="1.0" encoding="UTF-8"?>
<gambit:document xmlns:gambit="http://gambit.sourceforge.net/" version="0.1">
  <numbers decimals="4"/>
  <game>
    <nfgfile>
NFG 1 R "Prisoner's <Dilemma> & test" { "Alice" "Bob" }
{ { "Cooperate" "Defect" } { "Cooperate" "Defect" } }
""
3 3 0 5 5 0 1 1
    </nfgfile>
  </game>
</gambit:document>
"""
    )

    game = gbt.read_gbt(game_path)

    assert game.title == "Prisoner's <Dilemma> & test"
    assert game.get_players() == ["Alice", "Bob"]


def test_read_gbt_rejects_malformed_xml(tmp_path):
    game_path = tmp_path / "malformed.gbt"
    game_path.write_text("<gambit:document><game><nfgfile>NFG 1 R</game></gambit:document>")

    with pytest.raises(ValueError):
        gbt.read_gbt(game_path)


def test_write_efg():
    game = gbt.ExtensiveGame()
    serialized_game = game.to_efg()
    assert serialized_game[:3] == "EFG"


def test_write_nfg():
    game = gbt.StrategicGame([2, 2])
    serialized_game = game.to_nfg()
    assert serialized_game[:3] == "NFG"


def test_write_efg_as_nfg():
    result = """
NFG 1 R "Centipede game. Three inning with probability of altruism.  " { "Player 1" "Player 2" }

{ { "1" "2" "3" "4" }
{ "1" "2" "3" "4" }
}
""

20027/25000 2689/12500
541/1250 3983/2500
3299/6250 5053/3125
227/250 214/125
5081/6250 3283/12500
19931/6250 2677/3125
5362/3125 19903/3125
262/125 808/125
2689/3125 5659/12500
10114/3125 3271/3125
39814/3125 10696/3125
856/125 3184/125
163/125 163/500
92/25 23/25
1648/125 412/125
256/5 64/5
"""
    game = gbt.read_efg("tests/test_games/cent3.efg")
    assert game.to_nfg().strip() == result.strip()


def test_write_html():
    game = gbt.StrategicGame([2, 2])
    alice, bob = game.get_players()
    game.relabel_players({alice: "Alice", bob: "Bob"})
    serialized_game = game.to_html()
    assert isinstance(serialized_game, str)
    assert "Alice" in serialized_game
    assert "Bob" in serialized_game


def test_write_latex():
    game = gbt.StrategicGame([2, 2], title="Game title")
    alice, bob = game.get_players()
    game.relabel_players({alice: "Alice", bob: "Bob"})
    serialized_game = game.to_latex()
    assert "\\begin{game}" in serialized_game
    assert "[\\textbf{Alice}][\\textbf{Bob}]" in serialized_game
    assert "\\textbf{Game title}" in serialized_game


def test_read_write_efg():
    efg_game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    serialized_efg_game = efg_game.to_efg()
    deserialized_efg_game = gbt.read_efg(io.BytesIO(serialized_efg_game.encode()))
    double_serialized_efg_game = deserialized_efg_game.to_efg()
    assert serialized_efg_game == double_serialized_efg_game


def test_read_write_nfg():
    nfg_game = games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg")
    serialized_nfg_game = nfg_game.to_nfg()
    deserialized_nfg_game = gbt.read_nfg(io.BytesIO(serialized_nfg_game.encode()))
    double_serialized_nfg_game = deserialized_nfg_game.to_nfg()
    assert serialized_nfg_game == double_serialized_nfg_game


def _poker():
    return gbt.catalog.load("books/myerson1991/fig2_1")


def test_to_gtdraw_returns_text():
    drawing = _poker().to_gtdraw()
    lines = drawing.splitlines()
    assert lines[:2] == ["player 1 name Fred", "player 2 name Alice"]
    assert lines[2].startswith("level 0 node 1 player 0")
    assert "from 1 move Red~(\\frac{1}{2})" in lines[3]
    assert "payoffs 2 -2" in drawing
    assert lines[-1] == "iset 3 8 player 2"


def test_to_gtdraw_writes_path(tmp_path):
    path = tmp_path / "poker.ef"
    assert _poker().to_gtdraw(path) is None
    assert path.read_text() == _poker().to_gtdraw()


def test_to_gtdraw_writes_text_buffer():
    buffer = io.StringIO()
    assert _poker().to_gtdraw(buffer) is None
    assert buffer.getvalue() == _poker().to_gtdraw()


def test_to_gtdraw_writes_bytes_buffer():
    buffer = io.BytesIO()
    assert _poker().to_gtdraw(buffer) is None
    assert buffer.getvalue().decode() == _poker().to_gtdraw()


def test_to_gtdraw_options():
    game = _poker()
    assert game.to_gtdraw(width_scaling=2.0) != game.to_gtdraw()
    assert game.to_gtdraw(level_scaling=2.0) != game.to_gtdraw()
    shared = game.to_gtdraw(sublevel_scaling=0, shared_terminal_depth=True)
    terminal_levels = {line.split()[1] for line in shared.splitlines() if " payoffs " in line}
    assert len(terminal_levels) == 1


def test_to_gtdraw_chance_probability_text():
    game = gbt.ExtensiveGame(["Alice"])
    game.append_event(gbt.H.after(), {"heads": gbt.Rational(1, 4), "tails": gbt.Rational(3, 4)})
    assert "move heads~(\\frac{1}{4})" in game.to_gtdraw()
    game = gbt.ExtensiveGame(["Alice"])
    game.append_event(gbt.H.after(), {"heads": "0.25", "tails": "0.75"})
    assert "move heads~0.25" in game.to_gtdraw()


def test_to_gtdraw_spaces_in_labels():
    game = gbt.ExtensiveGame(["Big Bank"])
    game.append_move(gbt.H.after(), "Big Bank", ["go left", "go right"])
    drawing = game.to_gtdraw()
    assert "player 1 name Big~Bank" in drawing
    assert "move go~left" in drawing


@pytest.mark.parametrize("option", ["level_scaling", "sublevel_scaling", "width_scaling"])
def test_to_gtdraw_negative_scaling(option):
    with pytest.raises(ValueError):
        _poker().to_gtdraw(**{option: -1.0})


def test_to_gtdraw_too_many_players():
    game = gbt.ExtensiveGame(["1", "2", "3", "4", "5"])
    with pytest.raises(ValueError, match="at most 4 players"):
        game.to_gtdraw()


def test_to_gtdraw_strategic_game():
    assert not hasattr(gbt.StrategicGame([2, 2]), "to_gtdraw")


def _read_gtdraw(text):
    return gbt.read_gtdraw(io.StringIO(text))


_ROUND_TRIP_OPTIONS = [
    pytest.param({}, id="default"),
    # The layout is not part of the game, so it must not matter
    pytest.param(
        {
            "level_scaling": 2.0,
            "sublevel_scaling": 0,
            "width_scaling": 1.5,
            "shared_terminal_depth": True,
        },
        id="scaled",
    ),
]


@pytest.mark.parametrize("options", _ROUND_TRIP_OPTIONS)
@pytest.mark.parametrize("slug", gbt.catalog.games(is_tree=True)["Game"])
def test_read_write_gtdraw(slug, options):
    drawing = gbt.catalog.load(slug).to_gtdraw(**options)
    assert _read_gtdraw(drawing).to_gtdraw(**options) == drawing


def test_read_write_gtdraw_detects_a_change():
    drawing = _poker().to_gtdraw()
    assert "payoffs 2 -2" in drawing
    changed = drawing.replace("payoffs 2 -2", "payoffs 3 -2", 1)
    rewritten = _read_gtdraw(changed).to_gtdraw()
    assert rewritten != drawing
    assert rewritten == changed


_CURATED_DRAWINGS = sorted(glob(os.path.join("catalog", "games", "**", "*.ef"), recursive=True))


def test_curated_drawings_are_found():
    assert len(_CURATED_DRAWINGS) >= 12


@pytest.mark.parametrize("path", _CURATED_DRAWINGS)
def test_read_gtdraw_curated_drawing(path):
    game = gbt.read_gtdraw(path)
    assert isinstance(game, gbt.ExtensiveGame)
    assert game.to_gtdraw().count("\nlevel ") > 1


def test_read_gtdraw_sources(tmp_path):
    drawing = _poker().to_gtdraw()
    path = tmp_path / "poker.ef"
    path.write_text(drawing)
    for source in [path, str(path), io.StringIO(drawing), io.BytesIO(drawing.encode())]:
        assert gbt.read_gtdraw(source).to_gtdraw() == drawing


def test_read_gtdraw_returns_the_game():
    game = _read_gtdraw(_poker().to_gtdraw())
    expected = _poker()
    drawing = expected.to_gtdraw()
    assert game.get_players() == ["Fred", "Alice"]
    for player in ["Fred", "Alice"]:
        assert len(game.get_infosets(player)) == len(expected.get_infosets(player))
    # A drawing gives every terminal node its own, numbered outcome
    assert game.get_outcomes() == [str(i) for i in range(1, drawing.count(" payoffs ") + 1)]
    assert len(game.get_events()) == len(expected.get_events())


_HAND_DRAWN = r"""% a hand-written drawing in EF 1
player 1 name Big~Bank
level 0 node root player 0
level 2 node l xshift -a=2 yshift 0.5 from root move:l Heads~($\frac{1}{4}$)
level 2 node r xshift 2a from root move::0.3 3/4
level 4 node ll xshift -1 from l move:r: go~left payoffs 1 -1/2
level 4 node lr xshift 1 from l move go~right payoffs 0.5 $-2$
level 4 node rl xshift -1 from r move go~left payoffs
level 4 node rr xshift 1 from r move go~right payoffs 2 -.5
iset l r player 1
"""


def test_read_gtdraw_hand_drawn():
    game = _read_gtdraw(_HAND_DRAWN)
    assert game.get_players() == ["Big Bank", "2"]
    drawing = game.to_gtdraw()
    assert "move Heads~(\\frac{1}{4})" in drawing
    assert "move _1~(\\frac{3}{4})" in drawing
    assert "move go~left payoffs 1 -1/2" in drawing
    assert "move go~right payoffs 0.5 -2" in drawing
    assert "move go~left payoffs\n" in drawing
    assert "move go~right payoffs 2 -.5" in drawing
    assert drawing.splitlines()[-1] == "iset 2 5 player 1"


def test_read_gtdraw_node_outcomes():
    game = _read_gtdraw(_HAND_DRAWN)
    assert len(game.get_outcomes()) == 3


@pytest.mark.parametrize(
    "label, text",
    [
        (r"H~(\frac{1}{4})", "H~(\\frac{1}{4})"),
        ("H~1/4", "H~(\\frac{1}{4})"),
        ("H~0.25", "H~0.25"),
        (r"H~$\frac{1}{4}$", "H~(\\frac{1}{4})"),
        (r"($\frac{1}{4}$)", "_1~(\\frac{1}{4})"),
        (r"\frac{1}{4}", "_1~(\\frac{1}{4})"),
        ("big~win~(1/4)", "big~win~(\\frac{1}{4})"),
    ],
)
def test_read_gtdraw_chance_label(label, text):
    drawing = (
        "level 0 node 1 player 0\n"
        f"level 2 node 2 from 1 move {label}\n"
        "level 2 node 3 from 1 move T~(3/4)\n"
    )
    assert f"move {text} payoffs" in _read_gtdraw(drawing).to_gtdraw()


def test_read_gtdraw_ef0_and_ef1_agree():
    ef1 = (
        "player 1 name I\nplayer 2 name II\n"
        "level 0 node 1 player 1\n"
        "level 2 node 2 from 1 move L\n"
        "level 2 node 3 from 1 move R\n"
        "level 4 node 4 from 2 move l payoffs 1 0\n"
        "level 4 node 5 from 2 move r payoffs 0 1\n"
        "level 4 node 6 from 3 move l payoffs 1 0\n"
        "level 4 node 7 from 3 move r payoffs 0 1\n"
        "iset 2 3 player 2\n"
    )
    ef0 = (
        "player 1 name I\nplayer 2 name II\n"
        "level 0 node 1 player 1\n"
        "level 2 node 1 from 0,1 move L\n"
        "level 2 node 2 from 0,1 move R\n"
        "level 4 node 1 from 2,1 move l payoffs 1 0\n"
        "level 4 node 2 from 2,1 move r payoffs 0 1\n"
        "level 4 node 3 from 2,2 move l payoffs 1 0\n"
        "level 4 node 4 from 2,2 move r payoffs 0 1\n"
        "iset 2,1 2,2 player 2\n"
    )
    assert "iset" in _read_gtdraw(ef1).to_gtdraw()
    assert _read_gtdraw(ef0).to_gtdraw() == _read_gtdraw(ef1).to_gtdraw()


def test_read_gtdraw_ef0_levels_compare_as_numbers():
    drawing = (
        "level 0 node 1 player 1\n"
        "level 2.0 node 1 from 0,1 move L payoffs 1\n"
        "level 2 node 2 from 0.0,1 move R payoffs 2\n"
    )
    assert _read_gtdraw(drawing).to_gtdraw().count("payoffs") == 2


def test_read_gtdraw_children_in_the_order_of_the_lines():
    drawing = (
        "level 0 node 1 player 1\n"
        "level 2 node 3 from 1 move B payoffs 2\n"
        "level 2 node 2 from 1 move A payoffs 1\n"
    )
    lines = _read_gtdraw(drawing).to_gtdraw().splitlines()
    assert lines[-2].endswith("move B payoffs 2")
    assert lines[-1].endswith("move A payoffs 1")


def test_read_gtdraw_padded_payoffs():
    drawing = (
        "player 1 name I\nplayer 2 name II\n"
        "level 0 node 1 player 1\n"
        "level 2 node 2 from 1 move L payoffs 7\n"
        "level 2 node 3 from 1 move R payoffs 1 2\n"
    )
    assert "move L payoffs 7 0" in _read_gtdraw(drawing).to_gtdraw()


def test_read_gtdraw_iset_player_from_nodes():
    drawing = (
        "level 0 node 1 player 1\n"
        "level 2 node 2 player 2 from 1 move L\n"
        "level 2 node 3 player 2 from 1 move R\n"
        "level 4 node 4 from 2 move a payoffs 1 0\n"
        "level 4 node 5 from 2 move b payoffs 0 1\n"
        "level 4 node 6 from 3 move a payoffs 1 0\n"
        "level 4 node 7 from 3 move b payoffs 0 1\n"
        "iset 2 3\n"
    )
    assert _read_gtdraw(drawing).to_gtdraw().splitlines()[-1] == "iset 2 5 player 2"


def test_read_gtdraw_iset_takes_the_first_label_there_is():
    drawing = (
        "level 0 node 1 player 1\n"
        "level 2 node 2 from 1 move L\n"
        "level 2 node 3 from 1 move R\n"
        "level 4 node 4 from 2 move a payoffs 1 0\n"
        "level 4 node 5 from 2 payoffs 0 1\n"
        "level 4 node 6 from 3 move x payoffs 1 0\n"
        "level 4 node 7 from 3 move b payoffs 0 1\n"
        "iset 2 3 player 2\n"
    )
    assert _read_gtdraw(drawing).to_gtdraw().count("move a ") == 2


_MALFORMED_DRAWINGS = [
    pytest.param(
        "% comment\nplayer 1 name I\nlevel 0 node 1 player 1 colour red\n",
        3,
        "colour",
        id="unknown-field",
    ),
    pytest.param("% c\nplayer 1 name I\nfrobnicate 1\n", 3, "frobnicate", id="unknown-keyword"),
    pytest.param(
        "level 0 node 1 player 1\n\nlevel 2 node 2 from 9 move L payoffs 1\n",
        3,
        "'9'",
        id="missing-parent",
    ),
    pytest.param(
        "level 0 node 1 player 0\nlevel 2 node 2 from 1 move H~(1/2) payoffs 1\n"
        "level 2 node 3 from 1 move T payoffs 2\n",
        3,
        "no probability",
        id="chance-without-probability",
    ),
    pytest.param(
        "level 0 node 1 player 0\nlevel 2 node 2 from 1 payoffs 1\n"
        "level 2 node 3 from 1 move 1 payoffs 2\n",
        2,
        "no probability",
        id="chance-without-move",
    ),
    pytest.param(
        "level 0 node 1 player 1\n\n\nlevel 2 node 2 from 1 move L payoffs \\sqrt{2}\n",
        4,
        "sqrt",
        id="payoff-sqrt",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1 2 3 4 5\n",
        2,
        "5 payoffs",
        id="five-payoffs",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1/0\n",
        2,
        "1/0",
        id="zero-denominator",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1e3\n",
        2,
        "1e3",
        id="exponent",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs $1\n",
        2,
        r"\$1",
        id="unbalanced-dollar",
    ),
    pytest.param(
        "level 0 node 1\nlevel 2 node 2 from 1 move L payoffs 1\n",
        1,
        "no player",
        id="decision-node-without-player",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 1 from 1 move L payoffs 1\n",
        2,
        "defined twice",
        id="duplicate-node",
    ),
    pytest.param("level 0 node 1 player 1\nlevel 0 node 2 player 1\n", 2, "root", id="two-roots"),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 3 move L\nlevel 2 node 3 from 2 move R\n",
        2,
        "root",
        id="cycle",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1\niset 1 9\n",
        3,
        "'9'",
        id="iset-missing-node",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1\n"
        "level 2 node 3 from 1 move R payoffs 1\niset 2 3 player 1\n",
        4,
        "no moves",
        id="iset-of-terminal-nodes",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 player 2 from 1 move L\n"
        "level 2 node 3 from 1 move R\nlevel 4 node 4 from 2 move a payoffs 1\n"
        "level 4 node 5 from 2 move b payoffs 1\nlevel 4 node 6 from 3 move a payoffs 1\n"
        "level 4 node 7 from 3 move b payoffs 1\niset 2 3 player 1\n",
        2,
        "player 1",
        id="player-differs-from-iset",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1\n"
        "level 2 node 3 from 1 move R payoffs 1\niset 1\niset 1\n",
        5,
        "more than one iset",
        id="node-in-two-isets",
    ),
    pytest.param(
        "level 0 node 1 player 1 payoffs 1\nlevel 2 node 2 from 1 move L payoffs 1\n",
        1,
        "moves and payoffs",
        id="moves-and-payoffs",
    ),
    pytest.param(
        "level 0 node 1 player 1 xshift b\nlevel 2 node 2 from 1 move L\n",
        1,
        "undefined",
        id="undefined-shift",
    ),
    pytest.param(
        "level 0 node 1 player 1 xshift\nlevel 2 node 2 from 1 move L\n",
        1,
        "needs",
        id="shift-without-value",
    ),
    pytest.param("level 0 node 1 player 5\n", 1, "player", id="player-five"),
    pytest.param("level x node 1 player 1\n", 1, "level", id="level-not-a-number"),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move:q L payoffs 1\n",
        2,
        "move:q",
        id="bad-move-modifier",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 0,1 move L payoffs 1\n"
        "level 2 node 3 from 1 move R payoffs 2\n",
        3,
        "level,node",
        id="ef0-reference-without-level",
    ),
    pytest.param("player 1 name\nlevel 0 node 1 player 1\n", 1, "name", id="player-without-name"),
    pytest.param(
        "level 0 node 1 player 0\nlevel 2 node 2 from 1 move H~1/2 payoffs 1\n"
        "level 2 node 3 from 1 move T~1/3 payoffs 2\n",
        1,
        "sum",
        id="chance-probabilities-do-not-sum-to-one",
    ),
    pytest.param(
        "level 0 node 1 player 1\nlevel 2 node 2 from 1 move L payoffs 1\n"
        "level 2 node 3 from 1 move ~L payoffs 1\n",
        3,
        "label",
        id="invalid-label",
    ),
]


@pytest.mark.parametrize("text, line, fragment", _MALFORMED_DRAWINGS)
def test_read_gtdraw_malformed(text, line, fragment):
    with pytest.raises(ValueError, match=rf"line {line}\b.*{fragment}"):
        _read_gtdraw(text)


def test_read_gtdraw_empty():
    for text in ["", "% only a comment\n\n"]:
        with pytest.raises(ValueError, match="no nodes"):
            _read_gtdraw(text)


def test_read_gtdraw_missing_file(tmp_path):
    with pytest.raises(OSError):
        gbt.read_gtdraw(tmp_path / "missing.ef")
