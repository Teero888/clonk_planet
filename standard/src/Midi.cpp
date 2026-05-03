/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Play midis using TinySoundFont and miniaudio */

#include <Compat.h>
#include <Midi.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <mmsystem.h>
BOOL PlayMidi(const char *sFileName, HWND appWnd) {
  char buf[256];
  sprintf(buf, "open %s type sequencer alias ITSMYMUSIC", sFileName);
  if (mciSendString("close all", NULL, 0, NULL) != 0)
    return FALSE;
  if (mciSendString(buf, NULL, 0, NULL) != 0)
    return FALSE;
  if (mciSendString("play ITSMYMUSIC from 0 notify", NULL, 0, appWnd) != 0)
    return FALSE;
  return TRUE;
}

BOOL PauseMidi() {
  if (mciSendString("stop ITSMYMUSIC", NULL, 0, NULL) != 0)
    return FALSE;
  return TRUE;
}

BOOL ResumeMidi(HWND appWnd) {
  if (mciSendString("play ITSMYMUSIC notify", NULL, 0, appWnd) != 0)
    return FALSE;
  return TRUE;
}

BOOL StopMidi() {
  if (mciSendString("close all", NULL, 0, NULL) != 0)
    return FALSE;
  return TRUE;
}

BOOL ReplayMidi(HWND appWnd) {
  if (mciSendString("play ITSMYMUSIC from 0 notify", NULL, 0, appWnd) != 0)
    return FALSE;
  return TRUE;
}
#else

#define TSF_IMPLEMENTATION
#include <external/tsf.h>

#define TML_IMPLEMENTATION
#include <external/tml.h>

#include <external/miniaudio.h>

struct MidiPlayer {
    tsf* tiny_sf;
    tml_message* tiny_midi;
    tml_message* current_msg;
    double time_msec;
    ma_device device;
    bool initialized;
};

static MidiPlayer g_MidiPlayer = {0};

void midi_audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MidiPlayer* player = (MidiPlayer*)pDevice->pUserData;
    float* output = (float*)pOutput;

    if (!player->tiny_sf || !player->current_msg) return;

    // Process MIDI messages for this block
    for (player->time_msec += (double)frameCount * (1000.0 / pDevice->sampleRate);
         player->current_msg && player->time_msec >= player->current_msg->time;
         player->current_msg = player->current_msg->next) {
        switch (player->current_msg->type) {
            case TML_PROGRAM_CHANGE:
                tsf_channel_set_presetnumber(player->tiny_sf, player->current_msg->channel, player->current_msg->program, (player->current_msg->channel == 9));
                break;
            case TML_NOTE_ON:
                tsf_channel_note_on(player->tiny_sf, player->current_msg->channel, player->current_msg->key, player->current_msg->velocity / 127.0f);
                break;
            case TML_NOTE_OFF:
                tsf_channel_note_off(player->tiny_sf, player->current_msg->channel, player->current_msg->key);
                break;
            case TML_PITCH_BEND:
                tsf_channel_set_pitchwheel(player->tiny_sf, player->current_msg->channel, player->current_msg->pitch_bend);
                break;
            case TML_CONTROL_CHANGE:
                tsf_channel_midi_control(player->tiny_sf, player->current_msg->channel, player->current_msg->control, player->current_msg->control_value);
                break;
        }
    }

    // Loop MIDI
    if (!player->current_msg) {
        player->current_msg = player->tiny_midi;
        player->time_msec = 0;
    }

    // Synthesize the block
    tsf_render_float(player->tiny_sf, output, frameCount, 0);
}

BOOL StopMidi() {
    if (g_MidiPlayer.initialized) {
        ma_device_stop(&g_MidiPlayer.device);
        ma_device_uninit(&g_MidiPlayer.device);
        if (g_MidiPlayer.tiny_midi) tml_free(g_MidiPlayer.tiny_midi);
        if (g_MidiPlayer.tiny_sf) tsf_close(g_MidiPlayer.tiny_sf);
        g_MidiPlayer.initialized = false;
        g_MidiPlayer.tiny_midi = NULL;
        g_MidiPlayer.tiny_sf = NULL;
        g_MidiPlayer.current_msg = NULL;
    }
    return TRUE;
}

BOOL PlayMidi(const char *sFileName, HWND appWnd) {
    StopMidi();

    // 1. Load SoundFont (Try current dir and relative to exe)
    const char* sf_path = "FluidR3_GM_GS.sf2";
    g_MidiPlayer.tiny_sf = tsf_load_filename(sf_path);
    if (!g_MidiPlayer.tiny_sf) {
        // Try in planet_data
        g_MidiPlayer.tiny_sf = tsf_load_filename("planet_data/FluidR3_GM_GS.sf2");
    }
    
    if (!g_MidiPlayer.tiny_sf) {
        fprintf(stderr, "Midi: Could not load SoundFont FluidR3_GM_GS.sf2\n");
        return FALSE;
    }
    tsf_set_output(g_MidiPlayer.tiny_sf, TSF_STEREO_INTERLEAVED, 44100, 0);

    // 2. Load MIDI
    g_MidiPlayer.tiny_midi = tml_load_filename(sFileName);
    if (!g_MidiPlayer.tiny_midi) {
        fprintf(stderr, "Midi: Could not load MIDI %s\n", sFileName);
        tsf_close(g_MidiPlayer.tiny_sf);
        return FALSE;
    }
    g_MidiPlayer.current_msg = g_MidiPlayer.tiny_midi;
    g_MidiPlayer.time_msec = 0;

    // 3. Initialize Audio Device
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 44100;
    deviceConfig.dataCallback      = midi_audio_callback;
    deviceConfig.pUserData         = &g_MidiPlayer;

    if (ma_device_init(NULL, &deviceConfig, &g_MidiPlayer.device) != MA_SUCCESS) {
        tml_free(g_MidiPlayer.tiny_midi);
        tsf_close(g_MidiPlayer.tiny_sf);
        return FALSE;
    }

    if (ma_device_start(&g_MidiPlayer.device) != MA_SUCCESS) {
        ma_device_uninit(&g_MidiPlayer.device);
        tml_free(g_MidiPlayer.tiny_midi);
        tsf_close(g_MidiPlayer.tiny_sf);
        return FALSE;
    }

    g_MidiPlayer.initialized = true;
    return TRUE;
}

BOOL PauseMidi() {
    if (g_MidiPlayer.initialized) ma_device_stop(&g_MidiPlayer.device);
    return TRUE;
}

BOOL ResumeMidi(HWND appWnd) {
    if (g_MidiPlayer.initialized) ma_device_start(&g_MidiPlayer.device);
    return TRUE;
}

BOOL ReplayMidi(HWND appWnd) {
    if (g_MidiPlayer.initialized) {
        g_MidiPlayer.current_msg = g_MidiPlayer.tiny_midi;
        g_MidiPlayer.time_msec = 0;
    }
    return TRUE;
}

#endif
