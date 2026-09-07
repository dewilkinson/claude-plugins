---
name: requirements
description: Gather and write down the requirements for a feature or change before any design or code. Runs in the main session so it can ask the user questions. Produces docs/requirements/<slug>.md. Use when the user describes something they want built, or says "requirements", "spec this out", "what should this do".
---

# Requirements gatherer

You are running in the main session. You can and should ask the user questions. Nothing here is delegated to a subagent, because a subagent cannot talk to the user.

## Inputs

`$ARGUMENTS` is an optional short name or one-line description of the feature. If empty, ask what the feature is.

## Procedure

1. **Pick a slug.** Lower-case, hyphenated, from the feature name (e.g. `detach-camera-culled-tint`). The output file is `docs/requirements/<slug>.md`. If it already exists, read it and treat this run as a revision.

2. **Look before asking.** Spend a few minutes in the code so your questions are informed. Check `docs/USER_GUIDE.md`, `docs/SFLW_FORMAT_SPECIFICATION.md` if the change touches the file format, and the area of `src/SurfelsCore` or `libs/bluesec-codec` the feature lands in. Do not ask the user things the repo already answers.

3. **Interview, in at most two rounds.** Use AskUserQuestion. Cover only what is genuinely the user's call:
   - Goal: what the user should be able to do or see afterwards, in one or two sentences.
   - User-visible behaviour: UI, keys, config keys, command-line flags, file format changes.
   - Non-goals: what this explicitly does not cover.
   - Constraints: performance budget, render paths that must keep working (mesh / VS SM6 / VS SM5), Studio vs Viewer mode, backward compatibility of `.sflw` packages.
   - Public/private split: does any part depend on proprietary codec logic (streaming order, detail scoring, wavelet, occlusion volume)? That part lives in `libs/bluesec-codec` in the private repo and must be described only in general terms in public docs.
   - Acceptance criteria: concrete, checkable statements. Each one should be something the tester agent can verify by building, running an exe, reading a trace line, or looking at a screenshot.

4. **Draft the document** using the template below, show the user a short summary, and revise once if they push back. Then write the file.

5. **Hand off.** Tell the user the file path and that `/architect <slug>` is the next step.

## Where the file goes

- `docs/requirements/<slug>.md` is synced to the public mirror. Write it so it can be public: describe *what*, not proprietary *how*. Do not name internal algorithms or scoring methods.
- If a requirement can only be stated by describing proprietary method detail, put that section in `docs/private/<slug>-requirements.md` instead and reference it from the public file with one line ("further detail in the private design notes"). `docs/private/` is stripped from the public history by the sync script.

## Template

```markdown
# <Feature name>

Status: draft | agreed | implemented
Date: <YYYY-MM-DD>
Owner: <user>

## Goal
One or two sentences.

## User-visible behaviour
- ...

## Non-goals
- ...

## Constraints
- Render paths: ...
- Modes (Studio / Viewer): ...
- Package compatibility: ...
- Performance: ...

## Public / private placement
Which parts are public app code (src/SurfelsCore, src/DX12, tools/) and which touch the private codec library.

## Acceptance criteria
1. ... (each line independently checkable)

## Open questions
- ...
```
