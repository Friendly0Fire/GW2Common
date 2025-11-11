#include "ImGuiPopup.h"

ImGuiPopup::ImGuiPopup(const std::string& name, ImGuiWindowFlags flags) : name_(name), flags_(flags) { }

ImGuiPopup& ImGuiPopup::Position(ImVec2 centerPos, bool relative) {
    if(relative)
        centerPos *= ScreenDims();
    pos_ = centerPos;

    return *this;
}

ImGuiPopup& ImGuiPopup::Size(ImVec2 size, bool relative) {
    if(relative)
        size *= ScreenDims();
    size_ = size;

    return *this;
}

void ImGuiPopup::Display(const std::function<void(const ImVec2&)>& content, const std::function<void()>& closeCallback) {
    ImGui::SetNextWindowPos(pos_, 0, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size_);

    opened_ = ImGui::Begin(name_.c_str(), nullptr, flags_);
    if(opened_) {
        content(ImGui::GetWindowSize());

        ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.4f);
        ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ImGui::GetFontSize() * 2.5f);
        if(ImGui::Button("OK", ImVec2(ImGui::GetWindowSize().x * 0.2f, ImGui::GetFontSize() * 1.5f)))
            closeCallback();
    }
    ImGui::End();
}

ImVec2 ImGuiPopup::ScreenDims()
{ return ImGui::GetMainViewport()->WorkSize; }
