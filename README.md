# ProcessAudioControl

A small C++ library for windows to control basic sound features of a process.

## Features
- pause audio
- play audio
- play-pause toggle
- next track
- previous track
- play-pause toggle
- volume up
- volume down
- set volume
- read volume


## example

```
ProcessAudioControl audioControl;
audioControl.selectExecutableName(L"firefox.exe");

audioControl.play();

audioControl.volumeUp(0.10);// up by 10%

```
### What happens if the target process isn't running?
Every time a function is called that interacts with the specified process (e.g. `play()`, `volumeUp()`), it checks whether the previously acquired handles were valid, if not new handles are acquired. Handles get invalid if the target process closes or was never running at all. If new handles cannot be acquired nothing happens.
If `isValid()` returns true the current handles are valid and everything is in order. `reconnect()` can be used to require the handles again.