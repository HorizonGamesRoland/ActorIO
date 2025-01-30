// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPIEAuthorizer.h"

/**
 * Custom Play-In-Editor authorizer to abort PIE sessions if the plugin is configured incorrectly.
 */
class ACTORIOEDITOR_API FActorIOPIEAuthorizer : public IPIEAuthorizer
{
public:

	//~ Begin IPIEAuthorizer Interface
	virtual bool RequestPIEPermission(bool bIsSimulateInEditor, FString& OutReason) const override;
	//~ End IPIEAuthorizer Interface
};
