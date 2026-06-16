#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <filesystem>
#include <random>

#pragma comment(lib, "Winmm.lib")

// 音效播放类（使用 MCI + WAV，系统原生支持）
class MusicPlayer {
private:
    std::wstring musicFolder;
    int volume = 200; // MCI 音量 0-1000（200 ≈ 20%，较小声）

    int toMciVolume(int vol) const {
        long long scaled = 1LL * vol * vol / 1000;
        if (scaled < 0) scaled = 0;
        if (scaled > 1000) scaled = 1000;
        return static_cast<int>(scaled);
    }

    // 设置 MCI 音量
    void setMciVolume(const wchar_t* alias) {
        std::wstring volCmd = L"setaudio " + std::wstring(alias) + L" volume to " + std::to_wstring(toMciVolume(volume));
        mciSendString(volCmd.c_str(), NULL, 0, NULL);
    }

    // MCI 播放 WAV（注意：某些系统不支持 repeat 关键字，所以先播放再单独设置循环）
    void playMci(const std::wstring& path, const wchar_t* alias, bool loop) {
        closeMci(alias);
        std::wstring cmd = L"open \"" + path + L"\" type waveaudio alias " + alias;
        if (mciSendString(cmd.c_str(), NULL, 0, NULL) != 0) return;
        std::wstring play = L"play " + std::wstring(alias);
        if (loop) {
            // 先播放一次（不带 repeat），再尝试设置循环
            if (mciSendString(play.c_str(), NULL, 0, NULL) != 0) return;
            mciSendString((play + L" repeat").c_str(), NULL, 0, NULL);
        } else {
            mciSendString(play.c_str(), NULL, 0, NULL);
        }
        setMciVolume(alias); // 设置音量
    }

    void closeMci(const wchar_t* alias) {
        mciSendString((L"close " + std::wstring(alias)).c_str(), NULL, 0, NULL);
    }

    bool isPlaying(const wchar_t* alias) {
        wchar_t s[64] = {};
        mciSendString((std::wstring(L"status ") + alias + L" mode").c_str(), s, 64, NULL);
        return wcscmp(s, L"playing") == 0;
    }

public:
    MusicPlayer() = default;

    void setMusicFolder(const std::wstring& folder) { musicFolder = folder; }
    int getVolume() const { return volume; }
    bool isMuted() const { return volume == 0; }

    void playFlip()        { if (!isMuted()) PlaySound(L"SystemAsterisk", NULL, SND_ASYNC | SND_ALIAS); }
    void playSelect()      { if (!isMuted()) PlaySound(L"SystemDefault", NULL, SND_ASYNC | SND_ALIAS); }
    void playMove()        { if (!isMuted()) PlaySound(L"SystemHand", NULL, SND_ASYNC | SND_ALIAS); }
    void playCapture()     { if (!isMuted()) PlaySound(L"SystemExclamation", NULL, SND_ASYNC | SND_ALIAS); }

    // 设置音量（0-1000）
    void setVolume(int vol) {
        if (vol < 0) vol = 0;
        if (vol > 1000) vol = 1000;
        volume = vol;
        if (isMuted()) {
            PlaySound(NULL, NULL, 0);
            stopAll();
            return;
        }
        setMciVolume(L"bgm");
        setMciVolume(L"victory");
    }

    // 背景音乐（固定使用指定曲目）
    void playBackgroundMusic() {
        if (isMuted()) return;
        std::wstring path = musicFolder + L"\\詹雯婷 - 命运火焰.wav";
        playMci(path, L"bgm", true);
    }

    void stopBackgroundMusic() { closeMci(L"bgm"); }
    bool isBackgroundMusicPlaying() { return isPlaying(L"bgm"); }

    // 胜利音乐（单次，也用同一首）
    void playVictoryMusic(const std::wstring& folder) {
        if (isMuted()) return;
        std::wstring path = folder + L"\\詹雯婷 - 命运火焰.wav";
        closeMci(L"bgm");
        playMci(path, L"victory", false);
    }

    bool isVictoryMusicPlaying() { return isPlaying(L"victory"); }

    // 停止所有
    void stopAll() {
        closeMci(L"bgm");
        closeMci(L"victory");
    }
};
