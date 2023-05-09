#if BUILD_DEBUG
#include "game.h"
#else
#include "game.h"
#endif

#include <xaudio2.h>

#if BUILD_DEBUG
#define Assert(expression) if(!(expression)) { *(int*)0 = 0; }
#else
#define Assert(expression)
#endif

#define Kilobytes(value) ((value) * 1024ULL)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)

#define BASE_RENDER_RESOLUSION_WIDTH  640 / 4
#define BASE_RENDER_RESOLUSION_HEIGHT 480 / 4

#define WINDOW_CLIENT_WIDTH  640
#define WINDOW_CLIENT_HEIGHT 480

global BOOL GlobalIsRunning;
global BOOL GlobalIsPause;

global Shared_data global_shared_data;

typedef Offscreen_buffer OffscreenBuffer;
struct OffscreenBufferWrapper
{
    OffscreenBuffer offscreen;
    BITMAPINFO      info;
};

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
typedef bool32 Initialize_game_function(Shared_data shared_data);
typedef void Update_game_function(Shared_data shared_data);
typedef void Render_game_function(Shared_data shared_data);

global Initialize_game_function* initialize_game;
global Update_game_function* update_game;
global Render_game_function* render_game;

global HMODULE GlobalGameDll;
global WCHAR* GlobalGameDllName = L"../build/game.dll";
global WCHAR* GlobalTemporaryGameDllName = L"../build/game_temporary.dll";
#endif
