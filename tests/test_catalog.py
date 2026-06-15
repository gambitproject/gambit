import sys
from pathlib import Path
from unittest.mock import MagicMock

import pandas as pd
import pytest

import pygambit as gbt


@pytest.fixture(scope="module")
def all_games():
    return gbt.catalog.games()


@pytest.fixture
def game_slugs():
    """Fixture providing a set of all game slugs in the catalog."""
    game_slugs = set()
    for resource_path in gbt.catalog._CATALOG_RESOURCE.rglob("*"):
        if resource_path.is_file() and resource_path.suffix in gbt.catalog.READERS:
            rel_path = resource_path.relative_to(gbt.catalog._CATALOG_RESOURCE)
            slug = rel_path.with_suffix("").as_posix()
            game_slugs.add(slug)
    return game_slugs


def test_catalog_load_all_game_slugs(game_slugs):
    """Test loading all valid game files in the catalog."""
    errors = []
    for slug in game_slugs:
        try:
            g = gbt.catalog.load(slug)
            assert isinstance(g, gbt.Game), f"Expected gbt.Game, got {type(g)}"
        except Exception as e:
            errors.append(f"Slug '{slug}' failed with {type(e).__name__}: {e}")

    if errors:
        pytest.fail(f"Errors loading {len(errors)} game(s):\n" + "\n".join(errors))


def test_catalog_load_invalid_slug():
    """Test loading an invalid game slug."""
    with pytest.raises(FileNotFoundError):
        gbt.catalog.load("invalid_slug")


def test_catalog_games(game_slugs, all_games):
    """Test games() function returns df of game slugs and titles."""
    assert isinstance(all_games, pd.DataFrame)

    # Test that standard columns are present
    assert "Game" in all_games.columns
    assert "Title" in all_games.columns


def test_catalog_games_filter_n_actions(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'actions'"""
    filtered_games = gbt.catalog.games(n_actions=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.actions) == 2


def test_catalog_games_filter_n_contingencies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'contingencies'"""
    filtered_games = gbt.catalog.games(n_contingencies=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.contingencies) == 2


def test_catalog_games_filter_n_infosets(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'infosets'"""
    filtered_games = gbt.catalog.games(n_infosets=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.infosets) == 2


def test_catalog_games_filter_is_const_sum(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_const_sum'"""
    filtered_games = gbt.catalog.games(is_const_sum=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.is_const_sum


def test_catalog_games_filter_is_not_perfect_recall(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_perfect_recall'"""
    filtered_games = gbt.catalog.games(is_perfect_recall=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not g.is_perfect_recall


def test_catalog_games_filter_is_not_tree(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_tree'"""
    filtered_games = gbt.catalog.games(is_tree=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not g.is_tree


def test_catalog_games_filter_min_payoff_and_max_payoff(all_games):
    """Test games() function can filter on min and max payoff values"""
    filtered_games = gbt.catalog.games(min_payoff=0, max_payoff=10)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.min_payoff >= 0
        assert g.max_payoff <= 10


def test_catalog_games_filter_n_nodes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'nodes'"""
    filtered_games = gbt.catalog.games(n_nodes=5)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.nodes) == 5


def test_catalog_games_filter_n_outcomes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'outcomes'"""
    filtered_games = gbt.catalog.games(n_outcomes=3)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.outcomes) == 3


def test_catalog_games_filter_n_players(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'players'"""
    filtered_games = gbt.catalog.games(n_players=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.players) == 2


def test_catalog_games_filter_n_strategies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'strategies'"""
    filtered_games = gbt.catalog.games(n_strategies=4)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.strategies) == 4


def test_catalog_games_filter_bad_filter():
    """Test games() function raises error on invalid filter key"""
    with pytest.raises(TypeError):
        gbt.catalog.games(invalid_filter=123)


def test_catalog_games_include_descriptions():
    """Test games() function can include descriptions"""
    games_with_desc = gbt.catalog.games(include_descriptions=True)
    assert "Description" in games_with_desc.columns
    assert "Download" in games_with_desc.columns


# ---------------------------------------------------------------------------
# OpenSpiel dynamic loading tests (all mocked; open_spiel need not be installed)
# ---------------------------------------------------------------------------

_MOCK_NFG = gbt.Game.new_table([2, 2]).to_nfg()
_MOCK_EFG = gbt.catalog.load("journals/geb/bagwell1995").to_efg()


def _setup_pyspiel_mock(
    monkeypatch,
    *,
    nfg_str=None,
    nfg_raises=None,
    efg_str=None,
    efg_raises=None,
    load_raises=None,
    dynamics="sequential",
):
    """Inject a fake pyspiel + open_spiel.python.algorithms.gambit into sys.modules.

    ``dynamics`` controls which export path the code takes:
    - ``"sequential"`` → game.get_type().dynamics == pyspiel.GameType.Dynamics.SEQUENTIAL
      (OpenSpiel's term for extensive-form / tree games in Gambit)
    - ``"simultaneous"`` → SIMULTANEOUS (normal-form / strategic-form games in Gambit)
    - ``"other"`` → any other value, triggers the unsupported-dynamics ValueError
    """
    mock_ps = MagicMock()
    mock_export_fn = MagicMock()
    mock_game = MagicMock()

    # Wire the dynamics attribute so the == comparison in load_openspiel resolves correctly.
    # MagicMock attribute access is idempotent: mock_ps.GameType.Dynamics.SEQUENTIAL always
    # returns the same object, so the equality check passes.
    if dynamics == "sequential":
        mock_game.get_type.return_value.dynamics = mock_ps.GameType.Dynamics.SEQUENTIAL
    elif dynamics == "simultaneous":
        mock_game.get_type.return_value.dynamics = mock_ps.GameType.Dynamics.SIMULTANEOUS
    else:
        mock_game.get_type.return_value.dynamics = object()  # matches neither branch

    if load_raises is not None:
        mock_ps.load_game.side_effect = load_raises
    else:
        mock_ps.load_game.return_value = mock_game

    if nfg_raises is not None:
        mock_ps.game_to_nfg_string.side_effect = nfg_raises
    else:
        mock_ps.game_to_nfg_string.return_value = nfg_str

    if efg_raises is not None:
        mock_export_fn.side_effect = efg_raises
    else:
        mock_export_fn.return_value = efg_str

    mock_gambit_module = MagicMock()
    mock_gambit_module.export_gambit = mock_export_fn

    monkeypatch.setitem(sys.modules, "pyspiel", mock_ps)
    monkeypatch.setitem(sys.modules, "open_spiel", MagicMock())
    monkeypatch.setitem(sys.modules, "open_spiel.python", MagicMock())
    monkeypatch.setitem(sys.modules, "open_spiel.python.algorithms", MagicMock())
    monkeypatch.setitem(sys.modules, "open_spiel.python.algorithms.gambit", mock_gambit_module)
    return mock_ps, mock_export_fn


def test_openspiel_load_efg_success(monkeypatch):
    """Sequential (extensive-form) game: EFG export is used and returns a valid Game."""
    _setup_pyspiel_mock(monkeypatch, dynamics="sequential", efg_str=_MOCK_EFG)
    game = gbt.catalog.load_openspiel("tiny_hanabi")
    assert isinstance(game, gbt.Game)


def test_openspiel_load_nfg_success(monkeypatch):
    """Simultaneous (normal-form) game: NFG export is used and returns a valid Game."""
    _setup_pyspiel_mock(monkeypatch, dynamics="simultaneous", nfg_str=_MOCK_NFG)
    game = gbt.catalog.load_openspiel("matrix_rps")
    assert isinstance(game, gbt.Game)


def test_openspiel_load_import_error(monkeypatch):
    """Missing open_spiel raises ImportError with a helpful message."""
    monkeypatch.setitem(sys.modules, "pyspiel", None)
    with pytest.raises(ImportError, match="open_spiel"):
        gbt.catalog.load_openspiel("matrix_rps")


def test_openspiel_load_game_not_found(monkeypatch):
    """pyspiel.load_game errors propagate directly without wrapping."""
    _setup_pyspiel_mock(monkeypatch, load_raises=RuntimeError("Unknown game 'bogus_game'"))
    with pytest.raises(RuntimeError, match="Unknown game"):
        gbt.catalog.load_openspiel("bogus_game")


def test_openspiel_load_efg_export_failure(monkeypatch):
    """EFG export failure on a sequential game raises ValueError with format context."""
    _setup_pyspiel_mock(
        monkeypatch,
        dynamics="sequential",
        efg_raises=RuntimeError("export error"),
    )
    with pytest.raises(ValueError, match="EFG format"):
        gbt.catalog.load_openspiel("tiny_hanabi")


def test_openspiel_load_nfg_export_failure(monkeypatch):
    """NFG export failure on a simultaneous game raises ValueError with format context."""
    _setup_pyspiel_mock(
        monkeypatch,
        dynamics="simultaneous",
        nfg_raises=RuntimeError("export error"),
    )
    with pytest.raises(ValueError, match="NFG format"):
        gbt.catalog.load_openspiel("matrix_rps")


def test_openspiel_load_unsupported_dynamics(monkeypatch):
    """A game with unsupported dynamics (e.g. MEAN_FIELD) raises ValueError."""
    _setup_pyspiel_mock(monkeypatch, dynamics="other")
    with pytest.raises(ValueError, match="unsupported dynamics"):
        gbt.catalog.load_openspiel("some_mfg_game")


def test_openspiel_load_with_params(monkeypatch):
    """params dict is forwarded verbatim to pyspiel.load_game."""
    mock_ps, _ = _setup_pyspiel_mock(
        monkeypatch, dynamics="simultaneous", nfg_str=_MOCK_NFG
    )
    gbt.catalog.load_openspiel("blotto", params={"players": 2, "coins": 3, "fields": 2})
    mock_ps.load_game.assert_called_once_with("blotto", {"players": 2, "coins": 3, "fields": 2})


# ---------------------------------------------------------------------------
# GAMUT game generation tests (all mocked; Java and gamut.jar not required)
# ---------------------------------------------------------------------------


def _fake_gamut_run(nfg_content, returncode=0, stderr=""):
    """Return a fake subprocess.run callable that writes nfg_content to the -f path."""
    def _run(cmd, **kwargs):
        if returncode == 0:
            f_idx = cmd.index("-f")
            Path(cmd[f_idx + 1]).write_text(nfg_content)
        result = MagicMock()
        result.returncode = returncode
        result.stderr = stderr
        result.stdout = ""
        return result
    return _run


def test_generate_gamut_success(monkeypatch, tmp_path):
    """Happy path: GAMUT writes an NFG file and generate_gamut returns a Game."""
    fake_jar = tmp_path / "gamut.jar"
    fake_jar.touch()
    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: "/usr/bin/java")
    monkeypatch.setattr("pygambit.catalog.subprocess.run", _fake_gamut_run(_MOCK_NFG))
    game = gbt.catalog.generate_gamut("RandomGame", gamut_jar=fake_jar)
    assert isinstance(game, gbt.Game)


def test_generate_gamut_with_params(monkeypatch, tmp_path):
    """params dict is translated correctly to GAMUT command-line flags."""
    fake_jar = tmp_path / "gamut.jar"
    fake_jar.touch()
    captured = {}

    def _run(cmd, **kwargs):
        captured["cmd"] = cmd
        Path(cmd[cmd.index("-f") + 1]).write_text(_MOCK_NFG)
        result = MagicMock()
        result.returncode = 0
        return result

    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: "/usr/bin/java")
    monkeypatch.setattr("pygambit.catalog.subprocess.run", _run)
    gbt.catalog.generate_gamut(
        "RandomGame",
        params={"players": 2, "actions": [3, 3], "normalize": True, "min_payoff": 0},
        gamut_jar=fake_jar,
    )
    cmd = captured["cmd"]
    assert "-players" in cmd and cmd[cmd.index("-players") + 1] == "2"
    assert "-actions" in cmd
    actions_idx = cmd.index("-actions")
    assert cmd[actions_idx + 1] == "3" and cmd[actions_idx + 2] == "3"
    assert "-normalize" in cmd
    normalize_idx = cmd.index("-normalize")
    assert cmd[normalize_idx + 1] == "-min_payoff"  # no value token after -normalize
    assert "-min_payoff" in cmd and cmd[cmd.index("-min_payoff") + 1] == "0"


def test_generate_gamut_no_java(monkeypatch, tmp_path):
    """Missing java on PATH raises RuntimeError."""
    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: None)
    with pytest.raises(RuntimeError, match="Java is required"):
        gbt.catalog.generate_gamut("RandomGame", gamut_jar=tmp_path / "gamut.jar")


def test_generate_gamut_jar_not_found(monkeypatch, tmp_path):
    """Non-existent gamut_jar path raises FileNotFoundError."""
    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: "/usr/bin/java")
    with pytest.raises(FileNotFoundError, match="gamut.jar not found at"):
        gbt.catalog.generate_gamut("RandomGame", gamut_jar=tmp_path / "missing.jar")


def test_generate_gamut_no_jar_env(monkeypatch):
    """With gamut_jar=None and GAMUT_JAR unset, raises FileNotFoundError."""
    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: "/usr/bin/java")
    monkeypatch.delenv("GAMUT_JAR", raising=False)
    with pytest.raises(FileNotFoundError, match="GAMUT_JAR"):
        gbt.catalog.generate_gamut("RandomGame")


def test_generate_gamut_gamut_fails(monkeypatch, tmp_path):
    """Non-zero returncode from GAMUT raises ValueError naming the game class."""
    fake_jar = tmp_path / "gamut.jar"
    fake_jar.touch()
    monkeypatch.setattr("pygambit.catalog.shutil.which", lambda _: "/usr/bin/java")
    monkeypatch.setattr(
        "pygambit.catalog.subprocess.run",
        _fake_gamut_run("", returncode=1, stderr="Unknown game class 'Bogus'"),
    )
    with pytest.raises(ValueError, match="Bogus"):
        gbt.catalog.generate_gamut("Bogus", gamut_jar=fake_jar)


def test_gamut_games_returns_dataframe():
    assert isinstance(gbt.catalog.gamut_games(), pd.DataFrame)


def test_gamut_games_columns():
    assert list(gbt.catalog.gamut_games().columns) == ["Class", "Description", "Players"]


def test_gamut_games_count():
    assert len(gbt.catalog.gamut_games()) == 35


def test_gamut_games_known_classes():
    classes = set(gbt.catalog.gamut_games()["Class"])
    for name in [
        "RandomGame", "BattleOfTheSexes", "CovariantGame",
        "MajorityVoting", "PrisonersDilemma",
    ]:
        assert name in classes


def test_gamut_games_players_values():
    assert set(gbt.catalog.gamut_games()["Players"]) == {"2", "n"}
