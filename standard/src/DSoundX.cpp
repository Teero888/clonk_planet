/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper to miniaudio - replacing DirectSound */

#include <Compat.h>
#include <DSoundX.h>
#include <vector>
#include <math.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

static ma_engine g_maEngine;
static bool g_maEngineInitialized = false;

BOOL InitDirectSound(HWND hwnd)
{
    ma_result result = ma_engine_init(NULL, &g_maEngine);
    if (result != MA_SUCCESS) return FALSE;
    g_maEngineInitialized = true;
    return TRUE;
}

void DeInitDirectSound()
{
    if (g_maEngineInitialized) {
        ma_engine_uninit(&g_maEngine);
        g_maEngineInitialized = false;
    }
}

struct CSoundObject
{
    std::vector<uint8_t> waveData;
    std::vector<ma_sound*> sounds;
    std::vector<ma_decoder*> decoders;
    int iAlloc;
    int iCurrent;
};

CSoundObject *DSndObjCreate(BYTE *bpWaveBuf, int iWaveSize, int iConcurrent)
{
    if (!g_maEngineInitialized || !bpWaveBuf) return NULL;
    
    CSoundObject *pSO = new CSoundObject();
    pSO->waveData.assign(bpWaveBuf, bpWaveBuf + iWaveSize);
    pSO->iAlloc = iConcurrent > 0 ? iConcurrent : 1;
    pSO->iCurrent = 0;

    for (int i = 0; i < pSO->iAlloc; i++) {
        ma_decoder* pDecoder = new ma_decoder();
        ma_result result = ma_decoder_init_memory(pSO->waveData.data(), pSO->waveData.size(), NULL, pDecoder);
        if (result != MA_SUCCESS) {
            delete pDecoder;
            DSndObjDestroy(pSO);
            return NULL;
        }
        pSO->decoders.push_back(pDecoder);

        ma_sound* pSound = new ma_sound();
        result = ma_sound_init_from_data_source(&g_maEngine, pDecoder, 0, NULL, pSound);
        if (result != MA_SUCCESS) {
            delete pSound;
            DSndObjDestroy(pSO);
            return NULL;
        }
        pSO->sounds.push_back(pSound);
    }

    return pSO;
}

void DSndObjDestroy(CSoundObject *pSO)
{
    if (!pSO) return;
    for (auto s : pSO->sounds) {
        ma_sound_uninit(s);
        delete s;
    }
    for (auto d : pSO->decoders) {
        ma_decoder_uninit(d);
        delete d;
    }
    delete pSO;
}

BOOL DSndObjPlay(CSoundObject *pSO, DWORD dwPlayFlags)
{
    if (!pSO) return FALSE;

    ma_sound* pSound = pSO->sounds[pSO->iCurrent];
    
    if (ma_sound_is_playing(pSound)) {
        if (pSO->iAlloc > 1) {
            int next = (pSO->iCurrent + 1) % pSO->iAlloc;
            pSound = pSO->sounds[next];
            pSO->iCurrent = next;
        }
    }

    ma_sound_set_looping(pSound, (dwPlayFlags & DSBPLAY_LOOPING) ? MA_TRUE : MA_FALSE);
    ma_sound_seek_to_pcm_frame(pSound, 0);
    ma_result result = ma_sound_start(pSound);
    
    return (result == MA_SUCCESS);
}

BOOL DSndObjStop(CSoundObject *pSO)
{
    if (!pSO) return FALSE;
    for (auto s : pSO->sounds) {
        ma_sound_stop(s);
        ma_sound_seek_to_pcm_frame(s, 0);
    }
    return TRUE;
}

BOOL DSndObjPlaying(CSoundObject *pSO)
{
    if (!pSO) return FALSE;
    for (auto s : pSO->sounds) {
        if (ma_sound_is_playing(s)) return TRUE;
    }
    return FALSE;
}

BOOL DSndObjSetVolume(CSoundObject *pSO, long lVolume)
{
    if (!pSO) return FALSE;
    float db = (float)lVolume / 100.0f;
    float volume = powf(10.0f, db / 20.0f);
    
    for (auto s : pSO->sounds) {
        ma_sound_set_volume(s, volume);
    }
    return TRUE;
}

BOOL DSndObjGetVolume(CSoundObject *pSO, long *plVolume)
{
    if (!pSO || !plVolume || pSO->sounds.empty()) return FALSE;
    float volume = ma_sound_get_volume(pSO->sounds[0]);
    float db = 20.0f * log10f(volume);
    *plVolume = (long)(db * 100.0f);
    return TRUE;
}
