"""Regression tests pinning the principal branch traced by the agent logit QRE solver.

Each case records the full sequence of points traced along the path by the current
implementation, together with the path tracer's step and evaluation counts.  Changes to
how the equations or their Jacobian are computed must reproduce both: a subtly wrong
Jacobian may still yield points on the correct curve, because the corrector only drives
the equations themselves to zero, but it almost always changes the steps taken.

To regenerate the reference data after an intentional change in tracing behaviour, run
``python -m tests.test_logit_golden`` from the repository root.
"""
import dataclasses
import json
import pathlib

import numpy as np
import pytest

import pygambit as gbt

from . import games

GOLDEN_DIR = pathlib.Path(__file__).parent / "test_games" / "golden_logit"
FIRST_STEP = 0.03
MAX_ACCEL = 1.1
RTOL = 1.0e-7
ATOL = 1.0e-9


@dataclasses.dataclass(frozen=True)
class GoldenCase:
    factory: object
    maxregret: float | None = None
    lams: tuple[float, ...] | None = None


CASES = {
    "stripped_down_poker": GoldenCase(
        lambda: games.create_stripped_down_poker_efg(), maxregret=1.0e-4
    ),
    "stripped_down_poker_nonterm_outcomes": GoldenCase(
        lambda: games.create_stripped_down_poker_efg(nonterm_outcomes=True), maxregret=1.0e-4
    ),
    "kuhn_poker": GoldenCase(lambda: games.create_kuhn_poker_efg(), maxregret=1.0e-4),
    "chance_in_middle_nonterm_outcomes": GoldenCase(
        lambda: games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
        maxregret=1.0e-4,
    ),
    "nature_leaves_generic": GoldenCase(
        lambda: games.read_from_file("nature_leaves_generic.efg"), maxregret=1.0e-4
    ),
    "3_player_nonterm_outcomes": GoldenCase(
        lambda: games.read_from_file("3_player_with_nonterm_outcomes.efg"), maxregret=1.0e-4
    ),
    "6x6_long_LH_paths": GoldenCase(
        lambda: games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq(),
        maxregret=1.0e-4,
    ),
    "allpay_pv_p90_b11": GoldenCase(
        lambda: games.create_allpay_correlated_types_efg(gbt.Rational(9, 10), False, 11),
        maxregret=1.0e-8,
    ),
    "coordination_2x2_to_lambda_20": GoldenCase(
        lambda: games.create_EFG_for_nxn_bimatrix_coordination_game(2), lams=(20.0,)
    ),
    "allpay_cv_p60_b11_to_lambda_20": GoldenCase(
        lambda: games.create_allpay_correlated_types_efg(gbt.Rational(3, 5), True, 11),
        lams=(1.0, 5.0, 20.0),
    ),
}

COUNT_FIELDS = [
    "predictor_attempts", "accepted_steps", "rejected_distance", "rejected_contraction",
    "rejected_orientation", "corrector_iterations", "function_evals", "jacobian_evals",
    "factorizations",
]


def _point(qre) -> list[float]:
    return [qre.lam] + [qre[i] for i in range(len(qre))]


def _trace(case: GoldenCase) -> dict:
    game = case.factory()
    if case.lams is None:
        profiles, stats = gbt.gambit._logit_behavior_branch(
            game, case.maxregret, FIRST_STEP, MAX_ACCEL, _with_stats=True
        )
        points = [_point(qre) for qre in profiles]
    else:
        events = []
        _, stats = gbt.gambit._logit_behavior_lambda(
            game, list(case.lams), FIRST_STEP, MAX_ACCEL, events.append, _with_stats=True
        )
        points = [_point(e.qre) for e in events if isinstance(e, gbt.LogitPathEvent)]
    return {
        "counts": [{f: getattr(s, f) for f in COUNT_FIELDS} for s in stats],
        "points": points,
    }


def _golden_path(name: str) -> pathlib.Path:
    return GOLDEN_DIR / f"{name}.json"


@pytest.mark.parametrize("name", list(CASES))
def test_logit_behavior_path_matches_golden(name: str):
    path = _golden_path(name)
    if not path.exists():
        pytest.fail(f"no reference data at {path}; run `python -m tests.test_logit_golden`")
    expected = json.loads(path.read_text())
    actual = _trace(CASES[name])
    assert actual["counts"] == expected["counts"]
    assert len(actual["points"]) == len(expected["points"])
    np.testing.assert_allclose(
        np.array(actual["points"]), np.array(expected["points"]), rtol=RTOL, atol=ATOL
    )


def _regenerate() -> None:
    GOLDEN_DIR.mkdir(parents=True, exist_ok=True)
    for name, case in CASES.items():
        data = _trace(case)
        # One point per line keeps the files compact and their diffs readable.
        lines = [
            "{",
            f' "counts": {json.dumps(data["counts"])},',
            ' "points": [',
            ",\n".join(f"  {json.dumps(point)}" for point in data["points"]),
            " ]",
            "}",
        ]
        _golden_path(name).write_text("\n".join(lines) + "\n")
        print(f"{name}: {len(data['points'])} points, counts {data['counts']}")


if __name__ == "__main__":
    _regenerate()
