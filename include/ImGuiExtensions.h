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
ImVec2 operator+=(ImVec2& a, const ImVec2& b);
ImVec2 operator-=(ImVec2& a, const ImVec2& b);
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
    bool shown_;

public:
    explicit Widget(Widget&& other) noexcept
        : shown_{ std::exchange(other.shown_, false) } { }
    explicit Widget(auto&&... a)
        : shown_ { []<typename... Args>(Args&&... aa) {
            return Begin(std::forward<Args>(aa)...);
        }(std::forward<decltype(a)>(a)...) } { }
    ~Widget() {
        if(UnconditionalEnd || shown_)
            End();
    }

    explicit operator bool() const& { return shown_; }
    explicit operator bool() && = delete;
};

template<auto Push, auto Pop>
class Stack
{
    i32 count_ = 0;

public:
    explicit Stack(Stack&& other) noexcept
        : count_{ std::exchange(other.count_, 0) } { }
    explicit Stack(auto&&... a) : count_{ 1 } {
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
        ++count_;

        return *this;
    }

    ~Stack() {
        if constexpr (requires() { Pop(1); })
            Pop(count_);
        else if (count_ == 1)
            Pop();
        else
            GW2_ASSERT(count_ == 0);
    }
};

template<typename T>
struct Wrap
{
    T value;
    Wrap(T&& value) : value(std::forward<T>(value)) { }

    explicit operator bool() const& {
        if constexpr (requires(T v) { static_cast<bool>(v); })
            return static_cast<bool>(value);
        else
            return true;
    }

    explicit operator bool() && = delete;
};

} // namespace Detail

using Window = Detail::Widget<ImGui::Begin, ImGui::End, true>;
using TabBar = Detail::Widget<ImGui::BeginTabBar, ImGui::EndTabBar>;
using TabItem = Detail::Widget<ImGui::BeginTabItem, ImGui::EndTabItem>;
using Table = Detail::Widget<ImGui::BeginTable, ImGui::EndTable>;
using ListBox = Detail::Widget<ImGui::BeginListBox, ImGui::EndListBox>;
using Combo = Detail::Widget<ImGui::BeginCombo, ImGui::EndCombo>;

#define SCOPE_IMPL(n, v) if(auto n = v; n)
#define SCOPE(v) SCOPE_IMPL(CONCAT(scope, __COUNTER__), UI::Scoped::Detail::Wrap(UI::Scoped::v))
#define SCOPED(v) auto CONCAT(scope, __COUNTER__) = UI::Scoped::v

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
using Group = Detail::Stack<ImGui::BeginGroup, ImGui::EndGroup>;

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

class Tooltip
{
    bool shown_;
    inline static i32 lastTooltipFrameId_s = -1;
public:
    Tooltip(const ImVec2& size = { 0.f, 0.f }) {
        const i32 fc = ImGui::GetFrameCount();
        if (fc != lastTooltipFrameId_s) {
            if(size.x != 0.f || size.y != 0.f)
                ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            ImGui::BeginTooltip();
            shown_ = true;
            lastTooltipFrameId_s = fc;
        }
    }

    ~Tooltip() {
        if(shown_)
            ImGui::EndTooltip();
    }

    explicit operator bool() const& { return shown_; }
    explicit operator bool() && = delete;
};

} // namespace Scoped

template<typename T>
class SelectableListBox
{
public:
    enum class Flags
    {
        None = 0,
        DragReorder = 1,
    };

    static constexpr i32 UnselectedId = -1;
    static constexpr i32 DraggedId = -2;

    SelectableListBox(std::vector<T>& items, std::string typeName, std::string title, std::string hintsText = "", Flags flags = Flags::None)
        : items_{ items }, typeName_{ std::move(typeName) }, title_{ std::move(title) }, hintsText_{ std::move(hintsText) }, flags_{ flags } { }

    bool Draw() {
        SCOPED(Group());

        SCOPE(Font(Font::Bold, 1.2f)) {
            ImGui::TextUnformatted(title_.c_str());
        }

        auto showDraggedItemLine = [this, shown = false](f32 y) mutable {
            if (draggedItem_ && !shown) {
                ImVec2 lineStart(ImGui::GetWindowContentRegionMin().x, y - 1.f);
                ImVec2 lineEnd(ImGui::GetWindowContentRegionMax().x, y - 1.f);
                lineStart += ImGui::GetWindowPos();
                lineEnd += ImGui::GetWindowPos();
                auto* dl = ImGui::GetCurrentWindow()->DrawList;
                dl->AddLine(lineStart + ImVec2(10.f, 0.f), lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 2.f);
                dl->AddTriangleFilled(lineStart + ImVec2(0.f, -3.f), lineStart + ImVec2(5.f, 0.f), lineStart + ImVec2(0.f, 3.f), ImGui::GetColorU32(ImGuiCol_Text));
                shown = true;
            }
        };

        f32 minY = ImGui::GetCursorPosY(), maxY = minY;
        hoveredId_ = UnselectedId;
        SCOPE(ListBox(std::format("##{}List", typeName_).c_str())) {
            for(auto&& [id, item] : items_ | ranges::views::enumerate) {
                const f32 y = ImGui::GetCursorPosY();
                if (id == 0)
                    minY = y;
                else if (id == items_.size() - 1)
                    maxY = y;

                if(ImGui::Selectable(std::format("{}##{}", item.name, typeName_).c_str(), id_ == id))
                    id_ = id;

                if (ImGui::IsItemHovered()) {
                    hoveredId_ = id;
                    showDraggedItemLine(y);
                }
            }
            showDraggedItemLine(ImGui::GetCursorPosY());
        }

        if (draggedItem_) {
            SCOPE(Tooltip(ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
                ImGui::Selectable(std::format("{}##{}", draggedItem_->name, typeName_).c_str(), false);
            }
        }

        if(hoveredId_ == UnselectedId && draggedItem_) {
            const f32 mouseY = ImGui::GetMousePos().y;
            if (mouseY < minY || items_.empty())
                hoveredId_ = 0;
            else if (mouseY > maxY)
                hoveredId_ = items_.size();
        }

        if (hoveredId_ != UnselectedId) {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                if (!draggedItem_) {
                    draggedItem_ = std::make_unique<T>(std::move(items_[hoveredId_]));
                    items_.erase(items_.begin() + hoveredId_);
                    if (id_ > hoveredId_)
                        --id_;
                    else if (id_ == hoveredId_)
                        id_ = DraggedId;
                }
            }
            else {
                if (draggedItem_) {
                    items_.insert(items_.begin() + hoveredId_, std::move(*draggedItem_));
                    draggedItem_.reset();
                    if (id_ >= hoveredId_)
                        ++id_;
                    else if (id_ == DraggedId)
                        id_ = hoveredId_;
                }
            }
        }

        if(!hintsText_.empty())
        {
            SCOPE(FontScale(0.8f)) {
                ImGui::TextWrapped(hintsText_.c_str());
            }
        }

        SCOPE(Font(Font::Bold)) {
            if(ImGui::Button(std::format(ICON_FA_PLUS_CIRCLE " Add {}", typeName_).c_str()))
                return true;
        }

        return false;
    }

    [[nodiscard]] T* selectedItem() const {
        if (id_ == UnselectedId)
            return nullptr;
        if (id_ == DraggedId)
            return draggedItem_.get();

        return &items_[id_];
    }
    [[nodiscard]] bool selected() const { return id_ != UnselectedId; }

    void Deselect() {
        id_ = UnselectedId;
    }

    void Select(i32 newId) {
        GW2_ASSERT(newId < items_.size());
        id_ = newId;
    }

private:
    std::vector<T>& items_;
    std::unique_ptr<T> draggedItem_;

    i32 hoveredId_ = UnselectedId;
    i32 id_ = UnselectedId;
    std::string typeName_;
    std::string title_;
    std::string hintsText_;
    Flags flags_;
};

class SaveTracker
{
    bool shouldSave_ = false;
    mstime lastSaveTime_ = 0;

public:
    static inline constexpr mstime SaveDelay = 1000;

    SaveTracker() = default;
    SaveTracker(SaveTracker&&) = default;
    SaveTracker(const SaveTracker&) = delete;
    SaveTracker& operator=(SaveTracker&&) = default;
    SaveTracker& operator=(const SaveTracker&) = delete;

    bool operator()(bool modified) {
        if (modified)
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

template<typename T>
class ListEditor
{
public:
    struct Configuration
    {
        std::vector<T>& items;
        std::string typeName;
        std::string title;
        std::string hintsText;
        std::function<i32()> addItemCallback;
        std::function<void()> saveCallback;
        typename SelectableListBox<T>::Flags listFlags = SelectableListBox<T>::Flags::None;
    };
    ListEditor(const Configuration& cfg)
        : list_{ cfg.items, cfg.typeName, cfg.title, cfg.hintsText, cfg.listFlags }
        , addItem_{ cfg.addItemCallback }
        , saveCallback_{ cfg.saveCallback } { }

    void Draw(auto&& editor) {
        const f32 w = ImGui::GetContentRegionAvail().x * 0.4f;

        SCOPE(ItemWidth(w)) {
            SCOPE(TextWrapPos(w)) {
                if (list_.Draw()) {
                    i32 newId = addItem_();
                    if (newId != SelectableListBox<T>::UnselectedId) {
                        list_.Select(newId);
                        save_();
                    }
                }
            }
        }

        auto* v = list_.selectedItem();
        if (v) {
            ImGui::SameLine();
            SCOPE(Group()) {
                editor(*v, save_);
            }
        }
    }

    void MaybeSave() {
        if (save_.ShouldSave())
        {
            saveCallback_();
            save_.Saved();
        }
    }

    void Loaded() {
        list_.Deselect();
    }

    bool Editing() {
        return list_.selected();
    }

    void StopEditing() {
        list_.Deselect();
    }

    [[nodiscard]] T* selectedItem() const { return list_.selectedItem(); }

private:
    SelectableListBox<T> list_;
    std::function<i32()> addItem_;
    SaveTracker save_;
    std::function<void()> saveCallback_;
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

} // namespace GW2Clarity::UI