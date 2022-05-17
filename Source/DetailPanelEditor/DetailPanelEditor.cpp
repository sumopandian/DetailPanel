// Fill out your copyright notice in the Description page of Project Settings.

#include "DetailPanelEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "PropertyEditorDelegates.h"
#include "MineSweeperOnDetails.h"
#include "MineSweeperActor.h"


IMPLEMENT_MODULE(FDetailPanelEditorModule, DetailPanelEditor );

DEFINE_LOG_CATEGORY(DetailPanelEditor)


void FDetailPanelEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	PropertyEditorModule.RegisterCustomClassLayout(AMineSweeperActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&MineSweeperOnDetails::MakeInstance));
	UE_LOG(DetailPanelEditor, Log, TEXT("DetailPanelEditor : StartupModule"));
}

void FDetailPanelEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	PropertyEditorModule.UnregisterCustomClassLayout(AMineSweeperActor::StaticClass()->GetFName());
	UE_LOG(DetailPanelEditor, Log, TEXT("DetailPanelEditor : ShutdownModule"));
}
