// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "ActorIOPIEAuthorizer.h"
#include "ActorIOSettings.h"
#include "ActorIOSubsystemBase.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

bool FActorIOPIEAuthorizer::RequestPIEPermission(bool bIsSimulateInEditor, FString& OutReason) const
{
	const UActorIOSettings* IOSettings = UActorIOSettings::Get();
	if (IOSettings->ActorIOSubsystemClass == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("PIEAuthorizerError.IOSubsystemInvalid", "Actor I/O subsystem class is invalid in 'Project Settings -> Actor I/O'.\nPlease make sure that a class is selected."),
			LOCTEXT("PIEAuthorizerError.Title", "Play-In-Editor Aborted"));

		OutReason = TEXT("Actor I/O plugin is configured incorrectly.");
		return false;
	}
	
	return true;
}

#undef LOCTEXT_NAMESPACE
