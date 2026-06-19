"""Extract the changelog section for a given version from the ChangeLog file.

The ChangeLog at the repository root follows the Keep a Changelog format
(https://keepachangelog.com). Each release section looks like:

    ## [X.Y.Z] - YYYY-MM-DD

    ### Added
    - ...

    ### Fixed
    - ...

This script locates the section for a given version and writes it to an output
file, which is then used by the ``release.yml`` GitHub Actions workflow to
populate the GitHub release notes.

Usage
-----
Run from the repository root::

    python3 build_support/releases/extract_changelog.py X.Y.Z

Optional arguments::

    --changelog PATH   Path to the ChangeLog file (default: ChangeLog)
    --output PATH      Path to write extracted notes (default: release_notes.md)
"""

import argparse
import pathlib
import re
import sys


def extract(version: str, changelog: pathlib.Path, output: pathlib.Path) -> None:
    """Extract the release notes for *version* from *changelog* and write to *output*.

    Searches for a section header of the form ``## [X.Y.Z] - ...`` and captures
    everything up to the next version header (or end of file).  Exits with a
    non-zero status and an error message if the version is not found.

    Parameters
    ----------
    version:
        Version string without a leading ``v``, e.g. ``"16.6.0"``.
    changelog:
        Path to the ChangeLog file to read from.
    output:
        Path to the file to write the extracted notes to.
    """
    text = changelog.read_text()
    pattern = re.compile(
        rf"^## \[{re.escape(version)}\][^\n]*\n(.*?)(?=^## \[|\Z)",
        re.MULTILINE | re.DOTALL,
    )
    match = pattern.search(text)
    if not match:
        print(f"No ChangeLog entry found for version {version}", file=sys.stderr)
        sys.exit(1)
    output.write_text(match.group(1).strip())
    print(f"Extracted release notes for {version}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("version", help="Version string, e.g. 16.6.0")
    parser.add_argument(
        "--changelog",
        type=pathlib.Path,
        default=pathlib.Path("ChangeLog"),
        help="Path to the ChangeLog file (default: ChangeLog)",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=pathlib.Path("release_notes.md"),
        help="Path to write the extracted notes (default: release_notes.md)",
    )
    args = parser.parse_args()
    extract(args.version, args.changelog, args.output)
