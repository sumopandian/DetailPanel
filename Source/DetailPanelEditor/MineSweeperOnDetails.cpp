#include "MineSweeperOnDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/Images/SImage.h"
#include "Runtime/SlateCore/Public/Brushes/SlateImageBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SCanvas.h"
#include "DetailPanel/Public/MineSweeperActor.h"
#include "IDetailsView.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "PropertyCustomizationHelpers.h"


//The custom transaction object to help with modifying and setting the appropriate flags when editing the object
class FMineSweeperTransactionScope
{
public:
	FMineSweeperTransactionScope(FText TransactionName, UObject* InUObject)
	{
		check(InUObject);
		Object = InUObject;
		Transaction = new FScopedTransaction(TEXT("MineSweeper"), TransactionName, Object);
		//Sometimes this is not set based on scenarios. But we need this set to register transactions on the object.
		//Eg: For objects in levels this flag is usually set and for BP it is not
		if (!Object->HasAnyFlags(RF_Transactional))
		{
			bDidWeSetFlag = true;
			Object->SetFlags(RF_Transactional);
		}
		Object->Modify();
	}

	~FMineSweeperTransactionScope()
	{
		if (bDidWeSetFlag)
		{
			Object->ClearFlags(RF_Transactional);
		}
		delete Transaction;
	}
private:
	//The actual scoped transaction object
	FScopedTransaction *Transaction;
	bool bDidWeSetFlag;
	UObject* Object;
};

//The slate style class for our panel modifications. Takes care of loading and save the brushes and fonts we want to use.
class FSlateMinesStyle final : public FSlateStyleSet
{
public:
	FSlateMinesStyle()
		: FSlateStyleSet("SlateMinesStyle")
	{
		FVector2D Image48x48(48.0f, 48.0f);

		Set("Mine.Mine", new FSlateVectorImageBrush(GetSVGPath("mine"), Image48x48, FLinearColor::Black));
		Set("Mine.Cross", new FSlateVectorImageBrush(GetSVGPath("mine_cross"), Image48x48, FLinearColor::Red));
		Set("Mine.Flag", new FSlateVectorImageBrush(GetSVGPath("mine_flag"), Image48x48, FLinearColor::Red));
		Set("Mine.Smiley", new FSlateVectorImageBrush(GetSVGPath("smiley"), Image48x48));
		Set("Mine.SmileyWin", new FSlateVectorImageBrush(GetSVGPath("smileycool"), Image48x48));
		Set("MineFont.Calc", FSlateFontInfo(GetOTFPath("Segment7-4Gml"),20));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	FString GetSVGPath(const FString& RelativePath)
	{
		return GetResourcePath(RelativePath, ".svg");
	}

	FString GetOTFPath(const FString& RelativePath)
	{
		return GetResourcePath(RelativePath, ".otf");
	}

	FString GetResourcePath(const FString& RelativePath, const FString& pathExt)
	{
		static FString ResourcePath = FPaths::ProjectDir() / TEXT("Source/Resource/");
		return (ResourcePath / RelativePath) + pathExt;
	}

	static FSlateMinesStyle& Get()
	{
		static FSlateMinesStyle Inst;
		return Inst;
	}

	~FSlateMinesStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};

//A modified image slate class that absorbs all mouse up events when hittable
class SImageSink
	: public SImage
{
public:
	SLATE_BEGIN_ARGS(SImageSink)
		: _Image(FCoreStyle::Get().GetDefaultBrush())
		, _ColorAndOpacity(FLinearColor::White)
	{ }

	/** Image resource */
	SLATE_ATTRIBUTE(const FSlateBrush*, Image)

		/** Color and opacity */
	SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)

		/** When specified, ignore the brushes size and report the DesiredSizeOverride as the desired image size. */
	SLATE_ATTRIBUTE(TOptional<FVector2D>, DesiredSizeOverride)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SImage::Construct(SImage::FArguments()
			.Image(InArgs._Image)
			.ColorAndOpacity(InArgs._ColorAndOpacity)
			.DesiredSizeOverride(InArgs._DesiredSizeOverride)
		);
	}

public:
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return FReply::Handled();
	}
};

//A modified Slate Button class with all the arguments as a normal button but an extra event for right click
class SRightClickableButton
	: public SButton
{
	SLATE_BEGIN_ARGS(SRightClickableButton)
		: _Content()
		, _ButtonStyle(&FCoreStyle::Get().GetWidgetStyle< FButtonStyle >("Button"))
		, _TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText"))
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _ContentPadding(FMargin(4.0, 2.0))
		, _Text()
		, _ClickMethod(EButtonClickMethod::DownAndUp)
		, _TouchMethod(EButtonTouchMethod::DownAndUp)
		, _PressMethod(EButtonPressMethod::DownAndUp)
		, _DesiredSizeScale(FVector2D(1, 1))
		, _ContentScale(FVector2D(1, 1))
		, _ButtonColorAndOpacity(FLinearColor::White)
		, _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
		, _IsFocusable(true)
	{
	}

	/** Slot for this button's content (optional) */
	SLATE_DEFAULT_SLOT(FArguments, Content)

		/** The visual style of the button */
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)

		/** The text style of the button */
		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

		/** Horizontal alignment */
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)

		/** Vertical alignment */
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)

		/** Spacing between button's border and the content. */
		SLATE_ATTRIBUTE(FMargin, ContentPadding)

		/** The text to display in this button, if no custom content is specified */
		SLATE_ATTRIBUTE(FText, Text)

		/** Called when the button is clicked */
		SLATE_EVENT(FOnClicked, OnClicked)

		/** Called when the button is pressed */
		SLATE_EVENT(FSimpleDelegate, OnPressed)

		/** Called when the button is released */
		SLATE_EVENT(FSimpleDelegate, OnReleased)

		SLATE_EVENT(FSimpleDelegate, OnHovered)

		SLATE_EVENT(FSimpleDelegate, OnUnhovered)

		/** Sets the rules to use for determining whether the button was clicked.  This is an advanced setting and generally should be left as the default. */
		SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)

		/** How should the button be clicked with touch events? */
		SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)

		/** How should the button be clicked with keyboard/controller button events? */
		SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)

		SLATE_ATTRIBUTE(FVector2D, DesiredSizeScale)

		SLATE_ATTRIBUTE(FVector2D, ContentScale)

		SLATE_ATTRIBUTE(FSlateColor, ButtonColorAndOpacity)

		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)

		/** Sometimes a button should only be mouse-clickable and never keyboard focusable. */
		SLATE_ARGUMENT(bool, IsFocusable)

		/** The sound to play when the button is pressed */
		SLATE_ARGUMENT(TOptional<FSlateSound>, PressedSoundOverride)

		/** The sound to play when the button is hovered */
		SLATE_ARGUMENT(TOptional<FSlateSound>, HoveredSoundOverride)

		/** Which text shaping method should we use? (unset to use the default returned by GetDefaultTextShapingMethod) */
		SLATE_ARGUMENT(TOptional<ETextShapingMethod>, TextShapingMethod)

		/** Which text flow direction should we use? (unset to use the default returned by GetDefaultTextFlowDirection) */
		SLATE_ARGUMENT(TOptional<ETextFlowDirection>, TextFlowDirection)

		/** Event raised for when we do a right click on the  obejct. Only works on PC*/
		SLATE_EVENT(FOnClicked, OnRightClicked)

		SLATE_END_ARGS()

		SRightClickableButton() {}

	void SetOnRightClicked(FOnClicked InOnRightClicked)
	{
		OnRightClicked = InOnRightClicked;
	}

	void Construct(const FArguments& InArgs)
	{
		//Mirroring and sending the same arguments to the SButton's constructor
		SButton::Construct(SButton::FArguments()
			.ButtonStyle(InArgs._ButtonStyle)
			.TextStyle(InArgs._TextStyle)
			.HAlign(InArgs._HAlign)
			.VAlign(InArgs._VAlign)
			.ContentPadding(InArgs._ContentPadding)
			.HAlign(InArgs._HAlign)
			.Text(InArgs._Text)
			.OnClicked(InArgs._OnClicked)
			.OnPressed(InArgs._OnPressed)
			.OnReleased(InArgs._OnReleased)
			.OnHovered(InArgs._OnHovered)
			.OnUnhovered(InArgs._OnUnhovered)
			.ClickMethod(InArgs._ClickMethod)
			.TouchMethod(InArgs._TouchMethod)
			.PressMethod(InArgs._PressMethod)
			.DesiredSizeScale(InArgs._DesiredSizeScale)
			.ContentScale(InArgs._ContentScale)
			.ButtonColorAndOpacity(InArgs._ButtonColorAndOpacity)
			.ForegroundColor(InArgs._ForegroundColor)
			.IsFocusable(InArgs._IsFocusable)
			.PressedSoundOverride(InArgs._PressedSoundOverride)
			.HoveredSoundOverride(InArgs._HoveredSoundOverride)
			.TextShapingMethod(InArgs._TextShapingMethod)
			.TextFlowDirection(InArgs._TextFlowDirection)
		);

		SetButtonStyle(InArgs._ButtonStyle);

		//We are probably not inheriting this anywhere else
		SetCanTick(false);

		OnRightClicked = InArgs._OnRightClicked;
	}

public:
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FReply Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);;

		//We are not supporting all the click methods here. We are simply triggering the event OnRelease
		if (((MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)))
		{
			Release();

			if (IsEnabled())
			{
				if (OnRightClicked.IsBound())
				{
					OnRightClicked.Execute();
				}
				return FReply::Handled();
			}
		}
		return Reply;
	}

protected:

	FOnClicked OnRightClicked;
};

MineSweeperOnDetails::MineSweeperOnDetails()
{
}

MineSweeperOnDetails::~MineSweeperOnDetails()
{
}

TSharedRef<IDetailCustomization> MineSweeperOnDetails::MakeInstance()
{
	return MakeShareable(new MineSweeperOnDetails);
}

void MineSweeperOnDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	CacheDetailBuilder = DetailBuilder;
	IDetailCustomization::CustomizeDetails(DetailBuilder);
}

void MineSweeperOnDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	const FString MineSweeperString = "MineSweeper";
	const FText MineSweeperText = FText::FromString(MineSweeperString);
	const FName MineSweeperName = FName(*MineSweeperString);
	

	//Hide the search/filter
	IDetailsView* DetailsView = DetailBuilder.GetDetailsView();
	if (DetailsView)
	{
		DetailsView->HideFilterArea(true);
	}

	//Hide all the other categories , except for config and our MineSweeper category
	TArray<FName> CategoryNames;
	DetailBuilder.GetCategoryNames(CategoryNames);

	for (auto CategoryName : CategoryNames)
	{
		if (CategoryName != MineSweeperName)
		{
			DetailBuilder.HideCategory(CategoryName);
		}
	}

	//Get the minsweeper category builder and make sure it is not collapsed by default
	IDetailCategoryBuilder& MineSweeper = DetailBuilder.EditCategory(MineSweeperName, MineSweeperText).InitiallyCollapsed(false);

	//Only allow for play if we select one actor . Don't allow for multi actor modifications.
	if (ObjectsBeingCustomized.Num() != 1)
	{
		MineSweeper.AddCustomRow(MineSweeperText)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Can't play with multiple selected"))
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
			];
	}
	else
	{
		//Save the actor we are modifying locally for use
		MineActor = Cast<AMineSweeperActor>(ObjectsBeingCustomized[0]);

		if (MineActor.IsValid())
		{
			//Draw the variables for configuration
			IDetailCategoryBuilder& Config = DetailBuilder.EditCategory("Config",FText::GetEmpty(),ECategoryPriority::Important).InitiallyCollapsed(true);

			Config.AddProperty(DetailBuilder.GetProperty("RowNum"));
			Config.AddProperty(DetailBuilder.GetProperty("ColumnNum"));
			Config.AddProperty(DetailBuilder.GetProperty("MineChance"));

			check(MineActor->IsBoardGenerated());

			TSharedPtr<SVerticalBox> VerticalBox;

			//Save the rows and columns locally for use
			const int32 RowNum = MineActor->GetNumRows();
			const int32 ColNum = MineActor->GetNumColumns();

			//Grid Size for the UI 
			const float GridSize = 30.0f;
			FVector2D GridSize2D(GridSize, GridSize);

			//Use the Detail panel font for the numbers
			FSlateFontInfo NumberFont = IDetailLayoutBuilder::GetDetailFontBold();
			NumberFont.Size = GridSize2D.X * 0.65f;

			//Use our custom monospace font for showing the mine number display
			FSlateFontInfo LabelFont = FSlateMinesStyle::Get().GetFontStyle("MineFont.Calc");
			LabelFont.Size = GridSize2D.X * 1.50f;



			MineSweeper.AddCustomRow(MineSweeperText)
				[
					//Start out with a Canvas panel
					SNew(SConstraintCanvas)

					//Anchor our minesweeper board on the top left. You can use the slots offset to position where we want if needed.
					+ SConstraintCanvas::Slot()
					.Anchors(FAnchors(0.0f))
					.Alignment(FVector2D(0.0f, 0.0f))
					.AutoSize(true)
					[
						SNew(SVerticalBox)
						//Add the top display and reset smiley button first
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Fill)
						.VAlign(EVerticalAlignment::VAlign_Top)
						[
							SNew(SBorder)
							.Padding(4)
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Left)
								.VAlign(EVerticalAlignment::VAlign_Center)
								[
									SNew(SBorder)
									.Padding(4)
									[
										SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SNew(STextBlock)
											.Text(FText::FromString("888"))
											.Font(LabelFont)
											.ColorAndOpacity(FLinearColor(1.0f,1.0f,1.0f,0.1f))
										]
										+ SOverlay::Slot()
										.HAlign(HAlign_Fill)
										[
											SNew(STextBlock)
											.Justification(ETextJustify::Right)
											.Text_Lambda(
												[this]()
												{
													if (MineActor.IsValid())
													{
														return FText::AsNumber(MineActor->GetMineCountForVisual());
													}
													return FText::FromString("");
												}
											)
											.Font(LabelFont)
											.ColorAndOpacity(FLinearColor::Red)
										]
									]
									
								]
								+ SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Center)
								.VAlign(EVerticalAlignment::VAlign_Center)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SButton)
										.OnClicked_Lambda
										(
											[this]() 
											{
												if (MineActor.IsValid() && CacheDetailBuilder.IsValid())
												{
													{
														const FMineSweeperTransactionScope Transaction(FText::FromString("Reset the Board"), MineActor.Get());
														MineActor->ResetBoard();
													}
													//Checks for shared pointer uniqueness in subsequent calls, will assert if we directly call the function from the sharedpointer
													IDetailLayoutBuilder* DetailBuilder = CacheDetailBuilder.Pin().Get();
													if (DetailBuilder)
													{
														DetailBuilder->ForceRefreshDetails();
													}

												}
												return  FReply::Handled();
											}
										)
									]
									+ SOverlay::Slot()
									[
										SNew(SImage)
										.DesiredSizeOverride(GridSize2D * 1.75f)
										.Image_Lambda(
											[this]()
											{
												if (MineActor.IsValid() && MineActor->CheckAndUpdateHasWon())
												{
													return FSlateMinesStyle::Get().GetBrush("Mine.SmileyWin");
												}
												return FSlateMinesStyle::Get().GetBrush("Mine.Smiley");
											}
										)
										.Visibility(EVisibility::HitTestInvisible)
									]
								]
								+ SHorizontalBox::Slot()
								.HAlign(EHorizontalAlignment::HAlign_Left)
								[
									SNew(STextBlock)
								]
							]
						]
						//The second vertical box slot where our grid would be
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						.VAlign(EVerticalAlignment::VAlign_Top)
						[
							SNew(SBorder)
							.Padding(4)
							[
								SAssignNew(VerticalBox,SVerticalBox)
							]
						]
					]
				];


		//Actually form the grid of buttons to click
			for (int32 x = 0; x < RowNum; x++)
			{
				auto HorizontalBox = SNew(SHorizontalBox);
				VerticalBox->AddSlot()[HorizontalBox];

				for (int32 y = 0; y < ColNum; y++)
				{
					HorizontalBox->AddSlot()
						.AutoWidth()
						[
							SNew(SOverlay)
							.Visibility(EVisibility::SelfHitTestInvisible)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.DesiredSizeOverride(GridSize2D)
								.Visibility(EVisibility::Hidden)
							]
							+ SOverlay::Slot()
							[
								SNew(SRightClickableButton)
								.OnClicked(this, &MineSweeperOnDetails::OnClicked, y, x)
								.OnRightClicked(this, &MineSweeperOnDetails::OnRightClicked, y, x)
								.IsEnabled(this, &MineSweeperOnDetails::GetIsEnabledButton,y, x)
							]
							+ SOverlay::Slot()
							[
								SNew(SImageSink)
								.DesiredSizeOverride(GridSize2D)
								.Visibility_Lambda
								(
									[this, x, y]()
									{

										if (MineActor.IsValid())
										{
											if((MineActor->IsRevealed(y,x) && !MineActor->IsFlagged(y,x) ) || MineActor->IsGameOver())
											{
												return EVisibility::Visible;
											}
										}
										return EVisibility::Hidden;
									}
								)
								.ColorAndOpacity(FLinearColor(1.0f,1.0f,1.0f,0.25f))
							]
							+ SOverlay::Slot()
							[
								SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.Font(NumberFont)
								.Text_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											if (MineActor->IsRevealed(y,x))
											{
												int32 Val = MineActor->CalculateFieldNumber(y, x);
												if (Val > 0)
												{
													return FText::FromString(FString::FromInt(Val));
												}
											}
										}
										return FText::FromString("");
									}
								)
								.ColorAndOpacity_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											int32 Val = MineActor->CalculateFieldNumber(y, x);
											switch (Val)
												{
												case 1:return FLinearColor::Blue;
												case 2:return FLinearColor::Green;
												case 3:return FLinearColor::Red;
												case 4:return FLinearColor(FColor::FromHex("010123FF"));
												case 5:return FLinearColor(FColor::FromHex("170000FF"));
												case 6:return FLinearColor(FColor::FromHex("001D26FF"));
												case 7:return FLinearColor(FColor::FromHex("101010FF"));
												case 8:return FLinearColor(FColor::FromHex("101010FF"));
												case 9:return FLinearColor(FColor::FromHex("616C61FF"));
												default:
													break;
												}
										}
										return FLinearColor::White;
									}
								)
								.Visibility_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											if (MineActor->IsRevealed(y,x))
											{
												int32 Val = MineActor->CalculateFieldNumber(y, x);
												if (Val > 0)
												{
													return EVisibility::HitTestInvisible;
												}
											}
										}
										return EVisibility::Collapsed;
									}
								)
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.DesiredSizeOverride(GridSize2D)
								.Image(FSlateMinesStyle::Get().GetBrush("Mine.Mine"))
								.Visibility_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											if (MineActor->IsGameOver() && !MineActor->IsFlagged(y, x) && MineActor->IsMine(y, x))
											{
												return  EVisibility::HitTestInvisible;
											}
										}
										return EVisibility::Collapsed;
									}
								)
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.DesiredSizeOverride(GridSize2D)
								.Image(FSlateMinesStyle::Get().GetBrush("Mine.Flag"))
								.Visibility_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											if (MineActor->IsFlagged(y, x))
											{
												return EVisibility::HitTestInvisible;
											}
										}
										return EVisibility::Collapsed;
									}
								)
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.DesiredSizeOverride(GridSize2D)
								.Image(FSlateMinesStyle::Get().GetBrush("Mine.Cross"))
								.Visibility_Lambda
								(
									[this, x, y]()
									{
										if (MineActor.IsValid())
										{
											if (MineActor->IsGameOver() && MineActor->IsCrossed(y, x))
											{
												return EVisibility::HitTestInvisible;
											}
										}
										return EVisibility::Collapsed;
									}
								)
							]
					];
				}
			}

			
		}
	}
}

bool MineSweeperOnDetails::GetIsEnabledButton(int32 X, int32 Y) const
{
	if (MineActor.IsValid())
	{
		return !MineActor->IsRevealed(X, Y);
	}
	return false;
}

FReply MineSweeperOnDetails::OnClicked(int32 X, int32 Y)
{
	if (MineActor.IsValid())
	{
		if (MineActor->CanClickOnField(X, Y))
		{
			const FMineSweeperTransactionScope Transaction(FText::FromString("Mine Left Click"), MineActor.Get());
			MineActor->HandleClickOnField(X, Y);
		}
		return  FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply MineSweeperOnDetails::OnRightClicked(int32 X, int32 Y)
{
	if (MineActor.IsValid())
	{
		if (MineActor->CanRightClickOnField(X, Y))
		{
			const FMineSweeperTransactionScope Transaction(FText::FromString("Mine Right Click"), MineActor.Get());
			MineActor->HandleRightClickOnField(X, Y);
		}
		return  FReply::Handled();
	}
	return FReply::Unhandled();
}
