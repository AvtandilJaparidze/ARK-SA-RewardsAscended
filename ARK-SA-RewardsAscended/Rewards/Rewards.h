#pragma once
#include <memory>
#include <API/ARK/Ark.h>
#include <json.hpp>

class Rewards
{
public:
	Rewards() = default;
	~Rewards() = default;

	void GiveRewardFromConfig(AShooterPlayerController* PC, const FString& EOS_ID, const std::string& RewardID) const;

private:
	float GetStatValue(float StatModifier, float InitialValueConstant, float RandomizerRangeMultiplier, float StateModifierScale, bool bDisplayAsPercent) const;
	void ApplyItemStats(TArray<UPrimalItem*> Items, int Armor, int Durability, int Damage) const;

	void RewardItem(AShooterPlayerController* PC, const nlohmann::json& Obj) const;
	void RewardEngram(AShooterPlayerController* PC, const nlohmann::json& Obj) const;
	void RewardCommand(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Obj) const;
	void RewardDino(AShooterPlayerController* PC, const nlohmann::json& Obj) const;

	void ProcessItems(AShooterPlayerController* PC, const nlohmann::json& Section) const;
	void ProcessEngrams(AShooterPlayerController* PC, const nlohmann::json& Section) const;
	void ProcessCommands(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Section) const;
	void ProcessDinos(AShooterPlayerController* PC, const nlohmann::json& Section) const;

	std::vector<nlohmann::json> GetRandomRewardIndexes(const nlohmann::json& Section, const std::string& MainSectionKey, const std::string& SubSectionKey) const;

	void ProcessRandomItems(AShooterPlayerController* PC, const nlohmann::json& Section) const;
	void ProcessRandomEngrams(AShooterPlayerController* PC, const nlohmann::json& Section) const;
	void ProcessRandomCommands(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Section) const;
	void ProcessRandomDinos(AShooterPlayerController* PC, const nlohmann::json& Section) const;
};

inline std::unique_ptr<Rewards> RewardsMGR;