#pragma once
#include "Common.h"
#include "Condition.h"
#include "Input.h"
#include "Keybind.h"

class ActivationKeybind : public Keybind
{
public:
	using Callback = std::function<PassToGame(Activated)>;

	ActivationKeybind(std::string nickname, std::string displayName, std::string category, KeyCombo ks, bool saveToConfig)
		: Keybind(nickname, displayName, category, ks.key(), ks.mod(), saveToConfig) {
		Bind();
	}

	ActivationKeybind(std::string nickname, std::string displayName, std::string category, ScanCode key, Modifier mod, bool saveToConfig)
		: Keybind(nickname, displayName, category, key, mod, saveToConfig) {
		Bind();
	}
	ActivationKeybind(std::string nickname, std::string displayName, std::string category)
		: Keybind(nickname, displayName, category) {
		Bind();
	}
	virtual ~ActivationKeybind();

	void callback(Callback&& cb) { callback_ = std::move(cb); }
	Callback callback() const { return callback_; }
	void conditions(ConditionSetPtr ptr) { conditions_ = ptr; }

	[[nodiscard]] bool conditionsFulfilled() const { return conditions_ == nullptr || conditions_->passes(); }
	[[nodiscard]] int conditionsScore() const { return conditions_ == nullptr ? 0 : conditions_->score(); }
	[[nodiscard]] int keysScore() const { return std::popcount(ToUnderlying(mod_)); }

protected:
	void ApplyKeys() override {
		Rebind();
		Keybind::ApplyKeys();
	}
	void Bind();
	void Rebind();
	ConditionSetPtr conditions_;
	Callback callback_;
};
