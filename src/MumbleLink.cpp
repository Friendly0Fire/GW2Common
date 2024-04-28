#include "MumbleLink.h"

#include <nlohmann/json.hpp>

#include "Utility.h"

static_assert(alignof(vec3) == alignof(float[3]));

struct LinkedMem
{
#ifdef WIN32
    UINT32 uiVersion;
    DWORD uiTick;
#else
    uint32_t uiVersion;
    uint32_t uiTick;
#endif
    vec3 fAvatarPosition;
    vec3 fAvatarFront;
    vec3 fAvatarTop;
    wchar_t name[256];
    vec3 fCameraPosition;
    vec3 fCameraFront;
    vec3 fCameraTop;
    wchar_t identity[256];
#ifdef WIN32
    UINT32 context_len;
#else
    uint32_t context_len;
#endif
    u8 context[256];
    wchar_t description[2048];
};

struct MumbleContext
{
    std::byte serverAddress[28]; // contains sockaddr_in or sockaddr_in6
    uint32_t mapId;
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    // Additional data beyond the 48 bytes Mumble uses for identification
    uint32_t uiState; // Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight, Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game has
                      // focus, Bit 5 = Is in Competitive game mode, Bit 6 = Textbox has focus, Bit 7 = Is in Combat
    uint16_t compassWidth; // pixels
    uint16_t compassHeight; // pixels
    f32 compassRotation; // radians
    f32 playerX; // continentCoords
    f32 playerY; // continentCoords
    f32 mapCenterX; // continentCoords
    f32 mapCenterY; // continentCoords
    f32 mapScale;
    uint32_t processId;
    uint8_t mountIndex;
};

MumbleLink::MumbleLink() {
    if(auto* m = GetCommandLineArg(L"mumble"); m)
        fileMappingName_ = m;

    fileMapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), fileMappingName_.c_str());
    if(!fileMapping_) {
        LogError(L"Could not find MumbleLink map named '{}'!", fileMappingName_.c_str());
        return;
    }

    linkedMemory_ = static_cast<LinkedMem*>(MapViewOfFile(fileMapping_, FILE_MAP_READ, 0, 0, sizeof(LinkedMem)));
    if(!linkedMemory_) {
        LogError(L"Could not map to MumbleLink map named '{}'!", fileMappingName_.c_str());

        CloseHandle(fileMapping_);
        fileMapping_ = nullptr;
    }
}

MumbleLink::~MumbleLink() {
    if(linkedMemory_) {
        UnmapViewOfFile(linkedMemory_);
        linkedMemory_ = nullptr;
    }
    if(fileMapping_) {
        CloseHandle(fileMapping_);
        fileMapping_ = nullptr;
    }
}

void MumbleLink::OnUpdate() {
    identity_ = {};
    auto identityUtf8 = utf8_encode(linkedMemory_->identity);
    auto json = nlohmann::json::parse(identityUtf8, nullptr, false);

    auto updateIfExists = [&json](auto& value, const char* key) {
        auto f = json.find(key);
        if(f != json.end())
            value = *f;
    };

    updateIfExists(identity_.commander, "commander");
    updateIfExists(identity_.fov, "fov");
    updateIfExists(identity_.uiScale, "uisz");
    updateIfExists(identity_.race, "race");
    updateIfExists(identity_.specialization, "spec");
    updateIfExists(identity_.profession, "profession");
    updateIfExists(identity_.name, "name");
}

bool MumbleLink::isInMap() const {
    if(!linkedMemory_)
        return false;

    return context()->mapId != 0;
}

uint32_t MumbleLink::mapId() const {
    if(!linkedMemory_)
        return 0;

    return context()->mapId;
}

std::wstring MumbleLink::characterName() const {
    if(!linkedMemory_)
        return L"";

    return utf8_decode(identity_.name);
}

const f32 MinSurfaceThreshold = -1.15f;

bool MumbleLink::isSwimmingOnSurface() const {
    if(!linkedMemory_)
        return false;

    return linkedMemory_->fAvatarPosition.y <= -1.f && linkedMemory_->fAvatarPosition.y >= MinSurfaceThreshold;
}

bool MumbleLink::isUnderwater() const {
    if(!linkedMemory_)
        return false;

    return linkedMemory_->fAvatarPosition.y < MinSurfaceThreshold;
}

MumbleLink::EliteSpec MumbleLink::characterSpecialization() const {
    enum class AnetEliteSpec : uint8_t
    {
        None = 0,
        Druid = 5,
        Daredevil = 7,
        Berserker = 18,
        Dragonhunter = 27,
        Reaper = 34,
        Chronomancer = 40,
        Scrapper = 43,
        Tempest = 48,
        Herald = 52,
        Soulbeast = 55,
        Weaver = 56,
        Holosmith = 57,
        Deadeye = 58,
        Mirage = 59,
        Scourge = 60,
        Spellbreaker = 61,
        Firebrand = 62,
        Renegade = 63,
        Harbinger = 64,
        Willbender = 65,
        Virtuoso = 66,
        Catalyst = 67,
        Bladesworn = 68,
        Vindicator = 69,
        Mechanist = 70,
        Specter = 71,
        Untamed = 72
    };

    switch(AnetEliteSpec(identity_.specialization)) {
    default:
        return EliteSpec::None;
    case AnetEliteSpec::Druid:
        return EliteSpec::Druid;
    case AnetEliteSpec::Daredevil:
        return EliteSpec::Daredevil;
    case AnetEliteSpec::Berserker:
        return EliteSpec::Berserker;
    case AnetEliteSpec::Dragonhunter:
        return EliteSpec::Dragonhunter;
    case AnetEliteSpec::Reaper:
        return EliteSpec::Reaper;
    case AnetEliteSpec::Chronomancer:
        return EliteSpec::Chronomancer;
    case AnetEliteSpec::Scrapper:
        return EliteSpec::Scrapper;
    case AnetEliteSpec::Tempest:
        return EliteSpec::Tempest;
    case AnetEliteSpec::Herald:
        return EliteSpec::Herald;
    case AnetEliteSpec::Soulbeast:
        return EliteSpec::Soulbeast;
    case AnetEliteSpec::Weaver:
        return EliteSpec::Weaver;
    case AnetEliteSpec::Holosmith:
        return EliteSpec::Holosmith;
    case AnetEliteSpec::Deadeye:
        return EliteSpec::Deadeye;
    case AnetEliteSpec::Mirage:
        return EliteSpec::Mirage;
    case AnetEliteSpec::Scourge:
        return EliteSpec::Scourge;
    case AnetEliteSpec::Spellbreaker:
        return EliteSpec::Spellbreaker;
    case AnetEliteSpec::Firebrand:
        return EliteSpec::Firebrand;
    case AnetEliteSpec::Renegade:
        return EliteSpec::Renegade;
    case AnetEliteSpec::Harbinger:
        return EliteSpec::Harbinger;
    case AnetEliteSpec::Willbender:
        return EliteSpec::Willbender;
    case AnetEliteSpec::Virtuoso:
        return EliteSpec::Virtuoso;
    case AnetEliteSpec::Catalyst:
        return EliteSpec::Catalyst;
    case AnetEliteSpec::Bladesworn:
        return EliteSpec::Bladesworn;
    case AnetEliteSpec::Vindicator:
        return EliteSpec::Vindicator;
    case AnetEliteSpec::Mechanist:
        return EliteSpec::Mechanist;
    case AnetEliteSpec::Specter:
        return EliteSpec::Specter;
    case AnetEliteSpec::Untamed:
        return EliteSpec::Untamed;
    }
}

bool MumbleLink::isInWvW() const {
    if(!linkedMemory_)
        return false;

    auto mt = context()->mapType;

    return mt == 18 || (mt >= 9 && mt <= 15 && mt != 13);
}

uint32_t MumbleLink::uiState() const {
    if(!linkedMemory_)
        return 0;

    return context()->uiState;
}

vec3 MumbleLink::position() const {
    if(!linkedMemory_)
        return { 0, 0, 0 };

    return linkedMemory_->fAvatarPosition;
}

MumbleLink::MountType MumbleLink::currentMount() const {
    if(!linkedMemory_)
        return MountType::None;

    return MountType(context()->mountIndex);
}

bool MumbleLink::isMounted() const {
    if(!linkedMemory_)
        return false;

    return context()->mountIndex != 0;
}

const MumbleContext* MumbleLink::context() const {
    if(!linkedMemory_)
        return nullptr;

    return reinterpret_cast<const MumbleContext*>(&linkedMemory_->context);
}
