import ast
import pathlib
import pickle

import pytest

import pygambit as gbt
from pygambit import _hsel, _labels, _layout, _numbers, _random

_LEAVES = [_hsel, _labels, _layout, _numbers, _random]


def _runtime_imports(module) -> list[str]:
    """Names imported by `module` outside `if typing.TYPE_CHECKING:` blocks."""
    tree = ast.parse(pathlib.Path(module.__file__).read_text())
    found = []
    for node in ast.walk(tree):
        if isinstance(node, ast.If) and "TYPE_CHECKING" in ast.unparse(node.test):
            for child in ast.walk(node):
                child.skip = True
    for node in ast.walk(tree):
        if getattr(node, "skip", False):
            continue
        if isinstance(node, ast.ImportFrom):
            found.append(("." * node.level) + (node.module or ""))
        elif isinstance(node, ast.Import):
            found.extend(alias.name for alias in node.names)
    return found


@pytest.mark.parametrize("module", _LEAVES, ids=lambda m: m.__name__)
def test_leaf_module_does_not_import_the_extension_at_run_time(module):
    imports = _runtime_imports(module)
    assert not any(name in (".gambit", "pygambit.gambit") for name in imports), imports


def test_public_classes_are_defined_in_leaf_modules_and_reexported():
    assert gbt.Rational is _numbers.Rational is gbt.gambit.Rational
    assert gbt.Decimal is _numbers.Decimal
    assert gbt.TreeLayout is _layout.TreeLayout
    assert gbt.TreeLayoutCoordinates is _layout.TreeLayoutCoordinates
    for name in ("History", "HistoryTransition", "Selector", "GroupedSelector", "H"):
        assert getattr(gbt, name) is getattr(_hsel, name)
        assert getattr(gbt, name).__module__ == "pygambit._hsel"


def test_rational_pickles():
    value = pickle.loads(pickle.dumps(gbt.Rational(1, 3)))
    assert value == gbt.Rational(1, 3)
    assert isinstance(value, gbt.Rational)


@pytest.mark.parametrize(
    "value, expected",
    [(3, "3"), (0.1, "0.1"), ("2/4", "1/2"), (gbt.Rational(1, 3), "1/3"), ("1.50", "1.50")],
)
def test_to_number_string(value, expected):
    assert _numbers.to_number_string(value) == expected


@pytest.mark.parametrize("value", ["a/b", "abc", None])
def test_to_number_string_rejects_non_numbers(value):
    with pytest.raises(ValueError, match="Cannot convert"):
        _numbers.to_number_string(value)


def test_grid_distribution_sums_to_one():
    dist = _random._grid_distribution(["a", "b", "c"], 6, None)
    assert sum(dist.values()) == 1
    assert all(isinstance(p, gbt.Rational) for p in dist.values())
