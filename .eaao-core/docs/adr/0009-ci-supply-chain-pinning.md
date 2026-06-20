# ADR-0009: CI supply-chain pinning policy

## Status

Accepted

## Context

The audit flagged the factory's CI supply chain. EAAO's own workflow, the workflow templates it
renders, and several language profiles referenced third-party GitHub Actions by *movable* refs:

- floating tags (`actions/checkout@v6`, `softprops/action-gh-release@v2`, `actions/setup-*@vN`),
- and — worse, non-reproducible by definition — `@latest` (`lukka/get-cmake@latest`) and
  `go install …@latest` tool fetches.

`pip install pyyaml` in EAAO's own CI was unpinned. A moved tag or a compromised release runs
attacker-controlled code; the release workflow runs with `contents: write`. Because EAAO emits
these patterns into every generated repository, the weakness multiplies downstream.

Two facts shape the policy. First, the generated repos already ship a Dependabot config for the
`github-actions` ecosystem, which keeps version-tagged refs current and can bump SHA pins
(it reads the `# vX.Y.Z` trailing comment). Second, `dtolnay/rust-toolchain@stable|nightly|master`
is **not** a version pin — that action uses its git ref to *select the Rust channel*, so pinning
it to a SHA would break it.

## Decision

A tiered policy, by who authors the workflow and by reproducibility:

1. **EAAO-authored workflow surfaces are SHA-pinned**: `.github/workflows/ci.yml` (the factory's
   own CI) and the rendered baseline templates `ci.yml.tmpl` / `release.yml.tmpl`. Each pin
   carries a `# vX.Y.Z` comment so Dependabot can propose bumps. Pins target the latest patch of
   the major each file already used (no silent major bumps): checkout `v6.0.3`,
   setup-python `v5.6.0`, action-gh-release `v2.6.2`.
2. **`@latest` / floating refs are prohibited everywhere.** `lukka/get-cmake@latest` →
   SHA-pinned `v4.3.3` (the action has no stable major tag to ride); `go install …@latest` →
   pinned tool versions (`gofumpt@v0.10.0`, `govulncheck@v1.4.0`).
3. **Language-ecosystem actions inside profiles stay version-tag-pinned** (e.g.
   `actions/setup-go@v5`, `actions/setup-java@v4`, `golangci/golangci-lint-action@v6`) and are
   managed by the shipped Dependabot config. This is a deliberate, documented choice — not an
   oversight — trading a marginal hardening gain for far lower factory maintenance, and it keeps
   the per-language matrix legible. `dtolnay/rust-toolchain@<channel>` is left untouched by design.
4. **The pinned CI dependency is pinned + hashed**: PyYAML moves to
   `.eaao-core/tools/requirements-ci.txt` (`pyyaml==6.0.3` with `--require-hashes`), installed via
   `pip install --require-hashes -r …`. PyYAML remains CI-only; the rendering path stays
   standard-library-only.
5. **EAAO gains its own `.github/dependabot.yml`** (`github-actions` + `pip`) so the new SHA pins
   and the hashed requirement are actually bumped — SHA-pinning without an update mechanism rots.

## Consequences

- A moved tag or yanked release can no longer silently change what EAAO's CI or a generated
  repo's baseline workflow executes; bumps arrive as reviewable Dependabot PRs.
- The render-smoke output is unchanged in structure (39 templates); profile CI fragments still
  parse (the emitted-YAML gate covers them). The reference render stays byte-stable except for
  the intended get-cmake pin.
- Profiles remain readable and low-maintenance; the SHA-pin discipline is concentrated where EAAO
  is the author and the blast radius (a `contents: write` release job) is highest.
- Future work, if desired: extend SHA-pinning into the per-language profiles, and add per-wheel
  hash pinning for additional Python tools — both are incremental on this foundation.

## Addendum (2026-06-18)

A later audit found two drift gaps in this policy as implemented:

- **The literal pins in Decision §1 are point-in-time.** The factory's own
  `.github/workflows/ci.yml` has since been bumped by Dependabot (e.g. `actions/setup-python`
  v5.6.0 → v6.2.0) while the rendered templates were not. Dependabot's `github-actions`
  ecosystem only scans real workflow files — never the `.tmpl` copies, nor the YAML fragments
  embedded in language profiles — so the template pins are **not** kept current by the factory's
  Dependabot (contrary to the spirit of §5) and must be maintained deliberately.
- **The templates had pinned `actions/checkout` to the annotated-tag-object SHA** rather than
  the commit SHA the factory CI uses, so the two referenced `v6.0.3` by different object SHAs.

Both were reconciled in PR #11: the templates were re-pinned in lockstep with the factory CI,
and an `action-pins` gate was added to `tools/eaao_lint.py` that fails if a SHA-pinned action
shared by the factory CI and the workflow templates diverges. **The specific version numbers in
§1 are illustrative of the original decision, not the current pins — the `action-pins` gate, not
this prose, is now the source of truth for factory/template parity.**
