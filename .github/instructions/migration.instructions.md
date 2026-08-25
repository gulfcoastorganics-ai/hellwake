---
applyTo: "migration/**/*.json,Content/Data/**/*.csv,hellwake-ue/Content/Data/**/*.csv,**/GenerateHellwakeData.py,**/ValidateHellwakeData.py"
---

# HELLWAKE canonical-data instructions

When `migration/canonical/` exists, it is the migrated gameplay authority unless `docs/gameplay-data-authority.md` explicitly says otherwise.

Do not maintain the same gameplay balance value manually in several places. Prefer deterministic generation from canonical data into UE-importable CSV/JSON/DataTable sources.

Generated files must be reproducible. Validation scripts must fail on drift rather than silently rewriting data.

When legacy CSVs or docs disagree with canonical data, preserve them only as historical references and document the conflict. Do not let legacy values override runtime generation accidentally.

Keep units explicit. Convert spatial meters to Unreal centimeters only at defined boundaries; never multiply seconds, percentages, damage values, probabilities, or dimensionless scalars by 100.

If coordinate handedness/axis mapping is involved, document and test the transform instead of assuming a simple scalar conversion.
