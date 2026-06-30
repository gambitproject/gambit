"""Tests for build_support/catalog/update.py.

All catalog slugs used here are clearly fictional (e.g. ``"testgroup2000/fig1"``)
and do not correspond to any game in the real catalog.  This is intentional: the
tests construct their own temporary catalog directories and DataFrame rows so they
are completely isolated from the actual catalog on disk.

Monkeypatching strategy
-----------------------
``update.py`` depends on four external resources that are replaced in tests:

1. ``GTDRAW_SETTINGS_CONFIG`` (a ``Path``) — swapped for a tmp YAML file so
   ``catalog_gtdraw_settings`` reads controlled config without touching the
   real ``gtdraw_settings.yaml``.  ``monkeypatch.setattr(update,
   "GTDRAW_SETTINGS_CONFIG", yaml_file)`` replaces the module-level path for
   the duration of a single test and restores it automatically on teardown.

2. ``CATALOG_HIERARCHY_CONFIG`` (a ``Path``) — swapped for a tmp YAML file so
   ``load_hierarchy_labels`` reads controlled labels without touching the real
   ``catalog_hierarchy.yaml``.  Swap via ``monkeypatch.setattr(update,
   "CATALOG_HIERARCHY_CONFIG", yaml_file)``.

3. ``tex`` / ``png`` / ``pdf`` / ``svg``
   (functions imported from ``gtdraw``) — replaced with no-ops or
   call-tracking lambdas.  This lets us test RST-generation logic without
   actually invoking LaTeX, and lets us assert whether image
   generation was triggered at all.

4. ``catalog_dir`` (an argument to ``generate_rst_table`` and
   ``update_makefile``) — both functions accept an optional ``catalog_dir``
   kwarg that defaults to the real ``CATALOG_DIR``.  Tests pass a
   ``tmp_path``-based directory instead, keeping all file I/O inside pytest's
   temporary directory and avoiding any reads from or writes to the repo.
"""

import textwrap

import pytest

pytest.importorskip("gtdraw")  # update.py imports gtdraw at module level
pytest.importorskip("yaml")

import pandas as pd  # noqa: E402
import update  # noqa: E402

# ---------------------------------------------------------------------------
# Module-level test fixtures
# ---------------------------------------------------------------------------

# The expected dict produced by _YAML_CONFIG with no slug-specific override.
# Tests that expect defaults-only results compare against this constant.
_YAML_DEFAULTS = {
    "color_scheme": "gambit",
    "font_family": "sffamily",
    "font_italic": True,
    "shared_terminal_depth": True,
    "sublevel_scaling": 0,
}

# A self-contained gtdraw_settings YAML config used by settings tests.
# Slugs are entirely fictional:
#   "testgroup2000"       – group-level prefix covering testgroup2000/*
#   "othergroup1999"      – group-level prefix covering othergroup1999/*
#   "testgroup2000/fig2"  – game-specific entry that overrides the group above
_YAML_CONFIG = textwrap.dedent("""\
    defaults:
      color_scheme: gambit
      font_family: sffamily
      font_italic: true
      shared_terminal_depth: true
      sublevel_scaling: 0

    overrides:
      testgroup2000:
        sublevel_scaling: 1
      othergroup1999:
        shared_terminal_depth: false
      testgroup2000/fig2:
        action_label_position: 0.4
""")


# ---------------------------------------------------------------------------
# Helper functions
# ---------------------------------------------------------------------------


def _write_yaml(path, content=_YAML_CONFIG):
    """Write *content* to *path* and return *path*.

    Used to create a temporary gtdraw_settings YAML file that can be
    pointed at via ``monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG",
    path)`` without touching the real config file.
    """
    path.write_text(content, encoding="utf-8")
    return path


def _efg_row(slug, title="Test EFG Game", description="A description."):
    """Return a dict representing one row of the DataFrame produced by
    ``gbt.catalog.games(include_descriptions=True)`` for an extensive-form game.
    """
    return {
        "Game": slug,
        "Title": title,
        "Description": description,
        "Download": f":download:`{slug}.efg <../catalog/{slug}.efg>`",
        "Format": "efg",
    }


def _nfg_row(slug, title="Test NFG Game", description="A description."):
    """Return a dict representing one row of the DataFrame for a normal-form game."""
    return {
        "Game": slug,
        "Title": title,
        "Description": description,
        "Download": f":download:`{slug}.nfg <../catalog/{slug}.nfg>`",
        "Format": "nfg",
    }


def _make_df(*rows):
    """Build a DataFrame from one or more row dicts as ``generate_rst_table`` expects."""
    return pd.DataFrame(list(rows))


def _make_image_files(catalog_dir, slug, fmt="efg"):
    """Create stub image files under *catalog_dir*/img/ for *slug*.

    ``generate_rst_table`` checks that all expected image files exist before
    deciding whether to regenerate them.  Touching empty files satisfies that
    check without requiring real gtdraw output, so tests that are not
    specifically about image generation can use this helper to set up the
    pre-existing-images state.

    For EFG games the ``.ef`` intermediate file is also created, since it
    appears in the existence check and the download links.
    """
    img_dir = catalog_dir / "img"
    img_dir.mkdir(parents=True, exist_ok=True)
    slug_path = img_dir / slug
    slug_path.parent.mkdir(parents=True, exist_ok=True)
    for ext in ["tex", "png", "pdf", "svg"]:
        (img_dir / f"{slug}.{ext}").touch()
    if fmt == "efg":
        (img_dir / f"{slug}.ef").touch()


# ---------------------------------------------------------------------------
# Tests for catalog resource selection
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestCatalogResourceSelection:
    """Tests for selecting the catalog source used by update.py."""

    def test_use_catalog_dir_overrides_stale_package_catalog(self, tmp_path, monkeypatch):
        """The updater points pygambit's catalog helpers at the local catalog dir."""
        stale_catalog = tmp_path / "stale_catalog_data"
        local_catalog = tmp_path / "catalog"
        monkeypatch.setattr(update.gbt.catalog, "_CATALOG_RESOURCE", stale_catalog)

        selected = update.use_catalog_dir(local_catalog)

        assert selected == local_catalog
        assert local_catalog == update.gbt.catalog._CATALOG_RESOURCE


# ---------------------------------------------------------------------------
# Tests for catalog_gtdraw_settings
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestCatalogDrawTreeSettings:
    """Unit tests for ``catalog_gtdraw_settings(slug) -> dict``.

    Each test writes a temporary YAML config and redirects the module-level
    ``GTDRAW_SETTINGS_CONFIG`` path to it via ``monkeypatch.setattr``.
    This means the real ``gtdraw_settings.yaml`` is never read or modified.
    """

    def test_no_override_returns_defaults(self, tmp_path, monkeypatch):
        """A slug with no matching entry in ``overrides`` returns the defaults verbatim."""
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_gtdraw_settings("unknowngame/v1")
        assert result == _YAML_DEFAULTS

    def test_exact_slug_override_applied(self, tmp_path, monkeypatch):
        """A key in ``overrides`` that exactly matches the slug is merged into defaults."""
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_gtdraw_settings("testgroup2000/fig2")
        assert result["action_label_position"] == pytest.approx(0.4)
        assert result["color_scheme"] == "gambit"  # defaults still present

    def test_prefix_slug_override_applied(self, tmp_path, monkeypatch):
        """A group-level key (e.g. ``"testgroup2000"``) matches any slug that starts with it."""
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        # "testgroup2000/fig1" is not listed explicitly; it matches the group prefix
        result = update.catalog_gtdraw_settings("testgroup2000/fig1")
        assert result["sublevel_scaling"] == 1

    def test_specific_key_wins_over_group(self, tmp_path, monkeypatch):
        """When both a group key and a more specific key match, the specific key wins.

        The config has ``testgroup2000`` (sets sublevel_scaling=1) and
        ``testgroup2000/fig2`` (sets sublevel_scaling=2).  The game
        ``testgroup2000/fig2`` matches both, but the longer/specific key is
        applied last, so sublevel_scaling should be 2.
        """
        config = textwrap.dedent("""\
            defaults:
              color_scheme: gambit
              sublevel_scaling: 0
            overrides:
              testgroup2000:
                sublevel_scaling: 1
              testgroup2000/fig2:
                sublevel_scaling: 2
        """)
        yaml_file = _write_yaml(tmp_path / "settings.yaml", config)
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_gtdraw_settings("testgroup2000/fig2")
        assert result["sublevel_scaling"] == 2

    def test_group_override_does_not_bleed_to_other_game(self, tmp_path, monkeypatch):
        """A group-level override applies only to games whose slug starts with that prefix."""
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        # "othergroup1999" override sets shared_terminal_depth = False
        result_other = update.catalog_gtdraw_settings("othergroup1999/fig1")
        assert result_other["shared_terminal_depth"] is False
        # "testgroup2000" has a different override; shared_terminal_depth should be True (default)
        result_test = update.catalog_gtdraw_settings("testgroup2000/fig1")
        assert result_test["shared_terminal_depth"] is True

    def test_no_overrides_section_returns_defaults(self, tmp_path, monkeypatch):
        """A config with no ``overrides`` key at all returns only the defaults."""
        config = textwrap.dedent("""\
            defaults:
              color_scheme: gambit
              sublevel_scaling: 0
        """)
        yaml_file = _write_yaml(tmp_path / "settings.yaml", config)
        monkeypatch.setattr(update, "GTDRAW_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_gtdraw_settings("anygame/v1")
        assert result == {"color_scheme": "gambit", "sublevel_scaling": 0}


# ---------------------------------------------------------------------------
# Tests for catalog_ef_file_variants
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestCatalogEfFileVariants:
    """Tests for ``catalog_ef_file_variants(slug, catalog_dir) -> list[dict] | None``.

    The function scans the directory for the game slug inside ``catalog_dir``
    for ``.ef`` files matching the naming convention::

        {stem}.ef            primary variant
        {stem}__{suffix}.ef  additional variant

    All slugs use the fictional prefix ``fakevariant2000`` to make it clear
    these tests do not depend on the real catalog contents.
    """

    def _game_dir(self, catalog_dir, slug):
        """Create and return the directory that would contain the game's files."""
        game_dir = (catalog_dir / slug).parent
        game_dir.mkdir(parents=True, exist_ok=True)
        return game_dir

    def test_no_ef_files_returns_none(self, tmp_path):
        """No .ef files in the game directory → returns None (no tab-set needed)."""
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        self._game_dir(catalog_dir, slug)
        assert update.catalog_ef_file_variants(slug, catalog_dir) is None

    def test_single_ef_file_returns_none(self, tmp_path):
        """A single curated .ef file → returns None (single image, no tabs needed)."""
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1.ef").touch()
        assert update.catalog_ef_file_variants(slug, catalog_dir) is None

    def test_two_ef_files_returns_variant_list(self, tmp_path):
        """Two .ef files → 2-item list with correct label, ef_path, and variant_key."""
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__wide.ef").touch()
        result = update.catalog_ef_file_variants(slug, catalog_dir)
        assert result is not None
        assert len(result) == 2
        assert {v["label"] for v in result} == {"Default", "Wide"}
        assert {v["variant_key"] for v in result} == {slug, f"{slug}__wide"}

    def test_variant_without_primary_ef_returns_variants(self, tmp_path):
        """A variant .ef file exists but no primary .ef file exists ->
        returns variants list including Default.
        """
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1__wide.ef").touch()
        result = update.catalog_ef_file_variants(slug, catalog_dir)
        assert result is not None
        assert len(result) == 2
        assert {v["label"] for v in result} == {"Default", "Wide"}
        assert {v["variant_key"] for v in result} == {slug, f"{slug}__wide"}
        assert not (game_dir / "fig1.ef").exists()

    def test_label_derived_from_filename_suffix(self, tmp_path):
        """The tab label is the suffix after ``__``, title-cased."""
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__compact.ef").touch()
        result = update.catalog_ef_file_variants(slug, catalog_dir)
        assert {v["label"] for v in result} == {"Default", "Compact"}

    def test_multi_word_suffix_title_cased(self, tmp_path):
        """Underscores in the suffix become spaces: ``fig1__very_wide.ef`` → "Very Wide"."""
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__very_wide.ef").touch()
        result = update.catalog_ef_file_variants(slug, catalog_dir)
        assert "Very Wide" in {v["label"] for v in result}

    def test_file_without_double_underscore_excluded(self, tmp_path):
        """A file whose stem is ``{stem}extra`` (no ``__``) is not treated as a variant.

        Only files matching exactly ``{stem}.ef`` or ``{stem}__*.ef`` are counted.
        If the non-conforming file is the only candidate alongside the base, the
        function still returns None (only one conforming file found).
        """
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2000/fig1"
        game_dir = self._game_dir(catalog_dir, slug)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1extra.ef").touch()  # no __ separator — must be ignored
        assert update.catalog_ef_file_variants(slug, catalog_dir) is None


# ---------------------------------------------------------------------------
# Tests for generate_rst_table
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestGenerateRstTable:
    """Tests for ``generate_rst_table(df, rst_path, ...)``.

    Image generation (tex / png / etc.) is mocked out so
    that tests can run without LaTeX installed and without reading
    from the real catalog directory.

    ``_mock_generates`` uses ``monkeypatch.setattr`` to replace each of the
    four gtdraw functions in the ``update`` module's namespace with
    a no-op.  Because the replacement is scoped to the test, the originals are
    automatically restored afterward.

    Tests that need to verify *whether* generation was triggered replace the
    functions with lambdas that append to a ``calls`` list instead.

    All catalog directories and RST output files are created inside ``tmp_path``
    (pytest's per-test temporary directory) so nothing is written to the repo.
    """

    def _no_op_generate(self, *args, **kwargs):
        """Stand-in for gtdraw functions; does nothing."""

    def _mock_generates(self, monkeypatch):
        """Replace all four gtdraw image-generation functions with no-ops."""
        for name in ["tex", "png", "pdf", "svg"]:
            monkeypatch.setattr(update, name, self._no_op_generate)

    def test_efg_row_produces_rst_with_slug_and_title(self, tmp_path, monkeypatch):
        """An EFG game row appears in the RST with its title, load call, and download links."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "fakeauthor2000/fig1"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug, title="Fake Author (2000) Figure 1"))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "Fake Author (2000) Figure 1" in rst
        assert f'pygambit.catalog.load("{slug}")' in rst
        assert f":download:`{slug}.efg" in rst  # source game file download link
        assert f":download:`{slug}.ef" in rst  # gtdraw intermediate file download link

    def test_nfg_row_produces_rst_with_save_to(self, tmp_path, monkeypatch):
        """An NFG game row uses the ``save_to`` form of the draw call (no .ef involved)."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "fakeauthor2001/matrix1"
        _make_image_files(catalog_dir, slug, "nfg")
        df = _make_df(_nfg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert f'save_to="../catalog/img/{slug}.png"' in rst

    def test_unknown_format_row_is_skipped(self, tmp_path, monkeypatch):
        """A row whose Format is not 'efg' or 'nfg' is silently omitted from the RST."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        catalog_dir.mkdir()
        row = {
            "Game": "fakegame/v1",
            "Title": "Fake Game",
            "Description": "Has a description.",
            "Download": "",
            "Format": "efg_2",  # not in SUPPORTED_GAME_FORMATS
        }
        df = _make_df(row)
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "Fake Game" not in rst

    def test_row_without_description_is_skipped(self, tmp_path, monkeypatch):
        """A game with an empty description is not included in the RST output."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        catalog_dir.mkdir()
        df = _make_df(_efg_row("fakeauthor2000/fig1", description=""))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "fakeauthor2000/fig1" not in rst

    def test_curated_ef_used_in_draw_call(self, tmp_path, monkeypatch):
        """When a curated .ef file exists alongside the .efg, the RST draw call
        references the .ef path directly rather than ``pygambit.catalog.load``."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "fakeauthor1999/fig1"
        _make_image_files(catalog_dir, slug, "efg")
        # Place a curated .ef file alongside the game — this is what update.py checks for
        curated = catalog_dir / f"{slug}.ef"
        curated.parent.mkdir(parents=True, exist_ok=True)
        curated.touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        # Find the draw( call line in the jupyter-execute block
        draw_call = next(line for line in rst.splitlines() if "draw(" in line)
        assert f'"../catalog/{slug}.ef"' in draw_call
        assert "catalog.load" not in draw_call

    def test_images_not_regenerated_when_all_exist(self, tmp_path, monkeypatch):
        """If all expected image files are already present and ``regenerate_images`` is
        False, none of the gtdraw image-generation functions are called."""
        calls = []
        # Replace generate_* with lambdas that record invocations
        monkeypatch.setattr(update, "tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "fakeauthor2000/fig1"
        _make_image_files(catalog_dir, slug, "efg")  # all images already exist
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=False, catalog_dir=catalog_dir)
        assert calls == []

    def test_images_regenerated_when_flag_set(self, tmp_path, monkeypatch):
        """When ``regenerate_images=True``, all four generate functions are called even
        if the image files already exist.

        A curated .ef file is placed in the catalog dir so ``update.py`` uses it
        as the gtdraw source rather than calling ``gbt.catalog.load``, which
        would require the real catalog to be present.
        """
        calls = []
        monkeypatch.setattr(update, "tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "fakeauthor2000/fig1"
        _make_image_files(catalog_dir, slug, "efg")
        # Place a curated .ef file alongside the game — this is what update.py checks for
        curated = catalog_dir / f"{slug}.ef"
        curated.parent.mkdir(parents=True, exist_ok=True)
        curated.touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=True, catalog_dir=catalog_dir)
        assert set(calls) == {"tex", "png", "pdf", "svg"}

    def test_multi_variant_efg_produces_tab_set(self, tmp_path, monkeypatch):
        """Two curated .ef files alongside a game trigger a ``tab-set`` in the RST.

        The RST should contain ``.. tab-set::`` and one ``.. tab-item::`` per
        variant, with labels derived from the filename suffixes.
        """
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2001/fig1"
        game_dir = catalog_dir / "fakevariant2001"
        game_dir.mkdir(parents=True)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__wide.ef").touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=True, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. tab-set::" in rst
        assert ".. tab-item:: Default" in rst
        assert ".. tab-item:: Wide" in rst

    def test_multi_variant_efg_without_primary_ef_produces_tab_set(self, tmp_path, monkeypatch):
        """A variant .ef file without a primary .ef file triggers a tab-set in the RST
        containing both a Default and the custom variant, calling catalog.load for the Default.
        """
        self._mock_generates(monkeypatch)
        monkeypatch.setattr(update.gbt.catalog, "load", lambda slug: "dummy_game")
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2001/fig1"
        game_dir = catalog_dir / "fakevariant2001"
        game_dir.mkdir(parents=True)
        (game_dir / "fig1__wide.ef").touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=True, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. tab-set::" in rst
        assert ".. tab-item:: Default" in rst
        assert ".. tab-item:: Wide" in rst
        assert 'draw(pygambit.catalog.load("fakevariant2001/fig1")' in rst
        assert 'draw("../catalog/fakevariant2001/fig1__wide.ef"' in rst

    def test_single_variant_efg_produces_no_tab_set(self, tmp_path, monkeypatch):
        """A single curated .ef file (or no .ef file) does not produce a ``tab-set``."""
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2001/fig1"
        _make_image_files(catalog_dir, slug, "efg")
        game_dir = catalog_dir / "fakevariant2001"
        game_dir.mkdir(parents=True, exist_ok=True)
        (game_dir / "fig1.ef").touch()  # only one .ef — no tabs
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. tab-set::" not in rst

    def test_per_variant_images_generated(self, tmp_path, monkeypatch):
        """When multiple .ef variants exist, image generation is called once per variant.

        Two variants × four generate functions = eight total calls.
        """
        calls = []
        monkeypatch.setattr(update, "tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2001/fig1"
        game_dir = catalog_dir / "fakevariant2001"
        game_dir.mkdir(parents=True)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__wide.ef").touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=True, catalog_dir=catalog_dir)
        assert len(calls) == 8  # 4 functions × 2 variants

    def test_per_variant_images_not_regenerated_when_all_exist(self, tmp_path, monkeypatch):
        """If all variant image files already exist and ``regenerate_images`` is False,
        generate functions are not called."""
        calls = []
        monkeypatch.setattr(update, "tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "fakevariant2001/fig1"
        game_dir = catalog_dir / "fakevariant2001"
        game_dir.mkdir(parents=True)
        (game_dir / "fig1.ef").touch()
        (game_dir / "fig1__wide.ef").touch()
        # Pre-create all image files for both variants so nothing needs regenerating
        for vkey in [slug, f"{slug}__wide"]:
            _make_image_files(catalog_dir, vkey, "efg")
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=False, catalog_dir=catalog_dir)
        assert calls == []


# ---------------------------------------------------------------------------
# Tests for hierarchy helpers and hierarchical RST output
# ---------------------------------------------------------------------------

# A minimal catalog_hierarchy.yaml used by hierarchy tests.
_HIERARCHY_YAML = textwrap.dedent("""\
    labels:
      cat: "My Category"
      cat/src: "My Source"
""")


@pytest.mark.catalog_update
class TestHierarchyHelpers:
    """Unit tests for ``load_hierarchy_labels``, ``_node_label``, and ``_build_slug_tree``."""

    def test_load_hierarchy_labels_returns_dict(self, tmp_path, monkeypatch):
        """``load_hierarchy_labels`` returns the labels dict from the YAML."""
        yaml_file = tmp_path / "hier.yaml"
        yaml_file.write_text(_HIERARCHY_YAML, encoding="utf-8")
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", yaml_file)
        labels = update.load_hierarchy_labels()
        assert labels["cat"] == "My Category"
        assert labels["cat/src"] == "My Source"

    def test_node_label_uses_yaml(self, tmp_path, monkeypatch):
        """``_node_label`` returns the YAML label when the prefix is present."""
        yaml_file = tmp_path / "hier.yaml"
        yaml_file.write_text(_HIERARCHY_YAML, encoding="utf-8")
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", yaml_file)
        labels = update.load_hierarchy_labels()
        assert update._node_label("cat", labels) == "My Category"

    def test_node_label_fallback_title_case(self, tmp_path, monkeypatch):
        """``_node_label`` falls back to title-casing the last component."""
        yaml_file = tmp_path / "hier.yaml"
        yaml_file.write_text(_HIERARCHY_YAML, encoding="utf-8")
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", yaml_file)
        labels = update.load_hierarchy_labels()
        assert update._node_label("cat/unknownsrc", labels) == "Unknownsrc"

    def test_build_slug_tree_single_game(self):
        """A single-slug DataFrame builds a 2-level tree."""
        df = _make_df(_efg_row("cat/src/game1"))
        tree = update._build_slug_tree(df)
        assert "cat" in tree
        assert "src" in tree["cat"]
        assert "game1" in tree["cat"]["src"]

    def test_build_slug_tree_groups_siblings(self):
        """Two slugs sharing a prefix are grouped under the same intermediate node."""
        df = _make_df(_efg_row("cat/src/game1"), _efg_row("cat/src/game2"))
        tree = update._build_slug_tree(df)
        assert set(tree["cat"]["src"].keys()) == {"game1", "game2"}

    def test_build_slug_tree_skips_unknown_format(self):
        """Rows with unrecognised Format are excluded from the tree."""
        row = {**_efg_row("cat/src/game1"), "Format": "xyz"}
        df = _make_df(row)
        assert update._build_slug_tree(df) == {}

    def test_build_slug_tree_skips_empty_description(self):
        """Rows with an empty description are excluded from the tree."""
        df = _make_df(_efg_row("cat/src/game1", description=""))
        assert update._build_slug_tree(df) == {}


@pytest.mark.catalog_update
class TestHierarchicalRstOutput:
    """Tests that ``generate_rst_table`` produces correctly nested dropdown RST."""

    def _mock_generates(self, monkeypatch):
        for name in ["tex", "png", "pdf", "svg"]:
            monkeypatch.setattr(update, name, lambda *a, **k: None)

    def _write_hierarchy_yaml(self, tmp_path, content=_HIERARCHY_YAML):
        yaml_file = tmp_path / "hier.yaml"
        yaml_file.write_text(content, encoding="utf-8")
        return yaml_file

    def test_top_level_dropdown_is_open(self, tmp_path, monkeypatch):
        """Top-level category dropdowns carry ``:open:`` so the first level is visible."""
        self._mock_generates(monkeypatch)
        hier_yaml = self._write_hierarchy_yaml(tmp_path)
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", hier_yaml)
        catalog_dir = tmp_path / "catalog"
        slug = "cat/src/game1"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. dropdown:: My Category\n   :open:" in rst

    def test_second_level_dropdown_is_not_open(self, tmp_path, monkeypatch):
        """Sub-category dropdowns do NOT carry ``:open:`` so they are collapsed by default."""
        self._mock_generates(monkeypatch)
        hier_yaml = self._write_hierarchy_yaml(tmp_path)
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", hier_yaml)
        catalog_dir = tmp_path / "catalog"
        slug = "cat/src/game1"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "   .. dropdown:: My Source\n   \n" in rst
        # Confirm :open: does not immediately follow the second-level dropdown
        src_idx = rst.index("   .. dropdown:: My Source")
        assert ":open:" not in rst[src_idx : src_idx + 40]

    def test_game_dropdown_is_open(self, tmp_path, monkeypatch):
        """Individual game dropdowns carry ``:open:`` so game content is visible on expand."""
        self._mock_generates(monkeypatch)
        hier_yaml = self._write_hierarchy_yaml(tmp_path)
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", hier_yaml)
        catalog_dir = tmp_path / "catalog"
        slug = "cat/src/game1"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug, title="My Game Title"))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "      .. dropdown:: My Game Title\n         :open:" in rst

    def test_sibling_games_both_appear_under_source(self, tmp_path, monkeypatch):
        """Two games sharing a source prefix both appear nested under the source dropdown."""
        self._mock_generates(monkeypatch)
        hier_yaml = self._write_hierarchy_yaml(tmp_path)
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", hier_yaml)
        catalog_dir = tmp_path / "catalog"
        for slug in ["cat/src/game1", "cat/src/game2"]:
            _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(
            _efg_row("cat/src/game1", title="Game One"),
            _efg_row("cat/src/game2", title="Game Two"),
        )
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. dropdown:: My Category" in rst
        assert "   .. dropdown:: My Source" in rst
        assert "Game One" in rst
        assert "Game Two" in rst

    def test_no_list_table_in_output(self, tmp_path, monkeypatch):
        """The new output does not use ``.. list-table::`` (replaced by nested dropdowns)."""
        self._mock_generates(monkeypatch)
        hier_yaml = self._write_hierarchy_yaml(tmp_path)
        monkeypatch.setattr(update, "CATALOG_HIERARCHY_CONFIG", hier_yaml)
        catalog_dir = tmp_path / "catalog"
        slug = "cat/src/game1"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert ".. list-table::" not in rst
        assert ".. contents::" not in rst


# ---------------------------------------------------------------------------
# Tests for update_makefile
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestUpdateMakefile:
    """Tests for ``update_makefile(catalog_dir, am_path)``.

    Both arguments are injected via ``tmp_path`` so the real catalog directory
    and the real ``catalog.am`` file are never read or written.
    """

    def test_efg_and_nfg_files_included(self, tmp_path):
        """Game files with .efg and .nfg extensions appear in the generated catalog.am."""
        (tmp_path / "standalone.efg").touch()
        (tmp_path / "subfolder").mkdir()
        (tmp_path / "subfolder" / "matrix.nfg").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "catalog/standalone.efg" in content
        assert "catalog/subfolder/matrix.nfg" in content

    def test_curated_ef_included(self, tmp_path):
        """A curated .ef file committed alongside a game file appears in catalog.am."""
        (tmp_path / "fakegame").mkdir()
        (tmp_path / "fakegame" / "fig1.efg").touch()
        (tmp_path / "fakegame" / "fig1.ef").touch()  # curated layout file
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "catalog/fakegame/fig1.ef" in content

    def test_ef_in_img_dir_excluded(self, tmp_path):
        """Generated .ef files under the img/ subdirectory are excluded from catalog.am."""
        img = tmp_path / "img" / "fakegame"
        img.mkdir(parents=True)
        (img / "fig1.ef").touch()  # generated artifact — should not be distributed
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "img" not in content

    def test_non_game_file_excluded(self, tmp_path):
        """Files with non-game extensions (e.g. .efg_2, .txt) are not included."""
        (tmp_path / "fakegame.efg_2").touch()  # hidden/renamed file
        (tmp_path / "README.txt").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "efg_2" not in content
        assert "README" not in content

    def test_no_write_when_content_unchanged(self, tmp_path):
        """If catalog.am already contains the correct content, it is not rewritten.

        The mtime of the file is captured after the first write and compared
        after the second call.  If the file were overwritten, the mtime would
        change; if the content-equality check works correctly, it stays the same.
        """
        (tmp_path / "standalone.efg").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        mtime_after_first_write = am.stat().st_mtime
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        assert am.stat().st_mtime == mtime_after_first_write

    def test_empty_catalog_produces_valid_am(self, tmp_path):
        """An empty catalog directory produces a catalog.am with a valid (empty) CATALOG_FILES."""
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert content.startswith("CATALOG_FILES =")


@pytest.mark.catalog_update
class TestWarnMissingDescriptions:
    """Tests for ``_warn_missing_descriptions(df)``.

    The function prints to stderr for each game in a supported format that has
    an empty description.  Tests use ``capsys`` to capture stderr output.
    """

    def test_game_without_description_warns(self, capsys):
        """A game with an empty description produces a WARNING on stderr."""
        df = _make_df(_efg_row("journals/nobody2025/fig1", description=""))
        update._warn_missing_descriptions(df)
        err = capsys.readouterr().err
        assert "WARNING" in err
        assert "journals/nobody2025/fig1" in err

    def test_game_with_description_does_not_warn(self, capsys):
        """A game with a non-empty description produces no output."""
        df = _make_df(_efg_row("journals/nobody2025/fig1"))
        update._warn_missing_descriptions(df)
        err = capsys.readouterr().err
        assert err == ""
