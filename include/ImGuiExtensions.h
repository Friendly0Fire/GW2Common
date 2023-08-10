#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include "Common.h"
#include "ConfigurationOption.h"
#include "ImGuiImplDX11.h"
#include "Keybind.h"

ImVec2 operator*(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, f32 b);
ImVec2 operator/(const ImVec2& a, const ImVec2& b);
ImVec2 operator/(const ImVec2& a, f32 b);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);
ImVec2 operator+(const ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, f32 b);

ImVec4 operator*(const ImVec4& a, const ImVec4& b);
ImVec4 operator*(const ImVec4& a, f32 b);
ImVec4 operator/(const ImVec4& v, f32 f);
ImVec4 operator*=(ImVec4& a, f32 b);
ImVec4 operator*=(ImVec4& a, const ImVec4& b);

inline ImVec4 ConvertVector(const vec4& val) { return { val.x, val.y, val.z, val.w }; }

inline ImVec2 ConvertVector(const vec2& val) { return { val.x, val.y }; }

template<typename T, size_t N>
auto ToImGui(const glm::vec<N, T>& vec) {
    static_assert(N == 2 || N == 4);
    if constexpr(N == 2)
        return ImVec2(f32(vec.x), f32(vec.y));
    else
        return ImVec4(f32(vec.x), f32(vec.y), f32(vec.z), f32(vec.w));
}

template<typename T, size_t N, glm::qualifier Q, i32... Es>
auto ToImGui(const glm::detail::_swizzle<N, T, Q, Es...>& vec) {
    return ToImGui(glm::vec<N, T>(vec));
}

template<typename T = f32>
auto FromImGui(const ImVec2& vec) {
    return glm::vec<2, T>(T(vec.x), T(vec.y));
}

template<typename T = f32>
auto FromImGui(const ImVec4& vec) {
    return glm::vec<4, T>(T(vec.x), T(vec.y), T(vec.z), T(vec.w));
}

void ImGuiKeybindInput(Keybind& keybind, Keybind** keybindBeingModified, const char* tooltip);

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, const char* name, ConfigurationOption<T>& value, Args&&... args) {
    if(fct(name, &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, ConfigurationOption<T>& value, Args&&... args) {
    if(fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}

inline bool ImGuiInputIntFormat(const char* label, i32* v, const char* format, i32 step = 0, i32 step_fast = 0,
                                ImGuiInputTextFlags flags = 0) {
    return ImGui::InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL),
                              (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

void ImGuiTitle(const char* text, f32 scale = 1.f);
f32 ImGuiHelpTooltipSize();
enum class ImGuiHelpTooltipElementType
{
    DEFAULT = 0,
    BULLET = 1,
};
void ImGuiHelpTooltip(std::initializer_list<std::pair<ImGuiHelpTooltipElementType, const char*>> desc, f32 scale = 1.f,
                      bool includeScrollbars = true);
inline void ImGuiHelpTooltip(const char* desc, f32 scale = 1.f, bool includeScrollbars = true) {
    ImGuiHelpTooltip({ { ImGuiHelpTooltipElementType::DEFAULT, desc } }, scale, includeScrollbars);
}

inline f32 ImGuiGetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x; }

struct ImTimelineRange
{
    ImTimelineRange() = default;
    ImTimelineRange(i32 a, i32 b) {
        values[0] = a;
        values[1] = b;
    }
    ImTimelineRange(const std::pair<i32, i32>& p) {
        values[0] = p.first;
        values[1] = p.second;
    }

    i32 values[2];

    auto& operator[](i32 i) { return values[i]; }
};

struct ImTimelineResult
{
    bool changed = false;
    bool selected = false;
};

bool ImGuiBeginTimeline(const char* str_id, i32 max_value, f32 text_width, i32 number_elements);
ImTimelineResult ImGuiTimelineEvent(const char* str_id, const char* display_name, ImTimelineRange& values, bool selected);
void ImGuiEndTimeline(i32 line_count, i32* lines = nullptr, ImVec2* mouseTop = nullptr, i32* mouseNumber = nullptr);


namespace UI
{

enum class Font
{
    Default = 0,
    Bold,
    Italic,
    Monospace
};

namespace Scoped
{
namespace Detail
{
template<auto Begin, auto End, bool UnconditionalEnd = false>
class Widget
{
    bool shown;

public:
    explicit Widget(auto&&... a)
        : shown { []<typename... Args>(Args&&... aa) {
            return Begin(std::forward<Args>(aa)...);
        }(std::forward<decltype(a)>(a)...) } { }
    ~Widget() {
        if(UnconditionalEnd || shown)
            End();
    }
    explicit operator bool() const& { return shown; }
    explicit operator bool() && = delete;
};

template<auto Push, auto Pop>
class Stack
{
    i32 count = 1;

public:
    explicit Stack(auto&&... a) {
        []<typename... Args>(Args&&... aa) {
            return Push(std::forward<Args>(aa)...);
        }(std::forward<decltype(a)>(a)...);
    }

    Stack& operator()(auto&&... a)
        requires requires() { Pop(1); }
    {
        []<typename... Args>(Args&&... aa) {
            return Push(std::forward<Args>(aa)...);
        }(std::forward<decltype(a)>(a)...);
        ++count;

        return *this;
    }

    ~Stack() {
        if constexpr(requires() { Pop(1); })
            Pop(count);
        else
            Pop();
    }
};
} // namespace Detail

using Window = Detail::Widget<ImGui::Begin, ImGui::End, true>;
using TabBar = Detail::Widget<ImGui::BeginTabBar, ImGui::EndTabBar>;
using TabItem = Detail::Widget<ImGui::BeginTabItem, ImGui::EndTabItem>;
using Table = Detail::Widget<ImGui::BeginTable, ImGui::EndTable>;
using ListBox = Detail::Widget<ImGui::BeginListBox, ImGui::EndListBox>;

class FontScale
{
    f32 prevScale_;

public:
    FontScale(f32 scale) { prevScale_ = std::exchange(ImGui::GetIO().FontGlobalScale, scale); }

    ~FontScale() { ImGui::GetIO().FontGlobalScale = prevScale_; }
};


class Font : Detail::Stack<ImGui::PushFont, ImGui::PopFont>
{
    FontScale scale_;

public:
    Font(UI::Font f, f32 scale = 1.f);
};

using StyleColor = Detail::Stack<OVERLOADS_OF(ImGui::PushStyleColor), ImGui::PopStyleColor>;
using StyleVar = Detail::Stack<OVERLOADS_OF(ImGui::PushStyleVar), ImGui::PopStyleVar>;
using AllowKeyboardFocus = Detail::Stack<ImGui::PushAllowKeyboardFocus, ImGui::PopAllowKeyboardFocus>;
using ButtonRepeat = Detail::Stack<ImGui::PushButtonRepeat, ImGui::PopButtonRepeat>;
using ItemWidth = Detail::Stack<ImGui::PushItemWidth, ImGui::PopItemWidth>;
using TextWrapPos = Detail::Stack<ImGui::PushTextWrapPos, ImGui::PopTextWrapPos>;
using ID = Detail::Stack<OVERLOADS_OF(ImGui::PushID), ImGui::PopID>;
using Tree = Detail::Stack<OVERLOADS_OF(ImGui::TreePush), ImGui::TreePop>;
using ClipRect = Detail::Stack<ImGui::PushClipRect, ImGui::PopClipRect>;

class Disable
{
    inline static bool active_s = false;

public:
    Disable(bool condition) {
        if(!condition || active_s)
            return;

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha);
        const auto disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, disabledColor);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, disabledColor);
        ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
        ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);

        active_s = true;
    }

    ~Disable() {
        if(!active_s)
            return;

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
        active_s = false;
    }
};
} // namespace Scoped

class SelectableListBox
{
public:
    SelectableListBox(std::string typeName, std::string title, std::string hintsText = "")
        : typeName_ { std::move(typeName) }, title_ { std::move(title) }, hintsText_ { std::move(hintsText) } { }

    template<ranges::input_range T>
        requires ranges::sized_range<T> bool
    Draw(T&& itemNames) {
        {
            Scoped::Font _(Font::Bold, 1.2f);
            ImGui::TextUnformatted(title_.c_str());
        }

        if(auto _ = Scoped::ListBox(std::format("##{}List", typeName_).c_str(), ImVec2(-FLT_MIN, 0.f))) {
            for(auto&& [id, name] : itemNames | ranges::views::enumerate) {
                if(ImGui::Selectable(std::format("{}##{}", name, typeName_).c_str(), id_ == id))
                    id_ = id;
            }
        }

        if(!hintsText_.empty())
        {
            Scoped::FontScale _(0.8f);
            ImGui::TextUnformatted(hintsText_.c_str());
        }

        {
            Scoped::Font _(Font::Bold);
            if(ImGui::Button(std::format(ICON_FA_PLUS_CIRCLE " Add {}", typeName_).c_str())) {
                id_ = ranges::size(itemNames);
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] i32 id() const { return id_; }
    [[nodiscard]] bool selected() const { return id_ != -1; }

    void Deselect() {
        id_ = -1;
    }

private:
    i32 id_ = -1;
    std::string typeName_;
    std::string title_;
    std::string hintsText_;
};

namespace Detail
{
struct CloseButton
{
    bool operator()(const char* id, f32 scale = 1.f, bool includeScrollbars = true) {
        Scoped::Font _(Font::Default, scale);
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size() -
                             (includeScrollbars ? (ImGui::GetScrollX() + ImGui::GetStyle().ScrollbarSize) : 0.f));

        return ImGui::Button(std::format("{}##{}", ICON_FA_TIMES, id).c_str());
    }

    [[nodiscard]] f32 size() {
        GW2_ASSERT(ImGui::GetCurrentContext()->Font == GetBaseCore().fontBold() ||
                   ImGui::GetCurrentContext()->Font == GetBaseCore().font());
        return ImGui::CalcTextSize(ICON_FA_TIMES).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
    }
};
} // namespace Detail
inline static constexpr Detail::CloseButton CloseButton;

class SaveTracker
{
    bool shouldSave_ = false;
    mstime lastSaveTime_ = 0;

public:
    static inline constexpr mstime SaveDelay = 1000;

    bool operator()(bool modified) {
        if(modified)
            shouldSave_ = true;

        return modified;
    }

    void operator()() {
        shouldSave_ = true;
    }

    bool ShouldSave() const {
        return shouldSave_ && TimeInMilliseconds() - lastSaveTime_ > SaveDelay;
    }

    void Saved() {
        shouldSave_ = false;
        lastSaveTime_ = TimeInMilliseconds();
    }
};

} // namespace GW2Clarity::UI