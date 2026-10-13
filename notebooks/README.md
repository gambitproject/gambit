# Subgame detection — IPL paper reproducibility

This branch is Gambit **v16.6.0** with one change: `BuildSubgameRoots`
in `src/games/gametree.cc` is swapped from the interval Tarjan detector
shipped in 16.6 to the **Zobrist flux** variant, whose weight width
`p_bits` (default 64) is a parameter of `ZobristVisitor`. It exists only
to reproduce the experiments in the IPL paper; it is not intended to be
merged.

## Running times (Table 2)

- **Zobrist**: build this branch and run `notebooks/IPL-submission.ipynb`.
- **Interval Tarjan (Algorithm 1)**: build stock `v16.6.0` (unmodified
  upstream) and run the same notebook.

Both are anchored to the same release, so the only difference between
the two builds is the detector itself.

## False positives at small k

Set `p_bits` to 16 or 20 in `ZobristVisitor`, rebuild, set `K_BITS` to
match in the notebook's false-positive cell, and run that cell (Binary 18,
1,000 runs). At the default `p_bits = 64` it is a sanity check and should
report no false positives.

## Previous Gambit procedure (Gambit 16.5)

`notebooks/IPL-gambit165.ipynb` times Gambit 16.5 finding every subgame
root on one game from each family, the way Table 2 times Algorithm 1: a
fresh game per run, built off the clock, median of 1,000 runs. It gives the
paper's Centipede 3k figure. Gambit 16.5 has no call that returns every
subgame root, so the notebook enumerates the nodes and tests each one. Run
it in a separate environment:

    pip install pygambit==16.5.0 numpy

Centipede 3k takes about 5.5 hours on an Apple M1 Pro. The timing cell of
`IPL-submission.ipynb` cannot be reused under 16.5: there
`game.root.is_subgame_root` tests the root alone and returns at once.

## What the notebook does

Regenerates the benchmark games (centipede, binary, repeated) from
scratch, reports per-game subgame-detection timings, and runs the
false-positive experiment.

Correctness of the detectors is covered by Gambit's own test suite,
which both implementations pass; it is not re-checked here.
