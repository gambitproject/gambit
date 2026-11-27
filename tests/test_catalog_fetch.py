"""Tests of fetching catalog releases and commits (pygambit._catalog_fetch), with no network."""

import hashlib
import http.client
import json
import zipfile

import pytest

from pygambit import _catalog_fetch as cf

SHA = "a" * 40


def _write_archive(path, top, manifest=True, extra=()):
    path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(path, "w") as zf:
        if manifest:
            zf.writestr(f"{top}/games/manifest.json", json.dumps({"games": []}))
        for name in extra:
            zf.writestr(name, "x")
    return path


@pytest.fixture
def upstream(tmp_path):
    """A fake GitHub: a release 9.8.7 with its .sha256, a commit SHA, and branch 'main'."""
    root = tmp_path / "upstream"
    release = _write_archive(root / "gambit-catalog-9.8.7.zip", "gambit-catalog-9.8.7")
    (root / "gambit-catalog-9.8.7.zip.sha256").write_text(
        f"{hashlib.sha256(release.read_bytes()).hexdigest()}  gambit-catalog-9.8.7.zip\n"
    )
    _write_archive(root / f"{SHA}.zip", f"gambit-catalog-{SHA}")
    (root / "ref-main").write_text(f"{SHA}\n")
    return root


@pytest.fixture
def endpoints(upstream):
    return {
        "release_url": upstream.as_uri() + "/gambit-catalog-{version}.zip",
        "commit_url": upstream.as_uri() + "/{sha}.zip",
        "resolve_url": upstream.as_uri() + "/ref-{ref}",
    }


@pytest.mark.parametrize(
    "ref,release,commit",
    [
        ("0.2.0", True, False),
        ("10.0.12", True, False),
        ("01.2.3", False, False),
        ("0.2", False, False),
        (SHA, False, True),
        ("A" * 40, False, False),
        ("abc1234", False, False),
        ("main", False, False),
    ],
)
def test_ref_kinds(ref, release, commit):
    assert cf.is_release(ref) is release
    assert cf.is_commit(ref) is commit


@pytest.mark.parametrize(
    "platform,env,expected",
    [
        ("linux", {"XDG_CACHE_HOME": "/xdg"}, "/xdg/gambit/catalog"),
        ("linux", {}, "HOME/.cache/gambit/catalog"),
        ("darwin", {}, "HOME/Library/Caches/gambit/catalog"),
        ("win32", {"LOCALAPPDATA": "/local"}, "/local/gambit/catalog"),
    ],
)
def test_default_cache_dir(platform, env, expected, monkeypatch, tmp_path):
    monkeypatch.setattr(cf.sys, "platform", platform)
    monkeypatch.setattr(cf.Path, "home", lambda: tmp_path)
    for name in ("XDG_CACHE_HOME", "LOCALAPPDATA"):
        monkeypatch.delenv(name, raising=False)
    for name, value in env.items():
        monkeypatch.setenv(name, value)
    assert cf.default_cache_dir().as_posix() == expected.replace("HOME", tmp_path.as_posix())


def test_fetch_release_checks_published_sha256(tmp_path, upstream, endpoints):
    key, unpacked = cf.fetch("9.8.7", tmp_path / "cache", **endpoints)
    assert key == "9.8.7"
    assert unpacked == tmp_path / "cache" / "9.8.7" / "gambit-catalog-9.8.7"
    assert (unpacked / "games" / "manifest.json").is_file()


def test_fetch_release_reuses_cache(tmp_path, upstream, endpoints):
    cf.fetch("9.8.7", tmp_path / "cache", **endpoints)
    for path in upstream.iterdir():
        path.unlink()
    assert cf.fetch("9.8.7", tmp_path / "cache", **endpoints)[0] == "9.8.7"


def test_fetch_release_rejects_wrong_sha256(tmp_path, upstream, endpoints):
    (upstream / "gambit-catalog-9.8.7.zip.sha256").write_text("0" * 64)
    with pytest.raises(cf.CatalogError, match="SHA-256"):
        cf.fetch("9.8.7", tmp_path / "cache", **endpoints)
    assert not cf.archive_path("9.8.7", tmp_path / "cache").exists()


def test_fetch_release_without_published_sha256(tmp_path, upstream, endpoints):
    (upstream / "gambit-catalog-9.8.7.zip.sha256").unlink()
    with pytest.raises(cf.CatalogError, match="could not download"):
        cf.fetch("9.8.7", tmp_path / "cache", **endpoints)


def test_fetch_unknown_release(tmp_path, endpoints):
    with pytest.raises(cf.CatalogError, match="could not download"):
        cf.fetch("1.2.3", tmp_path / "cache", **endpoints)


def test_fetch_commit_by_sha_needs_no_resolution(tmp_path, endpoints):
    endpoints["resolve_url"] = "file:///nonexistent/{ref}"
    key, unpacked = cf.fetch(SHA, tmp_path / "cache", **endpoints)
    assert key == SHA
    assert unpacked == tmp_path / "cache" / SHA / f"gambit-catalog-{SHA}"
    assert (unpacked / "games" / "manifest.json").is_file()


def test_fetch_commit_by_branch(tmp_path, endpoints):
    assert cf.fetch("main", tmp_path / "cache", **endpoints)[0] == SHA


def test_resolve_rejects_non_sha(tmp_path, upstream, endpoints):
    (upstream / "ref-main").write_text("Not Found")
    with pytest.raises(cf.CatalogError, match="could not resolve 'main'"):
        cf.fetch("main", tmp_path / "cache", **endpoints)


def test_corrupt_cached_archive_is_downloaded_again(tmp_path, endpoints):
    archive = cf.archive_path(SHA, tmp_path / "cache")
    archive.parent.mkdir(parents=True)
    archive.write_bytes(b"not a zip")
    assert cf.fetch(SHA, tmp_path / "cache", **endpoints)[0] == SHA


def test_corrupt_download_is_discarded(tmp_path, upstream, endpoints):
    (upstream / f"{SHA}.zip").write_bytes(b"not a zip")
    with pytest.raises(cf.CatalogError, match="not a zip archive"):
        cf.fetch(SHA, tmp_path / "cache", **endpoints)
    assert not cf.archive_path(SHA, tmp_path / "cache").exists()


def test_interrupted_transfer_raises_catalog_error(tmp_path, monkeypatch):
    class Truncated:
        def __enter__(self):
            return self

        def __exit__(self, *args):
            return False

        def read(self, *args):
            raise http.client.IncompleteRead(b"")

    monkeypatch.setattr(cf.urllib.request, "urlopen", lambda *args, **kwargs: Truncated())
    with pytest.raises(cf.CatalogError, match="could not download"):
        cf.resolve_commit("main", "https://example.invalid/ref")
    with pytest.raises(cf.CatalogError, match="could not download"):
        cf.fetch_commit(SHA, tmp_path / "cache", "https://example.invalid/archive.zip")
    assert list((tmp_path / "cache" / SHA).iterdir()) == []


@pytest.mark.parametrize(
    "top,manifest,extra,message",
    [
        ("t", True, ["t/../../escape"], "unexpected member"),
        ("t", True, ["other/file"], "one top-level directory"),
        ("t", False, ["t/games/a.efg"], "no games/manifest.json"),
    ],
)
def test_unpack_rejects_bad_archives(tmp_path, top, manifest, extra, message):
    archive = _write_archive(tmp_path / "bad.zip", top, manifest, extra)
    with pytest.raises(cf.CatalogError, match=message):
        cf._unpack(archive, tmp_path / "target")
    assert not (tmp_path / "target").exists()


def test_unpack_keeps_a_valid_target(tmp_path):
    """A ref installed by another process meanwhile is left in place."""
    archive = _write_archive(tmp_path / "a.zip", "t")
    target = tmp_path / "target"
    cf._unpack(archive, target)
    before = (target / "games" / "manifest.json").stat().st_ino
    cf._unpack(archive, target)
    assert (target / "games" / "manifest.json").stat().st_ino == before
    assert sorted(p.name for p in tmp_path.iterdir()) == ["a.zip", "target"]
