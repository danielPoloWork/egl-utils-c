# ADR-0003: Generated repositories are self-governing

- **Status:** Accepted
- **Date:** 2026-06-15
- **Deciders:** Maintainer, Enterprise Project Architect
- **Related:** ADR-0001, `templates/AGENTS.md.tmpl`, `orchestrator/generate.md`

## Context

Once EAAO renders a new repository, that repository will live and evolve on its own — with
its own agents, CI, and maintainers. We must decide whether the generated repo stays coupled
to EAAO (pulling rules/templates at runtime) or is fully independent after generation.

## Decision

A generated repository is **self-governing**: generation copies a complete, standalone
`AGENTS.md` (and adapters, CI, lint, docs) into it, and from that point the new repo's own
`AGENTS.md` is authoritative. The generated repo has **no runtime dependency** on EAAO — it
does not fetch templates, rules, or the lint from EAAO. EAAO's responsibility ends at the
bootstrap PR; the two-contracts rule (`AGENTS.md` §header note) makes this explicit so an
agent never imports EAAO's own rules into a generated project.

## Alternatives Considered

- **Submodule / runtime link to EAAO.** Rejected — couples every project to the factory,
  creates version-skew and supply-chain risk, and contradicts the reference project's
  zero-external-governance-dependency posture.
- **Re-render on every EAAO change.** Rejected — projects diverge intentionally after
  bootstrap (their own ADRs, milestones, patterns); re-rendering would clobber that history.

## Consequences

- Generated repos are portable and durable; they outlive any EAAO version.
- Improvements to EAAO benefit *future* generations, not retroactively — acceptable, since
  each repo owns its evolution via its own ADRs.
- The generated `consistency_lint.py` is a self-contained copy with a filled CONFIG block,
  not a shared library.

## References

- `templates/AGENTS.md.tmpl` (header note: "this contract is now authoritative");
  `orchestrator/generate.md` Step 8 (hand-off).
