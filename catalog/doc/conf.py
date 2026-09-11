import pathlib
import sys

# The shared bibtex citation style lives in the main docs project; import it from
# there rather than duplicating it (see doc/_bibstyle.py).
sys.path.insert(0, str(pathlib.Path(__file__).parent.parent.parent / "doc"))
import _bibstyle  # noqa: E402,F401  registers the shared "keystyle" pybtex plugin

# -- General configuration -----------------------------------------------------

extensions = [
    "sphinx_design",
    "jupyter_sphinx",
    "sphinxcontrib.bibtex",
]

bibtex_bibfiles = [str(pathlib.Path(__file__).parent.parent.parent / "doc" / "references.bib")]

source_suffix = {".rst": "restructuredtext"}
master_doc = "index"

project = "Gambit Catalog"
copyright = "1994-2026, The Gambit Project"  # noqa

_version_file = pathlib.Path(__file__).parent.parent.parent / "build_support" / "GAMBIT_VERSION"
if _version_file.exists():
    _full_version = _version_file.read_text().strip()
    version = ".".join(_full_version.split(".")[:2])
    release = _full_version
else:
    raise FileNotFoundError("GAMBIT_VERSION file not found")

exclude_patterns = ["_build", "_table.rst"]

pygments_style = "sphinx"

# -- Options for HTML output ---------------------------------------------------

html_theme = "pydata_sphinx_theme"
html_theme_options = {
    "external_links": [
        {"name": "Gambit documentation", "url": "https://gambitproject.readthedocs.io/"},
        {"name": "GitHub", "url": "https://github.com/gambitproject/gambit"},
    ],
}
