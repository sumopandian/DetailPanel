#include "MineSweeperActor.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMineSweeperActor::AMineSweeperActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Since the board is random, Initialize will not be consistent in multiple calls.
	//That has a tendency to cause issues with the copy properties functions for objects.
	//Hence we try to initialize only for the CDO and copy for everything else.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		Initialize();
		CheckAndGenerateBoard();
	}
	else
	{
		if (AMineSweeperActor* Other = Cast<AMineSweeperActor>(ObjectInitializer.GetArchetype()))
		{
			MineCount = Other->MineCount;
			bBoardGenerated = Other->bBoardGenerated;
			bGameOver = Other->bGameOver;
			bHasWon = Other->bHasWon;
			HitMineIndex = Other->HitMineIndex;
			FieldArray = Other->FieldArray;
			RevealedArray = Other->RevealedArray;
			FlagedIndices = Other->FlagedIndices;
			Visited = Other->Visited;
		}
	}
}

void AMineSweeperActor::PostInitProperties()
{
	Super::PostInitProperties();
}

void AMineSweeperActor::Initialize()
{
	bGameOver = false;
	bHasWon = false;
	HitMineIndex = -1;
	FieldArray.Empty();
	RevealedArray.Empty();
	FlagedIndices.Empty();
	Visited.Empty();
	bBoardGenerated = false;
	MineCount = 0;
}

bool AMineSweeperActor::CanClickOnField(int32 ColIndex, int32 RowIndex) const
{
	if (bGameOver) return false;

	const int32 Index = CalcIndex(ColIndex, RowIndex);

	//Don't handle left click on a flaged tile
	if (FlagedIndices.Contains(Index))
	{
		return false;
	}

	return true;
}

void AMineSweeperActor::HandleClickOnField(int32 ColIndex, int32 RowIndex)
{
	if (!CanClickOnField(ColIndex,RowIndex)) return;

	const int32 Index = CalcIndex(ColIndex, RowIndex);

	if (FieldArray[Index])
	{
		HandleGameOverNative(Index);
	}
	else
	{
		Visited.Empty();
		RevealFieldNative(ColIndex, RowIndex);
	}
}


bool AMineSweeperActor::CanRightClickOnField(int32 ColIndex, int32 RowIndex) const
{
	if (bGameOver) return false;

	return true;
}

void AMineSweeperActor::HandleRightClickOnField(int32 ColIndex, int32 RowIndex)
{
	if (!CanRightClickOnField(ColIndex, RowIndex)) return;

	const int32 Index = CalcIndex(ColIndex, RowIndex);

	if (FlagedIndices.Contains(Index))
	{
		FlagedIndices.Remove(Index);
	}
	else if (FlagedIndices.Num() < MineCount)
	{
		FlagedIndices.Add(Index);
	}
}

int32 AMineSweeperActor::CalculateFieldNumber(int32 ColIndex, int32 RowIndex) const
{
	const int32 Index = CalcIndex(ColIndex, RowIndex);
	if (FieldArray[Index])
	{
		return -1;
	}

	return GetValue(ColIndex - 1, RowIndex - 1) +
		GetValue(ColIndex - 1, RowIndex) +
		GetValue(ColIndex - 1, RowIndex + 1) +
		GetValue(ColIndex, RowIndex - 1) +
		GetValue(ColIndex, RowIndex + 1) +
		GetValue(ColIndex + 1, RowIndex - 1) +
		GetValue(ColIndex + 1, RowIndex) +
		GetValue(ColIndex + 1, RowIndex + 1);
}

bool AMineSweeperActor::IsRevealed(int32 ColIndex, int32 RowIndex) const
{
	const int32 Index = CalcIndex(ColIndex, RowIndex);
	return RevealedArray[Index];
}

bool AMineSweeperActor::IsFlagged(int32 ColIndex, int32 RowIndex) const
{
	const int32 Index = CalcIndex(ColIndex, RowIndex);
	return FlagedIndices.Contains(Index);
}

bool AMineSweeperActor::IsCrossed(int32 ColIndex, int32 RowIndex) const
{
	const int32 Index = CalcIndex(ColIndex, RowIndex);
	return FlagedIndices.Contains(Index)
			&& !FieldArray[Index]
			|| Index == HitMineIndex;
}

bool AMineSweeperActor::IsMine(int32 ColIndex, int32 RowIndex) const
{
	const int32 Index = CalcIndex(ColIndex, RowIndex);
	return FieldArray[Index];
}

bool AMineSweeperActor::CheckAndGenerateBoard()
{
	if (!bBoardGenerated)
	{
		GenerateBoard();
		return true;
	}
	return false;
}

void AMineSweeperActor::ResetBoard()
{
	Initialize();
	CheckAndGenerateBoard();
}

void AMineSweeperActor::HandleGameOverNative(int32 ClickedIndex)
{
	bGameOver = true;
	HitMineIndex = ClickedIndex;
	
	for (int32 i = 0; i < RevealedArray.Num(); i++)
	{
		RevealedArray[i] = true;
	}
}

void AMineSweeperActor::RevealFieldNative(int32 ColIndex, int32 RowIndex)
{
	if (!IsValidIndex(ColIndex, RowIndex))
	{
		return;
	}

	const int32 Index = CalcIndex(ColIndex, RowIndex);

	//Don't revisit already visited fields
	if (Visited.Contains(Index))
	{
		return;
	}

	Visited.Add(Index);

	if (FlagedIndices.Contains(Index))
	{
		return;
	}

	RevealedArray[Index] = true;

	const int32 Value = CalculateFieldNumber(ColIndex, RowIndex);

	//Reveal adjacent fields 
	if (Value == 0)
	{
		RevealFieldNative(ColIndex - 1, RowIndex - 1);
		RevealFieldNative(ColIndex - 1, RowIndex);
		RevealFieldNative(ColIndex - 1, RowIndex + 1);
		RevealFieldNative(ColIndex, RowIndex - 1);
		RevealFieldNative(ColIndex, RowIndex + 1);
		RevealFieldNative(ColIndex + 1, RowIndex - 1);
		RevealFieldNative(ColIndex + 1, RowIndex);
		RevealFieldNative(ColIndex + 1, RowIndex + 1);
	}
}

bool AMineSweeperActor::IsValidIndex(int32 ColIndex, int32 RowIndex) const
{
	if (ColIndex < 0 || ColIndex >= ColumnNum || RowIndex < 0 || RowIndex >= RowNum)
	{
		return false;
	}
	return true;
}

void AMineSweeperActor::GenerateBoard()
{
	MineCount = 0;

	FieldArray.Empty();
	Visited.Empty();
	FlagedIndices.Empty();
	RevealedArray.Empty();

	const int32 TotalFields = ColumnNum * RowNum;
	FieldArray.SetNum(TotalFields);
	RevealedArray.SetNum(TotalFields);
	Visited.Reserve(TotalFields / 2);

	for (int32 i = 0; i < FieldArray.Num(); ++i)
	{
		const float RandomValue = UKismetMathLibrary::RandomFloat();
		FieldArray[i] = RandomValue < MineChance;
		if (FieldArray[i])
		{
			MineCount++;
		}
		RevealedArray[i] = false;
	}

	bBoardGenerated = true;
}

int32 AMineSweeperActor::CalcIndex(int32 ColIndex, int32 RowIndex) const
{
	return RowIndex * ColumnNum + ColIndex;
}

int32 AMineSweeperActor::GetValue(int32 ColIndex, int32 RowIndex) const
{
	if (!IsValidIndex(ColIndex, RowIndex))
	{
		return 0;
	}

	return FieldArray[CalcIndex(ColIndex, RowIndex)];
}

int32 AMineSweeperActor::GetMineCountForVisual() const
{
	return MineCount - FlagedIndices.Num();
}

bool AMineSweeperActor::CheckAndUpdateHasWon()
{
	if (!bGameOver && FlagedIndices.Num() == MineCount)
	{
		int count = 0;

		for (auto& r : RevealedArray)
		{
			if (!r)
				count++;
		}

		if (count == MineCount)
		{
			bool flag = true;

			for (auto& index : FlagedIndices)
			{
				flag &= FieldArray[index];
			}
			if (flag)
			{
				bGameOver = true;
				bHasWon = true;
			}
			return flag;
		}
	}
	return bHasWon;
}