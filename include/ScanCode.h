#pragma once
#include "Common.h"

/*
The scancode values come from:
- http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/scancode.doc (March 16, 2000).
- http://www.computer-engineering.org/ps2keyboard/scancodes1.html
- using MapVirtualKeyEx( VK_*, MAPVK_VK_TO_VSC_EX, 0 ) with the english us keyboard layout
- reading win32 WM_INPUT keyboard messages.
*/
enum class ScanCode : uint
{
    None = 0,
    Escape = 0x01,
    NumRow1 = 0x02,
    NumRow2 = 0x03,
    NumRow3 = 0x04,
    NumRow4 = 0x05,
    NumRow5 = 0x06,
    NumRow6 = 0x07,
    NumRow7 = 0x08,
    NumRow8 = 0x09,
    NumRow9 = 0x0A,
    NumRow0 = 0x0B,
    Minus = 0x0C,
    Equals = 0x0D,
    Backspace = 0x0E,
    Tab = 0x0F,
    Q = 0x10,
    W = 0x11,
    E = 0x12,
    R = 0x13,
    T = 0x14,
    Y = 0x15,
    U = 0x16,
    I = 0x17,
    O = 0x18,
    P = 0x19,
    BracketLeft = 0x1A,
    BracketRight = 0x1B,
    Enter = 0x1C,
    ControlLeft = 0x1D,
    A = 0x1E,
    S = 0x1F,
    D = 0x20,
    F = 0x21,
    G = 0x22,
    H = 0x23,
    J = 0x24,
    K = 0x25,
    L = 0x26,
    Semicolon = 0x27,
    Apostrophe = 0x28,
    Grave = 0x29,
    ShiftLeft = 0x2A,
    Backslash = 0x2B,
    Z = 0x2C,
    X = 0x2D,
    C = 0x2E,
    V = 0x2F,
    B = 0x30,
    N = 0x31,
    M = 0x32,
    Comma = 0x33,
    Period = 0x34,
    Slash = 0x35,
    ShiftRight = 0x36,
    NumpadMultiply = 0x37,
    AltLeft = 0x38,
    Space = 0x39,
    CapsLock = 0x3A,
    F1 = 0x3B,
    F2 = 0x3C,
    F3 = 0x3D,
    F4 = 0x3E,
    F5 = 0x3F,
    F6 = 0x40,
    F7 = 0x41,
    F8 = 0x42,
    F9 = 0x43,
    F10 = 0x44,
    NumLock = 0x45,
    ScrollLock = 0x46,
    NumPad7 = 0x47,
    NumPad8 = 0x48,
    NumPad9 = 0x49,
    NumPadMinus = 0x4A,
    NumPad4 = 0x4B,
    NumPad5 = 0x4C,
    NumPad6 = 0x4D,
    NumPadPlus = 0x4E,
    NumPad1 = 0x4F,
    NumPad2 = 0x50,
    NumPad3 = 0x51,
    NumPad0 = 0x52,
    NumPadPeriod = 0x53,
    AltPrintScreen = 0x54, /* Alt + print screen. MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54. */
    BracketAngle = 0x56, /* Key between the left shift and Z. */
    F11 = 0x57,
    F12 = 0x58,
    OEM1 = 0x5A, /* VK_OEMWSCTRL */
    OEM2 = 0x5B, /* VK_OEMFINISH */
    OEM3 = 0x5C, /* VK_OEMJUMP */
    EraseEOF = 0x5D,
    OEM4 = 0x5E, /* VK_OEMBACKTAB */
    OEM5 = 0x5F, /* VK_OEMAUTO */
    Zoom = 0x62,
    Help = 0x63,
    F13 = 0x64,
    F14 = 0x65,
    F15 = 0x66,
    F16 = 0x67,
    F17 = 0x68,
    F18 = 0x69,
    F19 = 0x6A,
    F20 = 0x6B,
    F21 = 0x6C,
    F22 = 0x6D,
    F23 = 0x6E,
    OEM6 = 0x6F, /* VK_OEMPA3 */
    Katakana = 0x70,
    OEM7 = 0x71, /* VK_OEMRESET */
    F24 = 0x76,
    SBCSChar = 0x77,
    Convert = 0x79,
    NonConvert = 0x7B, /* VK_OEMPA1 */

    MediaPrevious = 0xE010,
    MediaNext = 0xE019,
    NumPadEnter = 0xE01C,
    ControlRight = 0xE01D,
    VolumeMute = 0xE020,
    LaunchApp2 = 0xE021,
    MediaPlay = 0xE022,
    MediaStop = 0xE024,
    VolumeDown = 0xE02E,
    VolumeUp = 0xE030,
    BrowserHome = 0xE032,
    NumPadDivide = 0xE035,
    PrintScreen = 0xE037,
    /*
    PrintScreen:
    - make: 0xE02A 0xE037
    - break: 0xE0B7 0xE0AA
    - MapVirtualKeyEx( VK_SNAPSHOT, MAPVK_VK_TO_VSC_EX, 0 ) returns scancode 0x54;
    - There is no VK_KEYDOWN with VK_SNAPSHOT.
    */
    AltRight = 0xE038,
    Cancel = 0xE046, /* CTRL + PAUSE */
    Home = 0xE047,
    ArrowUp = 0xE048,
    PageUp = 0xE049,
    ArrowLeft = 0xE04B,
    ArrowRight = 0xE04D,
    End = 0xE04F,
    ArrowDown = 0xE050,
    PageDown = 0xE051,
    Insert = 0xE052,
    Delete = 0xE053,
    MetaLeft = 0xE05B,
    MetaRight = 0xE05C,
    Application = 0xE05D,
    Power = 0xE05E,
    Sleep = 0xE05F,
    Wake = 0xE063,
    BrowserSearch = 0xE065,
    BrowserFavorites = 0xE066,
    BrowserRefresh = 0xE067,
    BrowserStop = 0xE068,
    BrowserForward = 0xE069,
    BrowserBack = 0xE06A,
    LaunchApp1 = 0xE06B,
    LaunchEmail = 0xE06C,
    LaunchMedia = 0xE06D,

    Pause = 0xE11D45,
    /*
    Pause:
    - make: 0xE11D 45 0xE19D C5
    - make in raw input: 0xE11D 0x45
    - break: none
    - No repeat when you hold the key down
    - There are no break so I don't know how the key down/up is expected to work. Raw input sends "keydown" and "keyup" messages, and it
    appears that the keyup message is sent directly after the keydown message (you can't hold the key down) so depending on when GetMessage
    or PeekMessage will return messages, you may get both a keydown and keyup message "at the same time". If you use VK messages most of the
    time you only get keydown messages, but some times you get keyup messages too.
    - when pressed at the same time as one or both control keys, generates a 0xE046 (sc_cancel) and the string for that scancode is "break".
    */

    MouseFlag = 0xF0000,
    LButton = MouseFlag | VK_LBUTTON,
    RButton = MouseFlag | VK_RBUTTON,
    MButton = MouseFlag | VK_MBUTTON,
    X1Button = MouseFlag | VK_XBUTTON1,
    X2Button = MouseFlag | VK_XBUTTON2,
    MouseMask = LButton | RButton | MButton | X1Button | X2Button,

    UniversalModifierFlag = 0xF00000,
    Shift = UniversalModifierFlag | ShiftLeft,
    Control = UniversalModifierFlag | ControlLeft,
    Alt = UniversalModifierFlag | AltLeft,
    Meta = UniversalModifierFlag | MetaLeft,
    UniversalModifierMask = Shift | Control | Alt | Meta,

    MaxVal = (1u << 31) - 1, // 31 bits to fit in EventKey struct

    IsFlag = MaxVal
};

enum class Modifier : uint
{
    None = 0,

    Ctrl = 1,
    Shift = 2,
    Alt = 4,

    IsFlag
};

inline bool IsModifier(ScanCode a) {
    switch(a) {
    case ScanCode::ShiftLeft:
    case ScanCode::ShiftRight:
    case ScanCode::Shift:
    case ScanCode::ControlLeft:
    case ScanCode::ControlRight:
    case ScanCode::Control:
    case ScanCode::AltLeft:
    case ScanCode::AltRight:
    case ScanCode::Alt:
    case ScanCode::MetaLeft:
    case ScanCode::MetaRight:
    case ScanCode::Meta:
        return true;
    default:
        return false;
    }
}

inline Modifier ToModifier(ScanCode a) {
    switch(a) {
    case ScanCode::ShiftLeft:
    case ScanCode::ShiftRight:
    case ScanCode::Shift:
        return Modifier::Shift;
    case ScanCode::ControlLeft:
    case ScanCode::ControlRight:
    case ScanCode::Control:
        return Modifier::Ctrl;
    case ScanCode::AltLeft:
    case ScanCode::AltRight:
    case ScanCode::Alt:
        return Modifier::Alt;
    default:
        return Modifier::None;
    }
}

inline bool IsExtendedKey(ScanCode a) {
    switch(a) {
    case ScanCode::ControlRight:
    case ScanCode::AltRight:
    case ScanCode::MetaRight:
    // These are also extended keys: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown#remarks
    case ScanCode::Insert:
    case ScanCode::Delete:
    case ScanCode::Home:
    case ScanCode::End:
    case ScanCode::PageUp:
    case ScanCode::PageDown:
    case ScanCode::ArrowLeft:
    case ScanCode::ArrowDown:
    case ScanCode::ArrowUp:
    case ScanCode::ArrowRight:
    case ScanCode::NumLock:
    case ScanCode::PrintScreen:
    case ScanCode::NumPadDivide:
    case ScanCode::NumPadEnter:
        return true;
    default:
        return false;
    }
}

inline ScanCode MakeUniversal(const ScanCode& a) {
    switch(a) {
    case ScanCode::ShiftLeft:
    case ScanCode::ShiftRight:
    case ScanCode::Shift:
        return ScanCode::Shift;
    case ScanCode::ControlLeft:
    case ScanCode::ControlRight:
    case ScanCode::Control:
        return ScanCode::Control;
    case ScanCode::AltLeft:
    case ScanCode::AltRight:
    case ScanCode::Alt:
        return ScanCode::Alt;
    case ScanCode::MetaLeft:
    case ScanCode::MetaRight:
    case ScanCode::Meta:
        return ScanCode::Meta;
    default:
        return a;
    }
}

constexpr bool IsUniversal(ScanCode a) { return NotNone(a & ScanCode::UniversalModifierFlag); }

std::wstring GetScanCodeName(ScanCode scanCode);

constexpr bool IsSame(ScanCode a, ScanCode b) {
    if(IsUniversal(a) || IsUniversal(b)) {
        switch(a) {
        case ScanCode::ShiftLeft:
        case ScanCode::ShiftRight:
        case ScanCode::Shift:
            return b == ScanCode::ShiftLeft || b == ScanCode::ShiftRight || b == ScanCode::Shift;
        case ScanCode::ControlLeft:
        case ScanCode::ControlRight:
        case ScanCode::Control:
            return b == ScanCode::ControlLeft || b == ScanCode::ControlRight || b == ScanCode::Control;
        case ScanCode::AltLeft:
        case ScanCode::AltRight:
        case ScanCode::Alt:
            return b == ScanCode::AltLeft || b == ScanCode::AltRight || b == ScanCode::Alt;
        case ScanCode::MetaLeft:
        case ScanCode::MetaRight:
        case ScanCode::Meta:
            return b == ScanCode::MetaLeft || b == ScanCode::MetaRight || b == ScanCode::Meta;
        }
    }

    return a == b;
}

struct KeyLParam
{
    uint repeatCount : 16 = 1;
    uint scanCode : 8 = 0;
    uint extendedFlag : 1 = 0;
    uint reserved : 4 = 0;
    uint contextCode : 1 = 0;
    uint previousKeyState : 1 = 0;
    uint transitionState : 1 = 0;

    static KeyLParam& Get(LPARAM& lp) { return *(KeyLParam*)&lp; }
};

ScanCode GetScanCode(KeyLParam lParam);
inline ScanCode GetScanCodeFromVirtualKey(uint vk) { return ScanCode(MapVirtualKey(vk, MAPVK_VK_TO_VSC)); }

inline bool IsMouse(ScanCode sc) { return NotNone(sc & ScanCode::MouseFlag); }

struct ScanCodeComparator
{
    bool operator()(const ScanCode& a, const ScanCode& b) const { return Compare(a, b); }

    static bool Compare(const ScanCode& a, const ScanCode& b) {
        bool aIsModifier = IsModifier(a);
        bool bIsModifier = IsModifier(b);
        bool aIsMouse = IsMouse(a);
        bool bIsMouse = IsMouse(b);

        // Modifiers are always displayed first
        if(aIsModifier && !bIsModifier)
            return true;
        if(!aIsModifier && bIsModifier)
            return false;

        // Mouse buttons are always displayed last
        if(aIsMouse && !bIsMouse)
            return false;
        if(!aIsMouse && bIsMouse)
            return true;

        // Force reorder modifiers for simplicity (a == true implies b == true here)
        if(aIsModifier) {
            auto b2 = static_cast<ScanCode>(b);
            switch(a) {
            // Control goes first, a is less if b isn't control
            case ScanCode::Control:
            case ScanCode::ControlLeft:
            case ScanCode::ControlRight:
                return b2 != ScanCode::ControlLeft && b2 != ScanCode::ControlRight && b2 != ScanCode::Control;

            // Alt goes in between, a is less if b is shift
            case ScanCode::Alt:
            case ScanCode::AltLeft:
            case ScanCode::AltRight:
                return b2 == ScanCode::ShiftLeft || b2 == ScanCode::ShiftRight || b2 == ScanCode::Shift;

            // Shift goes last, a is never less
            case ScanCode::Shift:
            case ScanCode::ShiftLeft:
            case ScanCode::ShiftRight:
                return false;

            default:
                break;
            }
        }

        // If two ScanCodes are of the same type, compare numerically
        return ToUnderlying(a) < ToUnderlying(b);
    }
};
