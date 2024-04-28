#pragma once
#include "Common.h"
#include "Singleton.h"

#define PARSE_FLAG_BOOL(name, offset) \
    [[nodiscard]] inline bool name() const { return (uiState() & (1 << offset)) != 0; }

struct LinkedMem;
struct MumbleContext;

enum class ConditionalState : u32
{
    None = 0,
    Underwater = 1,
    OnWater = 2,
    InCombat = 4,
    InWvW = 8,

    All = Underwater | OnWater | InCombat | InWvW,

    IsFlag
};

class MumbleLink : public Singleton<MumbleLink>
{
public:
    enum class Profession : u8
    {
        None = 0,
        Guardian = 1,
        Warrior = 2,
        Engineer = 3,
        Ranger = 4,
        Thief = 5,
        Elementalist = 6,
        Mesmer = 7,
        Necromancer = 8,
        Revenant = 9
    };

    enum class EliteSpec : u8
    {
        None = 0,
        Berserker = 1,
        Bladesworn = 2,
        Catalyst = 3,
        Chronomancer = 4,
        Daredevil = 5,
        Deadeye = 6,
        Dragonhunter = 7,
        Druid = 8,
        Firebrand = 9,
        Harbinger = 10,
        Herald = 11,
        Holosmith = 12,
        Mechanist = 13,
        Mirage = 14,
        Reaper = 15,
        Renegade = 16,
        Scourge = 17,
        Scrapper = 18,
        Soulbeast = 19,
        Specter = 20,
        Spellbreaker = 21,
        Tempest = 22,
        Untamed = 23,
        Vindicator = 24,
        Virtuoso = 25,
        Weaver = 26,
        Willbender = 27
    };

    enum class Race : u8
    {
        Asura = 0,
        Charr = 1,
        Human = 2,
        Norn = 3,
        Sylvari = 4
    };

    enum class MountType : u32
    {
        None = 0,
        Jackal,
        Griffon,
        Springer,
        Skimmer,
        Raptor,
        RollerBeetle,
        Warclaw,
        Skyscale,
        Skiff,
        SiegeTurtle
    };

    MumbleLink();
    ~MumbleLink() override;

    void OnUpdate();

    [[nodiscard]] bool isInWvW() const;

    // uiState flags
    PARSE_FLAG_BOOL(isMapOpen, 0);
    PARSE_FLAG_BOOL(isCompassTopRight, 1);
    PARSE_FLAG_BOOL(doesCompassHaveRotationEnabled, 2);
    PARSE_FLAG_BOOL(gameHasFocus, 3);
    PARSE_FLAG_BOOL(isInCompetitiveMode, 4);
    PARSE_FLAG_BOOL(textboxHasFocus, 5);
    PARSE_FLAG_BOOL(isInCombat, 6);

    [[nodiscard]] vec3 position() const;

    [[nodiscard]] MountType currentMount() const;
    [[nodiscard]] bool isMounted() const;
    [[nodiscard]] bool isInMap() const;
    [[nodiscard]] uint32_t mapId() const;
    [[nodiscard]] std::wstring characterName() const;
    [[nodiscard]] bool isSwimmingOnSurface() const;
    [[nodiscard]] bool isUnderwater() const;

    [[nodiscard]] bool isOnOrUnderwater() const { return isSwimmingOnSurface() || isUnderwater(); }

    [[nodiscard]] ConditionalState currentState() const {
        return (isSwimmingOnSurface() ? ConditionalState::OnWater : ConditionalState::None) |
               (isUnderwater() ? ConditionalState::Underwater : ConditionalState::None) |
               (isInCombat() ? ConditionalState::InCombat : ConditionalState::None) |
               (isInWvW() ? ConditionalState::InWvW : ConditionalState::None);
    }

    [[nodiscard]] Profession characterProfession() const { return identity_.profession; }

    [[nodiscard]] EliteSpec characterSpecialization() const;

    [[nodiscard]] Race characterRace() const { return identity_.race; }

    [[nodiscard]] bool isCommander() const { return identity_.commander; }

    [[nodiscard]] f32 fov() const { return identity_.fov; }

    [[nodiscard]] uint8_t uiScale() const { return identity_.uiScale; }

protected:
    std::wstring fileMappingName_ = L"MumbleLink";

    [[nodiscard]] uint32_t uiState() const;

    struct Identity
    {
        Profession profession = Profession::None;
        EliteSpec specialization = EliteSpec::None;
        Race race = Race::Asura;
        bool commander = false;
        f32 fov = 0.f;
        uint8_t uiScale = 0;
        std::string name;
    };

    [[nodiscard]] const MumbleContext* context() const;
    HANDLE fileMapping_ = nullptr;
    LinkedMem* linkedMemory_ = nullptr;

    Identity identity_;
};
