# Changelog

## 1.1.0

- MIDI and Audio trigger modes fixed: audio now passes at unity gain while
  idle, and the curve starts on the MIDI note-on or the sidechain transient.
- Curve timing in MIDI and Audio modes follows the host tempo.
- Audio mode uses the sidechain input when the host provides one, and falls
  back to transients on the main input otherwise.
- AU users: re-insert On Tap once in existing sessions. The AU type changed
  from effect (aufx) to MIDI-controlled effect (aumf) so AU hosts send MIDI.
  VST3 and AAX are unaffected.
