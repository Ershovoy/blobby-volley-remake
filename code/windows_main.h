#include "game.h"

#define BASE_RENDER_RESOLUTION_WIDTH  800
#define BASE_RENDER_RESOLUTION_HEIGHT 600

#define WINDOW_CLIENT_WIDTH  800
#define WINDOW_CLIENT_HEIGHT 600

global BOOL GlobalIsRunning;
global BOOL GlobalIsPause;

global Key_code_id GlobalKeyCodeMap[KEY_COUNT];

global Shared_data global_shared_data;

typedef Offscreen_buffer OffscreenBuffer;
struct OffscreenBufferWrapper
{
    OffscreenBuffer offscreen;
    BITMAPINFO      info;
};

global OffscreenBufferWrapper GlobalOffscreenWrapper;

struct AudioVoice : IXAudio2VoiceCallback
{
    IXAudio2SourceVoice* sourceVoice;

    // Called when the voice has just finished playing a contiguous audio stream.
    void COM_DECLSPEC_NOTHROW OnStreamEnd() { }

    // Unused methods are stubs
    void COM_DECLSPEC_NOTHROW OnVoiceProcessingPassEnd() {}
    void COM_DECLSPEC_NOTHROW OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
    void COM_DECLSPEC_NOTHROW OnBufferEnd(void* pBufferContext) {}
    void COM_DECLSPEC_NOTHROW OnBufferStart(void* pBufferContext) {}
    void COM_DECLSPEC_NOTHROW OnLoopEnd(void* pBufferContext) {}
    void COM_DECLSPEC_NOTHROW OnVoiceError(void* pBufferContext, HRESULT Error) {}
};

struct AudioManager
{
    bool32 isInitialized;

    IXAudio2* xAudio2;
    IXAudio2MasteringVoice* masterVoice;

    WAVEFORMATEX waveFormat;

    XAUDIO2_BUFFER audioBuffer[256];

    IXAudio2SourceVoice* sourceVoice[64];
};

global AudioManager GlobalAudioManager;

#if BUILD_DEBUG
typedef bool32 Initialize_game_function(Shared_data* shared_data);
typedef void Update_game_function(Shared_data* shared_data);
typedef void Render_game_function(Shared_data* shared_data);

global Initialize_game_function* initialize_game;
global Update_game_function* update_game;
global Render_game_function* render_game;

global HMODULE GlobalGameDll;
global WCHAR* GlobalGameDllName = L"../build/game.dll";
global WCHAR* GlobalTemporaryGameDllName = L"../build/game_temporary.dll";
#endif
