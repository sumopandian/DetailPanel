// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
struct FSlateImageBrush;

class MineSweeperOnDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual ~MineSweeperOnDetails();

protected:
	MineSweeperOnDetails();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;

	bool GetIsEnabledButton(int32 X, int32 Y) const;

	FReply OnClicked(int32 X, int32 Y);

	FReply OnRightClicked(int32 X, int32 Y);

	TWeakObjectPtr<class AMineSweeperActor> MineActor;
	TWeakPtr<class IDetailLayoutBuilder> CacheDetailBuilder;
};