# Customization Overlays

How a maintainer or organization tunes EAAO **without editing the core**. Overlays are read
by the architect at interview/generate time and layered on top of the built-ins.

**Precedence (highest wins):** user overlay (this folder) → language profile → built-in
default. A blank/omitted overlay value falls through to the next layer.

## What you can override

| File | Effect |
|---|---|
| [`defaults.yaml`](defaults.yaml) | Pre-fills the interview's answers — default branch, license, group path, coverage target, commit scopes, i18n defaults. The architect uses these instead of the built-in defaults and still echoes them back for confirmation. |
| [`house-rules.md`](house-rules.md) | Organization-specific rules **injected into every generated `AGENTS.md`** as §13 (via `{{HOUSE_RULES}}`). Where a house rule conflicts with a default, the house rule wins. Empty by default (no §13 is emitted). |
| `agents/` | Drop custom role agents here, or a file overriding a shipped one (same name). They take precedence over [`../agent/`](../agent/README.md). |

## How it flows into a generated repo

1. The architect loads `defaults.yaml` and uses it to pre-fill the interview.
2. It copies the body of `house-rules.md` (if non-empty) into the manifest's
   `governance.house_rules`, which the renderer emits as `AGENTS.md` §13.
3. Custom `agents/` are offered alongside the shipped registry.

Nothing here is secret — keep tokens/webhooks out of these files (use CI secrets). Overlays are
committed and version-controlled like everything else, so a change to house rules is reviewable.
