// Copyright Lim Young.


#include "ActorIORegisterBase.h"

#include "ActorIOLibrary.h"

void UActorIORegisterBase::RegisterIOEvents_Implementation(AActor* RegisteredActor, FActorIOEventList& EventRegistry)
{
}

void UActorIORegisterBase::RegisterIOFunctions_Implementation(AActor* RegisteredActor,
                                                              FActorIOFunctionList& FunctionRegistry)
{
}

TSubclassOf<AActor> UActorIORegisterBase::GetActorsClassRequiredToRegister_Implementation() const
{
	return AActor::StaticClass();
}

void UActorIORegisterBase::BP_RegisterIOEventForRequiredActor(AActor* RegisteredActor, FActorIOEventList& Registry,
                                                              FName EventId, const FText& DisplayNameText,
                                                              const FText& TooltipText, FName EventDispatcherName,
                                                              FName EventProcessorName)
{
	UActorIOLibrary::K2_RegisterIOEvent(RegisteredActor, Registry, EventId, DisplayNameText, TooltipText,
	                                    EventDispatcherName, EventProcessorName);
}

void UActorIORegisterBase::BP_RegisterIOFunctionForRequiredActor(AActor* RegisteredActor,
                                                                 FActorIOFunctionList& Registry, FName FunctionId,
                                                                 const FText& DisplayNameText, const FText& TooltipText,
                                                                 FString FunctionToExec, FName SubobjectName)
{
	UActorIOLibrary::K2_RegisterIOFunction(RegisteredActor, Registry, FunctionId, DisplayNameText, TooltipText,
	                                       FunctionToExec, SubobjectName);
}
