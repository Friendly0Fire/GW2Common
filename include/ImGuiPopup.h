#pragma once
#include <functional>

#include "Common.h"

class ImGuiPopup
{
public:
    static constexpr i32 DefaultFlags() { return ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove; }

    ImGuiPopup(const std::string& name, ImGuiWindowFlags flags = DefaultFlags());
    ImGuiPopup& Position(ImVec2 centerPos, bool relative = true);
    ImGuiPopup& Size(ImVec2 size, bool relative = true);
    void Display(const std::function<void(const ImVec2&)>& content, const std::function<void()>& closeCallback);

protected:
    static ImVec2 ScreenDims();
    ImGuiWindowFlags flags_;
    std::string name_;
    bool opened_ = true;
    ImVec2 pos_ { 0.5f, 0.5f }, size_ { 0.5f, 0.5f };
};
