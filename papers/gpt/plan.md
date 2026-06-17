# Plan: Anchor-Conditioned Repair SFT for Terminal Agents

_Last updated: 2026-06-17_

This document turns the current brainstorm into an execution plan for fast validation experiments. The goal is to give the server-side coding/research agent a concrete target: build the minimum viable pipeline, run small predictive experiments, and decide whether the idea should be scaled.

The method is currently named:

> **Anchor-Conditioned Repair SFT (ACR-SFT)**

Alternative short names:

- **Anchor-Replay SFT**
- **Error-Coverage Recovery SFT**
- **Executable Replay Distillation**

Recommended paper-style title:

> **Anchor-Conditioned Repair SFT: Efficient Terminal-Agent Supervision from Replayable Failure States**

---

## 1. Core insight

Current terminal-agent SFT pipelines mostly learn from clean expert trajectories. These trajectories answer:

> How does an expert solve the task from a clean initial environment?

But deployed terminal agents often face a different situation:

> The agent has already executed some wrong commands and moved the workspace into an error state. How should it recover from there?

The core hypothesis is:

> Under a fixed teacher-call and training-token budget, covering **recoverable model-induced error states** can be more data-efficient than collecting more clean expert trajectories.

The proposed method does not train directly on failed trajectories. Instead, it finds **teachable anchors** inside failed rollouts, asks a teacher to repair from those anchors, verifies the repair, and trains on the verified repair suffix.

---

## 2. What problem are we solving?

### 2.1 SFT under-covers deployment states

Standard SFT optimizes:

\[
\mathcal{L}_{SFT}=-\sum_t \log \pi_\theta(a_t^{expert}\mid h_t^{expert})
\]

It increases the probability of expert tokens under expert prefixes. This is stable and efficient, but the training contexts are clean expert states.

At deployment time, the model visits:

\[
h_t^{student}\neq h_t^{expert}
\]

For terminal agents, the mismatch is stronger than ordinary text generation because earlier commands change the workspace:

- wrong file edits;
- failed tests;
- corrupted configs;
- dependency/path errors;
- repeated no-progress loops;
- timeout states;
- misleading stdout/stderr;
- verifier failures after partial progress.

Therefore, more clean trajectories may not teach the model how to recover from its own mistakes.

### 2.2 RL is on-policy but sparse and expensive

RL/GRPO uses student rollouts:

\[
\tau_\theta\sim \pi_\theta
\]

and optimizes roughly:

\[
\mathcal{L}_{RL}=-\sum_t A_t\log\pi_\theta(a_t\mid h_t)
\]

If \(A_t>0\), the sampled token/action is reinforced. If \(A_t<0\), it is suppressed. This is on-policy, but terminal tasks often provide only sparse final rewards. Credit assignment over long command sequences is difficult and rollout cost is high.

### 2.3 OPD is useful but too broad for terminal repair

On-policy distillation (OPD) asks the student to roll out first, then uses a teacher to score or supervise student-generated prefixes. A simplified sampled-token OPD signal is:

\[
\log\pi_T(a_t\mid h_t^{student})-\log\pi_\theta(a_t\mid h_t^{student})
\]

This gives dense teacher feedback on student states. However, in long-horizon terminal tasks, student mistakes can move the environment into a drifted executable state. Token-level KL on every prefix may be expensive and may not provide a concrete recovery path.

ACR-SFT borrows the core motivation of OPD, but changes the supervision unit:

> OPD asks whether each student token is teacher-like. ACR-SFT asks where the teacher should intervene and how to recover from that state.

---

## 3. Method overview

ACR-SFT has five stages:

1. **Collect or reuse failed rollouts** from current students or existing model trajectories.
2. **Mine candidate anchors** inside each failed trajectory.
3. **Score anchors** by recoverability, teachability, replayability, and error coverage.
4. **Ask a teacher to repair from selected anchors**, optionally using an anchor profile.
5. **Verify and train** on repair suffixes with ordinary SFT.

The final training loss is:

\[
\mathcal{L}=\mathcal{L}_{expert}+\lambda\mathcal{L}_{repair}
\]

where:

\[
\mathcal{L}_{repair}=-\sum_{(P,\rho)\in D_{repair}}w(P,\rho)\sum_i\log\pi_\theta(\rho_i\mid P,\rho_{<i})
\]

- \(P\): anchor profile, constructed from the student/model failed state.
- \(\rho\): verified teacher repair suffix.
- \(w(P,\rho)\): optional weight based on anchor quality, repair success, diversity, and cost.

The failed prefix is retained as context but masked from loss. The model learns the repair suffix, not the bad action.

---

## 4. Key definitions

### 4.1 Failed rollout

A terminal-agent rollout is:

\[
\tau=(s_0,a_0,o_1,s_1,a_1,o_2,\ldots,s_T)
\]

where:

- \(s_t\): terminal/workspace state;
- \(a_t\): command/action;
- \(o_{t+1}\): stdout, stderr, exit code, test output, verifier feedback;
- final verifier result indicates success/failure.

### 4.2 Anchor

An anchor is a selected intermediate state \(s_k\) inside a failed rollout.

It is not necessarily the first wrong action or the final failed state. It should be a **teachable failure state**:

- error evidence is visible;
- recovery is still possible;
- repair from this point is likely to teach reusable behavior;
- the state can be replayed or summarized;
- the failure type contributes to error coverage.

### 4.3 Anchor profile

An anchor profile is a compressed representation of the state given to the teacher:

```text
Task objective
Useful progress so far
Recent commands
Last failed command
stdout/stderr/test output
Relevant file diff
Suspected failure type
What should not be repeated
Verifier feedback if available
Optional reference summary from successful teacher trajectory
```

The profile avoids sending a long noisy transcript and makes teacher repair cheaper and more stable.

### 4.4 Repair suffix

A repair suffix is a teacher-generated continuation from the selected anchor state/profile. It is only used as training data if executing the suffix passes the verifier.

---

## 5. Role of existing multi-teacher trajectories

We already have trajectories from several teachers such as DeepSeek, Claude, and GLM. These trajectories are important, but their role must be stated carefully.

### 5.1 What they are

They are a **multi-teacher clean-start trajectory bank**:

\[
\mathcal{B}_T(x)=\{\tau_{DS}(x),\tau_{Claude}(x),\tau_{GLM}(x)\}
\]

### 5.2 What they are not

They are not anchor-conditioned repair trajectories. A clean-start teacher trajectory usually cannot be appended to a student failed prefix because the workspace states may differ.

### 5.3 Correct usage

Use the existing teacher trajectories as:

1. **Expert-only baseline data**: successful clean-start trajectories for ordinary SFT.
2. **Solvability prior**: if any teacher solves a task, the task is likely solvable.
3. **Recoverability prior**: if the student fails but a teacher succeeds on the same task, the failure state may be worth repairing.
4. **Reference signal for anchor mining**: compare failed traces with successful traces to find divergence points.
5. **Teacher routing signal**: historical teacher success can decide which teacher to call for a selected anchor.
6. **Repair diversity source**: for high-value anchors, multiple teachers can generate repair candidates and verifier selects the best.

Avoid making naive multi-teacher SFT the core method. Mixing teacher trajectories can introduce inconsistent styles and action priors. It is useful as a baseline or analysis, not necessarily as the main contribution.

---

## 6. Error coverage

The main research object is not just task coverage, but **error-state coverage**.

A failure state can be bucketed by:

1. **Failure type**
   - command syntax error;
   - wrong path;
   - dependency/setup failure;
   - permission error;
   - wrong file edit;
   - bad test interpretation;
   - insufficient inspection;
   - repeated no-progress loop;
   - timeout after partial progress;
   - verifier overfitting or shortcut.

2. **Evidence type**
   - stderr;
   - failing test;
   - hidden verifier failure;
   - file diff inconsistency;
   - runtime exception;
   - assertion failure;
   - no visible error but no progress.

3. **Recovery operation**
   - inspect relevant files;
   - revert or repair wrong edit;
   - fix dependency;
   - rerun tests;
   - localize failing function;
   - compare expected vs actual output;
   - clean generated artifacts;
   - change strategy after loop.

4. **Anchor type**
   - first critical stderr;
   - first failed test after edit;
   - repeated-loop start;
   - final verifier failure;
   - divergence from successful teacher reference;
   - last meaningful state before timeout.

A simple coverage objective:

\[
Coverage(D)=\sum_{b\in\mathcal{B}}w_b\min\left(\frac{count_D(b)}{target_b},1\right)
\]

When choosing anchors, prefer high marginal coverage gain:

\[
\Delta Coverage(a)=Coverage(D\cup\{a\})-Coverage(D)
\]

This creates a research question:

> Does error-state coverage explain SFT improvement better than raw trajectory count?

---

## 7. Anchor scorer

A small anchor scorer can be trained from 100-1000 labeled anchors.

### 7.1 Label generation

For each candidate anchor:

1. Build anchor profile.
2. Ask teacher to generate K repair candidates.
3. Execute each repair.
4. Label anchor positive if any repair passes verifier.

\[
y(a)=\mathbb{1}[\exists k\le K, Verifier(Repair_k(a))=1]
\]

### 7.2 Scorer objective

Train:

\[
q_\phi(a)=P(y(a)=1\mid a)
\]

with binary cross entropy:

\[
\mathcal{L}_{anchor}=-y\log q_\phi(a)-(1-y)\log(1-q_\phi(a))
\]

### 7.3 Selection score

Use:

\[
Score(a)=q_\phi(a)+\alpha\Delta Coverage(a)-\beta Cost(a)-\gamma Risk(a)
\]

where:

- \(q_\phi(a)\): predicted repairability/teachability;
- \(\Delta Coverage(a)\): error coverage gain;
- \(Cost(a)\): teacher call / replay / context cost;
- \(Risk(a)\): nondeterminism, missing metadata, irreversible workspace damage.

---

## 8. Pseudocode

### 8.1 Standard SFT

```python
for batch in expert_data:
    loss = 0
    for example in batch:
        for prefix, token in example.teacher_tokens:
            loss += -logprob(student, token, prefix)
    update(student, loss)
```

Learns: increase expert-token probability under expert prefixes.

### 8.2 RL / GRPO

```python
for tasks in task_batch:
    groups = []
    for task in tasks:
        rollouts = [student.rollout(task) for _ in range(G)]
        rewards = [verifier(r) for r in rollouts]
        advantages = group_normalize(rewards)
        groups.append((rollouts, advantages))

    loss = 0
    for rollouts, advantages in groups:
        for rollout, A in zip(rollouts, advantages):
            for prefix, token in rollout.student_tokens:
                loss += -A * logprob(student, token, prefix)
    update(student, loss)
```

Learns: reinforce or suppress student-sampled tokens using reward-derived advantage.

### 8.3 Standard sampled-token OPD

```python
for task in task_batch:
    rollout = student.rollout(task)
    loss = 0
    for prefix, token in rollout.student_tokens:
        student_logp = logprob(student, token, prefix)
        teacher_logp = logprob(teacher, token, prefix)
        reverse_kl = student_logp - teacher_logp
        loss += reverse_kl
    update(student, loss)
```

Learns: make student-sampled tokens more teacher-like under student prefixes.

### 8.4 ACR-SFT

```python
D_expert = extract_successful_clean_trajectories(teacher_bank)
D_failure = collect_or_reuse_failed_rollouts(student_or_models)
D_repair = []

# Pilot: label anchors
anchor_labels = []
for trace in sample(D_failure):
    candidates = propose_anchors(trace)
    for anchor in candidates:
        profile = build_anchor_profile(anchor)
        repairs = [teacher.generate(profile) for _ in range(K)]
        success = any(run_verifier(profile.env, repair) for repair in repairs)
        anchor_labels.append((anchor, success))

anchor_scorer = train_anchor_scorer(anchor_labels)

# Data construction
for trace in D_failure:
    candidates = propose_anchors(trace)
    scored = []
    for anchor in candidates:
        score = anchor_scorer(anchor) + coverage_gain(anchor) - replay_cost(anchor)
        scored.append((score, anchor))

    for anchor in select_topk(scored, budget=k):
        state_or_profile = replay_or_profile(anchor)
        teacher = route_teacher(anchor, teacher_bank)
        repairs = sample_repairs(teacher, state_or_profile, K=K)
        for repair in repairs:
            if run_verifier(state_or_profile, repair):
                D_repair.append(package_recovery_sft(anchor, repair))

# SFT training
D_train = mix(D_expert, D_repair, strategy="coverage_balanced")
train_sft(student, D_train)
```

Learns: under failed-state contexts, increase the probability of verified teacher repair suffixes.

---

## 9. Why this should work

### 9.1 From the token-gradient perspective

SFT/RL/OPD all change next-token distributions. For a simplified weighted SFT loss:

\[
\mathcal{L}=-w\log\pi_\theta(y\mid h)
\]

If \(w>0\), gradient descent increases the logit/probability of token \(y\) under context \(h\).

ACR-SFT chooses contexts \(h\) from failed model-induced states and chooses targets \(y\) from verified teacher repair suffixes. Therefore, the update specifically teaches:

> When the model sees a similar failed state, increase the probability of repair actions rather than repeating the bad action.

### 9.2 Why it may beat clean SFT

Clean SFT improves behavior on expert-like prefixes. ACR-SFT improves behavior on failure-like prefixes. If evaluation failures are caused by missing recovery behavior, repair data should be more efficient than adding more clean successes.

### 9.3 Why it may beat raw failure mixing

Raw failed trajectories contain bad actions and dead ends. ACR-SFT uses failed prefixes only as context, while training targets are verifier-passing repairs.

### 9.4 Why it may beat full OPD in our setting

Full OPD scores every student prefix/token, which is costly and potentially unstable in long terminal trajectories. ACR-SFT uses selective intervention and teacher-generated repair continuations, giving the model an explicit path out of the error state.

---

## 10. Fast validation experiments

These experiments are meant to be predictive, not final SOTA experiments.

### Experiment A: Data availability and replay feasibility

Goal: determine whether existing logs support anchor mining.

Tasks:

1. Parse existing trajectories.
2. Split success/failure.
3. Convert failed traces into step-level records.
4. Check which records have command history, stdout/stderr, exit code, tests, verifier, and Docker image.
5. Estimate prefix replay feasibility.

Deliverables:

- `trajectory_inventory.md`
- `trajectory_summary.csv`
- `failed_trace_index.jsonl`
- `replay_feasibility_report.md`

Success criterion:

- At least hundreds of failed traces have enough information for anchor profiles.
- A nontrivial subset can be replayed or approximated by transcript/profile.

### Experiment B: Anchor candidate mining

Goal: mine candidate anchors without teacher calls.

Anchor policies:

1. first critical stderr;
2. first failed test after edit;
3. repeated-loop start;
4. dependency/path/permission error;
5. last meaningful state before timeout;
6. final verifier failure;
7. divergence from successful teacher reference.

Deliverables:

- `anchor_candidates.jsonl`
- `anchor_bucket_distribution.csv`
- `anchor_examples.md`
- `error_coverage_report.md`

Success criterion:

- Candidate anchors cover multiple failure buckets.
- Manual/LLM inspection suggests many anchors are teachable.

### Experiment C: Teacher repair pilot

Goal: test whether selected anchors can be repaired.

Plan:

1. Sample 100-300 anchors across failure buckets.
2. Build anchor profiles.
3. Call one teacher first, then optionally multiple teachers for high-value anchors.
4. Execute/verify repairs where possible.

Baselines:

- random failed state;
- final failed state;
- first stderr;
- first failed test;
- anchor-score selected.

Metrics:

- teacher repair success rate;
- verifier pass rate;
- average repair length;
- teacher calls per successful repair;
- bucket-wise repair rate;
- profile quality;
- restart ratio;
- repeated bad action ratio.

Deliverables:

- `teacher_repair_pilot.md`
- `repair_attempts.jsonl`
- `verified_repairs.jsonl`
- `repair_success_by_anchor_policy.csv`

Success criterion:

- Anchor-selected repair has higher success rate than random/final-state repair.
- Enough verified repairs can be produced for a small SFT ablation.

### Experiment D: Anchor scorer pilot

Goal: train a small model/classifier to predict useful anchors.

Inputs:

- 100-1000 labeled anchors from Experiment C.

Candidate models:

- rule-based score;
- embedding + logistic regression;
- small LLM judge SFT;
- Qwen 7B-style classifier if available.

Metrics:

- AUC/accuracy for repair success prediction;
- precision@top-k;
- coverage diversity among selected top-k anchors;
- teacher calls saved per verified repair.

Deliverables:

- `anchor_scorer_report.md`
- `anchor_scorer_predictions.csv`
- `topk_anchor_selection.jsonl`

Success criterion:

- Top-k anchors selected by scorer are more repairable than heuristic/random anchors.

### Experiment E: Small SFT ablation

Goal: test whether verified repair data improves a student.

Training groups under equal token budget:

1. `expert_only`: clean successful trajectories only.
2. `expert_plus_raw_fail`: clean successes + raw failed trajectories.
3. `expert_plus_random_repair`: clean successes + teacher repairs from random/final anchors.
4. `expert_plus_acr_repair`: clean successes + anchor-selected verified repairs.
5. `expert_plus_acr_profile`: same but using anchor profiles instead of full transcripts.
6. optional: `expert_plus_acr_weighted`: repair examples weighted by anchor quality/coverage.

Evaluation:

- TB2 pass rate or subset pass rate;
- held-out Harbor tasks;
- timeout rate;
- repeated command ratio;
- stderr utilization;
- test usage;
- recovery-after-first-failure metric;
- failure-type-specific improvement.

Deliverables:

- `sft_ablation_plan.md`
- `sft_data_manifest.json`
- `eval_results.csv`
- `ablation_report.md`

Success criterion:

- `expert_plus_acr_repair` improves over expert-only and raw-failure mixing.
- Even small positive trend is enough to justify scaling.

---

## 11. What to do if first results are weak

Weak early results do not necessarily invalidate the idea. Likely failure modes and fixes:

### 11.1 Low teacher repair success

Possible causes:

- anchors selected too late;
- state is unrecoverable;
- profile misses key files/tests;
- teacher prompt allows restart or gets confused;
- verifier/replay is unstable.

Fixes:

- select earlier anchors;
- include file diff and failing test snippets;
- use matched successful teacher trajectory summary;
- restrict teacher to inspect before edit;
- allow multiple repair samples;
- use stronger teacher only for hard buckets.

### 11.2 Repair data does not improve SFT

Possible causes:

- repair suffixes too long/noisy;
- too few examples;
- bad prefix leaks into loss;
- repair distribution too teacher-specific;
- repair tasks overlap poorly with evaluation.

Fixes:

- mask bad prefix strictly;
- train only command/action tokens;
- add critique+repair format;
- filter by repair length and test usage;
- balance by failure type;
- mix with strong expert data;
- evaluate failure behavior, not only final pass rate.

### 11.3 Anchor scorer does not work

Possible causes:

- labels too noisy;
- features insufficient;
- too few anchors;
- random teacher sampling makes labels unstable.

Fixes:

- use multiple repair attempts per anchor;
- define soft label: repair success rate over K attempts;
- add deterministic features: stderr type, test failure, diff size, loop score;
- use LLM judge as auxiliary label;
- train simpler bucket-level selector first.

### 11.4 Multi-teacher repair conflicts

Possible causes:

- teachers use inconsistent styles;
- multiple correct repairs differ in setup assumptions;
- repair suffixes are not normalized.

Fixes:

- use verifier selection;
- normalize output format;
- teacher routing instead of naive mixture;
- diversity-select only among verified repairs;
- keep teacher id metadata for ablation.

---

## 12. Scaling plan if pilot is positive

If Experiment C/E is positive:

1. Scale anchor mining to all available failed rollouts.
2. Train anchor scorer on 1k-5k labeled anchors.
3. Generate 5k-20k verified repairs.
4. Build three training mixtures:
   - expert-heavy;
   - balanced expert/repair;
   - repair-heavy by failure bucket.
5. Train 7B/8B model first.
6. Evaluate on TB2, CLI-Gym-derived held-out tasks, and self-synthesized Harbor tasks.
7. Add second iteration:
   - rollout improved student;
   - mine new failures;
   - generate second-round repairs;
   - train again.

---

## 13. Paper positioning

### 13.1 Main claim

> Terminal-agent SFT should optimize not only task coverage, but error-state coverage. ACR-SFT identifies where teachers should intervene in failed student trajectories and turns verifier-passing repairs into efficient SFT supervision.

### 13.2 Contributions

1. **Problem formulation**: introduce error-state coverage as a missing dimension in terminal-agent SFT data construction.
2. **Method**: propose anchor-conditioned repair synthesis with anchor mining/scoring, teacher routing, verifier filtering, and recovery-SFT packaging.
3. **Empirical study**: compare clean expert SFT, raw failure mixing, injected-error repair, naive repair anchors, and coverage-aware anchor-conditioned repairs under equal budgets.

### 13.3 Key comparisons

- **Clean SFT**: learns expert behavior from clean starts but misses model-induced error states.
- **RL/GRPO**: uses student states but sparse rewards and high rollout cost.
- **OPD**: dense teacher feedback on student prefixes but costly and potentially unstable for long terminal trajectories.
- **TermiGen-style injected repair**: creates correction cycles, but errors may not match the student's actual failure distribution.
- **ACR-SFT**: selective teacher intervention at recoverable failed states, verified repair suffixes, ordinary SFT training.

### 13.4 Publication-ready abstract sketch

Terminal agents are commonly trained by supervised fine-tuning on expert trajectories collected from clean initial environments. While effective, this training distribution under-covers the executable error states that agents enter after their own commands, such as wrong-file edits, failed tests, dependency errors, and repeated no-progress loops. We propose Anchor-Conditioned Repair SFT, a recovery-oriented data construction framework that identifies teachable anchors inside failed rollouts, asks a stronger teacher to repair from these states, verifies the repairs by execution, and distills the resulting repair suffixes with standard SFT. Unlike full on-policy distillation, our method does not require online teacher KL supervision; unlike raw failure mixing, it trains only on verifier-passing recovery continuations. We study whether covering recoverable error states improves terminal-agent robustness more efficiently than adding clean expert trajectories under the same teacher-call and token budget.

---

## 14. Instructions for a coding agent

Use the following prompt to start implementation.

```text
You are working in the tb2-terminal-agent-scaling repository.

Goal:
Prototype Anchor-Conditioned Repair SFT (ACR-SFT) for terminal agents. The purpose is not to run a full training pipeline yet, but to validate whether failed trajectories contain teachable anchors that can be repaired by teachers and converted into SFT data.

Output directory:
analysis_reports/acr_sft_<timestamp>/

Constraints:
- Do not modify existing datasets or training code.
- Do not print secrets/API keys.
- If a job takes more than 10 minutes, use tmux and log the session name.
- Prefer dry-run scripts first.
- Keep all scripts rerunnable.

Tasks:
1. Locate trajectory logs and datasets.
   - MAP 5k teacher/model trajectories.
   - CLI-Gym trajectories.
   - Harbor/TB2 rollout logs.
   - Any existing student failed rollouts.

2. Build a trajectory inventory.
   Output: trajectory_summary.csv and trajectory_inventory.md.
   Include task_id, model, success/failure, number of turns, availability of stdout/stderr, verifier, docker image, and command history.

3. Build failed trace index.
   Output: failed_trace_index.jsonl.
   Each row should contain trajectory_id, task_id, model, final status, command history, observations, and metadata.

4. Build step-level records.
   Output: trajectory_graph_steps.jsonl.
   Each row:
   {
     "trajectory_id": "...",
     "task_id": "...",
     "model": "...",
     "step_id": 0,
     "prefix_commands": [...],
     "action": "...",
     "stdout": "...",
     "stderr": "...",
     "exit_code": null,
     "file_diff": null,
     "verifier_result": null,
     "final_success": false,
     "replay_status": "unknown",
     "metadata": {}
   }

5. Mine candidate anchors.
   Implement heuristics:
   - first critical stderr;
   - first failed test after edit;
   - dependency/path/permission error;
   - repeated command loop;
   - last meaningful state before timeout;
   - final verifier failure;
   - divergence from successful teacher reference if matched trajectories exist.
   Output: anchor_candidates.jsonl and anchor_bucket_distribution.csv.

6. Build anchor profiles.
   For top 100-300 candidates, produce compact profiles:
   - task objective;
   - useful progress so far;
   - recent commands;
   - last failed command;
   - stdout/stderr/test output;
   - relevant diff if available;
   - suspected failure type;
   - what not to repeat.
   Output: sample_anchor_profiles.jsonl.

7. Produce a dry-run teacher repair prompt file.
   Do not call paid models unless explicitly configured.
   Output: sample_repair_prompts.jsonl.

8. Produce an error coverage report.
   Include failure type counts, evidence type counts, anchor type counts, and recommended pilot sampling plan.
   Output: error_coverage_report.md.

9. Produce a final report.
   Output: report.md.
   Include feasibility, missing metadata, risks, and next-step instructions for teacher repair pilot.
```

---

## 15. Immediate next decision

Before large-scale generation, decide the first pilot source:

Option A: use existing failed trajectories from MAP 5k multi-model runs.

- Fastest.
- May not be strictly on-policy for the final student.
- Good for parser, anchor mining, and repair feasibility.

Option B: rollout current student/checkpoint on 200-500 tasks.

- More faithful to the method.
- Requires rollout cost.
- Better for paper claims.

Recommended path:

1. Start with Option A for infrastructure and anchor feasibility.
2. Once scripts work, run Option B for the main pilot.

---

## 16. Final reminder

The strongest version of this project is not:

> We use failed trajectories.

It is:

> We learn which failure states are worth repairing, generate verifier-passing teacher repairs from those states, and show that error-state coverage is a more efficient SFT signal than additional clean demonstrations.
