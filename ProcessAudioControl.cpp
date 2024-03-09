#include "ProcessAudioControl.h"

ProcessAudioControl::~ProcessAudioControl()
{
    cleanup();
}

void ProcessAudioControl::selectExecutableName(const std::wstring& _exe)
{
    exe = _exe;
    selectExecutableName();
}

void ProcessAudioControl::setVolume(float _value)
{
    if (_value < 0.0)
    {
        _value = 0.0;
    }
    if (_value > 1.0)
    {
        _value = 1.0;
    }
    
    findChannel();
    // set volume on all channels
    // channels here refer to speakers e.g. left speaker right speaker
    UINT32 count = 0;
    channel->GetChannelCount(&count);
    for (size_t i = 0; i < count; i++)
    {
        channel->SetChannelVolume(i, _value, NULL);
    }
}

float ProcessAudioControl::getVolume()
{
    findChannel();
    
    float max = 0;
    UINT32 count = 0;
    channel->GetChannelCount(&count);
    for (size_t i = 0; i < count; i++)
    {
        float volume = 0;
        channel->GetChannelVolume(i, &volume);
        if (volume > max)
        {
            max = volume;
        }
    }
    return max;
}

void ProcessAudioControl::cleanup()
{
    if (!needsCleanup)
    {
        return;
    }
    if (channel)
    {
        channel->Release();
        channel = nullptr;
    }

    CoUninitialize();
}

void ProcessAudioControl::selectExecutableName()
{
    PROCESSENTRY32 entry{ 0 };
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (memcmp(exe.c_str(), entry.szExeFile, sizeof(L"Spotify.exe")))
            {
                continue;
            }
            if (hwnd = getPIDHWND(entry.th32ProcessID); hwnd)
            {
                pid = entry.th32ProcessID;
                CloseHandle(snapshot);
                return;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

HWND ProcessAudioControl::getPIDHWND(DWORD _pid) const
{
    std::pair<HWND, DWORD> params = { 0, _pid };

    // Enumerate the windows using a lambda to process each window
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            auto pParams = (std::pair<HWND, DWORD>*)(lParam);

            DWORD processId;
            if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
            {
                // Stop enumerating
                SetLastError(-1);
                pParams->first = hwnd;
                return FALSE;
            }

            // Continue enumerating
            return TRUE;
        }, (LPARAM)&params);

    if (!bResult && GetLastError() == -1 && params.first)
    {
        return params.first;
    }

    return NULL;
}

void ProcessAudioControl::findChannel()
{
    if (channel)
    {
        return;
    }
    CoInitialize(NULL);

    needsCleanup = true;


    IMMDeviceEnumerator* deviceEnumerator = NULL;
    IMMDevice* defaultDevice = NULL;
    IAudioSessionManager2* sessionManager = NULL;

    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    defaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);

    IAudioSessionEnumerator* audioEnum;

    sessionManager->GetSessionEnumerator(&audioEnum);

    int count = 0;
    audioEnum->GetCount(&count);

    for (size_t i = 0; i < count; i++)
    {
        IAudioSessionControl* session;
        audioEnum->GetSession(i, &session);

        IAudioSessionControl2* session2;
        session->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&session2);

        DWORD id;
        session2->GetProcessId(&id);
        if (id == pid)

        {
            session2->QueryInterface(__uuidof(IChannelAudioVolume), (LPVOID*)&channel);
            session->Release();
            session2->Release();
            break;
        }

        session->Release();
        session2->Release();
    }
    audioEnum->Release();

    sessionManager->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
}

void ProcessAudioControl::selectProcess(DWORD _pid)
{
    pid = _pid;
    hwnd = getPIDHWND(_pid);
}

void ProcessAudioControl::stop() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_STOP));
}

void ProcessAudioControl::play() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_PLAY));
}

void ProcessAudioControl::pause() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_PAUSE));
}

void ProcessAudioControl::playPause() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_PLAY_PAUSE));
}

void ProcessAudioControl::nextTrack() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_NEXTTRACK));
}

void ProcessAudioControl::prevTrack() const
{
    SendMessage(hwnd, WM_APPCOMMAND, 0, MAKELONG(0, APPCOMMAND_MEDIA_PREVIOUSTRACK));
}

HWND ProcessAudioControl::getHWND() const
{
    return hwnd;
}

bool ProcessAudioControl::isValid() const
{
    return IsWindow(hwnd);
}

void ProcessAudioControl::volumeUp(float _value)
{
    setVolume(getVolume() + _value);
}

void ProcessAudioControl::volumeDown(float _value)
{
    setVolume(getVolume() - _value);
}
