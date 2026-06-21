# ADR-0004: Generate the API reference with Doxygen, gated warning-free in CI

- **Status:** Accepted
- **Date:** 2026-06-21
- **Deciders:** Daniel Polo (maintainer), project architect
- **Related:** AGENTS.md §9 (Coding Conventions), §10 (Enterprise Quality Bar), ROADMAP M8.2,
  ADR-0002 (module-oriented layout)

## Context

`AGENTS.md` §9 requires that "all public symbols [are] documented with Doxygen-compatible
comments" and §10 lists "Doxygen builds without warnings" as a quality gate. Through M1–M7 the
public headers carried thorough prose, but in **plain C block comments** (`/* … */`), which
Doxygen does not extract — so no API reference could be generated and the gate was unmet.

M8.2 ("Doxygen API docs published; README quickstart per module group") closes that gap. The
forces:

- **Standard tooling.** Doxygen is the de-facto API-doc generator for C/C++, reads annotated
  headers directly, and integrates with CI as a pass/fail step.
- **A real gate, not a vanity build.** §10 demands *warning-free*; the build must fail on any
  Doxygen warning so the docs cannot silently rot.
- **Discoverability.** 21 functional units across seven modules need to be navigable. The
  module-oriented layout (ADR-0002) maps naturally onto Doxygen *groups*.
- **No local-toolchain assumption in CI cells.** Diagram generation (Graphviz `dot`) is
  optional; the docs must build cleanly whether or not `dot` is present.
- **Publishing vs. building.** The roadmap says "published", but this repository has a
  deliberate human checkpoint for anything outward-facing, and (at the time of writing) no
  branch has been pushed. Auto-deploying to GitHub Pages is therefore out of scope here.

## Decision

Adopt **Doxygen** as the API-reference generator, driven by a curated, minimal `docs/Doxyfile`,
and enforce a **zero-warning** build as a required CI job.

- The public headers are converted to Doxygen comments (`/** … */`, `@brief`, `@param`,
  `@return`/`@retval`, `@note`/`@warning`). Each module is a Doxygen group (`@defgroup` defined
  once in `d4np_c.h`; each header joins via `@addtogroup … @{ … @}`), so the **Modules** tab
  mirrors the `d4np/<module>/` tree.
- `docs/api/mainpage.md` is the landing page (`USE_MDFILE_AS_MAINPAGE`).
- The Doxyfile sets `OPTIMIZE_OUTPUT_FOR_C`, `EXTRACT_ALL=YES` with `WARN_IF_UNDOCUMENTED=NO`
  (extraction is total, so a missed comment degrades to a blank entry rather than a build
  failure), and `WARN_AS_ERROR=FAIL_ON_WARNINGS` with `WARN_IF_DOC_ERROR=YES` (so *malformed*
  documentation — bad commands, mismatched `@param`, broken refs — does fail). `HAVE_DOT=NO`
  keeps the build independent of Graphviz.
- A CI `docs` job installs Doxygen, runs `doxygen docs/Doxyfile`, fails on any warning, and
  uploads the generated HTML as a build artifact.

**Publishing** (e.g. GitHub Pages) is intentionally deferred: the CI artifact is the
deliverable for now, and a Pages deploy will be wired when the branch/PR/release flow is
reconciled (it must not auto-publish from an unreviewed branch).

## Alternatives Considered

- **`EXTRACT_ALL=NO` + `WARN_IF_UNDOCUMENTED=YES`** (fail on any undocumented symbol) —
  Rejected as the *gate* mechanism. It is the strictest reading of §10, but it makes every
  struct field and enum value a build-breaker and is brittle to evolve. We instead document all
  public symbols by hand for quality and use `WARN_IF_DOC_ERROR` to catch *broken* docs; raising
  to undocumented-fails-build is a future tightening.
- **Sphinx + Breathe / Hawkmoth** — Rejected. Adds a Python/RST toolchain and a Doxygen XML
  bridge for output we do not need; disproportionate for a C library.
- **Standardese / cldoc** — Rejected. Less maintained and less portable than Doxygen across our
  Linux/macOS/Windows matrix.
- **Auto-deploy to GitHub Pages now** — Rejected for now. Outward-facing publishing is a human
  checkpoint here, and there is no reviewed mainline to publish from yet (see ADR-0003 context).

## Consequences

- **API / compatibility:** none — comment-only changes to headers; the code skeleton of all 24
  public headers is byte-for-byte unchanged.
- **Documentation:** the public surface is now navigable HTML; README gains a per-module-group
  quickstart and a docs-build pointer; `docs/development/local-build.md` gains the command.
- **Tooling / CI:** one new Linux job (~Doxygen install + build). Contributors run
  `doxygen docs/Doxyfile` locally to reproduce it. Output lands in the git-ignored
  `docs/doxygen/`.
- **Risks / limitations:** the gate catches malformed docs but not *missing* prose
  (`WARN_IF_UNDOCUMENTED=NO`); coverage of documentation completeness is by review, not by the
  tool. Publishing is not yet automated. Tightening to undocumented-fails-build and a Pages
  deploy are tracked for a later milestone.

## References

- AGENTS.md §9, §10; ROADMAP.md M8.2; ADR-0002 (module layout); ADR-0003 (coverage gate).
- Doxygen manual: <https://www.doxygen.nl/manual/> (grouping, `WARN_AS_ERROR`,
  `USE_MDFILE_AS_MAINPAGE`).
