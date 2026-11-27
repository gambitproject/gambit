"""Tests of fetching the pinned games catalog release (build_support/catalog_release.py)."""

import hashlib
import json
import zipfile

import catalog_release
import pytest


def _make_release(tmp_path, version="9.8.7", manifest_version=None):
    """Write a fake release archive and return (pin, file URL)."""
    archive = tmp_path / "upstream" / f"gambit-catalog-{version}.zip"
    archive.parent.mkdir()
    manifest = {"schema_version": 9, "catalog_version": manifest_version or version, "games": []}
    with zipfile.ZipFile(archive, "w") as zf:
        top = f"gambit-catalog-{version}"
        zf.writestr(f"{top}/games/manifest.json", json.dumps(manifest))
        zf.writestr(f"{top}/games/a/fig1/fig1.efg", "EFG 2 R")
        zf.writestr(f"{top}/thumbs/a/fig1.png", b"png")
    sha = hashlib.sha256(archive.read_bytes()).hexdigest()
    return catalog_release.Pin(version=version, sha256=sha), archive.as_uri()


def test_read_pin_of_repository():
    pin = catalog_release.read_pin()
    assert pin.version.count(".") == 2
    assert len(pin.sha256) == 64
    assert catalog_release.archive_path(pin).relative_to(catalog_release.CACHE_DIR).parts == (
        pin.version,
        f"gambit-catalog-{pin.version}.zip",
    )


def test_fetch_unpacks_release(tmp_path):
    pin, url = _make_release(tmp_path)
    games = catalog_release.fetch(pin, cache_dir=tmp_path / "cache", url=url)
    assert games == tmp_path / "cache" / "9.8.7" / "gambit-catalog-9.8.7" / "games"
    assert (games / "a" / "fig1" / "fig1.efg").read_text() == "EFG 2 R"
    assert (games.parent / "thumbs" / "a" / "fig1.png").exists()


def test_fetch_reuses_verified_archive(tmp_path):
    pin, url = _make_release(tmp_path)
    catalog_release.fetch(pin, cache_dir=tmp_path / "cache", url=url)
    games = catalog_release.fetch(pin, cache_dir=tmp_path / "cache", url="file:///nonexistent")
    assert (games / "manifest.json").exists()


def test_fetch_rejects_wrong_sha256(tmp_path):
    pin, url = _make_release(tmp_path)
    bad = catalog_release.Pin(version=pin.version, sha256="0" * 64)
    with pytest.raises(catalog_release.CatalogError, match="SHA-256"):
        catalog_release.fetch(bad, cache_dir=tmp_path / "cache", url=url)
    assert not catalog_release.archive_path(bad, tmp_path / "cache").exists()


def test_fetch_rejects_manifest_of_another_version(tmp_path):
    pin, url = _make_release(tmp_path, manifest_version="1.0.0")
    with pytest.raises(catalog_release.CatalogError, match="contains version 1.0.0"):
        catalog_release.fetch(pin, cache_dir=tmp_path / "cache", url=url)


def test_fetch_fails_clearly_without_download(tmp_path):
    pin, _ = _make_release(tmp_path)
    with pytest.raises(catalog_release.CatalogError, match="could not download"):
        catalog_release.fetch(pin, cache_dir=tmp_path / "cache", url="file:///nonexistent.zip")
