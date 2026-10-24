# Ryanair Turnaround Timer (Flipper Zero / Momentum)

A small Flipper Zero app concept for recording aircraft turnaround event timestamps.

## Event buttons

Use **UP/DOWN** to select an event and **OK** to record the Flipper's current date/time:
1. Plane stop
2. Beacon off
3. First PAX out
4. Last PAX out
5. Boarding start
6. Boarding end
7. Door close
8. Beacon on
9. Plane moved

Use **RIGHT** to show the selected event's full timestamp, **LEFT** to return to the list, and **BACK** to exit.

The app uses the Flipper's real-time clock. Check that the device date/time is correct before a shift.

## Important limitations in this starter build

- Timestamps are currently held in RAM only and are lost when the app exits or the Flipper reboots. Persistent CSV export is a recommended next step.
- This uses a text header reading `RYANAIR`, not official Ryanair logo artwork.
- The source is provided, but a `.fap` binary has not been compiled or device-tested in this environment. Build it with uFBT against a compatible SDK, then test on your Momentum version before relying on it for operational records.
- A timestamp is recorded when you press OK; it does not automatically detect aircraft movement or beacon state.

## Build on macOS

Install Python 3 and then:

```bash
python3 -m pip install --upgrade ufbt
cd ryanair_turnaround_timer
ufbt
```

uFBT normally creates the compiled `.fap` in a `dist/` directory (the exact path is printed by the build output).

## Build on Windows

```powershell
py -m pip install --upgrade ufbt
cd ryanair_turnaround_timer
ufbt
```

If the project needs SDK template setup on your system, create a clean sample app first with `ufbt create APPID=sample_app`, then use the generated manifest as a reference for your installed uFBT version.

## Install on Flipper Zero

1. Build the app and locate `ryanair_turnaround_timer.fap`.
2. Connect Flipper Zero by USB and open qFlipper.
3. Open **File Manager → SD Card → apps → Tools** (create `Tools` if needed).
4. Copy the `.fap` file into that folder.
5. On Flipper, open **Apps → Tools → Turnaround Timer**.

If it does not appear or reports an API/SDK mismatch, update uFBT and build against an SDK compatible with your installed Momentum firmware. Avoid flashing firmware just to install this app unless you understand the consequences.

## References

- Flipper developer documentation: https://developer.flipper.net/flipperzero/doxygen/applications.html
- First C app guide: https://developer.flipper.net/flipperzero/doxygen/app_dev_your_first_app_in_c.html
- FAP package guide: https://developer.flipper.net/flipperzero/doxygen/apps_on_sd_card.html
