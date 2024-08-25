#include "Input.h"

#include "ActivationKeybind.h"
#include "MumbleLink.h"
#include "Utility.h"

Input::Input() {
    auto makeMessageName = [buf = std::wstring()](const char* name) mutable {
        buf = GetAddonNameW() + utf8_decode(name);
        return buf.c_str();
    };
    // While WM_USER+n is recommended by MSDN, we do not know if the game uses special
    // window events, so avoid any potential conflict using explicit registration
    id_H_LBUTTONDOWN_ = RegisterWindowMessage(makeMessageName("_LBUTTONDOWN"));
    id_H_LBUTTONUP_ = RegisterWindowMessage(makeMessageName("_LBUTTONUP"));
    id_H_RBUTTONDOWN_ = RegisterWindowMessage(makeMessageName("_RBUTTONDOWN"));
    id_H_RBUTTONUP_ = RegisterWindowMessage(makeMessageName("_RBUTTONUP"));
    id_H_MBUTTONDOWN_ = RegisterWindowMessage(makeMessageName("_MBUTTONDOWN"));
    id_H_MBUTTONUP_ = RegisterWindowMessage(makeMessageName("_MBUTTONUP"));
    id_H_XBUTTONDOWN_ = RegisterWindowMessage(makeMessageName("_XBUTTONDOWN"));
    id_H_XBUTTONUP_ = RegisterWindowMessage(makeMessageName("_XBUTTONUP"));
    id_H_SYSKEYDOWN_ = RegisterWindowMessage(makeMessageName("_SYSKEYDOWN"));
    id_H_SYSKEYUP_ = RegisterWindowMessage(makeMessageName("_SYSKEYUP"));
    id_H_KEYDOWN_ = RegisterWindowMessage(makeMessageName("_KEYDOWN"));
    id_H_KEYUP_ = RegisterWindowMessage(makeMessageName("_KEYUP"));
    id_H_MOUSEMOVE_ = RegisterWindowMessage(makeMessageName("_MOUSEMOVE"));
}

WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam) {
    auto& kl = KeyLParam::Get(lParam);

    switch(vk) {
    case VK_SHIFT:
        return MapVirtualKey(kl.scanCode, MAPVK_VSC_TO_VK_EX);
    case VK_CONTROL:
        return kl.extendedFlag ? VK_RCONTROL : VK_LCONTROL;
    case VK_MENU:
        return kl.extendedFlag ? VK_RMENU : VK_LMENU;
    default:
        // Not a key we map from generic to left/right,
        // just return it.
        return vk;
    }
}

bool IsRawInputMouse(LPARAM lParam) {
    UINT dwSize = 40;
    static BYTE lpb[40];

    GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

    auto* raw = reinterpret_cast<RAWINPUT*>(lpb);

    return raw->header.dwType == RIM_TYPEMOUSE;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Input::OnInput(UINT& msg, WPARAM& wParam, LPARAM& lParam) {
    EventKey eventKey = { ScanCode::None, false };
    {
        bool eventDown = false;
        switch(msg) {
        case WM_INPUTLANGCHANGE:
            inputLanguageChangeEvent_();
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            eventDown = true;
            [[fallthrough]];
        case WM_SYSKEYUP:
        case WM_KEYUP:
            {
                auto& keylParam = KeyLParam::Get(lParam);
                const ScanCode sc = GetScanCode(keylParam);

                eventKey = { sc, eventDown };
                break;
            }
        case WM_LBUTTONDOWN:
            eventDown = true;
            [[fallthrough]];
        case WM_LBUTTONUP:
            eventKey = { ScanCode::LButton, eventDown };
            break;
        case WM_MBUTTONDOWN:
            eventDown = true;
            [[fallthrough]];
        case WM_MBUTTONUP:
            eventKey = { ScanCode::MButton, eventDown };
            break;
        case WM_RBUTTONDOWN:
            eventDown = true;
            [[fallthrough]];
        case WM_RBUTTONUP:
            eventKey = { ScanCode::RButton, eventDown };
            break;
        case WM_XBUTTONDOWN:
            eventDown = true;
            [[fallthrough]];
        case WM_XBUTTONUP:
            eventKey = { GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ScanCode::X1Button : ScanCode::X2Button, eventDown };
            break;
        default:
            break;
        }
    }

    // Eliminate repeat inputs
    if(eventKey.sc == lastDownKey_ && eventKey.down)
        eventKey.sc = ScanCode::None;

    const auto isRawInputMouse = msg == WM_INPUT && IsRawInputMouse(lParam);

    bool preventMouseMove = false;
    if(msg == WM_MOUSEMOVE || isRawInputMouse) {
        bool interrupt = false;
        mouseMoveEvent_(preventMouseMove);
    }

    bool preventMouseButton = false;
    if(eventKey.sc != ScanCode::None &&
       (eventKey.sc == ScanCode::LButton || eventKey.sc == ScanCode::MButton || eventKey.sc == ScanCode::RButton ||
        eventKey.sc == ScanCode::X1Button || eventKey.sc == ScanCode::X2Button)) {
        bool interrupt = false;
        mouseButtonEvent_(eventKey, preventMouseButton);
    }

    InputResponse response = preventMouseButton ? InputResponse::PreventMouse : InputResponse::PassToGame;
    if(inputRecordCallback_ && eventKey.sc != ScanCode::None) {
        response |= InputResponse::PreventKeyboard;
        if(eventKey.sc != ScanCode::LButton) {
            auto m = downModifiers_;
            if(!eventKey.down && IsModifier(eventKey.sc)) {
                m &= ~ToModifier(eventKey.sc);
            }
            (*inputRecordCallback_)(KeyCombo(eventKey.sc, m), !eventKey.down);
            if(!eventKey.down)
                inputRecordCallback_ = std::nullopt;
        }
    }

    // When releasing a key, immediately update modifiers to correctly release keybind
    if(!eventKey.down && IsModifier(eventKey.sc)) {
        auto mod = ToModifier(eventKey.sc);
        if(eventKey.down)
            downModifiers_ |= mod;
        else
            downModifiers_ &= ~mod;
    }

    // Only run these for key down/key up (incl. mouse buttons) events
    if(!keybindsBlocked() && eventKey.sc != ScanCode::None && (eventKey.sc != lastDownKey_ || !eventKey.down) &&
       !MumbleLink::i().textboxHasFocus()) {
        response |= TriggerKeybinds(eventKey) == PassToGame::Prevent ? InputResponse::PreventKeyboard : InputResponse::PassToGame;
        if(eventKey.down)
            lastDownKey_ = eventKey.sc;
        if(eventKey.sc == lastDownKey_ && !eventKey.down)
            lastDownKey_ = ScanCode::None;
    }

    // When pressing a key, delay modifiers until after handling in case the modifier key is used as a keybind
    if(eventKey.down && IsModifier(eventKey.sc)) {
        auto mod = ToModifier(eventKey.sc);
        if(eventKey.down)
            downModifiers_ |= mod;
        else
            downModifiers_ &= ~mod;
    }

    if(msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
        DelayedImguiInput dii { msg, wParam, lParam };
        imguiInputs_.try_push(dii);
    }
    else
        ImGui_ImplWin32_WndProcHandler(GetBaseCore().gameWindow(), msg, wParam, lParam);

    if(response == InputResponse::PreventAll)
        return true;

    if(response == InputResponse::PreventKeyboard) {
        switch(msg) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            return true;
        }
    }

    if(response == InputResponse::PreventMouse || preventMouseMove) {
        switch(msg) {
        // Outright prevent those two events
        case WM_MOUSEMOVE:
            return true;
        case WM_INPUT:
            if(isRawInputMouse)
                return true;
            break;

        // All other mouse events pass mouse position as well, so revert that
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDBLCLK:
            {
                const auto& io2 = ImGui::GetIO();

                i16 mx, my;
                mx = (i16)io2.MousePos.x;
                my = (i16)io2.MousePos.y;
                lParam = MAKELPARAM(mx, my);
                break;
            }
        }
    }

    // Prevent game from receiving input if ImGui requests capture
    const auto& io = ImGui::GetIO();
    switch(msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        if(io.WantCaptureMouse)
            return true;
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        if(io.WantCaptureKeyboard)
            return true;
        break;
    case WM_CHAR:
        if(io.WantTextInput)
            return true;
        break;
    }

    // Convert hook messages back into their original messages
    msg = ConvertHookedMessage(msg);

    return false;
}

void Input::OnFocusLost() { downModifiers_ = Modifier::None; }

void Input::OnFocus() {
    downModifiers_ = Modifier::None;
    if(GetAsyncKeyState(VK_SHIFT))
        downModifiers_ |= Modifier::Shift;
    if(GetAsyncKeyState(VK_CONTROL))
        downModifiers_ |= Modifier::Ctrl;
    if(GetAsyncKeyState(VK_MENU))
        downModifiers_ |= Modifier::Alt;
}

void Input::OnUpdate() { SendQueuedInputs(); }

void Input::KeyUpActive() {
    if(!activeKeybind_)
        return;

    SendKeybind(activeKeybind_->keyCombo(), std::nullopt, KeybindAction::Up);
}

void Input::ClearActive() {
    downModifiers_ = Modifier::None;
    activeKeybind_ = nullptr;
    LogInfo("Clearing active keybind {} and modifiers {}", activeKeybind_ ? activeKeybind_->nickname().c_str() : "null",
            ToUnderlying(downModifiers_));
}

void Input::BlockKeybinds(u32 id) {
    u32 old = blockKeybinds_;
    blockKeybinds_ |= id;
    if(old == blockKeybinds_)
        return;

    ClearActive();
    LogInfo("Blocking keybinds, flag {} -> {}", old, blockKeybinds_);
}

void Input::UnblockKeybinds(u32 id) {
    u32 old = blockKeybinds_;
    blockKeybinds_ &= ~id;
    if(old == blockKeybinds_)
        return;

    LogInfo("Unblocking keybinds, flag {} -> {}", old, blockKeybinds_);
}

PassToGame Input::TriggerKeybinds(const EventKey& ek) {
#ifdef _DEBUG
    auto dbgkeys = EventKeyToString(ek, downModifiers_);
    Log::i().Print(Severity::Debug, L"Triggering keybinds, active keys: {}", dbgkeys);
#endif

    // Key is pressed  => use it as main key
    // Key is released => if it's a modifier, keep last down key as main key
    //                 => if not, main key is nil (only modifiers may remain pressed)
    KeyCombo kc(ek.down ? ek.sc : ek.sc == lastDownKey_ ? ScanCode::None : lastDownKey_, downModifiers_);

    struct
    {
        i32 condiScore = -1;
        i32 keyScore = -1;
        ActivationKeybind* kb = nullptr;
    } bestKeybind;
    bool activeKeybindDeactivated =
        activeKeybind_ && !ek.down && (ek.sc == activeKeybind_->key() || NotNone(ToModifier(ek.sc) & activeKeybind_->modifier()));
    if(activeKeybind_ && !activeKeybindDeactivated) {
        LogInfo("Best candidate keybind set to prior active keybind '{}'", activeKeybind_->nickname());
        bestKeybind = { activeKeybind_->conditionsScore(), activeKeybind_->keysScore(), activeKeybind_ };
    }

    if(ek.down) {
        for(auto& kb : keybinds_[kc]) {
            if(kb->conditionsFulfilled()) {
                i32 condiScore = kb->conditionsScore();
                i32 keyScore = kb->keysScore();
                if(condiScore > bestKeybind.condiScore || condiScore == bestKeybind.condiScore && keyScore > bestKeybind.keyScore)
                    bestKeybind = { condiScore, keyScore, kb };
            }
        }

        if(bestKeybind.kb && bestKeybind.kb != activeKeybind_) {
            if(activeKeybind_ != nullptr)
                activeKeybind_->callback()(Activated::No);
            activeKeybind_ = bestKeybind.kb;

#ifdef _DEBUG
            LogInfo("Active keybind is now '{}'", activeKeybind_->nickname());
#endif

            return activeKeybind_->callback()(Activated::Yes);
        }
    }
    else if(activeKeybindDeactivated) {
        activeKeybind_->callback()(Activated::No);
        activeKeybind_ = nullptr;
#ifdef _DEBUG
        LogInfo("Active keybind is now null");
#endif
    }

    return PassToGame::Allow;
}

u32 Input::ConvertHookedMessage(u32 msg) const {
    if(msg == id_H_LBUTTONDOWN_)
        return WM_LBUTTONDOWN;
    if(msg == id_H_LBUTTONUP_)
        return WM_LBUTTONUP;
    if(msg == id_H_RBUTTONDOWN_)
        return WM_RBUTTONDOWN;
    if(msg == id_H_RBUTTONUP_)
        return WM_RBUTTONUP;
    if(msg == id_H_MBUTTONDOWN_)
        return WM_MBUTTONDOWN;
    if(msg == id_H_MBUTTONUP_)
        return WM_MBUTTONUP;
    if(msg == id_H_XBUTTONDOWN_)
        return WM_XBUTTONDOWN;
    if(msg == id_H_XBUTTONUP_)
        return WM_XBUTTONUP;
    if(msg == id_H_SYSKEYDOWN_)
        return WM_SYSKEYDOWN;
    if(msg == id_H_SYSKEYUP_)
        return WM_SYSKEYUP;
    if(msg == id_H_KEYDOWN_)
        return WM_KEYDOWN;
    if(msg == id_H_KEYUP_)
        return WM_KEYUP;
    if(msg == id_H_MOUSEMOVE_)
        return WM_MOUSEMOVE;

    return msg;
}

Input::DelayedInput Input::TransformScanCode(ScanCode sc, bool down, mstime t, const std::optional<Point>& cursorPos) {
    DelayedInput i {};
    i.t = t;
    i.cursorPos = cursorPos;
    if(IsMouse(sc)) {
        std::tie(i.wParam, i.lParamValue) = CreateMouseEventParams(cursorPos);
    }
    else {
        bool isUniversal = IsUniversal(sc);
        if(isUniversal)
            sc = sc & ~ScanCode::UniversalModifierFlag;

        i.wParam = MapVirtualKey(u32(sc), isUniversal ? MAPVK_VSC_TO_VK : MAPVK_VSC_TO_VK_EX);
        GW2_ASSERT(i.wParam != 0);
        i.lParamKey.repeatCount = 1;
        i.lParamKey.scanCode =
            u32(sc) & 0xFF; // Only take the first octet; there's a possibility the value won't fit in the bit field otherwise
        i.lParamKey.extendedFlag = IsExtendedKey(sc) ? 1 : 0;
        i.lParamKey.contextCode = 0;
        i.lParamKey.previousKeyState = down ? 0 : 1;
        i.lParamKey.transitionState = down ? 0 : 1;
    }

    switch(sc) {
    case ScanCode::LButton:
        i.msg = down ? id_H_LBUTTONDOWN_ : id_H_LBUTTONUP_;
        break;
    case ScanCode::MButton:
        i.msg = down ? id_H_MBUTTONDOWN_ : id_H_MBUTTONUP_;
        break;
    case ScanCode::RButton:
        i.msg = down ? id_H_RBUTTONDOWN_ : id_H_RBUTTONUP_;
        break;
    case ScanCode::X1Button:
    case ScanCode::X2Button:
        i.msg = down ? id_H_XBUTTONDOWN_ : id_H_XBUTTONUP_;
        i.wParam |= (WPARAM)(IsSame(sc, ScanCode::X1Button) ? XBUTTON1 : XBUTTON2) << 16;
        break;
    case ScanCode::AltLeft:
    case ScanCode::AltRight:
    case ScanCode::F10:
        i.msg = down ? id_H_SYSKEYDOWN_ : id_H_SYSKEYUP_;
        break;
    default:
        i.msg = down ? id_H_KEYDOWN_ : id_H_KEYUP_;
        break;
    }

    return i;
}

std::tuple<WPARAM, LPARAM> Input::CreateMouseEventParams(const std::optional<Point>& cursorPos) const {
    WPARAM wParam = 0;
    if(NotNone(downModifiers_ & Modifier::Ctrl))
        wParam += MK_CONTROL;
    if(NotNone(downModifiers_ & Modifier::Shift))
        wParam += MK_SHIFT;
#if 0
    if (downKeys_.count(ScanCode::LButton))
        wParam += MK_LBUTTON;
    if (downKeys_.count(ScanCode::RButton))
        wParam += MK_RBUTTON;
    if (downKeys_.count(ScanCode::MButton))
        wParam += MK_MBUTTON;
    if (downKeys_.count(ScanCode::X1Button))
        wParam += MK_XBUTTON1;
    if (downKeys_.count(ScanCode::X2Button))
        wParam += MK_XBUTTON2;
#endif

    const auto& io = ImGui::GetIO();

    LPARAM lParam = MAKELPARAM(cursorPos ? cursorPos->x : (static_cast<i32>(io.MousePos.x)),
                               cursorPos ? cursorPos->y : (static_cast<i32>(io.MousePos.y)));
    return { wParam, lParam };
}

void Input::SendKeybind(const KeyCombo& ks, const std::optional<Point>& cursorPos, KeybindAction action, bool ignoreChat, mstime sendTime) {
    if(ks.key() == ScanCode::None) {
        if(cursorPos.has_value()) {
            DelayedInput i {};
            i.cursorPos = cursorPos;
            i.t = sendTime;
            std::tie(i.wParam, i.lParamValue) = CreateMouseEventParams(cursorPos);
            i.msg = id_H_MOUSEMOVE_;
            i.ignoreChat = ignoreChat;
            queuedInputs_.push_back(i);
        }
        return;
    }

    mstime currentTime = sendTime;

    std::list<ScanCode> codes;
    if(NotNone(ks.mod() & Modifier::Shift))
        codes.push_back(ScanCode::Shift);
    if(NotNone(ks.mod() & Modifier::Ctrl))
        codes.push_back(ScanCode::Control);
    if(NotNone(ks.mod() & Modifier::Alt))
        codes.push_back(ScanCode::Alt);

    codes.push_back(ks.key());

    auto sendKeys = [&](ScanCode sc, bool down) {
        DelayedInput i = TransformScanCode(sc, down, currentTime, cursorPos);
        i.ignoreChat = ignoreChat;
        if(i.wParam != 0)
            queuedInputs_.push_back(i);
        currentTime += 20;
    };

    if(NotNone(action & KeybindAction::Down)) {
        for(auto sc : codes)
            sendKeys(sc, true);
    }

    if(action == KeybindAction::Both)
        currentTime += 50;

    if(NotNone(action & KeybindAction::Up)) {
        for(const auto& sc : reverse(codes))
            sendKeys(sc, false);
    }
}

void Input::SendQueuedInputs() {
    if(queuedInputs_.empty())
        return;

    const auto currentTime = TimeInMilliseconds();

    auto& qi = queuedInputs_.front();

    if(currentTime < qi.t)
        return;

    // Only send inputs that aren't too old
    if(currentTime < qi.t + 1000 && (!MumbleLink::i().textboxHasFocus() || qi.ignoreChat)) {
        if(qi.cursorPos) {
            Log::i().Print(Severity::Debug, L"Moving cursor to ({}, {})...", qi.cursorPos->x, qi.cursorPos->y);
            POINT p { qi.cursorPos->x, qi.cursorPos->y };
            ClientToScreen(GetBaseCore().gameWindow(), &p);
            SetCursorPos(p.x, p.y);
        }

        if(qi.msg != id_H_MOUSEMOVE_) {
#ifdef _DEBUG
            if(qi.msg == WM_CHAR)
                Log::i().Print(Severity::Debug, L"Sending char 0x{:x} ({})...", u32(qi.wParam), char(qi.wParam));
            else {
                wchar_t keyNameBuf[128];
                GetKeyNameTextW(LONG(qi.lParamValue), keyNameBuf, sizeof(keyNameBuf));
                Log::i().Print(Severity::Debug, L"Sending keybind 0x{:x} ({})...", u32(qi.wParam), keyNameBuf);
            }
#endif
            PostMessage(GetBaseCore().gameWindow(), qi.msg, qi.wParam, qi.lParamValue);
        }
    }

    queuedInputs_.pop_front();
}

void Input::RegisterKeybind(ActivationKeybind* kb) { keybinds_[kb->keyCombo()].push_back(kb); }

void Input::UpdateKeybind(ActivationKeybind* kb) {
    UnregisterKeybind(kb);
    RegisterKeybind(kb);
}

void Input::UnregisterKeybind(ActivationKeybind* kb) {
    if(activeKeybind_ == kb)
        activeKeybind_ = nullptr;

    for(auto& [kc, vec] : keybinds_) {
        auto it = std::remove(vec.begin(), vec.end(), kb);
        if(it != vec.end())
            vec.erase(it);
    }
}
