# Subgame detection — IPL paper reproducibility

This branch is Gambit **v16.6.0** with one change: `BuildSubgameRoots`
in `src/games/gametree.cc` is swapped from the interval Tarjan detector
shipped in 16.6 to the **Zobrist flux** variant. It exists only to reproduce
the timing comparison in the IPL paper; it is not intended to be merged.

## Reproducing the two columns

- **Zobrist**: build this branch and run `notebooks/IPL-submission.ipynb`.
- **Interval Tarjan (baseline)**: build stock `v16.6.0` (unmodified upstream)
  and run the same notebook.

Both are anchored to the same release, so the only difference between
the two builds is the detector itself.

## What the notebook does

Regenerates the benchmark games (centipede, binary, repeated) from
scratch and reports per-game subgame-detection timings.

Correctness of the detectors is covered by Gambit's own test suite,
which both implementations pass; it is not re-checked here.
