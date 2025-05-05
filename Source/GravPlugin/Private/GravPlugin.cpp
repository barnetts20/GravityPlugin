// Copyright Epic Games, Inc. All Rights Reserved.

#include "GravPlugin.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "GravPluginLibrary/ExampleLibrary.h"

#define LOCTEXT_NAMESPACE "FGravPluginModule"

void FGravPluginModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("GravPlugin")->GetBaseDir();
}

void FGravPluginModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGravPluginModule, GravPlugin)
