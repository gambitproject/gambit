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
and unpacked there.  Downloading, checking and unpacking are done by
``src/pygambit/_catalog_fetch.py``, which is loaded by path because pygambit is not built
when ``setup.py`` runs.

Usage::

    python build_support/catalog_release.py fetch   # fetch if needed; print the games directory
    python build_support/catalog_release.py path    # print the games directory
"""

import argparse
import importlib.util
import json
import sys
import tomllib
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PIN_FILE = Path(__file__).resolve().with_name("catalog.toml")
CACHE_DIR = REPO_ROOT / ".catalog"


def _load_catalog_fetch():
    path = REPO_ROOT / "src" / "pygambit" / "_catalog_fetch.py"
    spec = importlib.util.spec_from_file_location("_catalog_fetch", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


_catalog_fetch = _load_catalog_fetch()
CatalogError = _catalog_fetch.CatalogError


@dataclass(frozen=True)
class Pin:
    """A pinned catalog release: its version and the SHA-256 of its archive."""

    version: str
    sha256: str


def read_pin(pin_file: Path = PIN_FILE) -> Pin:
    """Read the pinned catalog release from *pin_file*."""
    with open(pin_file, "rb") as f:
        data = tomllib.load(f)
    return Pin(version=data["version"], sha256=data["sha256"].lower())


def archive_path(pin: Pin, cache_dir: Path = CACHE_DIR) -> Path:
    """Return where the archive of *pin* is kept."""
    return _catalog_fetch.archive_path(pin.version, cache_dir)


def games_dir(pin: Pin, cache_dir: Path = CACHE_DIR) -> Path:
    """Return the games directory of the unpacked release of *pin*."""
    return _catalog_fetch.unpacked_dir(pin.version, cache_dir) / "games"


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
    CatalogError
        If the archive cannot be downloaded, its SHA-256 differs from the pin, or its
        manifest is for another version.
    """
    pin = pin or read_pin()
    release = _catalog_fetch.fetch_release(pin.version, cache_dir, sha256=pin.sha256, url=url)
    games = release / "games"
    manifest = json.loads((games / "manifest.json").read_text(encoding="utf-8"))
    if manifest["catalog_version"] != pin.version:
        raise CatalogError(
            f"the games catalog archive pinned as {pin.version} contains version "
            f"{manifest['catalog_version']}"
        )
    return games


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Fetch the pinned games catalog release.")
    parser.add_argument("command", choices=["fetch", "path"])
    args = parser.parse_args(argv)
    try:
        print(fetch() if args.command == "fetch" else games_dir(read_pin()))
    except CatalogError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
