// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DetailPanelEditor : ModuleRules
{
	public DetailPanelEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" , "DetailPanel", "EditorStyle" , "PropertyEditor" , "UnrealEd"});
	}
}
