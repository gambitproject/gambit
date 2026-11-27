#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_catalog_fetch.py
# Fetch releases and commits of the Gambit games catalog
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
"""Fetch releases and commits of the Gambit games catalog into a local cache.

A catalog is named by a *ref*: a release version (``"0.2.0"``) or a commit (a full SHA, or a
branch, tag or short SHA, which is resolved to the full SHA first).  Each ref is kept in the
cache as ``<cache>/<key>/gambit-catalog-<key>.zip`` and unpacked beside it as
``<cache>/<key>/gambit-catalog-<key>/``, where the key is the version or the full SHA.

This module uses only the standard library and does not import pygambit, so that the build
(``setup.py``, ``build_support/catalog_release.py``) can load it before pygambit is built.
"""

import hashlib
import http.client
import os
import re
import shutil
import sys
import tempfile
import urllib.parse
import urllib.request
import zipfile
from collections.abc import Callable
from pathlib import Path, PurePosixPath

RELEASE_URL = (
    "https://github.com/gambitproject/gambit-catalog/releases/download/"
    "v{version}/gambit-catalog-{version}.zip"
)
COMMIT_URL = "https://codeload.github.com/gambitproject/gambit-catalog/zip/{sha}"
RESOLVE_URL = "https://api.github.com/repos/gambitproject/gambit-catalog/commits/{ref}"

_RELEASE_REF = re.compile(r"(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)")
_COMMIT_REF = re.compile(r"[0-9a-f]{40}")
_TIMEOUT = 60


class CatalogError(RuntimeError):
    """A games catalog could not be fetched, or is not one pygambit can read."""


def is_release(ref: str) -> bool:
    """Return whether *ref* names a release, i.e. is a version such as ``"0.2.0"``."""
    return _RELEASE_REF.fullmatch(ref) is not None


def is_commit(ref: str) -> bool:
    """Return whether *ref* is a full commit SHA."""
    return _COMMIT_REF.fullmatch(ref) is not None


def default_cache_dir() -> Path:
    """Return the per-user directory in which fetched catalogs are kept."""
    if sys.platform == "win32":
        base = Path(os.environ.get("LOCALAPPDATA") or Path.home() / "AppData" / "Local")
    elif sys.platform == "darwin":
        base = Path.home() / "Library" / "Caches"
    else:
        base = Path(os.environ.get("XDG_CACHE_HOME") or Path.home() / ".cache")
    return base / "gambit" / "catalog"


def stem(key: str) -> str:
    """Return the name of the archive and unpacked directory of the ref *key*."""
    return f"gambit-catalog-{key}"


def archive_path(key: str, cache_dir: Path) -> Path:
    """Return where the archive of the ref *key* is kept in *cache_dir*."""
    return cache_dir / key / f"{stem(key)}.zip"


def unpacked_dir(key: str, cache_dir: Path) -> Path:
    """Return where the ref *key* is unpacked in *cache_dir*."""
    return cache_dir / key / stem(key)


def sha256_of(path: Path) -> str:
    """Return the SHA-256 of the file at *path*, as lowercase hex."""
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 16), b""):
            digest.update(block)
    return digest.hexdigest()


def _open(url: str, accept: str | None = None) -> http.client.HTTPResponse:
    request = urllib.request.Request(url)
    if accept:
        request.add_header("Accept", accept)
    try:
        response: http.client.HTTPResponse = urllib.request.urlopen(request, timeout=_TIMEOUT)
        return response
    except (OSError, http.client.HTTPException) as e:
        raise CatalogError(f"could not download {url}: {e}") from None


def _read_text(url: str, accept: str | None = None) -> str:
    try:
        with _open(url, accept) as response:
            return response.read().decode("utf-8")
    except (OSError, http.client.HTTPException, UnicodeDecodeError) as e:
        raise CatalogError(f"could not download {url}: {e}") from None


def _download(url: str, target: Path) -> None:
    """Download *url* to *target*, which appears only once the download is complete."""
    target.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as tmp:
        try:
            with _open(url) as response:
                shutil.copyfileobj(response, tmp)
        except BaseException as e:
            tmp.close()
            Path(tmp.name).unlink()
            if isinstance(e, (OSError, http.client.HTTPException)):
                raise CatalogError(f"could not download {url}: {e}") from None
            raise
    Path(tmp.name).replace(target)


def _is_unpacked(target: Path) -> bool:
    return (target / "games" / "manifest.json").is_file()


def _unpack(archive: Path, target: Path) -> None:
    """Unpack *archive*, whose members all lie in one top-level directory, as *target*.

    The top-level directory's name in the archive is not used, so that GitHub's commit
    archives (named after the repository and SHA) and release archives unpack alike.
    """
    try:
        zf = zipfile.ZipFile(archive)
    except zipfile.BadZipFile:
        raise CatalogError(f"{archive} is not a zip archive") from None
    with zf:
        names = zf.namelist()
        for name in names:
            if name.startswith("/") or ".." in PurePosixPath(name).parts:
                raise CatalogError(f"{archive}: unexpected member {name!r}")
        tops = {PurePosixPath(name).parts[0] for name in names}
        if len(tops) != 1:
            raise CatalogError(f"{archive}: expected one top-level directory, not {sorted(tops)}")
        (top,) = tops
        target.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=target.parent) as tmp:
            try:
                zf.extractall(tmp)
            except (OSError, zipfile.BadZipFile) as e:
                raise CatalogError(f"{archive} could not be unpacked: {e}") from None
            if not _is_unpacked(Path(tmp, top)):
                raise CatalogError(f"{archive} holds no games/manifest.json")
            if target.exists() and not _is_unpacked(target):
                shutil.rmtree(target)
            try:
                Path(tmp, top).replace(target)
            except OSError:
                # Another process installed the same ref meanwhile.
                if not _is_unpacked(target):
                    raise


def _fetch_archive(
    url: str,
    archive: Path,
    target: Path,
    check: Callable[[Path], None] | None = None,
) -> None:
    """Unpack *archive* as *target*, downloading it from *url* if it is missing.

    A cached archive that fails *check* or cannot be unpacked is discarded and downloaded
    again, once.  A downloaded archive that fails is discarded before the error is raised.
    """

    def install() -> None:
        if check:
            check(archive)
        _unpack(archive, target)

    if archive.exists():
        try:
            install()
            return
        except CatalogError:
            archive.unlink(missing_ok=True)
    _download(url, archive)
    try:
        install()
    except CatalogError:
        archive.unlink(missing_ok=True)
        raise


def _published_sha256(url: str) -> str:
    """Return the SHA-256 published beside the release archive at *url*."""
    words = _read_text(f"{url}.sha256").split()
    if not words or not re.fullmatch(r"[0-9a-fA-F]{64}", words[0]):
        raise CatalogError(f"{url}.sha256 does not hold a SHA-256")
    return words[0].lower()


def fetch_release(
    version: str,
    cache_dir: Path,
    sha256: str | None = None,
    url: str | None = None,
) -> Path:
    """Fetch, check and unpack the catalog release *version*, unless already done.

    Parameters
    ----------
    version : str
        The release, e.g. ``"0.2.0"``.
    cache_dir : Path
        Where catalogs are kept.
    sha256 : str, optional
        The archive's expected SHA-256; by default the one published with the release.
    url : str, optional
        Where to download the archive from; by default the release's URL on GitHub.

    Returns
    -------
    Path
        The unpacked release directory, holding ``games/``.

    Raises
    ------
    CatalogError
        If the archive cannot be downloaded or does not match its SHA-256.
    """
    if not is_release(version):
        raise CatalogError(f"{version!r} is not a catalog release version")
    archive = archive_path(version, cache_dir)
    target = unpacked_dir(version, cache_dir)
    pinned = sha256.lower() if sha256 else None
    if pinned and archive.exists() and sha256_of(archive) != pinned:
        # The pin has moved to a different archive for this version.
        archive.unlink()
        if target.exists():
            shutil.rmtree(target)
    if _is_unpacked(target):
        return target
    url = url or RELEASE_URL.format(version=version)

    def check(path: Path) -> None:
        expected = pinned or _published_sha256(url)
        actual = sha256_of(path)
        if actual != expected:
            raise CatalogError(
                f"the games catalog archive for {version} has SHA-256 {actual}, "
                f"expected {expected}"
            )

    _fetch_archive(url, archive, target, check)
    return target


def resolve_commit(ref: str, url: str | None = None) -> str:
    """Return the full SHA of the commit that *ref* (a branch, tag or SHA) names now.

    Raises
    ------
    CatalogError
        If *ref* cannot be resolved.
    """
    if is_commit(ref):
        return ref
    url = url or RESOLVE_URL.format(ref=urllib.parse.quote(ref, safe=""))
    sha = _read_text(url, accept="application/vnd.github.sha").strip()
    if not is_commit(sha):
        raise CatalogError(f"could not resolve {ref!r} to a commit of the games catalog")
    return sha


def fetch_commit(sha: str, cache_dir: Path, url: str | None = None) -> Path:
    """Fetch and unpack the catalog at commit *sha*, unless already done.

    Parameters
    ----------
    sha : str
        The full SHA of the commit.
    cache_dir : Path
        Where catalogs are kept.
    url : str, optional
        Where to download the archive from; by default GitHub's archive of the commit.

    Returns
    -------
    Path
        The unpacked directory, holding ``games/``.

    Raises
    ------
    CatalogError
        If the archive cannot be downloaded or unpacked.
    """
    if not is_commit(sha):
        raise CatalogError(f"{sha!r} is not a full commit SHA")
    target = unpacked_dir(sha, cache_dir)
    if _is_unpacked(target):
        return target
    archive = archive_path(sha, cache_dir)
    _fetch_archive(url or COMMIT_URL.format(sha=sha), archive, target)
    return target


def fetch(
    ref: str,
    cache_dir: Path | None = None,
    *,
    release_url: str = RELEASE_URL,
    commit_url: str = COMMIT_URL,
    resolve_url: str = RESOLVE_URL,
) -> tuple[str, Path]:
    """Fetch the catalog named by *ref*; return its key and unpacked directory.

    The key is *ref* for a release, and the full SHA for a commit.

    Parameters
    ----------
    ref : str
        A release version, or a branch, tag or SHA of the catalog's repository.
    cache_dir : Path, optional
        Where catalogs are kept; by default :func:`default_cache_dir`.
    release_url, commit_url, resolve_url : str, optional
        Templates for the release archive (``{version}``), the commit archive (``{sha}``)
        and the commit resolution (``{ref}``); by default GitHub's.

    Raises
    ------
    CatalogError
        If *ref* cannot be resolved, or its archive cannot be downloaded, checked or unpacked.
    """
    cache_dir = cache_dir or default_cache_dir()
    if is_release(ref):
        return ref, fetch_release(ref, cache_dir, url=release_url.format(version=ref))
    if is_commit(ref):
        sha = ref
    else:
        sha = resolve_commit(ref, resolve_url.format(ref=urllib.parse.quote(ref, safe="")))
    return sha, fetch_commit(sha, cache_dir, commit_url.format(sha=sha))
