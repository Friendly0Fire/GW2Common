#pragma once
#include <array>

#include "Common.h"
#include "Input.h"
#include "KeyCombo.h"

class Keybind
{
public:
    Keybind(std::string nickname, std::string displayName, std::string category, KeyCombo ks, bool saveToConfig)
        : Keybind(nickname, displayName, category, ks.key(), ks.mod(), saveToConfig) { }

    Keybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig);
    Keybind(std::string nickname, std::string displayName, std::string category);

    virtual ~Keybind();

    KeyCombo keyCombo() const { return { key_, mod_ }; }
    ScanCode key() const { return key_; }
    Modifier modifier() const { return mod_; }

    void keyCombo(const KeyCombo& ks) {
        key_ = ks.key();
        mod_ = ks.mod();

        ApplyKeys();
    }
    void key(ScanCode key) {
        key_ = key;

        ApplyKeys();
    }
    void modifier(Modifier mod) {
        mod_ = mod;

        ApplyKeys();
    }

    void ParseKeys(const char* keys);
    void ParseConfig(const char* keys);

    [[nodiscard]] const std::string& displayName() const { return displayName_; }
    void displayName(const std::string& n) { displayName_ = n; }

    [[nodiscard]] const std::string& nickname() const { return nickname_; }
    void nickname(const std::string& n) { nickname_ = n; }

    [[nodiscard]] bool isSet() const { return key_ != ScanCode::None; }

    [[nodiscard]] const char* keysDisplayString() const { return keysDisplayString_.data(); }
    [[nodiscard]] char* keysDisplayString() { return keysDisplayString_.data(); }
    [[nodiscard]] const size_t keysDisplayStringSize() const { return keysDisplayString_.size(); }

    [[nodiscard]] bool matches(const KeyCombo& ks) const;

    int keyCount() const {
        if(key_ == ScanCode::None)
            return 0;

        return 1 + (NotNone(mod_ & Modifier::Ctrl) ? 1 : 0) + (NotNone(mod_ & Modifier::Shift) ? 1 : 0) +
               (NotNone(mod_ & Modifier::Alt) ? 1 : 0);
    }

    void UpdateDisplayString(const std::optional<KeyCombo>& kc = std::nullopt) const;

protected:
    virtual void ApplyKeys();

    std::string displayName_, nickname_, category_;
    ScanCode key_;
    Modifier mod_;
    bool saveToConfig_ = true;
    EventCallbackHandle languageChangeCallbackID_;

    mutable std::array<char, 128> keysDisplayString_ {};
};
