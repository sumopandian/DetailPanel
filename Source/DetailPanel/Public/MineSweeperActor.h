#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MineSweeperActor.generated.h"

UCLASS()
class DETAILPANEL_API AMineSweeperActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMineSweeperActor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitProperties() override;

public:

	UFUNCTION()
	bool CanClickOnField(int32 ColIndex, int32 RowIndex) const;

	// Call this when when field at certain col and row is clicked from the UI
	UFUNCTION()
	void HandleClickOnField(int32 ColIndex, int32 RowIndex);

	UFUNCTION()
	bool CanRightClickOnField(int32 ColIndex, int32 RowIndex) const;

	// Call this when field at certain col and row is right clicked from the UI
	UFUNCTION()
	void HandleRightClickOnField(int32 ColIndex, int32 RowIndex);

	//Reset the whole board to new values
	UFUNCTION()
	void ResetBoard();
	
	//Generates board if it is not already generated
	UFUNCTION()
	bool IsBoardGenerated() const { return bBoardGenerated; }

	//Returns the number of columns in the game
	UFUNCTION()
	int32 GetNumColumns() const { return ColumnNum; }

	//Returns the number of rows in the game
	UFUNCTION()
	int32 GetNumRows() const { return RowNum; }

	// for a row and col return the number of mines around it
	UFUNCTION()
	int32 CalculateFieldNumber(int32 ColIndex, int32 RowIndex) const;

	//returns true if a field is revealed
	UFUNCTION()
	bool IsRevealed(int32 ColIndex, int32 RowIndex) const;

	//returns true if a field is flagged
	UFUNCTION()
	bool IsFlagged(int32 ColIndex, int32 RowIndex) const;

	//returns true if a field is crossed
	UFUNCTION()
	bool IsCrossed(int32 ColIndex, int32 RowIndex) const;

	//returns true if a field has a mine
	UFUNCTION()
	bool IsMine(int32 ColIndex, int32 RowIndex) const;

	//returns true if the current game is overs
	UFUNCTION()
	bool IsGameOver() const { return bGameOver; }	

	//returns the value of mines assumed to be remaining based on set flags
	UFUNCTION()
	int32 GetMineCountForVisual() const;

	//returns the updated haswon variable
	UFUNCTION()
	bool CheckAndUpdateHasWon();

protected:

	UFUNCTION()
	bool CheckAndGenerateBoard();

	UFUNCTION()
	int32 CalcIndex(int32 ColIndex, int32 RowIndex) const;

	UFUNCTION()
	void Initialize();

	UFUNCTION()
	void HandleGameOverNative(int32 ClickedIndex);

	UFUNCTION()
	void RevealFieldNative(int32 ColIndex, int32 RowIndex);

	UFUNCTION()
	bool IsValidIndex(int32 ColIndex, int32 RowIndex) const;

	UFUNCTION()
	void GenerateBoard();

	UFUNCTION()
	int32 GetValue(int32 ColIndex, int32 RowIndex) const;

protected:

	
	UPROPERTY(EditAnywhere)
	int32 ColumnNum = 12;

	UPROPERTY(EditAnywhere)
	int32 RowNum = 12;

	UPROPERTY(EditAnywhere)
	float MineChance = 0.1;

	UPROPERTY()
	int32 MineCount;

	UPROPERTY()
	uint32 bBoardGenerated : 1;

	UPROPERTY()
	uint32 bGameOver : 1;

	UPROPERTY()
	uint32 bHasWon : 1;

	UPROPERTY()
	int32 HitMineIndex;

	UPROPERTY()
	TArray<bool> FieldArray;

	UPROPERTY()
	TArray<bool> RevealedArray;

	UPROPERTY()
	TSet<int32> FlagedIndices;

	UPROPERTY()
	TSet<int32> Visited;

};
