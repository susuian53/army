#pragma once

#include <windows.h>
#include <mmsystem.h>

#include <filesystem>
#include <string>

#pragma comment(lib, "Winmm.lib")

// 音效播放类（使用 WinMM/MCI，优先播放编译进 exe 的 WAV 资源）
class MusicPlayer {
private:
    std::wstring musicFolder;
    std::wstring lastInspectDebugInfo;
    int volume = 200; // MCI 音量 0-1000（200 ≈ 20%，较小声）

    int toMciVolume(int vol) const;
    void setMciVolume(const wchar_t* alias);
    void playMci(const std::wstring& path, const wchar_t* alias, bool loop);
    void closeMci(const wchar_t* alias);
    bool isPlaying(const wchar_t* alias);
    bool extractWaveResourceToTemp(int resourceId, const std::wstring& fileName);
    std::wstring resolveEmbeddedWavePath();
    std::wstring resolveInspectSkillPath();
    std::wstring resolveFallbackWavePath() const;
    std::wstring resolveMusicPath();

public:
    MusicPlayer() = default;

    void setMusicFolder(const std::wstring& folder) { musicFolder = folder; }
    int getVolume() const { return volume; }
    bool isMuted() const { return volume == 0; }

    void playFlip();
    void playSelect();
    void playMove();
    void playCapture();
    void playInspectSkill();
    const std::wstring& getLastInspectDebugInfo() const { return lastInspectDebugInfo; }

    void setVolume(int vol);
    void playBackgroundMusic();
    void stopBackgroundMusic();
    bool isBackgroundMusicPlaying();
    void playVictoryMusic();
    bool isVictoryMusicPlaying();
    void stopAll();
};
