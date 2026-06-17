# ACE-SFT Literature Reading Notes

This directory contains Chinese reading notes for papers that support or challenge the ACE-SFT / ACR-SFT idea.

Current method direction:

> **ACE-SFT learns where an expert should take over in a student-induced terminal failure trajectory, then distills verifier-passing expert continuations as SFT data.**

Recommended reading order:

1. `2512.14895_oec/reading.md` - closest prior work; must be understood before claiming novelty.
2. `1011.0686_dagger/reading.md` - theoretical ancestor of learner-state expert supervision.
3. `2604.00626_opd_survey/reading.md` - OPD taxonomy and motivation.
4. `2604.24005_tcod/reading.md` and `2606.15912_guided_opd/reading.md` - why multi-turn OPD can be unstable.
5. `2602.07274_termigen/reading.md` - closest terminal data synthesis baseline using injected error-correction cycles.
6. `2605.12652_mopd/reading.md` - success/failure peer rollouts as teacher context.
7. `2605.11882_fate/reading.md` - failed trajectories transformed into repair supervision.
8. `2606.06324_harnessfix/reading.md` - failed trace diagnosis and step-level attribution.
9. `2605.08083_autotts/reading.md` and `2603.28052_meta_harness/reading.md` - offline trace-driven controller/harness search methodology.

Each note follows the same format:

- Basic info
- One-sentence summary
- Problem and method
- Novel insights
- Relation to ACE-SFT
- Novelty risk
- Actionable takeaways
