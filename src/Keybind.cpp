#include "Keybind.h"

#include <sstream>

#include "ConfigurationFile.h"
#include "Utility.h"

Keybind::Keybind(std::string_view nickname, std::string_view displayName, std::string_view category, ScanCode key, Modifier mod, bool saveToConfig)
    : nickname_(nickname), displayName_(displayName), category_(category), saveToConfig_(saveToConfig) {
    keyCombo({ key, mod });
    languageChangeCallbackID_ = GetBaseCore().languageChangeEvent().AddCallback([this]() { UpdateDisplayString(); });
}

Keybind::Keybind(std::string_view nickname, std::string_view displayName, std::string_view category)
    : nickname_(nickname), displayName_(displayName), category_(category) {
    if(auto keys = INIConfigurationFile::i().ini().GetValue("Keybinds.2", nickname_.c_str())) {
        ParseConfig(keys);
    } else {
        keys = INIConfigurationFile::i().ini().GetValue("Keybinds", nickname_.c_str());
        if(keys)
            ParseKeys(keys);
        else
            keyCombo({ ScanCode::None, Modifier::None });
    }
    languageChangeCallbackID_ = GetBaseCore().languageChangeEvent().AddCallback([this]() { UpdateDisplayString(); });
}

Keybind::~Keybind() { GetBaseCore().languageChangeEvent().RemoveCallback(std::move(languageChangeCallbackID_)); }

void Keybind::ParseKeys(const char* keys) {
    key_ = ScanCode::None;
    mod_ = Modifier::None;

    if(strnlen_s(keys, 256) > 0) {
        std::stringstream ss(keys);
        std::vector<std::string> result;

        while(ss.good()) {
            std::string substr;
            std::getline(ss, substr, ',');
            auto val = std::stoi(substr);
            val = MapVirtualKeyA(val, MAPVK_VK_TO_VSC);

            ScanCode code = ScanCode(u32(val));

            if(IsModifier(code)) {
                if(key_ != ScanCode::None)
                    mod_ = mod_ | ToModifier(code);
                else
                    key_ = code;
            }
            else {
                if(IsModifier(key_))
                    mod_ = mod_ | ToModifier(key_);

                key_ = code;
            }
        }
    }

    ApplyKeys();
}

void Keybind::ParseConfig(const char* keys) {
    std::vector<std::string> k;
    SplitString(keys, ",", std::back_inserter(k));
    if(k.empty()) {
        key_ = ScanCode::None;
        return;
    }

    key_ = ScanCode(u32(std::stoi(k[0].c_str())));
    if(k.size() == 1)
        mod_ = Modifier::None;
    else
        mod_ = Modifier(u16(std::stoi(k[1].c_str())));

    ApplyKeys();
}

void Keybind::ApplyKeys() {
    UpdateDisplayString();

    if(saveToConfig_) {
        std::string settingValue = std::to_string(u32(key_)) + ", " + std::to_string(u32(mod_));

        auto& cfg = INIConfigurationFile::i();
        if(key_ != ScanCode::None)
            cfg.ini().SetValue("Keybinds.2", nickname_.c_str(), settingValue.c_str());
        else
            cfg.ini().DeleteValue("Keybinds.2", nickname_.c_str(), nullptr);
        cfg.Save();
    }
}

[[nodiscard]] bool Keybind::matches(const KeyCombo& ks) const { return key_ == ks.key() && (mod_ & ks.mod()) == mod_; }

void Keybind::UpdateDisplayString(std::optional<KeyCombo> kc) const {
    auto k = kc ? *kc : KeyCombo(key_, mod_);

    if(k.key() == ScanCode::None) {
        keysDisplayString_[0] = '\0';
        return;
    }

    std::wstring keybind;
    if(NotNone(k.mod() & Modifier::Ctrl))
        keybind += L"CTRL + ";

    if(NotNone(k.mod() & Modifier::Alt))
        keybind += L"ALT + ";

    if(NotNone(k.mod() & Modifier::Shift))
        keybind += L"SHIFT + ";

    keybind += GetScanCodeName(k.key());

    Log::i().Print(Severity::Debug, L"Setting keybind '{}' to display '{}'", utf8_decode(nickname()), keybind);

    strcpy_s(keysDisplayString_.data(), keysDisplayString_.size(), utf8_encode(keybind).c_str());
}
