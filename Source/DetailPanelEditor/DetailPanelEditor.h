#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"

DECLARE_LOG_CATEGORY_EXTERN(DetailPanelEditor, All, All)

class FDetailPanelEditorModule final : public IModuleInterface
{
public:
	void StartupModule() final;
	void ShutdownModule() final;
};
