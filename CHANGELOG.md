<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## Added

- Options at build-time to enable FLAC, MP3, OGG, WavPack. All of these options are off by default.
  Without setting any of these options to `ON`, the app will only support wave files.

## Changed

- TagLib is now a hard requirement.
- Depends on SoundTouch shared library.
- Removed dead code.
- Removed kissfft requirement because only dead code referenced it.
- License is now GPL-3 or later.

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

- Test dialogue: stop audo immediately on close.

[unreleased]: https://github.com/Tatsh/bpmdetect/compare/v0.7.2...HEAD
