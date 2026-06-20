# ADR-0007: Renderer write-containment, required-field validation, and an independent CI-YAML gate

## Status

Accepted

## Context

A follow-up audit of the renderer ([`tools/render.py`](../../tools/render.py)) and the
emitted-YAML gate ([`tools/profile_ci_lint.py`](../../tools/profile_ci_lint.py)) — after the
loader work in [ADR-0006](0006-manifest-loader-and-emitted-yaml-validation.md) — found three
correctness/security gaps the existing checks could not see, because the render-smoke only
exercises the one hand-tuned reference manifest:

- **Path traversal (confirmed, exploited).** Manifest fields flow unsanitized into filesystem
  paths (`SRC_MAIN = f"src/main/{lang}/{group_path}/{slug}"`, and `slug` into the spec
  filename). `write_file` joined and created without any containment check. A manifest with
  `project_slug: "../../../../tmp/PWNED"` produced `Render: OK` and wrote **outside** `--out`.
  `validate_manifest` only guarded unknown sections and `start_version`.
- **No required-field validation.** `build_context` defaults every scalar to `""`, so the
  "unresolved placeholder = hard error" promise never fires for *missing* data. A near-empty
  manifest rendered `Render: OK` with a blank title (`# `), no owner, and no license.
- **The CI-YAML gate trusted the code it guards.** `profile_ci_lint` read each profile with the
  hand-rolled `render.load_yaml`, then validated the extracted fragment with PyYAML — so a
  loader bug (or a dropped fragment, which `continue`s as "OK") was invisible to its own gate.

## Decision

1. **Confine every write to the output root.** `write_file` resolves both sides with
   `os.path.realpath` and refuses any destination outside `--out` (cross-drive `commonpath`
   raises → treated as outside). Defense-in-depth, independent of validation.
2. **Reject path-unsafe identifiers at validation time.** `project_slug`, `lang`, and
   `group_path` must be plain relative segments (no `.`, `..`, absolute/drive-qualified, or
   separators beyond `/`). Hard render failure.
3. **Reject missing required fields.** `PROJECT_NAME`, `PROJECT_SLUG`, `PROJECT_KIND`, `OWNER`,
   `LICENSE_ID`, `DEFAULT_BRANCH`, `LANG`, `GROUP_PATH` must be present and non-empty.
4. **Make the emitted-YAML gate independent.** `profile_ci_lint` now reads profiles with PyYAML
   (already its CI dependency) instead of `render.load_yaml`, and reports a malformed profile
   directly. It no longer imports from `render`, so a loader regression cannot hide from it.

## Consequences

- The renderer can no longer write outside its output directory, and no longer emits a hollow
  repo on incomplete input — both are hard, actionable failures.
- The CI-YAML gate is decoupled from the loader, closing the "the guard trusts the guarded"
  gap. The everyday rendering path stays standard-library-only; PyYAML remains CI-only.
- The reference render is unchanged (still 39 templates), so existing fixtures and the
  render-smoke pass without modification.
- **Out of scope (tracked separately):** the loader's *scalar* fidelity still diverges from
  spec YAML (double-/single-quote unescaping, `|-`/`|+` chomping). That touches the documented
  loader contract and warrants its own decision — likely adopting PyYAML-when-present in
  `load_yaml`, which would also retire the hand-rolled reader on the manifest path.
