#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN_32_EXTRA_LEAN

#include <Windows.h>
#include "../core/base.h"

#ifndef NDEBUG
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <new.h>
#endif

HINSTANCE Win32_instance;
HGLRC Win32_render_context;
HWND Win32_window;
WNDCLASSA Win32_window_class;
HDC Win32_window_dc;
BITMAPINFO Win32_bitmap_info;
RECT Win32_window_rect;
RAWINPUT Win32_raw_inputs;
HRAWINPUT Win32_raw_input_handle;
RAWINPUTDEVICE Win32_raw_input_device;
UINT Win32_raw_input_size;
PUINT Win32_raw_input_size_ptr = (PUINT)(&Win32_raw_input_size);
UINT Win32_raw_input_header_size = sizeof(RAWINPUTHEADER);

#ifndef NDEBUG
void Win32_DisplayError(LPTSTR lpszFunction) {
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    unsigned int last_error = GetLastError();

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, last_error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf, 0, nullptr);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

    if (FAILED( StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                                TEXT("%s failed with error code %d as follows:\n%s"), lpszFunction, last_error, lpMsgBuf)))
        printf("FATAL ERROR: Unable to output error code.\n");

    _tprintf((LPTSTR)"ERROR: %s\n", (LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}
#endif

void win32_closeFile(void *handle) {
    CloseHandle(handle);
}

void* win32_openFileForReading(const char* path) {
    HANDLE handle = CreateFileA(path,           // file to open
                                GENERIC_READ,          // open for reading
                                FILE_SHARE_READ,       // share for reading
                                nullptr,                  // default security
                                OPEN_EXISTING,         // existing file only
                                FILE_ATTRIBUTE_NORMAL, // normal file
                                nullptr);                 // no attr. template
#ifndef NDEBUG
    if (handle == INVALID_HANDLE_VALUE) {
        Win32_DisplayError((LPTSTR)"CreateFile");
        _tprintf((LPTSTR)"Terminal failure: unable to open file \"%s\" for read.\n", path);
        return nullptr;
    }
#endif
    return handle;
}

void* win32_openFileForWriting(const char* path) {
    HANDLE handle = CreateFileA(path,           // file to open
                                GENERIC_WRITE,          // open for writing
                                0,                      // do not share
                                nullptr,                   // default security
                                OPEN_ALWAYS,            // create new or open existing
                                FILE_ATTRIBUTE_NORMAL,  // normal file
                                nullptr);
#ifndef NDEBUG
    if (handle == INVALID_HANDLE_VALUE) {
        Win32_DisplayError((LPTSTR)"CreateFile");
        _tprintf((LPTSTR)"Terminal failure: unable to open file \"%s\" for write.\n", path);
        return nullptr;
    }
#endif
    return handle;
}

bool win32_readFromFile(LPVOID out, DWORD size, HANDLE handle) {
    DWORD bytes_read = 0;
    BOOL result = ReadFile(handle, out, size, &bytes_read, nullptr);
#ifndef NDEBUG
    if (result == FALSE) {
        Win32_DisplayError((LPTSTR)"ReadFile");
        printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", (unsigned int)GetLastError());
        CloseHandle(handle);
    }
#endif
    return result != FALSE;
}

bool win32_writeToFile(LPVOID out, DWORD size, HANDLE handle) {
    DWORD bytes_written = 0;
    BOOL result = WriteFile(handle, out, size, &bytes_written, nullptr);
#ifndef NDEBUG
    if (result == FALSE) {
        Win32_DisplayError((LPTSTR)"WriteFile");
        printf("Terminal failure: Unable to write to file.\n GetLastError=%08x\n", (unsigned int)GetLastError());
        CloseHandle(handle);
    }
#endif
    return result != FALSE;
}

long long int win32_getFileSizeWithoutOpening(const char* file_path) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(file_path, GetFileExInfoStandard, &fad)) {
#ifndef NDEBUG
        Win32_DisplayError((LPTSTR)"GetFileAttributesEx");
        printf("Terminal failure: Unable to get the file size attribute.\n GetLastError=%08x\n", (unsigned int)GetLastError());
#endif
        return -1;
    }

    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

long long int win32_getFileSize(HANDLE handle) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size)) {
#ifndef NDEBUG
        Win32_DisplayError((LPTSTR)"GetFileSizeEx");
        printf("Terminal failure: Unable to get the file size.\n GetLastError=%08x\n", (unsigned int)GetLastError());
#endif
        return -1;
    }

    return size.QuadPart;
}

void* win32_readEntireFile(const char* file_path, u64 *out_size) {
    HANDLE handle = CreateFileA(file_path,           // file to open
                                GENERIC_READ,          // open for reading
                                FILE_SHARE_READ,       // share for reading
                                nullptr,                  // default security
                                OPEN_EXISTING,         // existing file only
                                FILE_ATTRIBUTE_NORMAL, // normal file
                                nullptr);                 // no attr. template
#ifndef NDEBUG
    if (handle == INVALID_HANDLE_VALUE) {
        Win32_DisplayError((LPTSTR)"CreateFile");
        _tprintf((LPTSTR)"Terminal failure: unable to open file \"%s\" for read.\n", file_path);
        return nullptr;
    }
#endif

    LARGE_INTEGER large_size;
    if (!GetFileSizeEx(handle, &large_size)) {
#ifndef NDEBUG
        Win32_DisplayError((LPTSTR)"GetFileSizeEx");
        printf("Terminal failure: Unable to get the file size.\n GetLastError=%08x\n", (unsigned int)GetLastError());
#endif
        return nullptr;
    }

    *out_size = large_size.QuadPart;
    void *out = VirtualAlloc(nullptr, (SIZE_T)(*out_size), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    DWORD bytes_read = 0;
    BOOL result = ReadFile(handle, out, (DWORD)large_size.QuadPart, &bytes_read, nullptr);
#ifndef NDEBUG
    if (result == FALSE) {
        Win32_DisplayError((LPTSTR)"ReadFile");
        printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", (unsigned int)GetLastError());
        CloseHandle(handle);
        return nullptr;
    }
#endif

    CloseHandle(handle);

    return out;
}

LARGE_INTEGER performance_counter;

void os::setWindowTitle(char* str) {
    window::title = str;
    SetWindowTextA(Win32_window, str);
}

void os::setCursorVisibility(bool on) {
    ShowCursor(on);
}

void os::setWindowCapture(bool on) {
    if (on) SetCapture(Win32_window);
    else ReleaseCapture();
}

u64 timers::getTicks() {
    QueryPerformanceCounter(&performance_counter);
    return (u64)performance_counter.QuadPart;
}

void* os::getMemory(u64 size, u64 base) {
    return VirtualAlloc((LPVOID)base, (SIZE_T)size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void os::freeMemory(void* memory) {
    VirtualFree((void*)memory, 0, MEM_RELEASE);
}

void os::closeFile(void *handle) { return win32_closeFile(handle); }
void* os::openFileForReading(const char* path) { return win32_openFileForReading(path); }
void* os::openFileForWriting(const char* path) { return win32_openFileForWriting(path); }
bool os::readFromFile(LPVOID out, DWORD size, HANDLE handle) { return win32_readFromFile(out, size, handle); }
bool os::writeToFile(LPVOID out, DWORD size, HANDLE handle) { return win32_writeToFile(out, size, handle); }
long long int os::getFileSizeWithoutOpening(const char* path) { return win32_getFileSizeWithoutOpening(path); }
long long int os::getFileSize(void *handle) { return win32_getFileSize(handle); }
void*  os::readEntireFile(const char* file_path, u64 *out_size) { return win32_readEntireFile(file_path, out_size); }

void os::print(const char *message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    DWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, &number_written, 0);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    SetConsoleTextAttribute(console_handle, csbi.wAttributes);
}

void os::printError(const char *message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);

    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    DWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, &number_written, 0);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);
    SetConsoleTextAttribute(console_handle, csbi.wAttributes);
}