#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN_32_EXTRA_LEAN
#include "../gl/glad.c"
#include "./win32_base.h"

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


inline UINT getRawInput(LPVOID data) {
    return GetRawInputData(Win32_raw_input_handle, RID_INPUT, data, Win32_raw_input_size_ptr, Win32_raw_input_header_size);
}
inline bool hasRawInput() {
    return (getRawInput(0) == 0) && (Win32_raw_input_size != 0);
}
inline bool hasRawMouseInput(LPARAM lParam) {
    Win32_raw_input_handle = (HRAWINPUT)(lParam);
    return (
            (hasRawInput()) &&
            (getRawInput((LPVOID)&Win32_raw_inputs) == Win32_raw_input_size) &&
            (Win32_raw_inputs.header.dwType == RIM_TYPEMOUSE)
    );
}


SlimApp *CURRENT_APP;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    bool pressed = message == WM_SYSKEYDOWN || message == WM_KEYDOWN;
    u8 key = (u8)wParam;
    i32 x, y;
    f32 scroll_amount;

    switch (message) {
        case WM_CREATE:
        {
            Win32_window_dc = GetDC(hWnd);
            SetICMMode(Win32_window_dc, ICM_OFF);

            PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
                1,                     // version number
                PFD_DRAW_TO_WINDOW |   // support window
                PFD_SUPPORT_OPENGL |   // support OpenGL
                PFD_DOUBLEBUFFER,      // double buffered
                PFD_TYPE_RGBA,         // RGBA type
                24,                    // 24-bit color depth
                0, 0, 0, 0, 0, 0,      // color bits ignored
                0,                     // no alpha buffer
                0,                     // shift bit ignored
                0,                     // no accumulation buffer
                0, 0, 0, 0,            // accum bits ignored
                32,                    // 32-bit z-buffer
                0,                     // no stencil buffer
                0,                     // no auxiliary buffer
                PFD_MAIN_PLANE,        // main layer
                0,                     // reserved
                0, 0, 0                // layer masks ignored
            };
            int iPixelFormat = ChoosePixelFormat(Win32_window_dc, &pfd);
            SetPixelFormat(Win32_window_dc, iPixelFormat, &pfd);
            Win32_render_context = wglCreateContext(Win32_window_dc);
            wglMakeCurrent(Win32_window_dc, Win32_render_context);

            gladLoadGL();

            break;
        }

        case WM_DESTROY:
            CURRENT_APP->is_running = false;
            CURRENT_APP->_shutdown();

            ReleaseDC(hWnd, Win32_window_dc);
            wglDeleteContext(Win32_render_context);
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            if (lParam) {
                GetClientRect(hWnd, &Win32_window_rect);
                Win32_bitmap_info.bmiHeader.biWidth = Win32_window_rect.right - Win32_window_rect.left;
                Win32_bitmap_info.bmiHeader.biHeight = Win32_window_rect.bottom - Win32_window_rect.top;
                if (window::width == (u16)Win32_bitmap_info.bmiHeader.biWidth &&
                    window::height == (u16)Win32_bitmap_info.bmiHeader.biHeight) {
                    CURRENT_APP->_restore();
                } else {
                    CURRENT_APP->_resize((u16)Win32_bitmap_info.bmiHeader.biWidth, (u16)Win32_bitmap_info.bmiHeader.biHeight);
                }
            } else
                CURRENT_APP->_minimize();

            break;

        case WM_PAINT:
            if (CURRENT_APP->blit) {
                SetDIBitsToDevice(Win32_window_dc,
                                  0, 0, window::width, window::height,
                                  0, 0, 0, window::height,
                                  (u32*)window::content, &Win32_bitmap_info, DIB_RGB_COLORS);

            }
            CURRENT_APP->_redraw();
            SwapBuffers(Win32_window_dc);

            ValidateRgn(Win32_window, nullptr);
            break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
            switch (key) {
                case VK_CONTROL: controls::is_pressed::ctrl   = pressed; break;
                case VK_MENU   : controls::is_pressed::alt    = pressed; break;
                case VK_SHIFT  : controls::is_pressed::shift  = pressed; break;
                case VK_SPACE  : controls::is_pressed::space  = pressed; break;
                case VK_TAB    : controls::is_pressed::tab    = pressed; break;
                case VK_ESCAPE : controls::is_pressed::escape = pressed; break;
                case VK_LEFT   : controls::is_pressed::left   = pressed; break;
                case VK_RIGHT  : controls::is_pressed::right  = pressed; break;
                case VK_UP     : controls::is_pressed::up     = pressed; break;
                case VK_DOWN   : controls::is_pressed::down   = pressed; break;
                default: break;
            }
            CURRENT_APP->_keyChanged(key, pressed);

            break;

        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK:
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            mouse::Button *mouse_button;
            switch (message) {
                case WM_MBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONDBLCLK:
                    mouse_button = &mouse::middle_button;
                    break;
                case WM_RBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONDBLCLK:
                    mouse_button = &mouse::right_button;
                    break;
                default:
                    mouse_button = &mouse::left_button;
            }

            switch (message) {
                case WM_MBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                case WM_LBUTTONDBLCLK:
                    mouse_button->doubleClick(x, y);
                    mouse::double_clicked = true;
                    CURRENT_APP->_mouseButtonDoubleClicked(*mouse_button);
                    break;
                case WM_MBUTTONUP:
                case WM_RBUTTONUP:
                case WM_LBUTTONUP:
                    mouse_button->up(x, y);
                    CURRENT_APP->_mouseButtonUp(*mouse_button);
                    break;
                default:
                    mouse_button->down(x, y);
                    CURRENT_APP->_mouseButtonDown(*mouse_button);
            }

            break;

        case WM_MOUSEWHEEL:
            scroll_amount = (f32)(GET_WHEEL_DELTA_WPARAM(wParam)) / (f32)(WHEEL_DELTA);
            mouse::scroll(scroll_amount); CURRENT_APP->_mouseWheelScrolled(scroll_amount);
            break;

        case WM_MOUSEMOVE:
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            mouse::move(x, y);        CURRENT_APP->_mouseMovementSet(x, y);
            mouse::setPosition(x, y); CURRENT_APP->_mousePositionSet(x, y);
            break;

        case WM_INPUT:
            if ((hasRawMouseInput(lParam)) && (
                    Win32_raw_inputs.data.mouse.lLastX != 0 ||
                    Win32_raw_inputs.data.mouse.lLastY != 0)) {
                x = Win32_raw_inputs.data.mouse.lLastX;
                y = Win32_raw_inputs.data.mouse.lLastY;
                mouse::moveRaw(x, y); CURRENT_APP->_mouseRawMovementSet(x, y);
            }

        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}

int Win32_EntryPoint(HINSTANCE hInstance, int nCmdShow = SW_SHOW) {
    Win32_instance = hInstance;

    void* window_content_and_canvas_memory = GlobalAlloc(GPTR, WINDOW_CONTENT_SIZE + (CANVAS_SIZE * CANVAS_COUNT));
    if (!window_content_and_canvas_memory)
        return -1;

    window::content = (u32*)window_content_and_canvas_memory;
    memory::canvas_memory = (u8*)window_content_and_canvas_memory + WINDOW_CONTENT_SIZE;

    controls::key_map::ctrl = VK_CONTROL;
    controls::key_map::alt = VK_MENU;
    controls::key_map::shift = VK_SHIFT;
    controls::key_map::space = VK_SPACE;
    controls::key_map::tab = VK_TAB;
    controls::key_map::escape = VK_ESCAPE;
    controls::key_map::left = VK_LEFT;
    controls::key_map::right = VK_RIGHT;
    controls::key_map::up = VK_UP;
    controls::key_map::down = VK_DOWN;

    LARGE_INTEGER performance_frequency;
    QueryPerformanceFrequency(&performance_frequency);

    timers::ticks_per_second = (u64)performance_frequency.QuadPart;
    timers::seconds_per_tick = 1.0 / (f64)(timers::ticks_per_second);
    timers::milliseconds_per_tick = 1000.0 * timers::seconds_per_tick;
    timers::microseconds_per_tick = 1000.0 * timers::milliseconds_per_tick;
    timers::nanoseconds_per_tick  = 1000.0 * timers::microseconds_per_tick;

    Win32_bitmap_info.bmiHeader.biSize        = sizeof(Win32_bitmap_info.bmiHeader);
    Win32_bitmap_info.bmiHeader.biCompression = BI_RGB;
    Win32_bitmap_info.bmiHeader.biBitCount    = 32;
    Win32_bitmap_info.bmiHeader.biPlanes      = 1;

    Win32_window_class.lpszClassName  = "RnDer";
    Win32_window_class.hInstance      = hInstance;
    Win32_window_class.lpfnWndProc    = WndProc;
    Win32_window_class.style          = CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
    Win32_window_class.hCursor        = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512));

    if (!RegisterClassA(&Win32_window_class)) return -1;

    Win32_window_rect.top = 0;
    Win32_window_rect.left = 0;
    Win32_window_rect.right  = window::width;
    Win32_window_rect.bottom = window::height;
    AdjustWindowRect(&Win32_window_rect, WS_OVERLAPPEDWINDOW, false);

    Win32_window = CreateWindowA(
        Win32_window_class.lpszClassName,
        window::title,
        WS_OVERLAPPEDWINDOW,

        CW_USEDEFAULT,
        CW_USEDEFAULT,
        Win32_window_rect.right - Win32_window_rect.left,
        Win32_window_rect.bottom - Win32_window_rect.top,

        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    if (!Win32_window)
        return -1;

    Win32_raw_input_device.usUsagePage = 0x01;
    Win32_raw_input_device.usUsage = 0x02;
    if (!RegisterRawInputDevices(&Win32_raw_input_device, 1, sizeof(Win32_raw_input_device)))
        return -1;

    CURRENT_APP = createApp();
    if (!CURRENT_APP->is_running)
        return -1;

    CURRENT_APP->_init();

    Win32_window_rect.top = 0;
    Win32_window_rect.left = 0;
    Win32_window_rect.right  = window::width;
    Win32_window_rect.bottom = window::height;
    AdjustWindowRect(&Win32_window_rect, WS_OVERLAPPEDWINDOW, false);
    SetWindowPos(Win32_window, HWND_TOP,
                 0, 0,
                 window::width,
                 window::height,
                 SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);

    ShowWindow(Win32_window, nCmdShow);

    MSG message;
    while (true) {
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        if (CURRENT_APP->is_running)
            CURRENT_APP->_update();
        else
            break;

        InvalidateRgn(Win32_window, nullptr, false);
    }

    return 0;
}

int main(int argc, char **argv) {
    return Win32_EntryPoint(GetModuleHandleA(nullptr));
}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
    int argc = __argc;
    char **argv = __argv;
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Is the program running as console or GUI application
    bool is_console = false;

    // Attach output of application to parent console
    HANDLE consoleHandleOut;
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // Redirect unbuffered STDOUT to the console
        consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (consoleHandleOut != INVALID_HANDLE_VALUE) {
            AllocConsole(); //debug console
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
            is_console = true;
        }
    }

    int return_code = Win32_EntryPoint(hInstance, nCmdShow);

    // Send "enter" to release application from the console
    // This is a hack, but if not used the console doesn't know the application has
    // returned. The "enter" key only sent if the console window is in focus.
    if (is_console && (GetConsoleWindow() == GetForegroundWindow())) {
        // Send the "enter" to the console to release the command prompt
        // on the parent console
        INPUT ip;

        // Set up a generic keyboard event.
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0; // hardware scan code for key
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Send the "Enter" key
        ip.ki.wVk = 0x0D; // virtual-key code for the "Enter" key
        ip.ki.dwFlags = 0; // 0 for key press
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "Enter" key
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
        SendInput(1, &ip, sizeof(INPUT));
    }

    return return_code;
}