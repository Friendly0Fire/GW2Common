#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <optional>

#include <atomic_queue/atomic_queue.h>

#include "Common.h"
#include "ConfigurationOption.h"
#include "Event.h"
#include "KeyCombo.h"
#include "ScanCode.h"
#include "Singleton.h"
#include "Utility.h"

enum class PassToGame
{
    Prevent = 0,
    Allow = 1
};

enum class Activated
{
	No,
    Yes
};

enum class InputResponse : uint
{
	PassToGame = 0, // Do not prevent any input from reaching the game
	PreventMouse = 1, // Prevent mouse movement only from reaching the game
	PreventKeyboard = 2, // Prevent keyboard inputs only from reaching the game
	PreventAll = 3 // Prevent all input from reaching the game
};

enum class KeybindAction : uint {
	None = 0,
	Down = 1,
	Up = 2,
	Both = Down | Up,

	IsFlag
};

struct EventKey
{
    ScanCode sc : 31;
	bool down : 1;
	bool operator==(const EventKey&) const = default;
};

struct Point
{
	int x;
	int y;
};

class ActivationKeybind;

inline auto EventKeyToString(EventKey ek, Modifier activeModifiers)
{
    std::wstring dbgkeys = L"";
    if (!ek.down && IsNone(activeModifiers)) {
        dbgkeys = L"<NONE>";
    } else {
        if (NotNone(activeModifiers & Modifier::Ctrl))
            dbgkeys += L"CTRL + ";
        if (NotNone(activeModifiers & Modifier::Shift))
            dbgkeys += L"SHIFT + ";
        if (NotNone(activeModifiers & Modifier::Alt))
            dbgkeys += L"ALT + ";
        if (ek.down)
            dbgkeys += GetScanCodeName(ek.sc);
        else
            dbgkeys.resize(dbgkeys.size() - 3);
    }

    return dbgkeys;
}

class Input : public Singleton<Input>
{
public:
	using MouseMoveEvent = Event<void(bool& retval), bool&>;
	using MouseButtonEvent = Event<void(EventKey ek, bool& retval), EventKey, bool&>;
	using InputLanguageChangeEvent = Event<void()>;
	using RecordCallback = std::function<void(KeyCombo, bool)>;

	Input();

	uint id_H_LBUTTONDOWN() const { return id_H_LBUTTONDOWN_; }
	uint id_H_LBUTTONUP() const { return id_H_LBUTTONUP_; }
	uint id_H_RBUTTONDOWN() const { return id_H_RBUTTONDOWN_; }
	uint id_H_RBUTTONUP() const { return id_H_RBUTTONUP_; }
	uint id_H_MBUTTONDOWN() const { return id_H_MBUTTONDOWN_; }
	uint id_H_MBUTTONUP() const { return id_H_MBUTTONUP_; }
	uint id_H_SYSKEYDOWN() const { return id_H_SYSKEYDOWN_; }
	uint id_H_SYSKEYUP() const { return id_H_SYSKEYUP_; }
	uint id_H_KEYDOWN() const { return id_H_KEYDOWN_; }
	uint id_H_KEYUP() const { return id_H_KEYUP_; }

	bool keybindsBlocked() const { return blockKeybinds_ != 0; }

	// Returns true to consume message
	bool OnInput(UINT& msg, WPARAM& wParam, LPARAM& lParam);
	void OnFocusLost();
	void OnFocus();
	void OnUpdate();

	void KeyUpActive();
	void ClearActive();
	void BlockKeybinds(uint id);
	void UnblockKeybinds(uint id);

	auto& mouseMoveEvent() { return mouseMoveEvent_.Downcast(); }
	auto& mouseButtonEvent() { return mouseButtonEvent_.Downcast(); }
	auto& inputLanguageChangeEvent() { return inputLanguageChangeEvent_.Downcast(); }

	void SendKeybind(const KeyCombo& ks, std::optional<Point> const& cursorPos = std::nullopt, KeybindAction action = KeybindAction::Both, bool ignoreChat = false, mstime sendTime = TimeInMilliseconds() + 10);
	void BeginRecordInputs(RecordCallback&& cb) { inputRecordCallback_ = std::move(cb); }
	void CancelRecordInputs() { inputRecordCallback_ = std::nullopt; }

	struct DelayedImguiInput {
		UINT msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	auto& imguiInputs() { return imguiInputs_; }

protected:
	struct DelayedInput
	{
		uint msg;
		WPARAM wParam;
        union {
            KeyLParam lParamKey;
            LPARAM lParamValue;
        };

		mstime t;
		std::optional<Point> cursorPos;
		bool ignoreChat = false;
	};

	PassToGame TriggerKeybinds(const EventKey& ek);
	uint ConvertHookedMessage(uint msg) const;
	DelayedInput TransformScanCode(ScanCode sc, bool down, mstime t, const std::optional<Point>& cursorPos);
	std::tuple<WPARAM, LPARAM> CreateMouseEventParams(const std::optional<Point>& cursorPos) const;
	void SendQueuedInputs();

	// ReSharper disable CppInconsistentNaming
	uint id_H_LBUTTONDOWN_;
	uint id_H_LBUTTONUP_;
	uint id_H_RBUTTONDOWN_;
	uint id_H_RBUTTONUP_;
	uint id_H_MBUTTONDOWN_;
	uint id_H_MBUTTONUP_;
	uint id_H_XBUTTONDOWN_;
	uint id_H_XBUTTONUP_;
	uint id_H_SYSKEYDOWN_;
	uint id_H_SYSKEYUP_;
	uint id_H_KEYDOWN_;
	uint id_H_KEYUP_;
	uint id_H_MOUSEMOVE_;
	// ReSharper restore CppInconsistentNaming

	Modifier downModifiers_;
	ScanCode lastDownKey_;
	std::list<DelayedInput> queuedInputs_;
	uint blockKeybinds_ = 0;

	MouseMoveEvent mouseMoveEvent_;
	MouseButtonEvent mouseButtonEvent_;
	InputLanguageChangeEvent inputLanguageChangeEvent_;

	std::unordered_map<KeyCombo, std::vector<ActivationKeybind*>> keybinds_;
	ActivationKeybind* activeKeybind_ = nullptr;
	void RegisterKeybind(ActivationKeybind* kb);
	void UpdateKeybind(ActivationKeybind* kb);
	void UnregisterKeybind(ActivationKeybind* kb);

	std::optional<RecordCallback> inputRecordCallback_ = std::nullopt;

	friend class MiscTab;
	friend class ActivationKeybind;

	atomic_queue::AtomicQueue2<DelayedImguiInput, 128, true, true, true, true> imguiInputs_;
};

inline InputResponse operator|(InputResponse a, InputResponse b) {
	return InputResponse(uint(a) | uint(b));
}

inline InputResponse& operator|=(InputResponse& a, InputResponse b) {
	a = a | b;
	return a;
}

inline KeybindAction operator&(KeybindAction a, KeybindAction b) {
	return KeybindAction(uint(a) & uint(b));
}