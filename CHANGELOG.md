<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## Added

- Options at build time to enable FLAC, MP3, OGG, WavPack. All of these options are off by default.
  Without setting any of these options to `ON`, the app will only support wave files.
- CMake: `FHS` option, off by default. Please enable this option if you are packaging for Linux or
  a similar system.
- Save window size in settings.
- Save window position in settings (not working on Linux Wayland for me).
- Ability to build without GUI support. Still requires Qt Core.

## Changed

- Now requires Qt 6.7 or better.
- TagLib is now a hard requirement.
- Depends on SoundTouch shared library.
- Removed dead code.
- Removed kissfft requirement because only dead code referenced it.
- License is now GPL-3 or later.
- Do not enable _Save_ on first launch.
- Test BPM dialog is now a fixed size.
- _Test BPM_ menu option will not be displayed on Windows if Media Feature Pack is not installed.
- Allow _Test BPM_ on macOS.
- New icons.
- Supports any audio format that FFmpeg supports.

### Fixed

- Updated name filter so Wavpack files can be picked with the _Add files_ functionality.
- Possible crash when clearing the list of files and adding more or adding a directory.
- Not a great way to handle this: the test BPM dialog will close on fatal error.

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

[unreleased]: https://github.com/Tatsh/bpmdetect/compare/v0.7.2...HEAD
