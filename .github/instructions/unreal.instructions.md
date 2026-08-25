---
applyTo: "Source/Hellwake/**/*.h,Source/Hellwake/**/*.cpp,*.Build.cs,*.Target.cs,Config/**/*.ini"
---

# HELLWAKE Unreal-specific instructions

Target Unreal Engine 5.4.

Preserve the existing module/GAS/Enhanced Input architecture unless a concrete compile/runtime failure requires change. Use generated headers in the correct order, minimize includes with valid forward declarations, and keep module dependencies intentional.

Gameplay values must come from the designated repository source of truth; do not hardcode a new balance value merely to make a test pass.

Treat editor-only assets and Blueprint wiring as content dependencies, not proof of C++ correctness. If an asset reference is unavailable in the current environment, expose/configure the dependency cleanly and document the missing runtime validation.

Validation claims must distinguish static inspection, UHT/compile, commandlet/headless runtime, Editor launch, PIE, rendering, and packaging.

Prefer event/delegate-driven integration over broad per-frame world scans. Avoid repeated `GetAllActorsOfClass`, unnecessary Tick work, duplicate state machines, and hardcoded content paths.
