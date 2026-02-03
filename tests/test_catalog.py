import pygambit as gbt


def test_catalog_load_efg():
    g = gbt.catalog.load("2smp")
    assert isinstance(g, gbt.Game)


def test_catalog_load_nfg():
    g = gbt.catalog.load("pd")
    assert isinstance(g, gbt.Game)
