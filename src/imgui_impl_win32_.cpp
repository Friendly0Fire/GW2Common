#include "../imgui/backends/imgui_impl_win32.cpp"

#if 0
// Copy this line into your .cpp file to forward declare the function.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Data* bd = ImGui_ImplWin32_GetBackendData();

    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        // We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
        bd->MouseHwnd = hwnd;
        if (bd->MouseTrackedArea != 1)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            ::TrackMouseEvent(&tme);
            bd->MouseTrackedArea = 1;
        }
        POINT mouse_pos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ::ClientToScreen(hwnd, &mouse_pos);
        io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
        break;
    }
    case WM_MOUSELEAVE:
        if (bd->MouseHwnd == hwnd)
            bd->MouseHwnd = NULL;
        bd->MouseTrackedArea = 2;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        break;
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
        if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        //if (bd->MouseButtonsDown == 0 && ::GetCapture() == NULL)
        //    ::SetCapture(hwnd);
        bd->MouseButtonsDown |= 1 << button;
        io.AddMouseButtonEvent(button, true);
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if (msg == WM_LBUTTONUP) { button = 0; }
        if (msg == WM_RBUTTONUP) { button = 1; }
        if (msg == WM_MBUTTONUP) { button = 2; }
        if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        bd->MouseButtonsDown &= ~(1 << button);
        //if (bd->MouseButtonsDown == 0 && ::GetCapture() == hwnd)
        //    ::ReleaseCapture();
        io.AddMouseButtonEvent(button, false);
        return 0;
    }
    case WM_MOUSEWHEEL:
        io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
        return 0;
    case WM_MOUSEHWHEEL:
        io.AddMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        const bool is_key_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        if (wParam < 256)
        {
            // Submit modifiers
            ImGui_ImplWin32_UpdateKeyModifiers();

            // Obtain virtual key code
            // (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
            int vk = (int)wParam;
            if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
                vk = IM_VK_KEYPAD_ENTER;

            // Submit key event
            const ImGuiKey key = ImGui_ImplWin32_VirtualKeyToImGuiKey(vk);
            const int scancode = (int)LOBYTE(HIWORD(lParam));
            if (key != ImGuiKey_None)
                ImGui_ImplWin32_AddKeyEvent(key, is_key_down, vk, scancode);

            // Submit individual left/right modifier events
            if (vk == VK_SHIFT)
            {
                // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
                if (IsVkDown(VK_LSHIFT) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
                if (IsVkDown(VK_RSHIFT) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
            }
            else if (vk == VK_CONTROL)
            {
                if (IsVkDown(VK_LCONTROL) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
                if (IsVkDown(VK_RCONTROL) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
            }
            else if (vk == VK_MENU)
            {
                if (IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
                if (IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
            }
        }
        return 0;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        io.AddFocusEvent(msg == WM_SETFOCUS);
        return 0;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacterUTF16((unsigned short)wParam);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;
    case WM_DISPLAYCHANGE:
        bd->WantUpdateMonitors = true;
        return 0;
    }
    return 0;
}