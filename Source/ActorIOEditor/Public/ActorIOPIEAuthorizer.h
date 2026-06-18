// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "IPIEAuthorizer.h"
#include "Misc/EngineVersionComparison.h"

/**
 * Custom Play-In-Editor authorizer to abort PIE sessions if the plugin is configured incorrectly.
 */
class ACTORIOEDITOR_API FActorIOPIEAuthorizer : public IPIEAuthorizer
{
public:

	//~ Begin IPIEAuthorizer Interface
#if UE_VERSION_NEWER_THAN(5, 7, ENGINE_PATCH_VERSION)
	virtual TValueOrError<bool, FText> IsPIEAuthorizedInternal(bool bIsSimulateInEditor) const override;
	virtual TValueOrError<bool, FText> RequestPIEPermissionInternal(bool bIsSimulateInEditor) const override;
#else
	virtual bool RequestPIEPermission(bool bIsSimulateInEditor, FString& OutReason) const override;
#endif
	//~ End IPIEAuthorizer Interface
};
