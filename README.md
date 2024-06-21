# Time Warped Looper

## Description

[JUCE](https://juce.com/) project to prototype a time warped looper, based on the looper.h from the [DaisySP] library.

## Usage

#### Looper States

The looper operates in three states: `LISTENING`, `RECORDING`, and `PLAYING`. The `UpdateLooperState` method switches between these states. Use a button to toggle between the states.

#### Loop Division
The loop can be divided into smaller parts called segments. These divisions half the size of the recorded loop: 2, 4, 8, 16, 32, 64, 128.

#### Segment Selection
Select which of these segments to listen to when in the `PLAYING` state.

#### Time Manipulation
Time manipulation works in both `RECORDING` and `PLAYING` states. Available manipulations include:
- **Reverse**: Playback/Record in reverse.
- **Half-Time**: Playback/Record at half speed.
- **Double-Time**: Playback/Record the loop at double speed.

## TODO / Future Improvements

- [ ] Fix clicks on loop resets
- [ ] Optimize WrapPosToSegments method
- [ ] Add a PitchShifter
- [ ] Add additional fx, a parameter for selection and a macro over the effect

## Library Attribution

This project uses the `looper.h` & `dsp.h` files from the [daisysp library](https://github.com/electro-smith/DaisySP), which is licensed under the MIT License.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.
