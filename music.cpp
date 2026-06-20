#include "music.h"

#include "Resource.h"

#include <fstream>

namespace {
constexpr int kEmbeddedBgmResourceId = IDR_BGM_WAV;
constexpr int kInspectSkillResourceId = IDR_INSPECT_WAV;
constexpr wchar_t kBackgroundAlias[] = L"bgm";
constexpr wchar_t kVictoryAlias[] = L"victory";
constexpr wchar_t kInspectAlias[] = L"inspect";
constexpr wchar_t kFallbackFileName[] = L"詹雯婷 - 命运火焰.wav";
constexpr wchar_t kEmbeddedTempFileName[] = L"army_embedded_bgm.wav";
constexpr wchar_t kInspectSkillTempFileName[] = L"army_inspect_skill.wav";
}

int MusicPlayer::toMciVolume(int vol) const {
    long long scaled = 1LL * vol * vol / 1000;
    if (scaled < 0) scaled = 0;
    if (scaled > 1000) scaled = 1000;
    return static_cast<int>(scaled);
}

void MusicPlayer::setMciVolume(const wchar_t* alias) {
    std::wstring volCmd = L"setaudio " + std::wstring(alias) + L" volume to " + std::to_wstring(toMciVolume(volume));
    mciSendString(volCmd.c_str(), NULL, 0, NULL);
}

void MusicPlayer::playMci(const std::wstring& path, const wchar_t* alias, bool loop) {
    if (path.empty()) return;

    closeMci(alias);
    std::wstring cmd = L"open \"" + path + L"\" type waveaudio alias " + alias;
    if (mciSendString(cmd.c_str(), NULL, 0, NULL) != 0) return;

    std::wstring play = L"play " + std::wstring(alias);
    if (loop) {
        if (mciSendString(play.c_str(), NULL, 0, NULL) != 0) return;
        mciSendString((play + L" repeat").c_str(), NULL, 0, NULL);
    } else {
        mciSendString(play.c_str(), NULL, 0, NULL);
    }

    setMciVolume(alias);
}

void MusicPlayer::closeMci(const wchar_t* alias) {
    mciSendString((L"close " + std::wstring(alias)).c_str(), NULL, 0, NULL);
}

bool MusicPlayer::isPlaying(const wchar_t* alias) {
    wchar_t status[64] = {};
    if (mciSendString((std::wstring(L"status ") + alias + L" mode").c_str(), status, 64, NULL) != 0) {
        return false;
    }
    return wcscmp(status, L"playing") == 0;
}

bool MusicPlayer::extractWaveResourceToTemp(int resourceId, const std::wstring& fileName) {
    HMODULE module = GetModuleHandleW(NULL);
    if (module == NULL) return false;

    HRSRC resourceInfo = FindResourceW(module, MAKEINTRESOURCEW(resourceId), L"WAVE");
    if (resourceInfo == NULL) return false;

    HGLOBAL resource = LoadResource(module, resourceInfo);
    if (resource == NULL) return false;

    DWORD resourceSize = SizeofResource(module, resourceInfo);
    if (resourceSize == 0) return false;

    const void* resourceData = LockResource(resource);
    if (resourceData == NULL) return false;

    wchar_t tempPathBuffer[MAX_PATH] = {};
    DWORD tempPathLen = GetTempPathW(MAX_PATH, tempPathBuffer);
    if (tempPathLen == 0 || tempPathLen >= MAX_PATH) return false;

    std::filesystem::path outputDir = std::filesystem::path(tempPathBuffer) / L"Army";
    std::filesystem::create_directories(outputDir);
    std::filesystem::path outputPath = outputDir / fileName;

    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;

    out.write(static_cast<const char*>(resourceData), static_cast<std::streamsize>(resourceSize));
    out.close();

    return out.good();
}

std::wstring MusicPlayer::resolveEmbeddedWavePath() {
    wchar_t tempPathBuffer[MAX_PATH] = {};
    DWORD tempPathLen = GetTempPathW(MAX_PATH, tempPathBuffer);
    if (tempPathLen == 0 || tempPathLen >= MAX_PATH) return L"";

    std::filesystem::path outputPath = std::filesystem::path(tempPathBuffer) / L"Army" / kEmbeddedTempFileName;
    if (!extractWaveResourceToTemp(kEmbeddedBgmResourceId, kEmbeddedTempFileName) && !std::filesystem::exists(outputPath)) {
        return L"";
    }

    return outputPath.wstring();
}

std::wstring MusicPlayer::resolveInspectSkillPath() {
    wchar_t tempPathBuffer[MAX_PATH] = {};
    DWORD tempPathLen = GetTempPathW(MAX_PATH, tempPathBuffer);
    if (tempPathLen == 0 || tempPathLen >= MAX_PATH) return L"";

    std::filesystem::path outputPath = std::filesystem::path(tempPathBuffer) / L"Army" / kInspectSkillTempFileName;
    if (!extractWaveResourceToTemp(kInspectSkillResourceId, kInspectSkillTempFileName) && !std::filesystem::exists(outputPath)) {
        return L"";
    }

    return outputPath.wstring();
}

std::wstring MusicPlayer::resolveFallbackWavePath() const {
    if (musicFolder.empty()) return L"";

    std::filesystem::path fallbackPath = std::filesystem::path(musicFolder) / kFallbackFileName;
    if (!std::filesystem::exists(fallbackPath)) {
        return L"";
    }

    return fallbackPath.wstring();
}

std::wstring MusicPlayer::resolveMusicPath() {
    std::wstring embeddedPath = resolveEmbeddedWavePath();
    if (!embeddedPath.empty()) {
        return embeddedPath;
    }
    return resolveFallbackWavePath();
}

void MusicPlayer::playFlip() {
    if (!isMuted()) PlaySound(L"SystemAsterisk", NULL, SND_ASYNC | SND_ALIAS);
}

void MusicPlayer::playSelect() {
    if (!isMuted()) PlaySound(L"SystemDefault", NULL, SND_ASYNC | SND_ALIAS);
}

void MusicPlayer::playMove() {
    if (!isMuted()) PlaySound(L"SystemHand", NULL, SND_ASYNC | SND_ALIAS);
}

void MusicPlayer::playCapture() {
    if (!isMuted()) PlaySound(L"SystemExclamation", NULL, SND_ASYNC | SND_ALIAS);
}

void MusicPlayer::playInspectSkill() {
    #pragma region debug-point inspect-skill-play
    HMODULE module = GetModuleHandleW(NULL);
    HRSRC resourceInfo = module ? FindResourceW(module, MAKEINTRESOURCEW(kInspectSkillResourceId), L"WAVE") : NULL;
    DWORD resourceSize = (module && resourceInfo) ? SizeofResource(module, resourceInfo) : 0;
    if (isMuted()) {
        lastInspectDebugInfo = L"验牌音效调试: muted=1 res=" + std::to_wstring(resourceInfo != NULL) +
            L" size=" + std::to_wstring(resourceSize) + L" play=-1";
        return;
    }

    std::wstring inspectPath = resolveInspectSkillPath();
    bool bgmPlaying = isPlaying(kBackgroundAlias);
    if (!inspectPath.empty()) {
        playMci(inspectPath, kInspectAlias, false);
    }
    lastInspectDebugInfo = L"验牌音效调试: muted=0 res=" + std::to_wstring(resourceInfo != NULL) +
        L" size=" + std::to_wstring(resourceSize) + L" file=" + std::to_wstring(!inspectPath.empty()) +
        L" bgm=" + std::to_wstring(bgmPlaying);
    #pragma endregion
}

void MusicPlayer::setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 1000) vol = 1000;
    volume = vol;

    if (isMuted()) {
        PlaySound(NULL, NULL, 0);
        stopAll();
        return;
    }

    setMciVolume(kBackgroundAlias);
    setMciVolume(kVictoryAlias);
}

void MusicPlayer::playBackgroundMusic() {
    if (isMuted()) return;
    playMci(resolveMusicPath(), kBackgroundAlias, true);
}

void MusicPlayer::stopBackgroundMusic() {
    closeMci(kBackgroundAlias);
}

bool MusicPlayer::isBackgroundMusicPlaying() {
    return isPlaying(kBackgroundAlias);
}

void MusicPlayer::playVictoryMusic() {
    if (isMuted()) return;
    closeMci(kBackgroundAlias);
    playMci(resolveMusicPath(), kVictoryAlias, false);
}

bool MusicPlayer::isVictoryMusicPlaying() {
    return isPlaying(kVictoryAlias);
}

void MusicPlayer::stopAll() {
    closeMci(kBackgroundAlias);
    closeMci(kInspectAlias);
    closeMci(kVictoryAlias);
}
