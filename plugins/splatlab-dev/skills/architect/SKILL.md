---
name: architect
description: Turn an agreed requirements document into a technical design the coder agent can implement without further questions. Runs in the main session so design decisions can be confirmed with the user. Produces docs/design/<slug>.md (and docs/private/<slug>-design.md for proprietary detail). Use after /requirements, or when the user says "design this", "how should we build", "architecture".
---

# Architect

You are running in the main session. Ask the user when a design choice is genuinely theirs; decide the rest yourself and record why.

## Inputs

`$ARGUMENTS` is the requirements slug. Read `docs/requirements/<slug>.md` first. If it does not exist, say so and suggest `/requirements` rather than inventing requirements.

## Procedure

1. **Read the requirements and the affected code.** Use the Explore agent for broad sweeps so file dumps stay out of this context. You need to know, concretely: which files change, which shared code in `src/SurfelsCore` both SplatLab and Surfels_DX12 pick up, which shaders are involved, and whether the codec library's interface changes.

2. **Respect the standing rules of this codebase.**
   - Shared logic goes in `surfels_core` (`src/SurfelsCore`), not duplicated in the executables.
   - Splat-mode codec logic, streaming order, detail scoring, wavelet and occlusion-volume code live in the private `libs/bluesec-codec`. The public tree gets prebuilt libraries plus open stand-ins. Never design proprietary logic into public paths.
   - The render path copies SuperSplat math exactly. Do not propose "improvements" to it.
   - Three render paths must keep working: mesh, VS SM6, VS SM5 (FXC). Say explicitly how each is affected.
   - Any `.sflw` format change updates `docs/SFLW_FORMAT_SPECIFICATION.md` in the same change.
   - Every change lands in the private repo `C:\github\surfels` first, then syncs to the public mirror. Design for that: no file that the sync strips may be required by public code.

3. **Decide, then confirm.** Where two designs are both reasonable and the difference matters to the user (UI shape, format compatibility, performance vs simplicity), present the options with AskUserQuestion and a recommendation. Otherwise choose and record the reason under "Decisions".

4. **Write the design** using the template. Split proprietary detail out:
   - `docs/design/<slug>.md` is public after sync. Component names, data flow, file list, test plan, risks. No algorithm detail for the codec.
   - `docs/private/<slug>-design.md` for anything the public scrub would have to remove (see the term list in `scripts/public-history-scrub.py`). Link it from the public file with one line.

5. **Write the implementation brief.** The last section of the public design file is a numbered task list for the coder agent: files to create or edit, in order, each with a one-line intent and a "done when" check that maps to an acceptance criterion. This is what the coder is prompted with, so it must be self-contained.

6. **Hand off.** Tell the user the paths and that the coder agent can be started with the design path as its prompt. Set the requirements doc's status to `agreed` if the user confirmed.

## Template

```markdown
# Design: <Feature name>

Requirements: docs/requirements/<slug>.md
Status: proposed | agreed | implemented
Date: <YYYY-MM-DD>

## Summary
Three to five sentences a reviewer can check the diff against.

## Components touched
| Area | Files | Change |
|------|-------|--------|

## Data flow
Where the data enters, what transforms it, where it is consumed. A short mermaid diagram if it helps.

## Render path impact
- mesh: ...
- VS SM6: ...
- VS SM5 (FXC): ...

## Public / private placement
Which files are public app code and which are private library code. Confirm no public file depends on a stripped path.

## Decisions
- <choice>: <why>, <what was rejected>

## Test plan
Which existing test exes cover this (TestGPUSort, TestBitonicCPU, TestGeometryCull, TestOcclusionVolume, TestLoadPackage, CompressVenus), whether tests/stress_test.py is affected, what the app smoke test looks at, and any new test needed.

## Risks
- ...

## Implementation brief (for the coder agent)
1. <file>: <intent>. Done when: <check>.
2. ...
```
