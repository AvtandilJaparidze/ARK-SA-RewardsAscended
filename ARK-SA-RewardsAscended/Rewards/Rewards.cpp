#include "Rewards.h"
#include <json.hpp>
#include "../Random/Random.h"
#include "../Config/Config.h"

void Rewards::GiveRewardFromConfig(AShooterPlayerController* PC, const FString& EOS_ID, const std::string& RewardID) const
{
	const auto& RewardsConfig = config["Rewards"];

	if (RewardsConfig.find(RewardID) == RewardsConfig.end())
	{
		throw std::runtime_error(fmt::format("Reward ID: {} not found in configuration", RewardID));
	}

	const auto& Reward = RewardsConfig[RewardID];

	this->ProcessItems(PC, Reward);
	this->ProcessEngrams(PC, Reward);
	this->ProcessCommands(PC, EOS_ID, Reward);
	this->ProcessDinos(PC, Reward);

	this->ProcessRandomItems(PC, Reward);
	this->ProcessRandomEngrams(PC, Reward);
	this->ProcessRandomCommands(PC, EOS_ID, Reward);
	this->ProcessRandomDinos(PC, Reward);
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/ArkShop.cpp
float Rewards::GetStatValue(float StatModifier, float InitialValueConstant, float RandomizerRangeMultiplier, float StateModifierScale, bool bDisplayAsPercent) const
{
	float ItemStatValue;

	if (bDisplayAsPercent)
		InitialValueConstant += 100;

	if (InitialValueConstant > StatModifier)
		StatModifier = InitialValueConstant;

	ItemStatValue = (StatModifier - InitialValueConstant) / (InitialValueConstant * RandomizerRangeMultiplier * StateModifierScale);

	return ItemStatValue;
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/ArkShop.cpp
void Rewards::ApplyItemStats(TArray<UPrimalItem*> Items, int Armor, int Durability, int Damage) const
{
	if (Armor > 0 || Durability > 0 || Damage > 0)
	{
		for (UPrimalItem* Item : Items)
		{
			bool Updated = false;

			auto ItemStatInfos = Item->ItemStatInfosField();

			if (Armor > 0)
			{
				FItemStatInfo ItemStat = ItemStatInfos()[EPrimalItemStat::Armor];

				if (ItemStat.bUsed)
				{
					float NewStat = 0.f;
					bool IsPercent = ItemStat.bDisplayAsPercent;

					NewStat = this->GetStatValue((float)Armor, ItemStat.InitialValueConstant, ItemStat.RandomizerRangeMultiplier, ItemStat.StateModifierScale, IsPercent);

					if (NewStat >= 65536.f)
						NewStat = 65535;

					Item->ItemStatValuesField()()[EPrimalItemStat::Armor] = (int)NewStat;
					Updated = true;
				}
			}

			if (Durability > 0)
			{
				FItemStatInfo ItemStat = ItemStatInfos()[EPrimalItemStat::MaxDurability];

				if (ItemStat.bUsed)
				{
					float NewStat = 0.f;
					bool Percent = ItemStat.bDisplayAsPercent;

					NewStat = this->GetStatValue((float)Durability, ItemStat.InitialValueConstant, ItemStat.RandomizerRangeMultiplier, ItemStat.StateModifierScale, Percent) + 1;

					if (NewStat >= 65536.f)
						NewStat = 65535;

					Item->ItemStatValuesField()()[EPrimalItemStat::MaxDurability] = (int)NewStat;
					Item->ItemDurabilityField() = Item->GetItemStatModifier(EPrimalItemStat::MaxDurability);
					Updated = true;
				}
			}

			if (Damage > 0)
			{
				FItemStatInfo ItemStat = ItemStatInfos()[EPrimalItemStat::WeaponDamagePercent];

				if (ItemStat.bUsed)
				{
					float NewStat = 0.f;
					bool Percent = ItemStat.bDisplayAsPercent;

					NewStat = this->GetStatValue((float)Damage, ItemStat.InitialValueConstant, ItemStat.RandomizerRangeMultiplier, ItemStat.StateModifierScale, Percent);

					if (NewStat >= 65536.f)
						NewStat = 65535;

					Item->ItemStatValuesField()()[EPrimalItemStat::WeaponDamagePercent] = (int)NewStat;
					Updated = true;
				}
			}

			if (Updated)
				Item->UpdatedItem(false, false);
		}
	}
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/Helpers.h
TArray<float> GetStatPoints(APrimalDinoCharacter* Dino)
{
	TArray<float> Floats;
	UPrimalCharacterStatusComponent* Comp = Dino->GetCharacterStatusComponent();
	int NumEntries = EPrimalCharacterStatusValue::MAX - 1;
	for (int i = 0; i < NumEntries; i++)
		Floats.Add(
			Comp->NumberOfLevelUpPointsAppliedField()()[(EPrimalCharacterStatusValue::Type)i]
			+
			Comp->NumberOfMutationsAppliedTamedField()()[(EPrimalCharacterStatusValue::Type)i]
		);

	for (int i = 0; i < NumEntries; i++)
		Floats.Add(Comp->NumberOfLevelUpPointsAppliedTamedField()()[(EPrimalCharacterStatusValue::Type)i]);

	return Floats;
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/Helpers.h
TArray<float> GetCharacterStatsAsFloats(APrimalDinoCharacter* Dino)
{
	TArray<float> Floats;
	UPrimalCharacterStatusComponent* Comp = Dino->GetCharacterStatusComponent();
	int NumEntries = EPrimalCharacterStatusValue::MAX - 1;
	for (int i = 0; i < NumEntries; i++)
		Floats.Add(Comp->CurrentStatusValuesField()()[(EPrimalCharacterStatusValue::Type)i]);

	for (int i = 0; i < NumEntries; i++)
		Floats.Add(Comp->MaxStatusValuesField()()[(EPrimalCharacterStatusValue::Type)i]);

	for (int i = 0; i < NumEntries; i++)
		Floats.Add(Comp->GetStatusValueRecoveryRate((EPrimalCharacterStatusValue::Type)i));

	Floats.Add((float)Dino->bIsFemale()());

	int len = Floats.Num();

	Floats.Append(GetStatPoints(Dino));
	Floats.Add(len);
	return Floats;
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/Helpers.h
TArray<FString> GetSaddleData(UPrimalItem* Saddle)
{
	TArray<FString> Data;
	TArray<FString> Colors;

	Data.Add("");
	Data.Add("");
	Data.Add("");
	Colors.Add("0");
	Colors.Add("0");
	Colors.Add("0");

	if (!Saddle)
	{
		Data.Append(Colors);
		return Data;
	}

	const float Modifier = Saddle->GetItemStatModifier(EPrimalItemStat::Armor);
	Data[0] = FString::FromInt(FMath::Floor(Modifier));

	FLinearColor FColor;
	UPrimalGameData* GameData = AsaApi::GetApiUtils().GetGameData();
	auto Index = Saddle->ItemQualityIndexField();

	auto entry = GameData->ItemQualityDefinitionsField()[Index];
	FColor = entry.QualityColorField();

	Colors[0] = FString(std::to_string(FColor.R));
	Colors[1] = FString(std::to_string(FColor.G));
	Colors[2] = FString(std::to_string(FColor.B));
	FString QualityNameStr = entry.QualityNameField();

	Data[1] = FString::Format("{} {}", API::Tools::Utf8Encode(*QualityNameStr), API::Tools::Utf8Encode(*Saddle->DescriptiveNameBaseField()));
	Data[2] = AsaApi::GetApiUtils().GetBlueprint(Saddle);

	Data.Append(Colors);
	return Data;
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/Helpers.h
TArray<FString> GetDinoDataStrings(APrimalDinoCharacter* Dino, const FString& DinoNameInMAP, const FString& DinoName, UPrimalItem* Saddle)
{
	TArray<FString> Strings;
	Strings.Add(DinoNameInMAP);
	Strings.Add(DinoName);

	FString Temp;
	Dino->GetColorSetInidcesAsString(&Temp);
	Strings.Add(Temp);

	Strings.Add(Dino->bNeutered()() ? "NEUTERED" : "");

	Temp = "";
	if (Dino->bUsesGender()())
		if (Dino->bIsFemale()())
			Temp = "FEMALE";
		else
			Temp = "MALE";
	Strings.Add(Temp);
	Strings.Add(""); // empty
	Strings.Add("0"); // should get bitmasks for buffs but doesn't seem to get used

	//strings.Append(GetSaddleData(saddle)); 
	//bypassing saddle data for now
	Strings.Add("");
	Strings.Add("");
	Strings.Add("");
	Strings.Add("0");
	Strings.Add("0");
	Strings.Add("0");

	Strings.Add(""); // extra data like harvest levels - not needed
	Temp = "";
	Dino->GetCurrentDinoName(&Temp, nullptr);
	Strings.Add(Temp);
	return Strings;
}

//credit to ArkShop https://github.com/ArkServerApi/ASA-Plugins/blob/master/ArkShop/ArkShop/Private/ArkShop.cpp
FCustomItemData GetCryoPodData(APrimalDinoCharacter* Dino, UPrimalItem* Saddle)
{
	FCustomItemData CustomItemData;

	FARKDinoData DinoData;
	Dino->GetDinoData(&DinoData);

	//
	// Custom Data Name
	//
	CustomItemData.CustomDataName = FName("Dino", EFindName::FNAME_Add);

	TArray<FName> Names;
	Dino->GetColorSetNamesAsArray(&Names);
	CustomItemData.CustomDataNames = Names;
	// one time use settings
	CustomItemData.CustomDataNames.Add(FName("MissionTemporary", EFindName::FNAME_Add));
	CustomItemData.CustomDataNames.Add(FName("None", EFindName::FNAME_Find));

	//
	// Custom Data Floats
	//
	CustomItemData.CustomDataFloats = GetCharacterStatsAsFloats(Dino);

	//
	// Custom Data Doubles
	//
	auto T1 = AsaApi::GetApiUtils().GetShooterGameMode()->GetWorld()->TimeSecondsField();
	CustomItemData.CustomDataDoubles.Doubles.Add(T1);
	CustomItemData.CustomDataDoubles.Doubles.Add(Dino->BabyNextCuddleTimeField() - T1);
	CustomItemData.CustomDataDoubles.Doubles.Add(Dino->NextAllowedMatingTimeField());

	const float D1 = static_cast<float>(Dino->RandomMutationsMaleField());
	const double D2 = static_cast<double>(D1);
	CustomItemData.CustomDataDoubles.Doubles.Add(D2);

	const float D3 = static_cast<float>(Dino->RandomMutationsFemaleField());
	const double D4 = static_cast<double>(D3);
	CustomItemData.CustomDataDoubles.Doubles.Add(D4);

	auto stat = Dino->MyCharacterStatusComponentField();
	if (stat)
	{
		const double d5 = static_cast<double>(stat->DinoImprintingQualityField());
		CustomItemData.CustomDataDoubles.Doubles.Add(d5);
	}
	CustomItemData.CustomDataDoubles.Doubles.Add(std::time(nullptr));

	//
	// Custom Data Strings
	//
	CustomItemData.CustomDataStrings = GetDinoDataStrings(Dino, DinoData.DinoNameInMap, DinoData.DinoName, Saddle);

	//
	// Custom Data Classes
	//
	CustomItemData.CustomDataClasses.Add(DinoData.DinoClass);

	//
	//	Custom Data Soft Classes
	//
	//customItemData.CustomDataSoftClasses.Add(dinoData.DinoClass);

	//
	// Custom Data Bytes
	//
	FCustomItemByteArray dinoBytes, saddlebytes;
	dinoBytes.Bytes = DinoData.DinoData;
	CustomItemData.CustomDataBytes.ByteArrays.Add(dinoBytes);

	if (Saddle)
	{
		Saddle->GetItemBytes(&saddlebytes.Bytes);
		CustomItemData.CustomDataBytes.ByteArrays.Add(saddlebytes);
	}
	CustomItemData.CustomDataBytes.ByteArrays.Add(FCustomItemByteArray());

	FCustomItemByteArray arr = FCustomItemByteArray();
	arr.Bytes.Add(Dino->TamedAggressionLevelField());
	CustomItemData.CustomDataBytes.ByteArrays.Add(arr);

	return CustomItemData;
}

void Rewards::RewardItem(AShooterPlayerController* PC, const nlohmann::json& Obj) const
{
	const std::string& SBlueprint = Obj.value("Blueprint", std::string());

	if (SBlueprint.empty())
	{
		return;
	}

	//default variables

	const float Quality = Obj.value("Quality", 1.f);
	const bool ForceBlueprint = Obj.value("ForceBlueprint", false);
	const int Amount = Obj.value("Amount", 1);
	const int Armor = Obj.value("Armor", 0);
	const int Durability = Obj.value("Durability", 0);
	const int Damage = Obj.value("Damage", 0);

	//random variables

	const bool UseRandomQuality = Obj.value("UseRandomQuality", false);
	const int MinRandomQuality = Obj.value("MinRandomQuality", 0);
	const int MaxRandomQuality = Obj.value("MaxRandomQuality", 0);

	const bool UseRandomAmount = Obj.value("UseRandomAmount", false);
	const int MinRandomAmount = Obj.value("MinRandomAmount", 0);
	const int MaxRandomAmount = Obj.value("MaxRandomAmount", 0);

	const bool UseRandomArmor = Obj.value("UseRandomArmor", false);
	const int MinRandomArmor = Obj.value("MinRandomArmor", 0);
	const int MaxRandomArmor = Obj.value("MaxRandomArmor", 0);

	const bool UseRandomDurability = Obj.value("UseRandomDurability", false);
	const int MinRandomDurability = Obj.value("MinRandomDurability", 0);
	const int MaxRandomDurability = Obj.value("MaxRandomDurability", 0);

	const bool UseRandomDamage = Obj.value("UseRandomDamage", false);
	const int MinRandomDamage = Obj.value("MinRandomDamage", 0);
	const int MaxRandomDamage = Obj.value("MaxRandomDamage", 0);

	float FinalQuality = Quality;
	if (UseRandomQuality)
	{
		FinalQuality = (float)RNG->Get<int>(MinRandomQuality, MaxRandomQuality);
	}

	int FinalAmount = Amount;
	if (UseRandomAmount)
	{
		FinalAmount = RNG->Get<int>(MinRandomAmount, MaxRandomAmount);
	}

	int FinalArmor = Armor;
	if (UseRandomArmor)
	{
		FinalArmor = RNG->Get<int>(MinRandomArmor, MaxRandomArmor);
	}

	int FinalDurability = Durability;
	if (UseRandomDurability)
	{
		FinalDurability = RNG->Get<int>(MinRandomDurability, MaxRandomDurability);
	}

	int FinalDamage = Damage;
	if (UseRandomDamage)
	{
		FinalDamage = RNG->Get<int>(MinRandomDamage, MaxRandomDamage);
	}

	FString FBlueprint(SBlueprint);

	TSubclassOf<UObject> ArcheType;
	UVictoryCore::StringReferenceToClass(&ArcheType, &FBlueprint);
	auto ItemClass = ArcheType.uClass;

	bool StacksInOne = false;
	if (ItemClass)
	{
		UPrimalItem* DefaultItem = static_cast<UPrimalItem*>(ItemClass->GetDefaultObject(true));
		if (DefaultItem)
		{
			StacksInOne = DefaultItem->GetMaxItemQuantity(AsaApi::GetApiUtils().GetWorld()) <= 1;
		}
	}

	UPrimalInventoryComponent* Inventory = PC->GetPlayerInventoryComponent();
	if (!Inventory)
	{
		return;
	}

	if (StacksInOne || ForceBlueprint)
	{
		TArray<UPrimalItem*> OutItems{};
		for (int i = 0; i < FinalAmount; ++i)
		{
			OutItems.Add(
				UPrimalItem::AddNewItem(
					ItemClass,
					Inventory,
					false,
					false,
					FinalQuality,
					!ForceBlueprint,
					1,
					ForceBlueprint,
					0,
					false,
					nullptr,
					0,
					false,
					false,
					true,
					true,
					true
				)
			);
		}

		this->ApplyItemStats(OutItems, FinalArmor, FinalDurability, FinalDamage);
	}
	else
	{
		Inventory->IncrementItemTemplateQuantity(ItemClass, FinalAmount, true, ForceBlueprint, nullptr, nullptr, false, false, false, false, true, false, true, true, false);
	}
}

void Rewards::RewardEngram(AShooterPlayerController* PC, const nlohmann::json& Obj) const
{
	const std::string& SBlueprint = Obj.value("Blueprint", std::string());

	if (SBlueprint.empty())
	{
		return;
	}

	const FString FBlueprint(SBlueprint);

	auto* CheatManager = static_cast<UShooterCheatManager*>(PC->CheatManagerField().Get());

	if (!CheatManager)
	{
		return;
	}

	CheatManager->UnlockEngram(&FBlueprint);
}

void Rewards::RewardCommand(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Obj) const
{
	const bool RunAsAdmin = Obj.value("RunAsAdmin", false);
	const std::string& Command = Obj.value("Command", std::string());

	if (Command.empty())
	{
		return;
	}

	FString FCommand = FString(fmt::format(
		Command, fmt::arg("eos_id", eos_id.ToString()),
		fmt::arg("playerid", AsaApi::GetApiUtils().GetPlayerID(PC)),
		fmt::arg("tribeid", AsaApi::GetApiUtils().GetTribeID(PC))));

	const bool WasAdmin = PC->bIsAdmin()();

	if (!WasAdmin && RunAsAdmin)
	{
		PC->bIsAdmin() = true;
	}

	FString Result;
	PC->ConsoleCommand(&Result, &FCommand, false);

	if (!WasAdmin && RunAsAdmin)
	{
		PC->bIsAdmin() = false;
	}
}

void Rewards::RewardDino(AShooterPlayerController* PC, const nlohmann::json& Obj) const
{
	const std::string& SBlueprint = Obj.value("Blueprint", std::string());

	if (SBlueprint.empty())
	{
		return;
	}

	const std::string CryoPodBlueprint = config["General"].value("CryoPodItemPath", 
		"Blueprint'/Game/Extinction/CoreBlueprints/Weapons/PrimalItem_WeaponEmptyCryopod.PrimalItem_WeaponEmptyCryopod'");

	const bool GiveInCryoPod = Obj.value("GiveInCryoPod", false);

	const int Level = Obj.value("Level", 1);
	const bool Neutered = Obj.value("Neutered", false);
	const std::string& Gender = Obj.value("Gender", "Random");
	const std::string SaddleBlueprint = Obj.value("SaddleBlueprint", std::string());

	const bool UseRandomLevel = Obj.value("UseRandomLevel", false);
	const int MinRandomLevel = Obj.value("MinRandomLevel", 0);
	const int MaxRandomLevel = Obj.value("MaxRandomLevel", 0);

	int FinalLevel = Level;
	if (UseRandomLevel)
	{
		FinalLevel = RNG->Get<int>(MinRandomLevel, MaxRandomLevel);
	}

	const FString FBlueprint(SBlueprint);

	APrimalDinoCharacter* Dino = AsaApi::GetApiUtils().SpawnDino(PC, FBlueprint, nullptr, FinalLevel, true, Neutered);

	if (!Dino)
	{
		return;
	}

	if (Dino->bUsesGender()())
	{
		if (Gender == "Male")
		{
			Dino->bIsFemale() = false;
		}
		else if (Gender == "Female")
		{
			Dino->bIsFemale() = true;
		}
	}

	UPrimalItem* DinoSaddle = nullptr;

	if (!SaddleBlueprint.empty() && Dino->MyInventoryComponentField())
	{
		FString FSaddleBlueprint(SaddleBlueprint);
		UClass* SaddleClass = UVictoryCore::BPLoadClass(FSaddleBlueprint);
		DinoSaddle = UPrimalItem::AddNewItem(SaddleClass, Dino->MyInventoryComponentField(), true, false, 0, false, 0, false, 0, false, nullptr, 0, false, false, true, true, true);
	}

	if (GiveInCryoPod)
	{
		TSubclassOf<UPrimalItem> CryoPodClass = UVictoryCore::BPLoadClass(FString(CryoPodBlueprint));
		UPrimalItem* CryoPod = UPrimalItem::AddNewItem(CryoPodClass, nullptr, false, false, 0, false, 0, false, 0, false, nullptr, 0, false, false, true, false, false);

		if (!CryoPod)
		{
			return;
		}

		const bool UseCryoPodCustomLimitTime = Obj.value("UseCryoPodCustomTimeLimit", false);
		const int CryoPodTimeLimitMinutes = Obj.value("CryoPodCustomTimeLimitInMinutes", 60);

		if (UseCryoPodCustomLimitTime)
		{
			CryoPod->AddItemDurability((CryoPod->ItemDurabilityField() - (CryoPodTimeLimitMinutes * 60)) * -1, false);
		}

		FCustomItemData CustomItemData = GetCryoPodData(Dino, DinoSaddle);
		CryoPod->SetCustomItemData(&CustomItemData);
		CryoPod->UpdatedItem(true, false);

		if (!PC->GetPlayerInventoryComponent())
		{
			return;
		}

		UPrimalItem* UpdatedCryoPod = PC->GetPlayerInventoryComponent()->AddItemObject(CryoPod);

		if (!UpdatedCryoPod)
		{
			return;
		}

		Dino->Destroy(true, false);
	}
}

void Rewards::ProcessItems(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& it : Section.value("Items", nlohmann::json::array()))
	{
		this->RewardItem(PC, it);
	}
}

void Rewards::ProcessEngrams(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& it : Section.value("Engrams", nlohmann::json::array()))
	{
		this->RewardEngram(PC, it);
	}
}

void Rewards::ProcessCommands(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Section) const
{
	for (const auto& it : Section.value("Commands", nlohmann::json::array()))
	{
		this->RewardCommand(PC, eos_id, it);
	}
}

void Rewards::ProcessDinos(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& it : Section.value("Dinos", nlohmann::json::array()))
	{
		this->RewardDino(PC, it);
	}
}

std::vector<nlohmann::json> Rewards::GetRandomRewardIndexes(const nlohmann::json& Section, const std::string& MainSectionKey, const std::string& SubSectionKey) const
{
	std::vector<nlohmann::json> RewardsVec;

	const nlohmann::json& RandConf = Section.value(MainSectionKey, nlohmann::json::object());

	if (RandConf.empty())
	{
		return RewardsVec;
	}

	int NumToGive = RandConf.value("NumToGive", 0);
	const bool AllowDuplicates = RandConf.value("AllowDuplicates", true);
	const nlohmann::json& RewardsArr = RandConf.value(SubSectionKey, nlohmann::json::array());

	if (RewardsArr.empty())
	{
		return RewardsVec;
	}

	const int ArrayMax = RewardsArr.size() - 1;

	if (ArrayMax == 0)
	{
		RewardsVec.push_back(RewardsArr[0]);
		return RewardsVec;
	}
	else if (ArrayMax == -1)
	{
		return RewardsVec;
	}

	if (AllowDuplicates)
	{
		for (int i = 0; i < NumToGive; i++)
		{
			RewardsVec.push_back(RewardsArr[RNG->Get<int>(0, ArrayMax)]);
		}
	}
	else
	{
		if (NumToGive > ArrayMax + 1)
		{
			NumToGive = ArrayMax + 1;
		}

		std::vector<int> IndexesVec;

		for (int i = 0; i <= ArrayMax; i++)
		{
			IndexesVec.push_back(i);
		}

		RNG->ShuffleVector(IndexesVec);

		for (int i = 0; i < NumToGive; i++)
		{
			RewardsVec.push_back(RewardsArr[IndexesVec[i]]);
		}
	}

	return RewardsVec;
}

void Rewards::ProcessRandomItems(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& reward : this->GetRandomRewardIndexes(Section, "RandomItems", "Items"))
	{
		this->RewardItem(PC, reward);
	}
}

void Rewards::ProcessRandomEngrams(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& reward : this->GetRandomRewardIndexes(Section, "RandomEngrams", "Engrams"))
	{
		this->RewardEngram(PC, reward);
	}
}

void Rewards::ProcessRandomCommands(AShooterPlayerController* PC, const FString& eos_id, const nlohmann::json& Section) const
{
	for (const auto& reward : this->GetRandomRewardIndexes(Section, "RandomCommands", "Commands"))
	{
		this->RewardCommand(PC, eos_id, reward);
	}
}

void Rewards::ProcessRandomDinos(AShooterPlayerController* PC, const nlohmann::json& Section) const
{
	for (const auto& reward : this->GetRandomRewardIndexes(Section, "RandomDinos", "Dinos"))
	{
		this->RewardDino(PC, reward);
	}
}