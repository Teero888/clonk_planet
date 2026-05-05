#define TSF_IMPLEMENTATION
#include <external/tsf.h>

#define TML_IMPLEMENTATION
#include <external/tml.h>

#define MINIAUDIO_IMPLEMENTATION
#include <external/miniaudio.h>

#include <stdio.h>
#include <stdlib.h>

struct MidiPlayer {
  tsf *tiny_sf;
  tml_message *tiny_midi;
  tml_message *current_msg;
  double time_msec;
  ma_device device;
};

void audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
  MidiPlayer *player = (MidiPlayer *)pDevice->pUserData;
  float *output = (float *)pOutput;

  for (player->time_msec += (double)frameCount * (1000.0 / pDevice->sampleRate); player->current_msg && player->time_msec >= player->current_msg->time;
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

  if (!player->current_msg) {
    player->current_msg = player->tiny_midi;
    player->time_msec = 0;
  }

  tsf_render_float(player->tiny_sf, output, frameCount, 0);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: clonk_midi <song.mid> <font.sf2>\n");
    return 1;
  }

  MidiPlayer player = {0};

  player.tiny_sf = tsf_load_filename(argv[2]);
  if (!player.tiny_sf) {
    printf("Could not load SoundFont %s\n", argv[2]);
    return 1;
  }

  int preset_count = tsf_get_presetcount(player.tiny_sf);
  printf("Successfully loaded %d presets.\n", preset_count);

  if (preset_count == 0) {
    printf("ERROR: File loaded, but no instruments found. File might be corrupted.\n");
  }

  tsf_set_output(player.tiny_sf, TSF_STEREO_INTERLEAVED, 44100, 0);

  player.tiny_midi = tml_load_filename(argv[1]);
  if (!player.tiny_midi) {
    printf("Could not load MIDI %s\n", argv[1]);
    tsf_close(player.tiny_sf);
    return 1;
  }
  player.current_msg = player.tiny_midi;

  ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = ma_format_f32;
  deviceConfig.playback.channels = 2;
  deviceConfig.sampleRate = 44100;
  deviceConfig.dataCallback = audio_callback;
  deviceConfig.pUserData = &player;

  if (ma_device_init(NULL, &deviceConfig, &player.device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    tml_free(player.tiny_midi);
    tsf_close(player.tiny_sf);
    return 1;
  }

  if (ma_device_start(&player.device) != MA_SUCCESS) {
    printf("Failed to start playback device.\n");
    ma_device_uninit(&player.device);
    tml_free(player.tiny_midi);
    tsf_close(player.tiny_sf);
    return 1;
  }

  printf("Playing %s (ESC or Ctrl+C to stop)\n", argv[1]);

  while (1) {
    ma_sleep(100);
  }

  ma_device_uninit(&player.device);
  tml_free(player.tiny_midi);
  tsf_close(player.tiny_sf);

  return 0;
}
