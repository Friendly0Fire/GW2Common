#pragma once
#include <set>

#include "ScanCode.h"

struct KeyCombo
{
    ScanCode& key() { return key_; }
    Modifier& mod() { return mod_; }
    ScanCode key() const { return key_; }
    Modifier mod() const { return mod_; }

    KeyCombo() {
        key_ = ScanCode::None;
        mod_ = Modifier::None;
    }
    KeyCombo(ScanCode k) : key_(k), mod_(Modifier::None) { }
    KeyCombo(ScanCode k, Modifier m) : key_(k), mod_(m) { }
    explicit KeyCombo(const std::set<ScanCode>& keys) : KeyCombo() {
        for(auto sc : keys) {
            if(IsModifier(sc))
                mod_ = mod_ | ToModifier(sc);
            else
                key_ = sc;
        }
    }

    friend std::strong_ordering operator<=>(KeyCombo a, KeyCombo b) { return a.storage_ <=> b.storage_; }
    friend bool operator==(KeyCombo a, KeyCombo b) { return a.storage_ == b.storage_; }

private:
    union
    {
        uint64_t storage_;
        struct
        {
            ScanCode key_;
            Modifier mod_;
        };
    };

    friend struct std::hash<KeyCombo>;
};

template<>
struct std::hash<KeyCombo>
{
    hash() = default;
    std::uint64_t operator()(KeyCombo kc) const noexcept(noexcept(std::hash<uint64_t>()(kc.storage_))) {
        return std::hash<uint64_t>()(kc.storage_);
    }
};
