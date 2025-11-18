#include "../Include/RewardsAPI.h"
#include "../Rewards/Rewards.h"

bool RewardsAPI::GiveReward(AShooterPlayerController* PC, const std::string& RewardID)
{
	try
	{
		if (!PC)
		{
			throw std::runtime_error("PlayerController is null");
		}

		const FString EOS_ID = AsaApi::GetApiUtils().GetEOSIDFromController(PC);

		if (EOS_ID.IsEmpty())
		{
			throw std::runtime_error("Failed to get EOS ID from PlayerController");
		}

		RewardsMGR->GiveRewardFromConfig(PC, EOS_ID, RewardID);
		return true;
	}
	catch (const std::exception& e)
	{
		Log::GetLog()->error("Failed to give reward to player: {}", e.what());
		return false;
	}

	return false;
}