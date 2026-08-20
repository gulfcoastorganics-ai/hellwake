# Camera

Fixed elevated 3/4 isometric perspective — the approved composition the
design chat explicitly **froze**: "FREEZE the current: ... camera
composition ... Do NOT continue cosmetically redesigning this screen."

## Static setup (`AHellwakeCharacter`)

`CameraBoom` (`USpringArmComponent`) is attached directly to the character
root with no player-input rotation:
- `TargetArmLength = 3700` (37m)
- Relative rotation `(-47°, -45°, 0°)` — pitched down and yawed for the 3/4
  isometric read
- `bInheritPitch/Yaw/Roll = false` — the boom never rotates with the
  character, matching the prototype's static `CAM_OFF = (0, 27, 25)`
- `TopDownCamera` FOV = 34°, matching `new THREE.PerspectiveCamera(34, ...)`

This alone reproduces the prototype's non-rotating isometric read. Do not
add mouse-look or controller-rotation binding to this boom — see
combat.md's targeting section for why that would also change combat feel,
not just the camera.

## Dynamic behavior (`UHellwakeCameraDirector`)

Everything the prototype's per-frame camera code did beyond the fixed
framing:

| Prototype (`hellwake-game.js` tick()) | Port |
|---|---|
| `camZoom` lerp toward 0.82 (cinematic) / 1.12 (boss fight) / 1.0 | `CurrentZoom` lerp at `ZoomLerpRate` (3/s), applied as `TargetArmLength = BaseArmLength * CurrentZoom` |
| `shake = max(shake, X)`, decays `dt*2.4`/s, jitters camera position | `ShakeLevel`, same decay rate, jitters `CameraBoom->SocketOffset` |
| `hitStop`: `dt *= 0.12` while active | `TriggerHitStop()` — brief `UGameplayStatics::SetGlobalTimeDilation(0.12)`, auto-restored |
| `camTarget.lerp(focus, min(1,6*dt))`, blends 28% toward boss during the fight | Not yet ported — see below |

**Not yet ported:** the focus-blend-toward-boss behavior (prototype:
`if (bossActive) focus.lerp(warden.obj.position, 0.28)`). `CameraBoom` is
rigidly attached to the character, so this needs either (a) a small
world-space camera-focus actor the boom's parent lerps toward instead of
being a direct child of the character, or (b) a `SpringArm` target-offset
computed each tick in `UHellwakeCameraDirector::TickComponent`. Left as a
TODO rather than guessed at, since it changes the attachment model.

## Triggering shake/hit-stop

Call `GetComponentByClass<UHellwakeCameraDirector>()->TriggerShake(X)` /
`TriggerHitStop(Seconds)` from wherever the prototype called them — every
ability's `.cpp` has a `TODO(Camera):` comment at the exact call site with
the prototype's original magnitude.

## Build in Editor

Nothing — this system is pure C++ and fully wired. Only tuning (arm length,
rotation, FOV, zoom targets) needs adjustment once real level geometry
exists to frame against.
