import hashlib
import json
import shutil
import sys
import tomllib
import zipfile
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
    """Fixture providing a set of all game slugs in the catalog's manifest."""
    return {entry["slug"] for entry in gbt.catalog._manifest()[1]["games"]}


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
    """Test games() function can filter on the total number of actions across
    the game's personal players' information sets"""
    filtered_games = gbt.catalog.games(n_actions=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        n_game_actions = sum(
            len(g.get_actions(gbt.H.path(*history)))
            for player in g.get_players()
            for history in g.get_infosets(player)
        )
        assert n_game_actions == 2


def test_catalog_games_filter_n_contingencies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'contingencies'"""
    filtered_games = gbt.catalog.games(n_contingencies=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.get_contingencies()) == 2


def test_catalog_games_filter_n_infosets(all_games):
    """Test games() function can filter on the number of information sets in the game"""
    filtered_games = gbt.catalog.games(n_infosets=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert sum(len(g.get_infosets(p)) for p in g.get_players()) == 2


def test_catalog_games_filter_is_const_sum(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_const_sum'"""
    filtered_games = gbt.catalog.games(is_const_sum=True)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.is_const_sum()


def test_catalog_games_filter_is_not_perfect_recall(all_games):
    """Test games() function can filter on boolean gbt.Game attribute 'is_perfect_recall'"""
    filtered_games = gbt.catalog.games(is_perfect_recall=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not g.is_perfect_recall()


def test_catalog_games_filter_is_not_tree(all_games):
    """Test games() function can filter on the 'is_tree' query parameter"""
    filtered_games = gbt.catalog.games(is_tree=False)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert not isinstance(g, gbt.ExtensiveGame)


def test_catalog_games_filter_min_payoff_and_max_payoff(all_games):
    """Test games() function can filter on min and max payoff values"""
    filtered_games = gbt.catalog.games(min_payoff=0, max_payoff=10)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert g.get_min_payoff() >= 0
        assert g.get_max_payoff() <= 10


def test_catalog_games_filter_n_nodes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'nodes'"""
    filtered_games = gbt.catalog.games(n_nodes=5)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.get_histories(gbt.H.after())) == 5


def test_catalog_games_filter_n_outcomes(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'outcomes'"""
    filtered_games = gbt.catalog.games(n_outcomes=3)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.get_outcomes()) == 3


def test_catalog_games_filter_n_players(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'players'"""
    filtered_games = gbt.catalog.games(n_players=2)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert len(g.get_players()) == 2


def test_catalog_games_filter_n_strategies(all_games):
    """Test games() function can filter on length of gbt.Game attribute 'strategies'"""
    filtered_games = gbt.catalog.games(n_strategies=4)
    assert isinstance(filtered_games, pd.DataFrame)
    assert len(filtered_games) < len(all_games)
    if len(filtered_games) > 0:
        g = gbt.catalog.load(filtered_games.Game.iloc[0])
        assert sum(len(g.get_strategies(p)) for p in g.get_players()) == 4


def test_catalog_games_filter_bad_filter():
    """Test games() function raises error on invalid filter key"""
    with pytest.raises(TypeError):
        gbt.catalog.games(invalid_filter=123)


def test_catalog_games_include_descriptions():
    """Test games() function can include descriptions"""
    games_with_desc = gbt.catalog.games(include_descriptions=True)
    assert "Description" in games_with_desc.columns
    assert "Download" in games_with_desc.columns


def test_catalog_version_is_pinned_release():
    """The catalog in use is the release pinned in build_support/catalog.toml."""
    pin = Path(__file__).resolve().parent.parent / "build_support" / "catalog.toml"
    with open(pin, "rb") as f:
        pinned = tomllib.load(f)["version"]
    assert gbt.catalog.version() == pinned


def test_catalog_dir_from_environment(tmp_path, monkeypatch):
    """GAMBIT_CATALOG_DIR names the catalog's games directory to use."""
    root = tmp_path / "games"
    shutil.copytree(gbt.catalog._catalog_root(), root)
    manifest = json.loads((root / "manifest.json").read_text())
    manifest["catalog_version"] = "9.9.9"
    (root / "manifest.json").write_text(json.dumps(manifest))
    monkeypatch.setenv("GAMBIT_CATALOG_DIR", str(root))
    assert gbt.catalog.version() == "9.9.9"


def test_catalog_not_fetched_names_fetch_command(tmp_path, monkeypatch):
    """A source checkout without the pinned release fetched says how to fetch it."""
    (tmp_path / "build_support").mkdir()
    (tmp_path / "build_support" / "catalog.toml").write_text('version = "9.9.9"\nsha256 = ""\n')
    monkeypatch.delenv("GAMBIT_CATALOG_DIR", raising=False)
    monkeypatch.setattr(gbt.catalog, "files", lambda package: tmp_path)
    monkeypatch.setattr(gbt.catalog, "_REPO_ROOT", tmp_path)
    monkeypatch.setattr(gbt.catalog, "_REPO_PIN", tmp_path / "build_support" / "catalog.toml")
    with pytest.raises(FileNotFoundError, match="catalog_release.py fetch"):
        gbt.catalog.load("journals/geb/bagwell1995")


def test_catalog_read_from_zip(tmp_path, monkeypatch):
    """Listing and loading work on a catalog that is not on the filesystem (a zipped install)."""
    root = Path(gbt.catalog._catalog_root())
    archive = tmp_path / "catalog.zip"
    with zipfile.ZipFile(archive, "w") as zf:
        for path in root.rglob("*"):
            if path.is_file():
                zf.write(path, "games/" + path.relative_to(root).as_posix())
    expected = gbt.catalog.games()
    monkeypatch.setattr(gbt.catalog, "_catalog_root", lambda: zipfile.Path(archive, "games/"))
    pd.testing.assert_frame_equal(gbt.catalog.games(), expected)
    assert gbt.catalog.load("journals/geb/bagwell1995").title == (
        "Bagwell (GEB 1995) commitment and (un)observability"
    )


# ---------------------------------------------------------------------------
# Choosing a catalog version (no network: refs are served from file:// fixtures)
# ---------------------------------------------------------------------------

_SLUG = "journals/geb/bagwell1995"
_SHA = "b" * 40


def _manifest_of(version, schema=0):
    """A manifest holding one game of the installed catalog, as a catalog *version*."""
    entry = next(e for e in gbt.catalog._manifest()[1]["games"] if e["slug"] == _SLUG)
    return {"schema_version": schema, "catalog_version": version, "games": [entry]}


def _write_catalog(root, manifest):
    """Write a catalog tree (``games/`` with its manifest and game) at *root*."""
    entry = manifest["games"][0]
    game = root / "games" / entry["game"]
    game.parent.mkdir(parents=True)
    source = gbt.catalog._catalog_root().joinpath(*entry["game"].split("/"))
    game.write_bytes(source.read_bytes())
    (root / "games" / "manifest.json").write_text(json.dumps(manifest))
    return root


def _zip_catalog(archive, top, manifest, tmp_path):
    tree = _write_catalog(tmp_path / "trees" / top, manifest)
    with zipfile.ZipFile(archive, "w") as zf:
        for path in tree.rglob("*"):
            if path.is_file():
                zf.write(path, f"{top}/{path.relative_to(tree).as_posix()}")


@pytest.fixture
def upstream(tmp_path, monkeypatch):
    """Serve release 9.9.9, commit _SHA and branch 'main' from file:// URLs."""
    root = tmp_path / "upstream"
    root.mkdir()
    release = root / "gambit-catalog-9.9.9.zip"
    _zip_catalog(release, "gambit-catalog-9.9.9", _manifest_of("9.9.9"), tmp_path)
    digest = hashlib.sha256(release.read_bytes()).hexdigest()
    (root / "gambit-catalog-9.9.9.zip.sha256").write_text(digest)
    _zip_catalog(root / f"{_SHA}.zip", "gambit-catalog-main", _manifest_of("9.9.9"), tmp_path)
    (root / "ref-main").write_text(_SHA)
    monkeypatch.setattr(
        gbt.catalog,
        "_ENDPOINTS",
        {
            "release_url": root.as_uri() + "/gambit-catalog-{version}.zip",
            "commit_url": root.as_uri() + "/{sha}.zip",
            "resolve_url": root.as_uri() + "/ref-{ref}",
        },
    )
    return root


def test_default_catalog_is_installed_release():
    catalog = gbt.catalog.Catalog()
    assert catalog.ref == gbt.catalog.version()
    assert repr(catalog) == f"Catalog({gbt.catalog.version()!r})"
    assert gbt.catalog.load(_SLUG).title == catalog.load(_SLUG).title


def test_installed_release_is_not_downloaded(monkeypatch):
    def no_fetch(*args, **kwargs):
        raise AssertionError("fetched")

    monkeypatch.setattr(gbt.catalog, "fetch", no_fetch)
    catalog = gbt.catalog.Catalog(gbt.catalog.version())
    assert catalog.ref == gbt.catalog.version()
    assert str(catalog.path) == str(gbt.catalog._catalog_root())


def test_catalog_of_release(upstream, tmp_path):
    catalog = gbt.catalog.Catalog("9.9.9", cache_dir=tmp_path / "cache")
    assert catalog.ref == "9.9.9"
    assert list(catalog.games()["Game"]) == [_SLUG]
    assert catalog.load(_SLUG).title == gbt.catalog.load(_SLUG).title


def test_catalog_of_branch_reports_full_sha(upstream, tmp_path):
    catalog = gbt.catalog.Catalog("main", cache_dir=tmp_path / "cache")
    assert catalog.ref == _SHA
    assert repr(catalog) == f"Catalog({_SHA!r})"
    again = gbt.catalog.Catalog(catalog.ref, cache_dir=tmp_path / "cache")
    assert again.path == catalog.path


@pytest.mark.parametrize("games_dir", [False, True])
def test_catalog_from_path(tmp_path, games_dir):
    root = _write_catalog(tmp_path / "checkout", _manifest_of("0.0.1"))
    catalog = gbt.catalog.Catalog(path=root / "games" if games_dir else root)
    assert catalog.ref is None
    assert catalog.path == root / "games"
    assert catalog.load(_SLUG).title == gbt.catalog.load(_SLUG).title


def test_catalog_from_path_without_catalog(tmp_path):
    with pytest.raises(gbt.catalog.CatalogError, match="holds no games catalog"):
        gbt.catalog.Catalog(path=tmp_path)


def test_catalog_ref_and_path_exclusive(tmp_path):
    with pytest.raises(ValueError):
        gbt.catalog.Catalog("0.2.0", path=tmp_path)


def test_catalog_dir_from_environment_has_no_ref(tmp_path, monkeypatch):
    root = _write_catalog(tmp_path / "checkout", _manifest_of("0.0.1"))
    monkeypatch.setenv("GAMBIT_CATALOG_DIR", str(root / "games"))
    assert gbt.catalog.Catalog().ref is None


def test_catalog_of_unsupported_schema(tmp_path):
    root = _write_catalog(tmp_path / "checkout", _manifest_of("0.0.1", schema=99))
    with pytest.raises(gbt.catalog.CatalogError, match="schema version 99.*upgrade pygambit"):
        gbt.catalog.Catalog(path=root)


@pytest.mark.parametrize(
    "change,message",
    [
        (lambda m: m.update(schema_version=False), "no integer 'schema_version'"),
        (lambda m: m.pop("catalog_version"), "no 'catalog_version'"),
        (lambda m: m.update(catalog_version="main"), "no 'catalog_version'"),
        (lambda m: m.update(games={}), "no list of 'games'"),
        (lambda m: m["games"][0].update(format="txt"), "invalid game entry"),
        (lambda m: m["games"][0].update(format="nfg"), "invalid game entry"),
        (lambda m: m["games"][0].update(game="../x.efg"), "invalid game entry"),
        (lambda m: m["games"][0].pop("slug"), "invalid game entry"),
    ],
)
def test_catalog_of_malformed_manifest(tmp_path, change, message):
    manifest = _manifest_of("0.0.1")
    change(manifest)
    root = _write_catalog(tmp_path / "checkout", _manifest_of("0.0.1"))
    (root / "games" / "manifest.json").write_text(json.dumps(manifest))
    with pytest.raises(gbt.catalog.CatalogError, match=message):
        gbt.catalog.Catalog(path=root)


def test_catalog_of_invalid_json(tmp_path):
    root = _write_catalog(tmp_path / "checkout", _manifest_of("0.0.1"))
    (root / "games" / "manifest.json").write_text("{")
    with pytest.raises(gbt.catalog.CatalogError, match="cannot be read"):
        gbt.catalog.Catalog(path=root)


# ---------------------------------------------------------------------------
# OpenSpiel dynamic loading tests (all mocked; open_spiel need not be installed)
# ---------------------------------------------------------------------------

_MOCK_NFG = gbt.StrategicGame([2, 2]).to_nfg()
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

    # Wire the dynamics attribute so the == comparison in generate_openspiel resolves correctly.
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


def test_generate_openspiel_efg_success(monkeypatch):
    """Sequential (extensive-form) game: EFG export is used and returns a valid Game."""
    _setup_pyspiel_mock(monkeypatch, dynamics="sequential", efg_str=_MOCK_EFG)
    game = gbt.catalog.generate_openspiel("tiny_hanabi")
    assert isinstance(game, gbt.Game)


def test_generate_openspiel_nfg_success(monkeypatch):
    """Simultaneous (normal-form) game: NFG export is used and returns a valid Game."""
    _setup_pyspiel_mock(monkeypatch, dynamics="simultaneous", nfg_str=_MOCK_NFG)
    game = gbt.catalog.generate_openspiel("matrix_rps")
    assert isinstance(game, gbt.Game)


def test_generate_openspiel_import_error(monkeypatch):
    """Missing open_spiel raises ImportError with a helpful message."""
    monkeypatch.setitem(sys.modules, "pyspiel", None)
    with pytest.raises(ImportError, match="open_spiel"):
        gbt.catalog.generate_openspiel("matrix_rps")


def test_generate_openspiel_game_not_found(monkeypatch):
    """pyspiel.load_game errors propagate directly without wrapping."""
    _setup_pyspiel_mock(monkeypatch, load_raises=RuntimeError("Unknown game 'bogus_game'"))
    with pytest.raises(RuntimeError, match="Unknown game"):
        gbt.catalog.generate_openspiel("bogus_game")


def test_generate_openspiel_efg_export_failure(monkeypatch):
    """EFG export failure on a sequential game raises ValueError with format context."""
    _setup_pyspiel_mock(
        monkeypatch,
        dynamics="sequential",
        efg_raises=RuntimeError("export error"),
    )
    with pytest.raises(ValueError, match="EFG format"):
        gbt.catalog.generate_openspiel("tiny_hanabi")


def test_generate_openspiel_nfg_export_failure(monkeypatch):
    """NFG export failure on a simultaneous game raises ValueError with format context."""
    _setup_pyspiel_mock(
        monkeypatch,
        dynamics="simultaneous",
        nfg_raises=RuntimeError("export error"),
    )
    with pytest.raises(ValueError, match="NFG format"):
        gbt.catalog.generate_openspiel("matrix_rps")


def test_generate_openspiel_unsupported_dynamics(monkeypatch):
    """A game with unsupported dynamics (e.g. MEAN_FIELD) raises ValueError."""
    _setup_pyspiel_mock(monkeypatch, dynamics="other")
    with pytest.raises(ValueError, match="unsupported dynamics"):
        gbt.catalog.generate_openspiel("some_mfg_game")


def test_generate_openspiel_with_params(monkeypatch):
    """params dict is forwarded verbatim to pyspiel.load_game."""
    mock_ps, _ = _setup_pyspiel_mock(
        monkeypatch, dynamics="simultaneous", nfg_str=_MOCK_NFG
    )
    gbt.catalog.generate_openspiel("blotto", params={"players": 2, "coins": 3, "fields": 2})
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
