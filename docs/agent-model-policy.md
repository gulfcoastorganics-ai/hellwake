# HELLWAKE Agent Model Policy

The repository intentionally does **not** hard-code model identifiers in `.github/agents/*.agent.md`.

GitHub Copilot model availability is account/product dependent and changes over time. Omitting `model:` makes each profile inherit the currently selected/default supported model instead of breaking when a named model is unavailable.

Use this allocation when choosing models in the client/workspace:

| Agent | Preferred model class |
|---|---|
| `hellwake-director` | strongest available reasoning/coding model |
| `hellwake-architect` | strongest available reasoning model |
| `unreal-engineer` | strongest available coding model with long context |
| `gameplay-engineer` | strong coding/reasoning model |
| `automation-engineer` | fast reliable coding model |
| `world-assets` | strong coding/reasoning model |
| `hellwake-qa` | independent strong reasoning model when possible |
| `hellwake-reviewer` | strongest available reasoning model, ideally different from implementer |

## Why reviewers should differ

When the product exposes more than one frontier model family, use a different family for QA/review than for implementation. The goal is independent error detection, not stylistic diversity.

## Cost/resource policy

Use expensive frontier reasoning for architecture, hard UE failures, cross-system debugging, and final review. Use faster/cheaper coding models for deterministic generators, tests, documentation updates, straightforward refactors, and repetitive build-fix loops.

If you want to pin a model later, add the supported GitHub model identifier to the desired agent frontmatter:

```yaml
model: <supported-model-id>
```

Only pin identifiers that the current Copilot environment actually exposes.
