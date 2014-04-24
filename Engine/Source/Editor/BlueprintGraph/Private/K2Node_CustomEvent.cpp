// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "BlueprintGraphPrivatePCH.h"

#include "EngineLevelScriptClasses.h"
#include "Kismet2NameValidators.h"
#include "K2Node_BaseMCDelegate.h"
#include "CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "K2Node_CustomEvent"

#define SNAP_GRID (16) // @todo ensure this is the same as SNodePanel::GetSnapGridSize()

/**
 * Attempts to find a CustomEvent node associated with the specified function.
 * 
 * @param  CustomEventFunc	The function you want to find an associated node for.
 * @return A pointer to the found node (NULL if a corresponding node wasn't found)
 */
static UK2Node_CustomEvent const* FindCustomEventNodeFromFunction(UFunction* CustomEventFunc)
{
	UK2Node_CustomEvent const* FoundEventNode = NULL;
	if (CustomEventFunc != NULL)
	{
		UObject const* const FuncOwner = CustomEventFunc->GetOuter();
		check(FuncOwner != NULL);

		// if the found function is a NOT a native function (it's user generated)
		if (FuncOwner->IsA(UBlueprintGeneratedClass::StaticClass()))
		{
			UBlueprintGeneratedClass* FuncClass = Cast<UBlueprintGeneratedClass>(CustomEventFunc->GetOuter());
			check(FuncClass != NULL);
			UBlueprint* FuncBlueprint = Cast<UBlueprint>(FuncClass->ClassGeneratedBy);
			check(FuncBlueprint != NULL);

			TArray<UK2Node_CustomEvent*> BpCustomEvents;
			FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_CustomEvent>(FuncBlueprint, BpCustomEvents);

			// look to see if the function that this is overriding is a custom-event
			for (UK2Node_CustomEvent const* const UserEvent : BpCustomEvents)
			{
				if (UserEvent->CustomFunctionName == CustomEventFunc->GetFName())
				{
					FoundEventNode = UserEvent;
					break;
				}
			}
		}
	}

	return FoundEventNode;
}

/**
 * Custom handler for validating CustomEvent renames
 */
class FCustomEventNameValidator : public FKismetNameValidator
{
public:
	FCustomEventNameValidator(UK2Node_CustomEvent const* CustomEventIn)
		: FKismetNameValidator(CustomEventIn->GetBlueprint(), CustomEventIn->CustomFunctionName)
		, CustomEvent(CustomEventIn)
	{
		check(CustomEvent != NULL);
	}

	// Begin INameValidatorInterface
	virtual EValidatorResult IsValid(FString const& Name, bool bOriginal = false) OVERRIDE
	{
		EValidatorResult NameValidity = FKismetNameValidator::IsValid(Name, bOriginal);
		if ((NameValidity == EValidatorResult::Ok) || (NameValidity == EValidatorResult::ExistingName))
		{
			UBlueprint* Blueprint = CustomEvent->GetBlueprint();
			check(Blueprint != NULL);

			UFunction* ParentFunction = FindField<UFunction>(Blueprint->ParentClass, *Name);
			// if this custom-event is overriding a function belonging to the blueprint's parent
			if (ParentFunction != NULL)
			{
				UK2Node_CustomEvent const* OverriddenEvent = FindCustomEventNodeFromFunction(ParentFunction);
				// if the function that we're overriding isn't another custom event,
				// then we can't name it this (only allow custom-event to override other custom-events)
				if (OverriddenEvent == NULL)
				{
					NameValidity = EValidatorResult::AlreadyInUse;
				}		
			}
		}
		return NameValidity;
	}
	// End INameValidatorInterface

private:
	UK2Node_CustomEvent const* CustomEvent;
};

UK2Node_CustomEvent::UK2Node_CustomEvent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bOverrideFunction = false;
	bIsEditable = true;
	bCanRenameNode = true;
}

FText UK2Node_CustomEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::EditableTitle)
	{
		return FText::FromName(CustomFunctionName);
	}
	else if(TitleType == ENodeTitleType::ListView)
	{
		return NSLOCTEXT("K2Node", "CustomEvent_Title", "Custom Event");
	}
	else
	{
		FString RPCString = UK2Node_Event::GetLocalizedNetString(FunctionFlags, false);

		FFormatNamedArguments Args;
		Args.Add(TEXT("FunctionName"), FText::FromName(CustomFunctionName));
		Args.Add(TEXT("RPCString"), FText::FromString(RPCString));
		return FText::Format(NSLOCTEXT("K2Node", "CustomEvent_Name", "{FunctionName}{RPCString}\nCustom Event"), Args);
	}
}

FString UK2Node_CustomEvent::GetNodeNativeTitle(ENodeTitleType::Type TitleType) const
{
	// Do not setup this function for localization, intentionally left unlocalized!
	if (TitleType == ENodeTitleType::EditableTitle)
	{
		return CustomFunctionName.ToString();
	}
	else if(TitleType == ENodeTitleType::ListView)
	{
		return TEXT("Custom Event");
	}
	else
	{
		FString RPCString = UK2Node_Event::GetLocalizedNetString(FunctionFlags, false);
		return FString::Printf(TEXT("%s\nCustom Event"), *CustomFunctionName.ToString()) + RPCString;
	}
}

UEdGraphPin* UK2Node_CustomEvent::CreatePinFromUserDefinition(const TSharedPtr<FUserPinInfo> NewPinInfo)
{
	UEdGraphPin* NewPin = CreatePin(
		EGPD_Output, 
		NewPinInfo->PinType.PinCategory, 
		NewPinInfo->PinType.PinSubCategory, 
		NewPinInfo->PinType.PinSubCategoryObject.Get(), 
		NewPinInfo->PinType.bIsArray, 
		NewPinInfo->PinType.bIsReference, 
		NewPinInfo->PinName);
	NewPin->DefaultValue = NewPin->AutogeneratedDefaultValue = NewPinInfo->PinDefaultValue;
	return NewPin;
}

void UK2Node_CustomEvent::RenameCustomEventCloseToName(int32 StartIndex)
{
	bool bFoundName = false;
	const FString& BaseName = CustomFunctionName.ToString();

	for (int32 NameIndex = StartIndex; !bFoundName; ++NameIndex)
	{
		const FString NewName = FString::Printf(TEXT("%s_%d"), *BaseName, NameIndex);
		if (Rename(*NewName, GetOuter(), REN_Test))
		{
			CustomFunctionName = FName(NewName.GetCharArray().GetData());
			Rename(*NewName);
			bFoundName = true;
		}
	}
}

void UK2Node_CustomEvent::OnRenameNode(const FString& NewName)
{
	CustomFunctionName = *NewName;
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

TSharedPtr<class INameValidatorInterface> UK2Node_CustomEvent::MakeNameValidator() const
{
	return MakeShareable(new FCustomEventNameValidator(this));
}

bool UK2Node_CustomEvent::IsOverride() const
{
	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint != NULL);

	UFunction* ParentFunction = FindField<UFunction>(Blueprint->ParentClass, CustomFunctionName);
	UK2Node_CustomEvent const* OverriddenEvent = FindCustomEventNodeFromFunction(ParentFunction);

	return (OverriddenEvent != NULL);
}

uint32 UK2Node_CustomEvent::GetNetFlags() const
{
	uint32 NetFlags = (FunctionFlags & FUNC_NetFuncFlags);
	if (IsOverride())
	{
		UBlueprint* Blueprint = GetBlueprint();
		check(Blueprint != NULL);

		UFunction* ParentFunction = FindField<UFunction>(Blueprint->ParentClass, CustomFunctionName);
		check(ParentFunction != NULL);

		// inherited net flags take precedence 
		NetFlags = (ParentFunction->FunctionFlags & FUNC_NetFuncFlags);
	}
	return NetFlags;
}

void UK2Node_CustomEvent::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint != NULL);

	UFunction* ParentFunction = FindField<UFunction>(Blueprint->ParentClass, CustomFunctionName);
	// if this custom-event is overriding a function belonging to the blueprint's parent
	if (ParentFunction != NULL)
	{
		UObject const* const FuncOwner = ParentFunction->GetOuter();
		check(FuncOwner != NULL);

		// if this custom-event is attempting to override a native function, we can't allow that
		if (!FuncOwner->IsA(UBlueprintGeneratedClass::StaticClass()))
		{
			MessageLog.Error(*FString::Printf(*LOCTEXT("NativeFunctionConflict", "@@ name conflicts with a native '%s' function").ToString(), *FuncOwner->GetName()), this);
		}
		else 
		{
			UK2Node_CustomEvent const* OverriddenEvent = FindCustomEventNodeFromFunction(ParentFunction);
			// if the function that this is attempting to override is NOT another 
			// custom-event, then we want to error (a custom-event shouldn't override something different)
			if (OverriddenEvent == NULL)
			{
				MessageLog.Error(*FString::Printf(*LOCTEXT("NonCustomEventOverride", "@@ name conflicts with a '%s' function").ToString(), *FuncOwner->GetName()), this);
			}
			// else, we assume the user was attempting to override the parent's custom-event
			// the signitures could still be off, but FKismetCompilerContext::PrecompileFunction() should catch that
		}		
	}
}

void UK2Node_CustomEvent::ReconstructNode()
{
	const UEdGraphPin* DelegateOutPin = FindPin(DelegateOutputName);

	const UK2Node_BaseMCDelegate* OtherNode = (DelegateOutPin && DelegateOutPin->LinkedTo.Num() && DelegateOutPin->LinkedTo[0]) ?
		Cast<const UK2Node_BaseMCDelegate>(DelegateOutPin->LinkedTo[0]->GetOwningNode()) : NULL;
	const UFunction* DelegateSignature = OtherNode ? OtherNode->GetDelegateSignature() : NULL;
	const bool bUseDelegateSignature = (NULL == FindEventSignatureFunction()) && DelegateSignature;

	if (bUseDelegateSignature)
	{
		UserDefinedPins.Empty();
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		for (TFieldIterator<UProperty> PropIt(DelegateSignature); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			const UProperty* Param = *PropIt;
			if (!Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm))
			{
				FEdGraphPinType PinType;
				K2Schema->ConvertPropertyToPinType(Param, /*out*/ PinType);

				FString NewPinName = Param->GetName();
				int32 Index = 1;
				while ((DelegateOutputName == NewPinName) || (K2Schema->PN_Then == NewPinName))
				{
					++Index;
					NewPinName += FString::FromInt(Index);
				}
				TSharedPtr<FUserPinInfo> NewPinInfo = MakeShareable( new FUserPinInfo() );
				NewPinInfo->PinName = NewPinName;
				NewPinInfo->PinType = PinType;
				UserDefinedPins.Add(NewPinInfo);
			}
		}
	}

	Super::ReconstructNode();
}

UK2Node_CustomEvent* UK2Node_CustomEvent::CreateFromFunction(FVector2D GraphPosition, UEdGraph* ParentGraph, const FString& Name, const UFunction* Function, bool bSelectNewNode/* = true*/)
{
	UK2Node_CustomEvent* CustomEventNode = NULL;
	if(ParentGraph && Function)
	{
		CustomEventNode = NewObject<UK2Node_CustomEvent>(ParentGraph);
		CustomEventNode->CustomFunctionName = FName(*Name);
		CustomEventNode->SetFlags(RF_Transactional);
		ParentGraph->AddNode(CustomEventNode, true, bSelectNewNode);
		CustomEventNode->CreateNewGuid();
		CustomEventNode->PostPlacedNewNode();
		CustomEventNode->AllocateDefaultPins();

		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		for (TFieldIterator<UProperty> PropIt(Function); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			const UProperty* Param = *PropIt;
			if (!Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm))
			{
				FEdGraphPinType PinType;
				K2Schema->ConvertPropertyToPinType(Param, /*out*/ PinType);
				CustomEventNode->CreateUserDefinedPin(Param->GetName(), PinType);
			}
		}

		CustomEventNode->NodePosX = GraphPosition.X;
		CustomEventNode->NodePosY = GraphPosition.Y;
		CustomEventNode->SnapToGrid(SNAP_GRID);
	}

	return CustomEventNode;
}

bool UK2Node_CustomEvent::IsEditable() const
{
	const UEdGraphPin* DelegateOutPin = FindPin(DelegateOutputName);
	if(DelegateOutPin && DelegateOutPin->LinkedTo.Num())
	{
		return false;
	}
	return Super::IsEditable();
}

bool UK2Node_CustomEvent::IsUsedByAuthorityOnlyDelegate() const
{
	if(const UEdGraphPin* DelegateOutPin = FindPin(DelegateOutputName))
	{
		for(auto PinIter = DelegateOutPin->LinkedTo.CreateConstIterator(); PinIter; ++PinIter)
		{
			const UEdGraphPin* LinkedPin = *PinIter;
			const UK2Node_BaseMCDelegate* Node = LinkedPin ? Cast<const UK2Node_BaseMCDelegate>(LinkedPin->GetOwningNode()) : NULL;
			if(Node && Node->IsAuthorityOnly())
			{
				return true;
			}
		}
	}

	return false;
}

FString UK2Node_CustomEvent::GetTooltip() const
{
	return LOCTEXT("AddCustomEvent_Tooltip", "An event with customizable name and parameters.").ToString();
}

FString UK2Node_CustomEvent::GetDocumentationLink() const
{
	// Use the main k2 node doc
	return UK2Node::GetDocumentationLink();
}

FString UK2Node_CustomEvent::GetDocumentationExcerptName() const
{
	return TEXT("UK2Node_CustomEvent");
}

#undef LOCTEXT_NAMESPACE
