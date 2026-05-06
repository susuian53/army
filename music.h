#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>

class MusicPlayer {
public:
    MusicPlayer() {
        // Simple beep system if no files exist, or use mciSendString
    }

    void playFlip() {
        // Play flip sound
        PlaySound(L"SystemAsterisk", NULL, SND_ASYNC | SND_ALIAS);
    }

    void playSelect() {
        // Play select sound
        PlaySound(L"SystemDefault", NULL, SND_ASYNC | SND_ALIAS);
    }

    void playMove() {
        // Play move sound
        PlaySound(L"SystemHand", NULL, SND_ASYNC | SND_ALIAS);
    }

    void playCapture() {
        // Play capture sound
        PlaySound(L"SystemExclamation", NULL, SND_ASYNC | SND_ALIAS);
    }

    void stopAll() {
        mciSendString(L"close all", NULL, 0, NULL);
    }
};
