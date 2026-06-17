# Plan: Anchor-Conditioned Expert Correction SFT for Terminal Agents

_Last updated: 2026-06-17_

This document is the current execution plan for the paper idea tentatively called **ACE-SFT / ACR-SFT**.

Recommended method name:

> **ACE-SFT: Anchor-Conditioned Expert Correction for Terminal-Agent Fine-Tuning**

Alternative names:

- **ACR-SFT: Anchor-Conditioned Repair SFT**
- **Anchor-Replay SFT**
- **Error-Coverage Recovery SFT**
- **Executable Replay Distillation**

Current preferred one-sentence summary:

> **ACE-SFT learns where an expert should take over in a student-induced terminal failure trajectory, then distills verifier-passing expert continuations as SFT data.**

Chinese summary:

> **ACE-SFT 学习在学生失败轨迹中的哪个状态让专家接管最有价值，然后把专家从该状态继续完成并通过 verifier 的后缀转成 SFT 数据。**

---

## 0. Important corrections from the latest discussion

This section fixes several ambiguities in earlier drafts.

### 0.1 Anchor source must match the target student

If the target model is Qwen3-32B, the main training anchors should come from **Qwen3-32B or its current checkpoint** rolled out on non-evaluation task instances.

The core method should not be described as mining anchors from arbitrary existing teacher trajectories. Existing teacher/model failures can be used for a pilot, but the main paper claim should rely on target-student failed rollouts.

Correct main pipeline:

```text
target student checkpoint
  -> rollout on non-TB2 training tasks
  -> failed trajectories
  -> turn-level anchor selection
  -> expert/teacher continuation from selected anchors
  -> verifier filtering
  -> recovery-SFT data
```

### 0.2 Do not use TB2 final evaluation tasks for training-data construction

If Terminal-Bench 2 is used as the final benchmark, we must not roll out on TB2 tasks, mine TB2 failure anchors, generate repairs, and then evaluate on TB2. That would create benchmark contamination, because the training process would have used evaluation task environments and verifier feedback.

Recommended split:

| Usage | Task sources |
|---|---|
| Anchor mining / repair data construction | MAP 5k, CLI-Gym, TerminalTraj instances, self-synthesized Harbor tasks, NVIDIA synthetic tasks, and other non-TB2 training pools |
| Dev / ablation selection | held-out subset from non-TB2 sources |
| Final evaluation | TB2 / TB-Pro / separate held-out benchmark |

### 0.3 Anchor is turn-level, not token-level

For terminal agents, an anchor should be defined at the **turn/action/state level**:

> after an environment observation and before the next assistant action.

It is not a token index inside an assistant message.

A typical anchor context is:

```text
task instruction
student command/action history
latest observation: stdout/stderr/exit_code/test output
workspace diff if available
<ANCHOR: teacher takes over here>
```

The teacher generates the next repair continuation from this point.

### 0.4 Existing multi-teacher clean-start trajectories are not repair data

The DS / Claude / GLM trajectories already collected from clean initial states are a **reference bank**, not an anchor-conditioned repair bank. They can help with baseline construction, solvability priors, reference summaries, and optional teacher routing, but they should not be treated as the main new supervision.

### 0.5 Main method can use a single strong teacher

Multi-teacher repair is optional. The cleanest main method is:

```text
target student failed rollouts + anchor selection + one strong teacher + verifier + SFT
```

Multi-teacher trajectories can remain as:

- pilot proxy data;
- reference bank;
- optional teacher-routing ablation;
- optional multi-repair diversity ablation.

Do not make naive multi-teacher SFT the core contribution.

---

## 1. Core research question

Current terminal-agent SFT mostly answers:

> How does an expert solve a task from a clean initial environment?

But deployed terminal agents often face:

> The agent has already executed wrong commands and moved the workspace into an error state. How should it recover from there?

The central hypothesis is:

> Under a fixed teacher-call and training-token budget, covering **recoverable student-induced error states** is more useful than collecting more clean expert trajectories.

The research object is therefore **error-state coverage**, not only task coverage.

---

## 2. Relation to SFT, RL, OPD, DAgger, and OEC

### 2.1 Standard SFT

Standard SFT optimizes:

\[
\mathcal{L}_{SFT}=-\sum_t \log \pi_\theta(a_t^{expert}\mid h_t^{expert})
\]

It increases expert-token probability under expert prefixes. This is stable, cheap, and scalable, but the training contexts are clean expert states.

### 2.2 RL / GRPO

RL/GRPO uses student rollouts:

\[
\tau_\theta\sim \pi_\theta
\]

and optimizes roughly:

\[
\mathcal{L}_{RL}=-\sum_t A_t\log\pi_\theta(a_t\mid h_t)
\]

If \(A_t>0\), the sampled action/token is reinforced; if \(A_t<0\), it is suppressed. This is on-policy but terminal rewards are often sparse and credit assignment is hard over long command sequences.

### 2.3 OPD

On-policy distillation lets the student generate trajectories and asks the teacher to supervise the student-generated prefixes. A sampled-token OPD signal can be written as:

\[
\log\pi_T(a_t\mid h_t^{student})-\log\pi_\theta(a_t\mid h_t^{student})
\]

OPD attacks the same distribution-shift problem as ACE-SFT, but it often relies on token-level KL/logprob feedback over many student prefixes. In long-horizon terminal tasks, this can be expensive and may not provide a concrete recovery path from a corrupted workspace state.

### 2.4 DAgger and OEC

DAgger is the classical imitation-learning ancestor: roll out the learner, query the expert on learner-visited states, and aggregate those examples into the training set.

OEC (On-policy Expert Corrections) is the closest LLM-agent precedent: start the rollout with a student model, then switch to an expert model partway through the trajectory, producing partially on-policy expert correction data.

Our intended novelty should therefore not be "student begins, teacher continues" in general. That is already very close to OEC. The sharper novelty is:

> **terminal-specific, verifier-grounded, anchor-conditioned expert correction**: we learn/select which turn-level failure states are worth expert intervention, use executable evidence such as stderr/tests/diffs/verifier feedback, and optimize repair-data yield under a teacher-call budget.

---

## 3. Method: ACE-SFT

### 3.1 Inputs

- A target student checkpoint to improve.
- Non-TB2 task pool for rollout and data construction.
- Strong teacher model(s), preferably one primary teacher first.
- Verifier/test infrastructure.
- Optional clean-start teacher trajectory bank as reference.

### 3.2 Output datasets

- \(D_{expert}\): clean expert SFT data.
- \(D_{fail}\): failed target-student rollouts on training tasks.
- \(D_{anchor}\): selected turn-level candidate anchors.
- \(D_{repair}\): verifier-passing teacher continuations from selected anchors.
- \(D_{train}\): final SFT mixture.

### 3.3 High-level pipeline

1. Roll out the **target student** on non-TB2 train tasks.
2. Keep failed trajectories and convert them into turn-level records.
3. Propose candidate anchors with high-recall execution-event detectors.
4. Build anchor profiles.
5. Select anchors by repairability, teachability, cost, and error coverage.
6. Ask teacher to generate K repair continuations from selected anchors.
7. Execute verifier and keep successful repairs.
8. Train with ordinary SFT: failed prefix/profile as context, teacher repair suffix as target, bad prefix masked from loss.

---

## 4. Anchor definition and scorer

### 4.1 Candidate anchor

An anchor is a turn-level takeover point:

\[
a = (H_t, o_t, m_t)
\]

where:

- \(H_t\): task instruction and command/action history up to the current turn;
- \(o_t\): latest environment observation;
- \(m_t\): metadata such as anchor type, failure type, diff, verifier info.

### 4.2 Candidate proposal is not the contribution

Candidate proposal can use cheap rules for high recall:

- after first critical stderr;
- after first failed test following an edit;
- after dependency/path/permission error;
- before or at repeated no-progress loop;
- at last meaningful state before timeout;
- after final verifier failure observation;
- after divergence from a successful reference trajectory, if available.

These rules are not the core contribution. They are only a proposal layer. The core research question is whether we can identify **which** proposed anchors are worth teacher intervention.

### 4.3 Anchor profile

For each candidate anchor, build a compact teacher-facing profile:

```text
Task objective
Useful progress so far
Recent commands
Last student action
Latest stdout/stderr/test output
Relevant file diff if available
Verifier feedback if available
Suspected failure type
What should not be repeated
Optional summary of a successful reference trajectory
```

### 4.4 Anchor scorer

The scorer input is a candidate anchor profile, or a trajectory slice with an explicit `<ANCHOR>` marker.

The scorer output is a scalar:

\[
q_\phi(a)=P(\text{teacher repair from anchor }a\text{ will pass verifier and be useful})
\]

Labels are generated by teacher repair attempts and verifier execution:

\[
y(a)=\frac{1}{K}\sum_{k=1}^{K}\mathbb{1}[Verifier(Repair_k(a))=1]
\]

The scorer can be trained with binary/soft-label cross entropy:

\[
\mathcal{L}_{anchor}=-y\log q_\phi(a)-(1-y)\log(1-q_\phi(a))
\]

Selection score:

\[
Score(a)=q_\phi(a)+\alpha\Delta Coverage(a)-\beta Cost(a)-\gamma Risk(a)
\]

where coverage measures whether the anchor covers underrepresented error buckets.

---

## 5. Teacher repair

### 5.1 Primary repair path

Use one strong teacher first for the main method. Examples:

- Qwen3-72B if open-weight reproducibility/logprobs are important;
- Claude/DeepSeek if completion quality is more important;
- Qwen3-32B self-repair only as a self-training or weaker ablation.

### 5.2 Sampling

For each selected anchor, sample K repairs:

\[
\rho_1,\ldots,\rho_K\sim\pi_T(\cdot\mid P_a)
\]

Run each repair in the environment and keep only verifier-passing continuations:

\[
D_{repair}=\{(P_a,\rho_k): Verifier(Replay(a),\rho_k)=1\}
\]

### 5.3 Closed-source vs open-source teacher

Closed-source API teachers may only provide completions. That is sufficient for SFT.

Open-source teachers may additionally provide token logprobs or entropy, which can support optional weighting, but logprobs are not required for the first version.

---

## 6. Training objective

The final objective is ordinary weighted SFT:

\[
\mathcal{L}=\mathcal{L}_{expert}+\lambda\mathcal{L}_{repair}
\]

with:

\[
\mathcal{L}_{repair}=-\sum_{(P,\rho)\in D_{repair}}w(P,\rho)\sum_i\log\pi_\theta(\rho_i\mid P,\rho_{<i})
\]

The failed prefix/profile is context. Only the teacher repair suffix is supervised.

From the token-gradient perspective, the update teaches:

> under a failed-state context, increase the probability of verified repair actions instead of repeating the bad trajectory.

---

## 7. Error coverage

Error-state coverage is the proposed scientific lens.

Bucket failures by:

1. **Failure type**: wrong file, bad path, dependency error, failed test, loop, timeout, wrong verifier use, bad test interpretation.
2. **Evidence type**: stderr, test output, file diff, hidden verifier failure, no-progress signal.
3. **Recovery operation**: inspect, revert, repair edit, rerun test, fix dependency, change strategy, verify.
4. **Anchor type**: first stderr, first failed test, loop start, final meaningful state, divergence from success reference.

Coverage objective:

\[
Coverage(D)=\sum_{b\in\mathcal{B}}w_b\min\left(\frac{count_D(b)}{target_b},1\right)
\]

Marginal coverage gain:

\[
\Delta Coverage(a)=Coverage(D\cup\{a\})-Coverage(D)
\]

---

## 8. What exactly should early experiments prove?

### Experiment 1: Anchor selection improves repair yield

This is a data-construction efficiency experiment.

Given the same number of teacher calls, compare how many verifier-passing repairs are obtained by different anchor policies:

| Anchor policy | Metric |
|---|---|
| random candidate anchor | verified repairs per 100 teacher calls |
| final failed state | verified repairs per 100 teacher calls |
| first stderr | verified repairs per 100 teacher calls |
| first failed test | verified repairs per 100 teacher calls |
| learned/scored anchor | verified repairs per 100 teacher calls |

If scored anchors produce more verified repairs, anchor selection is useful.

### Experiment 2: Anchor-conditioned repairs improve SFT

Compare equal-token SFT groups:

1. clean expert only;
2. expert + raw failed traces;
3. expert + final-state repairs;
4. expert + random-anchor repairs;
5. expert + scored-anchor repairs;
6. optional: expert + scored-anchor repairs with profile compression.

Evaluate not only final pass rate but also recovery behavior:

- timeout rate;
- repeated command ratio;
- stderr utilization;
- test usage;
- wrong-file edit rate;
- recovery-after-first-error;
- failure-type-specific improvement.

### Experiment 3: Anchor scorer feasibility

Train a turn-level scorer from 100-1000 labeled anchors.

Metrics:

- AUC / accuracy for repair success prediction;
- precision@top-k;
- teacher calls saved per verified repair;
- coverage diversity among top-k selected anchors.

---

## 9. Minimal implementation plan

### Stage A: Task and data split

1. Identify non-TB2 train task pool.
2. Identify dev task pool.
3. Keep TB2 only for final evaluation.

### Stage B: Target-student failed rollouts

1. Choose target checkpoint.
2. Roll out on 200-500 non-TB2 tasks.
3. Store full traces.
4. Split success/failure.

### Stage C: Anchor candidate mining

1. Convert traces into turn-level records.
2. Generate candidates with high-recall rules.
3. Build anchor profiles.
4. Produce `anchor_candidates.jsonl` and `sample_anchor_profiles.jsonl`.

### Stage D: Teacher repair pilot

1. Sample 100-300 candidate anchors.
2. For each anchor, sample K repairs from a strong teacher.
3. Execute verifier.
4. Save `repair_attempts.jsonl` and `verified_repairs.jsonl`.

### Stage E: Anchor scorer pilot

1. Train simple baseline scorer.
2. Compare top-k selection with random/final/first-stderr baselines.
3. Estimate verified-repair yield.

### Stage F: Small SFT ablation

1. Build small repair SFT dataset.
2. Train one or two small models first.
3. Compare expert-only vs repair-enhanced variants.

---

## 10. Closest related work and positioning

### DAgger

DAgger shows the classical issue: in sequential prediction, learner actions change the future observation distribution. It addresses this by querying experts on learner-visited states and aggregating the resulting data.

Our relation:

> ACE-SFT is DAgger-like in motivation, but the expert query is selective, turn-level, verifier-grounded, and produces a repair continuation rather than a single action label.

### OEC

OEC is the closest LLM-agent work. It starts rollouts with a student and switches to an expert partway through, producing partially on-policy expert correction trajectories for SWE tasks.

Our intended distinction:

> ACE-SFT studies how to choose the takeover point in terminal failures, labels anchor usefulness with teacher repair + verifier execution, and optimizes error-state coverage and teacher-call efficiency.

### OPD / OPD Survey

OPD provides dense teacher feedback on student-generated prefixes, often with KL/logprob signals. It motivates why student-induced states matter.

Our distinction:

> ACE-SFT does not do online KL over every prefix. It performs selective state-level expert correction and trains on verified repair suffixes with standard SFT.

### TCOD / Guided-OPD / OPD failure analyses

These works show that multi-turn OPD can suffer from prefix drift, compounding errors, and unreliable teacher signals.

Our relation:

> ACE-SFT responds by not supervising every drifted prefix. It identifies recoverable anchors and asks the teacher to generate a concrete repair path.

### MOPD

MOPD uses successful and failed peer rollouts to construct more informative teacher signals.

Our relation:

> Multi-rollout information can be used in anchor profiles or reference summaries, but the main target is anchor-conditioned verified repair.

### TermiGen

TermiGen injects errors to synthesize error-correction cycles for terminal agents.

Our distinction:

> TermiGen creates correction cycles by error injection; ACE-SFT mines failure states from the target student and selects repairable anchors.

### FATE

FATE transforms verifier-scored failure trajectories into repair supervision for agentic safety alignment.

Our relation:

> It supports the broad principle that failed trajectories can become repair supervision, but ACE-SFT is focused on terminal task solving, teacher correction, anchor selection, and error coverage.

### HarnessFix

HarnessFix diagnoses failed trajectories and repairs harness flaws.

Our relation:

> It validates step-level trajectory diagnosis as important, but ACE-SFT uses diagnosis to construct training data rather than patching harnesses.

### AutoTTS / Meta-Harness

AutoTTS and Meta-Harness support the broader methodological idea of using pre-collected traces, execution feedback, and search/selection policies instead of hand-designed heuristics alone.

Our relation:

> ACE-SFT uses offline trajectory traces to search/select data-construction interventions, not inference-time controllers or harness patches.

---

## 11. Coding-agent prompt for the next pilot

```text
You are working in the tb2-terminal-agent-scaling repository.

Goal:
Prototype ACE-SFT / ACR-SFT. The goal is to validate whether target-student failed rollouts on non-TB2 tasks contain turn-level anchors from which a strong teacher can generate verifier-passing repair continuations.

Output directory:
analysis_reports/ace_sft_<timestamp>/

Critical constraints:
- Do not use Terminal-Bench 2 final evaluation tasks for training-data construction.
- Use non-TB2 task pools first: MAP 5k, CLI-Gym, TerminalTraj instances, self-synthesized Harbor tasks, or NVIDIA synthetic tasks.
- If target model is Qwen3-32B, collect failures from Qwen3-32B or its current checkpoint when possible.
- Existing DS/Claude/GLM trajectories may be used as pilot/reference data, but do not treat them as anchor-conditioned repairs.
- Do not print secrets/API keys.
- If a job takes more than 10 minutes, use tmux and log the session name.

Tasks:
1. Identify train/dev/test task pools and explicitly mark TB2 as evaluation-only.
2. Locate target-student rollout logs or prepare a dry-run rollout plan on 200-500 non-TB2 tasks.
3. Parse failed rollouts into turn-level records.
4. Generate candidate anchors after observations, not token-level positions.
5. Build compact anchor profiles.
6. Sample 100-300 anchors across failure buckets.
7. Prepare dry-run teacher repair prompts.
8. If teacher calls are enabled, sample K repairs per anchor and verify them.
9. Train or simulate a simple anchor scorer from repair outcomes.
10. Report verified-repair yield by anchor policy.

Deliverables:
- task_split_report.md
- trajectory_inventory.md
- failed_turn_records.jsonl
- anchor_candidates.jsonl
- sample_anchor_profiles.jsonl
- repair_prompt_dryrun.jsonl
- repair_attempts.jsonl, if teacher calls are enabled
- verified_repairs.jsonl, if verifier runs are enabled
- anchor_policy_yield_report.md
- next_step_sft_ablation_plan.md
```

---

## 12. What counts as positive signal?

A small pilot is positive if:

1. target-student failed rollouts have enough structured observations to build anchor profiles;
2. selected anchors yield substantially more verifier-passing repairs than random/final-state anchors;
3. verified repairs are clean enough to become SFT data;
4. a small SFT ablation shows better recovery behavior than expert-only or raw-failure mixing.

Even if final pass-rate gain is small at first, strong repair-yield improvement is already useful: it means anchor selection improves the quality/cost ratio of synthetic recovery data.

---

## 13. What to avoid

- Do not claim we invented the general idea of student-prefix expert continuation; OEC is very close.
- Do not use TB2 failure trajectories as training data if TB2 is final evaluation.
- Do not define anchors at token-level for terminal command agents; use turn-level takeover points.
- Do not treat clean-start teacher trajectories as repair suffixes.
- Do not make naive multi-teacher SFT the main contribution.
- Do not train on raw failed traces as positive supervision.

---

## 14. Updated contribution statement

A publication-ready contribution statement should be:

1. **Anchor-conditioned expert correction for terminal agents.** We formulate where an expert should take over in a student-induced terminal failure trajectory as a learnable/verifier-grounded selection problem.
2. **Error-state coverage.** We argue that terminal-agent SFT should cover recoverable error states, not only clean task solutions.
3. **Efficient recovery-data construction.** We show that anchor scoring improves verifier-passing repair yield under a fixed teacher-call budget and that these repairs can be distilled with standard SFT.

---

## 15. Updated abstract sketch

Terminal agents are commonly trained by supervised fine-tuning on expert trajectories collected from clean initial environments. While effective, such data under-covers the executable error states that agents enter after their own commands. Inspired by interactive imitation learning and on-policy expert correction, we propose ACE-SFT, an anchor-conditioned data construction framework for terminal-agent fine-tuning. ACE-SFT rolls out the target student on non-evaluation tasks, identifies turn-level failure anchors where expert intervention is likely to be useful, asks a stronger teacher to continue from those anchors, and retains only verifier-passing repair continuations as SFT targets. Unlike standard OPD, ACE-SFT does not require token-level online KL supervision; unlike raw failure mixing, it masks the failed prefix and trains only on verified repairs. We study whether error-state coverage and anchor selection improve the efficiency of terminal-agent SFT under fixed teacher-call and training-token budgets.
