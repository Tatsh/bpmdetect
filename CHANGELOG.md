<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.8.3]

### Added

- Column indicating save state.
- Column to display last (`ffmpegutils`) error message (debug builds only).

### Changed

- Save BPM to `tmpo` atom (integer only) in M4A files. Unfortunately, the tag is not read back by
  ffmpeg.

### Fixed

- Filtering files.

## [0.8.2]

### Fixed

- Install path of man page when using FHS.
- Opening files in Windows (path handling issue).
- Removed invalid check for Media Feature Pack.

## [0.8.1] - 2025-08-27

### Fixed

- CMake: fix installation of man page (revert 60b50b1).

## [0.8.0] - 2025-08-27

## Added

- CMake: `FHS` option, off by default. Please enable this option if you are packaging for Linux or
  a similar system.
- Save window size in settings.
- Save window position in settings (not working on Linux Wayland for me).
- Ability to build without GUI support. Still requires Qt Core.

## Changed

- Now requires Qt 6.7 or later.
- Depends on SoundTouch shared library.
- Removed dead code.
- Removed kissfft requirement because only dead code referenced it.
- License is now GPL-3 or later.
- Do not enable _Save_ on first launch.
- Test BPM dialog is now a fixed size.
- _Test BPM_ menu option will be disabled on Windows if Media Feature Pack is not installed.
- Allow _Test BPM_ on macOS.
- New icons.
- Supports any audio format that FFmpeg supports.
- Files that are not detected to have an audio track will be ignored when dropped in or selected
  using the open file/directory dialogues.

### Fixed

- Possible crash when clearing the list of files and adding more or adding a directory.
- Not a great way to handle this: the test BPM dialog will close on fatal error.
- Fixed getting files recursively when a directory is added.

### Removed

- TagLib is no longer used.

## [0.7.2] - 2025-08-08

### Added

- WavPack support.

### Changed

- Better playback in the test BPM dialogue.

### Fixed

- In the test BPM dialogue, selecting number of beats applies immediately.
- Removed unused code.
- Removed unnecessary dependency on PortAudio.

## [0.7.1] - 2025-06-19

### Changed

- Build against Qt 6.

### Fixed

- Test dialogue: stop audio immediately on close.

[unreleased]: https://github.com/Tatsh/bpmdetect/compare/v0.8.3...HEAD
[0.8.3]: https://github.com/Tatsh/bpmdetect/compare/v0.8.2...v0.8.3
[0.8.2]: https://github.com/Tatsh/bpmdetect/compare/v0.8.1...v0.8.2
[0.8.1]: https://github.com/Tatsh/bpmdetect/compare/v0.7.2...v0.8.1
[0.7.2]: https://github.com/Tatsh/bpmdetect/compare/v0.7.1...v0.7.2
[0.7.1]: https://github.com/Tatsh/bpmdetect/compare/v0.6.2...v0.7.1
