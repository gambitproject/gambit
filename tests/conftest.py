import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--run-tutorials",
        action="store_true",
        default=False,
        help="Run tutorial notebook tests",
    )


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "tutorials: mark test as a tutorial notebook test"
    )


def pytest_collection_modifyitems(config, items):
    if config.getoption("--run-tutorials"):
        # --run-tutorials given in cli: do not skip tutorial tests
        return

    skip_tutorials = pytest.mark.skip(reason="need --run-tutorials option to run")
    for item in items:
        if "tutorials" in item.keywords:
            item.add_marker(skip_tutorials)
