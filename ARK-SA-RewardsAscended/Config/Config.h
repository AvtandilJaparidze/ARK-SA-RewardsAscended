#pragma once
#include <json.hpp>
#include <fstream>
#include <API/ARK/Ark.h>

inline nlohmann::json config;

inline void ReadConfig()
{
	const std::string PluginDir = AsaApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/" + PROJECT_NAME;

	const std::string config_path = PluginDir + "/config.json";
	std::ifstream ConfigFile{ config_path };
	if (!ConfigFile.is_open())
		throw std::runtime_error("Can't open config.json");

	ConfigFile >> config;

	ConfigFile.close();

	if (config.value("Config", nlohmann::json::object())
		.value("UseOverride", false))
	{
		std::ifstream ExternalConfigFile{ config.value("Config", nlohmann::json::object())
			.value("OverridePath", std::string()) };

		if (!ExternalConfigFile.is_open())
			throw std::runtime_error("Can't open External config.json");

		config.clear();

		ExternalConfigFile >> config;
		ExternalConfigFile.close();
	}
}