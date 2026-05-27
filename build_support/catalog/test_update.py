"""Tests for build_support/catalog/update.py."""

import pytest

pytest.importorskip("draw_tree")  # update.py imports draw_tree at module level
pytest.importorskip("yaml")

import pandas as pd  # noqa: E402
import update  # noqa: E402

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

_YAML_DEFAULTS = {
    "color_scheme": "gambit",
    "font_family": "sffamily",
    "font_italic": True,
    "shared_terminal_depth": True,
    "sublevel_scaling": 0,
}

_YAML_CONFIG = """
defaults:
  color_scheme: gambit
  font_family: sffamily
  font_italic: true
  shared_terminal_depth: true
  sublevel_scaling: 0

overrides:
  watson2013:
    sublevel_scaling: 1
  selten1975:
    shared_terminal_depth: false
  myerson1991/fig2_1:
    action_label_position: 0.4
"""


def _write_yaml(path, content=_YAML_CONFIG):
    path.write_text(content, encoding="utf-8")
    return path


def _efg_row(slug, title="Game Title", description="A description."):
    return {
        "Game": slug,
        "Title": title,
        "Description": description,
        "Download": f":download:`{slug}.efg <../catalog/{slug}.efg>`",
        "Format": "efg",
    }


def _nfg_row(slug, title="NFG Title", description="NFG description."):
    return {
        "Game": slug,
        "Title": title,
        "Description": description,
        "Download": f":download:`{slug}.nfg <../catalog/{slug}.nfg>`",
        "Format": "nfg",
    }


def _make_df(*rows):
    return pd.DataFrame(list(rows))


def _make_image_files(catalog_dir, slug, fmt="efg"):
    """Create the stub image files so the existence check passes."""
    img = catalog_dir / "img" / slug
    img.parent.mkdir(parents=True, exist_ok=True)
    for ext in ["tex", "png", "pdf", "svg"]:
        (catalog_dir / "img" / f"{slug}.{ext}").touch()
    if fmt == "efg":
        (catalog_dir / "img" / f"{slug}.ef").touch()


# ---------------------------------------------------------------------------
# catalog_draw_tree_settings
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestCatalogDrawTreeSettings:
    def test_no_override_returns_defaults(self, tmp_path, monkeypatch):
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("unknown/game")
        assert result == _YAML_DEFAULTS

    def test_exact_slug_override_applied(self, tmp_path, monkeypatch):
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("myerson1991/fig2_1")
        assert result["action_label_position"] == pytest.approx(0.4)
        assert result["color_scheme"] == "gambit"  # defaults still present

    def test_prefix_slug_override_applied(self, tmp_path, monkeypatch):
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("watson2013/exercise29_6")
        assert result["sublevel_scaling"] == 1

    def test_specific_key_wins_over_group(self, tmp_path, monkeypatch):
        config = """
                    defaults:
                      color_scheme: gambit
                      sublevel_scaling: 0
                    overrides:
                      myerson1991:
                        sublevel_scaling: 1
                      myerson1991/fig2_1:
                        sublevel_scaling: 2
                """
        yaml_file = _write_yaml(tmp_path / "settings.yaml", config)
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("myerson1991/fig2_1")
        assert result["sublevel_scaling"] == 2

    def test_group_override_does_not_bleed_to_other_game(self, tmp_path, monkeypatch):
        yaml_file = _write_yaml(tmp_path / "settings.yaml")
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("selten1975/fig1")
        assert result["shared_terminal_depth"] is False
        result2 = update.catalog_draw_tree_settings("watson2013/fig29_1")
        assert result2["shared_terminal_depth"] is True  # selten override not applied

    def test_no_overrides_section_returns_defaults(self, tmp_path, monkeypatch):
        config = "defaults:\n  color_scheme: gambit\n  sublevel_scaling: 0\n"
        yaml_file = _write_yaml(tmp_path / "settings.yaml", config)
        monkeypatch.setattr(update, "DRAW_TREE_SETTINGS_CONFIG", yaml_file)
        result = update.catalog_draw_tree_settings("any/game")
        assert result == {"color_scheme": "gambit", "sublevel_scaling": 0}


# ---------------------------------------------------------------------------
# generate_rst_table
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestGenerateRstTable:
    def _no_op_generate(self, *args, **kwargs):
        """Replacement for draw_tree generate_* functions that does nothing."""

    def _mock_generates(self, monkeypatch):
        for name in ["generate_tex", "generate_png", "generate_pdf", "generate_svg"]:
            monkeypatch.setattr(update, name, self._no_op_generate)

    def test_efg_row_produces_rst_with_slug_and_title(self, tmp_path, monkeypatch):
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "bagwell1995"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug, title="Bagwell 1995"))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "Bagwell 1995" in rst
        assert f'pygambit.catalog.load("{slug}")' in rst
        assert f":download:`{slug}.efg" in rst
        assert f":download:`{slug}.ef" in rst

    def test_nfg_row_produces_rst_with_save_to(self, tmp_path, monkeypatch):
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "nau2004/sec3"
        _make_image_files(catalog_dir, slug, "nfg")
        df = _make_df(_nfg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert f'save_to="../catalog/img/{slug}.png"' in rst

    def test_unknown_format_row_is_skipped(self, tmp_path, monkeypatch):
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        catalog_dir.mkdir()
        row = {
            "Game": "bogus/game",
            "Title": "Bogus",
            "Description": "Has a description.",
            "Download": "",
            "Format": "efg_2",
        }
        df = _make_df(row)
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "Bogus" not in rst

    def test_row_without_description_is_skipped(self, tmp_path, monkeypatch):
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        catalog_dir.mkdir()
        df = _make_df(_efg_row("bagwell1995", description=""))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        assert "bagwell1995" not in rst

    def test_curated_ef_used_in_draw_tree_call(self, tmp_path, monkeypatch):
        self._mock_generates(monkeypatch)
        catalog_dir = tmp_path / "catalog"
        slug = "selten1975/fig1"
        _make_image_files(catalog_dir, slug, "efg")
        # Place a curated .ef alongside the .efg
        curated = catalog_dir / f"{slug}.ef"
        curated.parent.mkdir(parents=True, exist_ok=True)
        curated.touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, catalog_dir=catalog_dir)
        rst = rst_path.read_text()
        # The draw_tree() call line should reference the .ef file path directly
        draw_tree_call = next(line for line in rst.splitlines() if "draw_tree(" in line)
        assert f'"../catalog/{slug}.ef"' in draw_tree_call
        assert "catalog.load" not in draw_tree_call

    def test_images_not_regenerated_when_all_exist(self, tmp_path, monkeypatch):
        calls = []
        monkeypatch.setattr(update, "generate_tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "generate_png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "generate_pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "generate_svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "bagwell1995"
        _make_image_files(catalog_dir, slug, "efg")
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=False, catalog_dir=catalog_dir)
        assert calls == []

    def test_images_regenerated_when_flag_set(self, tmp_path, monkeypatch):
        calls = []
        monkeypatch.setattr(update, "generate_tex", lambda *a, **k: calls.append("tex"))
        monkeypatch.setattr(update, "generate_png", lambda *a, **k: calls.append("png"))
        monkeypatch.setattr(update, "generate_pdf", lambda *a, **k: calls.append("pdf"))
        monkeypatch.setattr(update, "generate_svg", lambda *a, **k: calls.append("svg"))
        catalog_dir = tmp_path / "catalog"
        slug = "bagwell1995"
        _make_image_files(catalog_dir, slug, "efg")
        # Use a curated .ef so gbt.catalog.load is not called (avoids needing real catalog)
        curated = catalog_dir / f"{slug}.ef"
        curated.touch()
        df = _make_df(_efg_row(slug))
        rst_path = tmp_path / "out.rst"
        update.generate_rst_table(df, rst_path, regenerate_images=True, catalog_dir=catalog_dir)
        assert set(calls) == {"tex", "png", "pdf", "svg"}


# ---------------------------------------------------------------------------
# update_makefile
# ---------------------------------------------------------------------------


@pytest.mark.catalog_update
class TestUpdateMakefile:
    def test_efg_and_nfg_files_included(self, tmp_path):
        (tmp_path / "foo.efg").touch()
        (tmp_path / "sub").mkdir()
        (tmp_path / "sub" / "bar.nfg").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "catalog/foo.efg" in content
        assert "catalog/sub/bar.nfg" in content

    def test_curated_ef_included(self, tmp_path):
        (tmp_path / "myerson1991").mkdir()
        (tmp_path / "myerson1991" / "fig1.efg").touch()
        (tmp_path / "myerson1991" / "fig1.ef").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "catalog/myerson1991/fig1.ef" in content

    def test_ef_in_img_dir_excluded(self, tmp_path):
        img = tmp_path / "img" / "selten1975"
        img.mkdir(parents=True)
        (img / "fig1.ef").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "img" not in content

    def test_non_game_file_excluded(self, tmp_path):
        (tmp_path / "foo.efg_2").touch()
        (tmp_path / "readme.txt").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert "efg_2" not in content
        assert "readme" not in content

    def test_no_write_when_content_unchanged(self, tmp_path):
        (tmp_path / "foo.efg").touch()
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        mtime_after_first = am.stat().st_mtime
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        assert am.stat().st_mtime == mtime_after_first

    def test_empty_catalog_produces_valid_am(self, tmp_path):
        am = tmp_path / "catalog.am"
        update.update_makefile(catalog_dir=tmp_path, am_path=am)
        content = am.read_text()
        assert content.startswith("CATALOG_FILES =")
