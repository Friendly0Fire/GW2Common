#pragma once
#include <type_traits>

#include "Common.h"
#include "ConfigurationFile.h"

template<typename T>
class ConfigurationOption
{
public:
    ConfigurationOption(std::string displayName, std::string nickname, std::string category, T defaultValue = T())
        : displayName_(std::move(displayName)), nickname_(std::move(nickname)), category_(std::move(category)), value_(defaultValue) {
        LoadValue();
    }
    ConfigurationOption(ConfigurationOption &&) = default;

    const std::string &displayName() const { return displayName_; }
    void displayName(const std::string &displayName) { displayName_ = displayName; }

    const std::string &category() const { return category_; }
    void category(const std::string &category) { category_ = category; }

    const T &value() const { return value_; }
    T &value() { return value_; }
    void value(const T &value) {
        value_ = value;
        ForceSave();
    }

    void Reload() { LoadValue(); }

    void ForceSave() const {
        SaveValue();
        INIConfigurationFile::i().Save();
    }

protected:
    void LoadValue() {
        if constexpr(std::is_same_v<T, i32>)
            value_ = INIConfigurationFile::i().ini().GetLongValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, i16>)
            value_ = static_cast<i16>(INIConfigurationFile::i().ini().GetLongValue(category_.c_str(), nickname_.c_str(), value()));
        else if constexpr(std::is_same_v<T, double>)
            value_ = INIConfigurationFile::i().ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, f32>)
            value_ = f32(INIConfigurationFile::i().ini().GetDoubleValue(category_.c_str(), nickname_.c_str(), value()));
        else if constexpr(std::is_same_v<T, bool>)
            value_ = INIConfigurationFile::i().ini().GetBoolValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, const char *>)
            value_ = INIConfigurationFile::i().ini().GetValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_enum_v<T> && sizeof(T) <= sizeof(i32))
            value_ = static_cast<T>(
                INIConfigurationFile::i().ini().GetLongValue(category_.c_str(), nickname_.c_str(), static_cast<i32>(value())));
        else if constexpr(std::is_union_v<T> && sizeof(T) <= sizeof(i32))
            value_.value = INIConfigurationFile::i().ini().GetLongValue(category_.c_str(), nickname_.c_str(), value().value);
        else
            static_assert(!sizeof(T), "Unsupported value type");
    }

    void SaveValue() const {
        if constexpr(std::is_same_v<T, i32>)
            INIConfigurationFile::i().ini().SetLongValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, i16>)
            INIConfigurationFile::i().ini().SetLongValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, double>)
            INIConfigurationFile::i().ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, f32>)
            INIConfigurationFile::i().ini().SetDoubleValue(category_.c_str(), nickname_.c_str(), double(value()));
        else if constexpr(std::is_same_v<T, bool>)
            INIConfigurationFile::i().ini().SetBoolValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_same_v<T, const char *>)
            INIConfigurationFile::i().ini().SetValue(category_.c_str(), nickname_.c_str(), value());
        else if constexpr(std::is_enum_v<T> && sizeof(T) <= sizeof(i32))
            INIConfigurationFile::i().ini().SetLongValue(category_.c_str(), nickname_.c_str(), static_cast<i32>(value()));
        else if constexpr(std::is_union_v<T> && sizeof(T) <= sizeof(i32))
            INIConfigurationFile::i().ini().SetLongValue(category_.c_str(), nickname_.c_str(), value().value);
        else
            static_assert(!sizeof(T), "Unsupported value type");
    }

    std::string displayName_, nickname_, category_;
    T value_;
};

namespace ImGui
{
template<typename F, typename T, typename... Args>
bool ConfigurationWrapper(F fct, const char* name, ConfigurationOption<T>& value, Args&&... args) {
    if (fct(name, &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}

template<typename F, typename T, typename... Args>
bool ConfigurationWrapper(F fct, ConfigurationOption<T>& value, Args&&... args) {
    if (fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}
}