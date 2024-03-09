#include <string>
#include <audiopolicy.h>
#include <tlhelp32.h>
#include <Mmdeviceapi.h>

#pragma comment(lib, "Kernel32.lib")


class ProcessAudioControl {
public:
    ProcessAudioControl() : channel(nullptr), hwnd(NULL), pid(NULL), needsCleanup(false) {};
    ~ProcessAudioControl();

    void selectExecutableName(const std::wstring& _exe);

    void selectProcess(DWORD _pid);

    void stop() const;

    void play() const;

    void pause() const;

    void playPause() const;

    void nextTrack() const;

    void prevTrack() const;

    HWND getHWND() const;

    bool isValid() const;

    // @param _value between 0.0 and 1.0
    void volumeUp(float _value);
    // @param _value between 0.0 and 1.0
    void volumeDown(float _value);
    // @param _value between 0.0 and 1.0
    void setVolume(float _value);

    float getVolume();

    void cleanup();
private:
    void selectExecutableName();

    HWND getPIDHWND(DWORD _pid) const;

    void findChannel();

    IChannelAudioVolume* channel;

    HWND hwnd;

    DWORD pid;

    std::wstring exe;

    bool needsCleanup;
};