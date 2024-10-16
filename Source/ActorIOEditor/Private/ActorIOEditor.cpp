// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ISettingsModule.h"
#include "Developer/MessageLog/Public/MessageLogInitializationOptions.h"
#include "Developer/MessageLog/Public/MessageLogModule.h"
#include "Developer/MessageLog/Public/IMessageLogListing.h"

#define LOCTEXT_NAMESPACE "FActorIOEditorModule"

void FActorIOEditorModule::StartupModule()
{
	// Initialize the editor style of the plugin.
	FActorIOEditorStyle::Initialize();

	// Register various engine extensions.
	RegisterSettings();
	RegisterMessageLog();
}

void FActorIOEditorModule::ShutdownModule()
{
	// Unregister various engine extensions.
	UnRegisterSettings();
	UnRegisterPlacementCategory();
	UnRegisterMessageLog();

	// Unregister the editor style of the plugin.
	FActorIOEditorStyle::Shutdown();
}

void FActorIOEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		const FText DisplayName = LOCTEXT("ActorIOConfigName", "ActorIO Matchmaking");
		const FText Description = LOCTEXT("ActorIOConfigDescription", "Settings for the ActorIO Matchmaking plugin.");
		const TWeakObjectPtr<UObject> SettingsObject = GetMutableDefault<UActorIOConfig>(UActorIOConfig::StaticClass());

		SettingsModule->RegisterSettings("Project", "Game", "ActorIOConfig", DisplayName, Description, SettingsObject);
	}
}

void FActorIOEditorModule::RegisterMessageLog()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	MessageLogModule.RegisterLogListing("ActorIO", LOCTEXT("LogActorIO", "ActorIO Matchmaking"), InitOptions);
}

void FActorIOEditorModule::UnRegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Projects", "Plugins", "ActorIOConfig");
	}
}

void FActorIOEditorModule::UnRegisterMessageLog()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing("ActorIO");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditorModule, ActorIOEditor)