"""Count the logical code lines of pygambit's Cython and Python sources.

A line counts if it carries a token other than a comment or a statement-level string
(a docstring).  Blank lines, comments and docstrings change neither count.

Usage: python build_support/cython_lines.py [--max N]

Exits 1 if the Cython count exceeds N.
"""

import argparse
import io
import pathlib
import sys
import tokenize

ROOT = pathlib.Path(__file__).resolve().parent.parent
PACKAGE = ROOT / "src" / "pygambit"

_STRING_PARTS = {
    tokenize.STRING,
    getattr(tokenize, "FSTRING_START", -1),
    getattr(tokenize, "FSTRING_MIDDLE", -1),
    getattr(tokenize, "FSTRING_END", -1),
}
_BLANK = {
    tokenize.COMMENT,
    tokenize.NL,
    tokenize.NEWLINE,
    tokenize.INDENT,
    tokenize.DEDENT,
    tokenize.ENDMARKER,
    tokenize.ENCODING,
}


def count_lines(path: pathlib.Path) -> int:
    """Return the number of code lines in `path`."""
    source = path.read_text(encoding="utf-8")
    lines: set[int] = set()
    logical: list[tokenize.TokenInfo] = []

    def flush() -> None:
        if logical and not all(t.type in _STRING_PARTS for t in logical):
            for t in logical:
                lines.update(range(t.start[0], t.end[0] + 1))
        logical.clear()

    for tok in tokenize.generate_tokens(io.StringIO(source).readline):
        if tok.type == tokenize.NEWLINE:
            flush()
        elif tok.type not in _BLANK:
            logical.append(tok)
    flush()
    return len(lines)


def count_files(paths) -> int:
    return sum(count_lines(p) for p in sorted(paths))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--max",
        type=int,
        default=None,
        metavar="N",
        help="fail if the Cython line count exceeds N",
    )
    args = parser.parse_args()

    cython = count_files([*PACKAGE.glob("*.pyx"), *PACKAGE.glob("*.pxi")])
    python = count_files(PACKAGE.rglob("*.py"))
    share = 100 * cython / (cython + python)
    print(f"Cython: {cython} lines (src/pygambit/*.pyx, *.pxi)")
    print(f"Python: {python} lines (src/pygambit/**/*.py)")
    print(f"Cython share: {share:.1f}%")

    if args.max is not None and cython > args.max:
        print(
            f"error: {cython} Cython lines exceeds the ceiling of {args.max}.\n"
            "pygambit's Cython layer converts values and handles; validation, label\n"
            "resolution and messages belong in Python (see 'Layers of pygambit' in\n"
            "doc/developer.overview.rst). Move code into Python and lower the ceiling\n"
            "(--max in .github/workflows/lint.yml) to the new count. Raise it only for a\n"
            "new binding of C++ functionality.",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
