# Ryanair Turnaround Timer — v1.1

Changes in this version:
- Added a tiny aircraft icon for the Flipper app list.
- Added a small aircraft mark beside RYANAIR in the app header.
- Removed the “Beacon off” event.
- Cleaner list layout and timestamp display.
- A short BACK press returns from event details to the list; on the list, a short press does nothing.
- Hold BACK (long press) to exit the app from any screen, reducing accidental exits.

## Build

Open this folder in Terminal and run:

```sh
ufbt
```

The `.fap` output is normally in `dist/`. Copy it to `SD Card/apps/Tools/`.

## Firmware compatibility note

If Flipper shows “App too new”, the `.fap` was built against an SDK/API newer than the API exposed by your installed firmware. Build with the SDK matching your exact Momentum firmware release. Do not merely change metadata to hide the warning: that does not make incompatible API calls safe. Check **Settings → System → Firmware** on the Flipper and use that version to select a matching uFBT SDK/firmware target.

Note: the exit safety behavior depends on the firmware delivering `InputTypeLong` for a held BACK press. Timestamps are currently kept in RAM and are lost when the app exits or the device restarts.
