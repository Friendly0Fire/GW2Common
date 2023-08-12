#pragma once

#include <list>

#include "ActivationKeybind.h"
#include "Common.h"
#include "Input.h"
#include "Singleton.h"

KeyCombo GetSettingsKeyCombo();

class SettingsMenu : public Singleton<SettingsMenu>
{
public:
    class Implementer
    {
    public:
        virtual ~Implementer() = default;
        virtual const char* GetTabName() const = 0;
        virtual void DrawMenu(Keybind** currentEditedKeybind) = 0;
        virtual bool visible() { return true; }
    };

    SettingsMenu();

    void Draw();
    void OnInputLanguageChange();

    const Keybind& showKeybind() const { return showKeybind_; }

    void AddImplementer(Implementer* impl) {
        implementers_.push_back(impl);
        if(!currentTab_)
            currentTab_ = impl;
    }
    void RemoveImplementer(Implementer* impl) {
        implementers_.remove(impl);
        if(currentTab_ == impl)
            currentTab_ = nullptr;
    }

    void MakeVisible();

    bool isVisible() const { return isVisible_; }

    const std::string& title() const { return title_; }

protected:
    std::list<Implementer*> implementers_;
    Implementer* currentTab_ = nullptr;

    bool isVisible_ = false;
    bool isFocused_ = false;
    ActivationKeybind showKeybind_;
    Keybind* currentEditedKeybind_ = nullptr;
    ScanCode allowThroughAlt_ = ScanCode::None;
    ScanCode allowThroughShift_ = ScanCode::None;

    std::string title_;
};
