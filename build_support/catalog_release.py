#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: build_support/catalog_release.py
# Fetch the pinned release of the Gambit games catalog
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
"""Fetch the pinned release of the Gambit games catalog.

The pin is ``build_support/catalog.toml``. The release archive is downloaded once into
``.catalog/<version>/`` at the root of the repository, checked against the pinned SHA-256,
and unpacked there.

Usage::

    python build_support/catalog_release.py fetch   # fetch if needed; print the games directory
    python build_support/catalog_release.py path    # print the games directory
"""

import argparse
import hashlib
import json
import shutil
import sys
import tempfile
import tomllib
import urllib.error
import urllib.request
import zipfile
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PIN_FILE = Path(__file__).resolve().with_name("catalog.toml")
CACHE_DIR = REPO_ROOT / ".catalog"
RELEASE_URL = (
    "https://github.com/gambitproject/gambit-catalog/releases/download/"
    "v{version}/gambit-catalog-{version}.zip"
)


class CatalogReleaseError(RuntimeError):
    """The pinned catalog release could not be fetched or does not match its pin."""


@dataclass(frozen=True)
class Pin:
    """A pinned catalog release: its version and the SHA-256 of its archive."""

    version: str
    sha256: str

    @property
    def stem(self) -> str:
        return f"gambit-catalog-{self.version}"

    @property
    def url(self) -> str:
        return RELEASE_URL.format(version=self.version)


def read_pin(pin_file: Path = PIN_FILE) -> Pin:
    """Read the pinned catalog release from *pin_file*."""
    with open(pin_file, "rb") as f:
        data = tomllib.load(f)
    return Pin(version=data["version"], sha256=data["sha256"].lower())


def archive_path(pin: Pin, cache_dir: Path = CACHE_DIR) -> Path:
    """Return where the archive of *pin* is kept."""
    return cache_dir / pin.version / f"{pin.stem}.zip"


def games_dir(pin: Pin, cache_dir: Path = CACHE_DIR) -> Path:
    """Return the games directory of the unpacked release of *pin*."""
    return cache_dir / pin.version / pin.stem / "games"


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 16), b""):
            digest.update(block)
    return digest.hexdigest()


def _download(url: str, target: Path) -> None:
    target.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as tmp:
        try:
            with urllib.request.urlopen(url, timeout=60) as response:
                shutil.copyfileobj(response, tmp)
        except (urllib.error.URLError, OSError) as e:
            tmp.close()
            Path(tmp.name).unlink()
            raise CatalogReleaseError(
                f"could not download the games catalog from {url}: {e}. "
                "Building Gambit needs the pinned catalog release once; "
                "check the network connection and try again."
            ) from None
    Path(tmp.name).replace(target)


def _unpack(archive: Path, pin: Pin, release_dir: Path) -> None:
    """Unpack *archive* so that its top-level directory becomes *release_dir*."""
    with zipfile.ZipFile(archive) as zf:
        for name in zf.namelist():
            if not name.startswith(f"{pin.stem}/") or ".." in Path(name).parts:
                raise CatalogReleaseError(f"{archive}: unexpected member {name!r}")
        with tempfile.TemporaryDirectory(dir=release_dir.parent) as tmp:
            zf.extractall(tmp)
            if release_dir.exists():
                shutil.rmtree(release_dir)
            Path(tmp, pin.stem).replace(release_dir)


def fetch(pin: Pin | None = None, cache_dir: Path = CACHE_DIR, url: str | None = None) -> Path:
    """Fetch, check and unpack the pinned catalog release, unless already done.

    Parameters
    ----------
    pin : Pin, optional
        The release to fetch; by default the one in ``build_support/catalog.toml``.
    cache_dir : Path, optional
        Where releases are kept; by default ``.catalog`` at the root of the repository.
    url : str, optional
        Where to download the archive from; by default the release's URL on GitHub.

    Returns
    -------
    Path
        The games directory of the unpacked release.

    Raises
    ------
    CatalogReleaseError
        If the archive cannot be downloaded, its SHA-256 differs from the pin, or its
        manifest is for another version.
    """
    pin = pin or read_pin()
    archive = archive_path(pin, cache_dir)
    if not archive.exists() or _sha256(archive) != pin.sha256:
        _download(url or pin.url, archive)
    actual = _sha256(archive)
    if actual != pin.sha256:
        archive.unlink()
        raise CatalogReleaseError(
            f"the games catalog archive for {pin.version} has SHA-256 {actual}, "
            f"but build_support/catalog.toml pins {pin.sha256}"
        )
    games = games_dir(pin, cache_dir)
    manifest = games / "manifest.json"
    if not manifest.exists():
        _unpack(archive, pin, games.parent)
    version = json.loads(manifest.read_text(encoding="utf-8"))["catalog_version"]
    if version != pin.version:
        raise CatalogReleaseError(
            f"the games catalog archive pinned as {pin.version} contains version {version}"
        )
    return games


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Fetch the pinned games catalog release.")
    parser.add_argument("command", choices=["fetch", "path"])
    args = parser.parse_args(argv)
    try:
        print(fetch() if args.command == "fetch" else games_dir(read_pin()))
    except CatalogReleaseError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
