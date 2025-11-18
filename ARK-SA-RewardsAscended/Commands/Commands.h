#pragma once
#include <memory>
#include <string>
#include <API/ARK/Ark.h>

class Commands
{
public:
	Commands() = default;
	~Commands() = default;

	void LoadCommands() const;
	void UnloadCommands() const;

private:
	void ReloadConfigInternal() const;
	void ReloadConfigConsole(APlayerController* APC, FString*, bool) const;
	void ReloadConfigRcon(RCONClientConnection* Connection, RCONPacket* Packet, UWorld*) const;

	void GiveRewardInternal(const FString& CMD) const;
	void GiveRewardConsole(APlayerController* APC, FString* CMD, bool) const;
	void GiveRewardRcon(RCONClientConnection* Connection, RCONPacket* Packet, UWorld*) const;
};

inline std::unique_ptr<Commands> CommandsMGR;