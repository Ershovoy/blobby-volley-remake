#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef double DOUBLE;
#include <gl/GL.h>

#include "windows_main.h"

#if BUILD_DEBUG
void UnloadGameCode()
{
    if (GlobalGameDll)
    {
        FreeLibrary(GlobalGameDll);
    }
}

void LoadGameCode()
{
    CopyFileW(GlobalGameDllName, GlobalTemporaryGameDllName, FALSE);

    GlobalGameDll = LoadLibraryW(GlobalTemporaryGameDllName);

    if (GlobalGameDll)
    {
        initialize_game = (Initialize_game_function*)GetProcAddress(GlobalGameDll, "initialize_game");
        update_game = (Update_game_function*)GetProcAddress(GlobalGameDll, "update_game");
        render_game = (Render_game_function*)GetProcAddress(GlobalGameDll, "render_game");
    }
}
#else
#include "game.cpp"
#endif

File ReadFile(char16* file_name)
{
    File result = {};

    HANDLE fileHandle = CreateFileW(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle == INVALID_HANDLE_VALUE)
        return result;

    DWORD fileSize = GetFileSize(fileHandle, NULL);
    if (fileSize != INVALID_FILE_SIZE)
    {
        result.memory = VirtualAlloc(0, fileSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (result.memory)
        {
            // TODO: Read file function can read maximum 2048 megabytes, due to size of its third parameter.
            //       How can we get around it?
            if (!(ReadFile(fileHandle, result.memory, fileSize, (LPDWORD)&result.size, 0) &&
                  ((uint32)result.size == fileSize)))
            {
                result = {};
                VirtualFree(result.memory, 0, MEM_RELEASE);
            }
        }
    }

    CloseHandle(fileHandle);
    return result;
}

bool32 WriteFile(char16* file_name, File file)
{
    bool32 result = false;

    HANDLE FileHandle = CreateFileW(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle == INVALID_HANDLE_VALUE)
        return result;

    DWORD bytesWritten;
    if (WriteFile(FileHandle, file.memory, file.size, &bytesWritten, 0))
    {
        result = ((uint32)file.size == bytesWritten);
    }

    CloseHandle(FileHandle);
    return result;
}

struct WaveHeader
{
    char8 riffId[4];
    uint32 waveHeaderSize;
    char8 waveId[4];
    char8 fmtId[4];
    uint32 fmtSize;
    uint16 audioFormat;
    uint16 numChannels;
    uint32 sampleRate;
    uint32 bytesPerSecond;
    uint16 blockAlign;
    uint16 bitsPerSample;
    char8 dataId[4];
    uint32 dataSize;
};

FILETIME GetLastFileWriteTime(WCHAR* fileName)
{
    FILETIME lastFileWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if(GetFileAttributesExW(fileName, GetFileExInfoStandard, &fileAttributeData))
    {
        lastFileWriteTime = fileAttributeData.ftLastWriteTime;
    }

    return lastFileWriteTime;
}

OffscreenBufferWrapper ResizeOffscreenBuffer(HWND window)
{
    RECT clientRectangle;
    GetClientRect(window, &clientRectangle);
    LONG clientWidth  = BASE_RENDER_RESOLUSION_WIDTH;
    LONG clientHeight = BASE_RENDER_RESOLUSION_HEIGHT;

    BITMAPINFO bitmapInfo = {};
    bitmapInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth       = clientWidth;
    bitmapInfo.bmiHeader.biHeight      = clientHeight;
    bitmapInfo.bmiHeader.biPlanes      = 1;
    bitmapInfo.bmiHeader.biBitCount    = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    OffscreenBuffer offscreen = {};
    offscreen.width      = clientWidth;
    offscreen.height     = clientHeight;
    offscreen.pixel_size = bitmapInfo.bmiHeader.biBitCount / 4;
    offscreen.size       = clientWidth * clientHeight * offscreen.pixel_size;
    offscreen.memory     = VirtualAlloc(0, (SIZE_T)offscreen.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    OffscreenBufferWrapper result = {};
    result.info      = bitmapInfo;
    result.offscreen = offscreen;

    return result;
}

LONG64 GetCounter()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

LONG64 GetCounterFrequency()
{
    LARGE_INTEGER counterFrequency;
    QueryPerformanceFrequency(&counterFrequency);
    return counterFrequency.QuadPart;
}

BOOL InitializeOpenGL(HDC deviceContext)
{
    PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
    desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    desiredPixelFormat.nVersion = 1;
    desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    desiredPixelFormat.cColorBits = 32;
    desiredPixelFormat.cAlphaBits = 8;

    int suggestedPixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
    PIXELFORMATDESCRIPTOR suggestedPixelFormat;
    if (!DescribePixelFormat(deviceContext, suggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &suggestedPixelFormat))
        return FALSE;

    if (!SetPixelFormat(deviceContext, suggestedPixelFormatIndex, &suggestedPixelFormat))
        return FALSE;

    HGLRC openGlRenderingContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, openGlRenderingContext);

    return TRUE;
}

LRESULT CALLBACK WindowProcedure(_In_ HWND   window,
                                 _In_ UINT   message,
                                 _In_ WPARAM wParam,
                                 _In_ LPARAM lParam)
{
    LRESULT result = {};
    switch(message)
    {
        case WM_DESTROY:
        {
            GlobalIsRunning = false;
            PostQuitMessage(0);
            break;
        }
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            WORD vkCode = LOWORD(wParam);
            WORD keyFlags = HIWORD(lParam);

            BOOL isKeyDown = (keyFlags & KF_UP) != KF_UP;
            BOOL wasKeyDown = (keyFlags & KF_REPEAT) == KF_REPEAT;

            if (isKeyDown != wasKeyDown)
            {
                switch (vkCode)
                {
                    case 'W':
                    case VK_UP:
                    {
                        global_shared_data.keyboard.key_w = isKeyDown;
                        break;
                    }
                    case 'A':
                    case VK_LEFT:
                    {
                        global_shared_data.keyboard.key_a = isKeyDown;
                        break;
                    }
                    case 'S':
                    case VK_DOWN:
                    {
                        global_shared_data.keyboard.key_s = isKeyDown;
                        break;
                    }
                    case 'D':
                    case VK_RIGHT:
                    {
                        global_shared_data.keyboard.key_d = isKeyDown;
                        break;
                    }
                    case 'P':
                    {
                        if(isKeyDown)
                        {
                            GlobalIsPause = !GlobalIsPause;
                        }
                        break;
                    }
                }
            }
            break;
        }
        case WM_KILLFOCUS:
        {
            global_shared_data.keyboard = {};
            break;
        }
        default:
        {
            result = DefWindowProcW(window, message, wParam, lParam);
            break;
        }
    }
    return result;
}

HWND InitializeWindow(HINSTANCE instance)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize        = sizeof(WNDCLASSEXW);
    windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc   = WindowProcedure;
    windowClass.hInstance     = instance;
    // TODO: Do we need LR_LOADTRANSPARENT?
    windowClass.hIcon         = (HICON)LoadImageW(NULL, L"game_icon.ico", IMAGE_ICON,
                                                  0, 0, LR_LOADFROMFILE | LR_LOADTRANSPARENT);
    windowClass.hIconSm       = windowClass.hIcon;
    windowClass.hCursor       = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    windowClass.lpszClassName = L"Road Fighter Remake";

    RegisterClassExW(&windowClass);

    // TODO: Do we need WS_EX_OVERLAPPEDWINDOW?
    DWORD windowExtendedStyle = WS_EX_OVERLAPPEDWINDOW;
    DWORD windowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_VISIBLE;

    RECT clientRectangle = { 0, 0, WINDOW_CLIENT_WIDTH, WINDOW_CLIENT_HEIGHT };
    RECT windowRectangle = clientRectangle;
    AdjustWindowRectEx(&windowRectangle, windowStyle, FALSE, windowExtendedStyle);
    LONG requiredWindowWidth = windowRectangle.right - windowRectangle.left;
    LONG requiredWindowHeight = windowRectangle.bottom - windowRectangle.top;

    HWND window = CreateWindowExW(windowExtendedStyle, windowClass.lpszClassName,
                                  windowClass.lpszClassName, windowStyle,
                                  CW_USEDEFAULT, CW_USEDEFAULT, requiredWindowWidth, requiredWindowHeight,
                                  NULL, NULL, instance, NULL);

    return window;
}

AudioManager InitializeAudioManager()
{
        AudioManager result = {};

        if (FAILED(XAudio2Create(&result.xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
            return result;

        if (FAILED(result.xAudio2->CreateMasteringVoice(&result.masterVoice)))
            return result;

        WAVEFORMATEX waveFormat = {};
        waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
        waveFormat.nChannels       = 2;
        waveFormat.nSamplesPerSec  = 48000;
        waveFormat.wBitsPerSample  = 16;
        waveFormat.nBlockAlign     = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize          = 0;

        result.waveFormat = waveFormat;
        result.isInitialized = true;
        return result;
}

int WINAPI wWinMain(_In_     HINSTANCE instance,
                    _In_opt_ HINSTANCE previousInstance,
                    _In_     LPWSTR    lpCmdLine,
                    _In_     int       nShowCmd)
{
#if BUILD_DEBUG
    LoadGameCode();
    FILETIME previousFileWriteTime = GetLastFileWriteTime(GlobalGameDllName);
#endif

    HWND window = InitializeWindow(instance);
    if (window)
    {
        HDC deviceContext = GetDC(window);

        GlobalAudioManager = InitializeAudioManager();
        if (!GlobalAudioManager.isInitialized)
            return -1;

        IXAudio2SourceVoice* SourceVoice;

        if (FAILED(GlobalAudioManager.xAudio2->CreateSourceVoice(&SourceVoice, &GlobalAudioManager.waveFormat)))
            return -1;

        File soundFile = DebugReadFile(L"sample.wav");
        if (soundFile.memory == 0)
            return -1;

        WaveHeader* waveHeader = (WaveHeader*)soundFile.memory;

        XAUDIO2_BUFFER audioBuffer = {};
        audioBuffer.AudioBytes = waveHeader->dataSize;
        audioBuffer.pAudioData = (BYTE*)(waveHeader + 1);
        audioBuffer.Flags      = 0;
        audioBuffer.LoopCount  = XAUDIO2_LOOP_INFINITE;

        if (FAILED(SourceVoice->SubmitSourceBuffer(&audioBuffer)))
            return -1;

        if (FAILED(SourceVoice->Start()))
            return -1;

#if 0
        if (!InitializeOpenGL(deviceContext))
            return -1;
#endif

        OffscreenBufferWrapper offscreenWrapper = ResizeOffscreenBuffer(window);
        global_shared_data.offscreen = offscreenWrapper.offscreen;
#if BUILD_DEBUG
        LPVOID baseAdress = (LPVOID)Terabytes(4);
#else
        LPVOID baseAdress = (LPVOID)0;
#endif
        global_shared_data.game_storage.size = Megabytes(256);
        // TODO: Specify MEM_LARGE_PAGES and enable necessary token privileges, so that allocated pages will be larger than 4 kilobytes.
        global_shared_data.game_storage.memory = VirtualAlloc(baseAdress, global_shared_data.game_storage.size,
                                                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        global_shared_data.platform_read_file = ReadFile;
        global_shared_data.platform_write_file = WriteFile;

        LONG64 perfomanceCounterFrequency = GetCounterFrequency();
        DOUBLE secondsPerUpdate = 1.0 / 60.0;

        LONG64 previousCounter = GetCounter();
        DOUBLE accumulator = 0.0;

        // TODO: Launch the game in seperate from window thread, so its update/render function won't freeze after focus lost.
        GlobalIsRunning = initialize_game(global_shared_data);
        while (GlobalIsRunning)
        {
#if BUILD_DEBUG
            FILETIME currentFileWriteTime = GetLastFileWriteTime(GlobalGameDllName);
            if (CompareFileTime(&previousFileWriteTime, &currentFileWriteTime) != 0)
            {
                UnloadGameCode();
                LoadGameCode();
            }
#endif
            MSG message = {};
            while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }

            LONG64 currentCounter = GetCounter();
            LONG64 counterElapsed = currentCounter - previousCounter;
            if (!GlobalIsPause)
            {
                DOUBLE secondsElapsed = (DOUBLE)counterElapsed / perfomanceCounterFrequency;
                accumulator += secondsElapsed;
            }
            previousCounter = currentCounter;
            while (accumulator >= secondsPerUpdate)
            {
                update_game(global_shared_data);

                accumulator -= secondsPerUpdate;
                if (accumulator < secondsPerUpdate)
                {
                    render_game(global_shared_data);
                }
            }
            StretchDIBits(deviceContext,
                          0, 0, WINDOW_CLIENT_WIDTH, WINDOW_CLIENT_HEIGHT,
                          0, 0, offscreenWrapper.offscreen.width, offscreenWrapper.offscreen.height,
                          offscreenWrapper.offscreen.memory, &offscreenWrapper.info,
                          DIB_RGB_COLORS, SRCCOPY);
#if 0
            glViewport(0, 0, WINDOW_CLIENT_WIDTH, WINDOW_CLIENT_HEIGHT);
            glClearColor(1.0f, 0.0f, 1.0, 0.5f);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(deviceContext);
#endif
            Sleep(1);
        }
    }
#if BUILD_DEBUG
    UnloadGameCode();
#endif
    return 0;
}
