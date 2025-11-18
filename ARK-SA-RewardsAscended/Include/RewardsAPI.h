#pragma once
#include <API/ARK/Ark.h>

#ifdef ARKSAREWARDSASCENDED_EXPORTS
#define REWARD_API __declspec(dllexport)
#else
#define REWARD_API __declspec(dllimport)
#endif

namespace RewardsAPI
{
	bool REWARD_API GiveReward(AShooterPlayerController* PC, const std::string& RewardID);
}