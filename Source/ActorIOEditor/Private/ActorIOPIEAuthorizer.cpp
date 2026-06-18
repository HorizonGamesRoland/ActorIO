// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOPIEAuthorizer.h"
#include "ActorIOSettings.h"
#include "ActorIOSubsystemBase.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

#if UE_VERSION_NEWER_THAN(5, 7, ENGINE_PATCH_VERSION)
TValueOrError<bool, FText> FActorIOPIEAuthorizer::IsPIEAuthorizedInternal(bool bIsSimulateInEditor) const
{
	return MakeValue(true);
}

TValueOrError<bool, FText> FActorIOPIEAuthorizer::RequestPIEPermissionInternal(bool bIsSimulateInEditor) const
{
	const UActorIOSettings* IOSettings = UActorIOSettings::Get();
	if (IOSettings->ActorIOSubsystemClass == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("PIEAuthorizerError.IOSubsystemInvalid", "Actor I/O subsystem class is invalid in 'Project Settings -> Actor I/O'.\nPlease make sure that a class is selected."),
			LOCTEXT("PIEAuthorizerError.Title", "Play-In-Editor Aborted"));

		return MakeError(INVTEXT("Actor I/O plugin is configured incorrectly."));
	}

	return MakeValue(true);
}
#else
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
#endif

#undef LOCTEXT_NAMESPACE
