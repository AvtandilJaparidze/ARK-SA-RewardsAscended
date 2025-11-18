#include "Config/Config.h"
#include "Random/Random.h"
#include "Rewards/Rewards.h"
#include "Commands/Commands.h"

static void InitInternal()
{
	try
	{
		Log::Get().Init(PROJECT_NAME);

		ReadConfig();
		RNG = std::make_unique<Random>();
		RewardsMGR = std::make_unique<Rewards>();
		CommandsMGR = std::make_unique<Commands>();
		CommandsMGR->LoadCommands();
	}
	catch (const std::exception& e)
	{
		Log::GetLog()->error("Error while loading plugin, please check your config files: {}", e.what());
		throw;
	}
}

static void UnloadInternal()
{
	CommandsMGR->UnloadCommands();
}

extern "C" __declspec(dllexport) void Plugin_Init()
{
	InitInternal();
}

extern "C" __declspec(dllexport) void Plugin_Unload()
{
	UnloadInternal();
}