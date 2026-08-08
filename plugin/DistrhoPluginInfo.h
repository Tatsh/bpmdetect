// SPDX-License-Identifier: GPL-3.0-or-later
/** @file Compile-time plugin metadata required by DPF. */
#pragma once

#define DISTRHO_PLUGIN_BRAND "Tatsh"
#define DISTRHO_PLUGIN_BRAND_ID Ttsh
#define DISTRHO_PLUGIN_CLAP_FEATURES "audio-effect", "analyzer", "stereo"
#define DISTRHO_PLUGIN_CLAP_ID "sh.tat.bpmdetect"
#define DISTRHO_PLUGIN_HAS_UI 1
#define DISTRHO_PLUGIN_IS_RT_SAFE 1
#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:AnalyserPlugin"
#define DISTRHO_PLUGIN_NAME "BPM Detect"
#define DISTRHO_PLUGIN_NUM_INPUTS 2
#define DISTRHO_PLUGIN_NUM_OUTPUTS 2
#define DISTRHO_PLUGIN_UNIQUE_ID bpmd
#define DISTRHO_PLUGIN_URI "https://tatsh.github.io/bpmdetect/plugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Analyzer"
#define DISTRHO_UI_DEFAULT_HEIGHT 190
#define DISTRHO_UI_DEFAULT_WIDTH 280
#define DISTRHO_UI_USE_NANOVG 1
#define DISTRHO_UI_USER_RESIZABLE 1
