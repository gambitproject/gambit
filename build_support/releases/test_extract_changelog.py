"""Tests for extract_changelog.py.

Covers two concerns:
1. Correctness of the extraction logic (unit tests against fixture content).
2. Format validity of the repository's actual ChangeLog file.

The format tests act as a CI guard: if a contributor adds a malformed version
header or an unrecognised section type to ChangeLog, the test suite will fail.
"""

import pathlib
import re

import pytest
from extract_changelog import extract

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = pathlib.Path(__file__).parent.parent.parent
CHANGELOG = REPO_ROOT / "ChangeLog"

# ---------------------------------------------------------------------------
# ChangeLog format rules (Keep a Changelog)
# ---------------------------------------------------------------------------

# New pre-release identifiers follow Semantic Versioning. Historical PEP 440
# spellings remain valid so that existing ChangeLog entries continue to pass.
VERSION_HEADER_RE = re.compile(
    r"^## \[\d+\.\d+\.\d+(?:(?:a|b|rc)\d+|-(?:alpha|beta|rc)\.\d+)?\] - "
    r"(\d{4}-\d{2}-\d{2}|unreleased)$"
)
# 'General' is a project-specific extension used for cross-cutting changes.
SECTION_HEADER_RE = re.compile(r"^### (Added|Changed|Deprecated|Removed|Fixed|Security|General)$")

# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

SAMPLE_CHANGELOG = """\
# Changelog

## [2.0.0] - 2024-01-15

### Added
- Feature A

### Fixed
- Bug B

## [1.0.0] - 2023-06-01

### Added
- Initial release
"""

UNRELEASED_CHANGELOG = (
    """\
# Changelog

## [3.0.0] - unreleased

### Added
- Upcoming feature

"""
    + SAMPLE_CHANGELOG
)

PRERELEASE_CHANGELOG = """\
# Changelog

## [17.0.0-rc.1] - 2026-08-03

### Fixed
- Release candidate fix

## [17.0.0-beta.2] - 2026-07-27

### Changed
- Second beta change

## [17.0.0-alpha.3] - 2026-07-20

### Added
- Third alpha feature
"""


# ---------------------------------------------------------------------------
# Extraction unit tests
# ---------------------------------------------------------------------------


def test_extract_known_version(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(SAMPLE_CHANGELOG)
    output = tmp_path / "notes.md"

    extract("2.0.0", changelog, output)

    content = output.read_text()
    assert "Feature A" in content
    assert "Bug B" in content


def test_extract_does_not_bleed_into_next_version(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(SAMPLE_CHANGELOG)
    output = tmp_path / "notes.md"

    extract("2.0.0", changelog, output)

    assert "Initial release" not in output.read_text()


def test_extract_last_version_in_file(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(SAMPLE_CHANGELOG)
    output = tmp_path / "notes.md"

    extract("1.0.0", changelog, output)

    assert "Initial release" in output.read_text()


def test_extract_unreleased_version(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(UNRELEASED_CHANGELOG)
    output = tmp_path / "notes.md"

    extract("3.0.0", changelog, output)

    assert "Upcoming feature" in output.read_text()


@pytest.mark.parametrize(
    ("version", "expected"),
    [
        ("17.0.0-alpha.3", "Third alpha feature"),
        ("17.0.0-beta.2", "Second beta change"),
        ("17.0.0-rc.1", "Release candidate fix"),
    ],
)
def test_extract_prerelease_version(tmp_path, version, expected):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(PRERELEASE_CHANGELOG)
    output = tmp_path / "notes.md"

    extract(version, changelog, output)

    assert expected in output.read_text()


def test_extract_missing_version_exits(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(SAMPLE_CHANGELOG)
    output = tmp_path / "notes.md"

    with pytest.raises(SystemExit) as exc_info:
        extract("99.0.0", changelog, output)

    assert exc_info.value.code != 0


def test_extract_output_is_stripped(tmp_path):
    changelog = tmp_path / "ChangeLog"
    changelog.write_text(SAMPLE_CHANGELOG)
    output = tmp_path / "notes.md"

    extract("2.0.0", changelog, output)

    content = output.read_text()
    assert content == content.strip()


# ---------------------------------------------------------------------------
# ChangeLog format validation tests
# ---------------------------------------------------------------------------


def _changelog_lines():
    """Return (line_number, line) pairs for the repository ChangeLog."""
    return list(enumerate(CHANGELOG.read_text().splitlines(), start=1))


@pytest.mark.parametrize(
    "lineno,line",
    [(line_number, line) for line_number, line in _changelog_lines() if line.startswith("## ")],
)
def test_version_header_format(lineno, line):
    """Every '## ' line must contain a supported release version and date."""
    assert VERSION_HEADER_RE.match(line), (
        f"ChangeLog line {lineno}: invalid version header: {line!r}\n"
        "Expected: ## [X.Y.Z] - DATE or "
        "## [X.Y.Z-(alpha|beta|rc).W] - DATE; DATE is YYYY-MM-DD or unreleased "
        "(historical PEP 440 prerelease versions are also accepted)"
    )


@pytest.mark.parametrize(
    "lineno,line",
    [(line_number, line) for line_number, line in _changelog_lines() if line.startswith("### ")],
)
def test_section_header_type(lineno, line):
    """Every '### ' line must be one of the recognised Keep a Changelog types."""
    assert SECTION_HEADER_RE.match(line), (
        f"ChangeLog line {lineno}: unrecognised section header: {line!r}\n"
        "Allowed: ### Added, ### Changed, ### Deprecated, "
        "### Removed, ### Fixed, ### Security"
    )
