# bpmdetect

[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/tags)
[![License](https://img.shields.io/github/license/Tatsh/bpmdetect)](https://github.com/Tatsh/bpmdetect/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/bpmdetect/v0.7.1/master)](https://github.com/Tatsh/bpmdetect/compare/v0.7.1...master)
[![CodeQL](https://github.com/Tatsh/bpmdetect/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/codeql.yml)
[![QA](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/qa.yml)
[![Tests](https://github.com/Tatsh/bpmdetect/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/bpmdetect/actions/workflows/tests.yml)
[![Coverage Status](https://coveralls.io/repos/github/Tatsh/bpmdetect/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/bpmdetect?branch=master)
[![GitHub Pages](https://github.com/Tatsh/bpmdetect/actions/workflows/pages/pages-build-deployment/badge.svg)](https://tatsh.github.io/bpmdetect/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/bpmdetect?logo=github&style=flat)](https://github.com/Tatsh/bpmdetect/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Ddid%3Aplc%3Auq42idtvuccnmtl57nsucz72%26query%3D%24.followersCount%26style%3Dsocial%26logo%3Dbluesky%26label%3DFollow%2520%40Tatsh&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)

![Screenshot](screenshot.png)

BPM Detect is an automatic BPM (beats per minute) detection utility.

This version has been modified to use the Qt Multimedia framework for BPM testing. This feature is
only available on Windows and Linux at this time.

It uses SoundTouch's BPM library for detecting BPMs, TagLib for reading and writing BPMs to tags
and Qt 6 for the GUI. Aditional libraries (mad, vorbisfile, FLAC) are used for decoding and the
Qt Multimedia framework is also used for audio output (testing BPMs).

All file-loading libraries (flac, libmad, etc) are optional.

[Original project](https://sourceforge.net/projects/bpmdetect/)
