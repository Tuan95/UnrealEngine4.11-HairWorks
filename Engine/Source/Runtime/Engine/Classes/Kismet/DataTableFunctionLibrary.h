// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataTableFunctionLibrary.generated.h"

/** Enum used to indicate success or failure of EvaluateCurveTableRow */
UENUM()
namespace EEvaluateCurveTableResult
{
    enum Type
    {
        /** Found the row successfully */
        RowFound,
        /** Failed to find row */
        RowNotFound,
    };
}

UCLASS(MinimalAPI)
class UDataTableFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "DataTable", meta = (ExpandEnumAsExecs="OutResult", DataTablePin="CurveTable"))
	static void EvaluateCurveTableRow(UCurveTable* CurveTable, FName RowName, float InXY, TEnumAsByte<EEvaluateCurveTableResult::Type>& OutResult, float& OutXY);
    
    /** Get a Row from a DataTable given a RowName */
    UFUNCTION(BlueprintCallable, CustomThunk, Category = "DataTable", meta=(CustomStructureParam = "OutRow", BlueprintInternalUseOnly="true"))
    static bool GetDataTableRowFromName(UDataTable* Table, FName RowName, FTableRowBase& OutRow);
    
    /** Based on UDataTableFunctionLibrary::GetDataTableRow */
    DECLARE_FUNCTION(execGetDataTableRowFromName)
    {
        P_GET_OBJECT(UDataTable, Table);
        P_GET_PROPERTY(UNameProperty, RowName);
        
        Stack.StepCompiledIn<UStructProperty>(NULL);
        void* OutRowPtr = Stack.MostRecentPropertyAddress;
        
        P_FINISH;
        
        bool foundRow = false;
        
        if (OutRowPtr != NULL)
        {
            void* RowPtr = Table->FindRowUnchecked(RowName);
            
            if (RowPtr != NULL)
            {
                UScriptStruct* StructType = Table->RowStruct;
                
                if (StructType != NULL)
                {
                    StructType->CopyScriptStruct(OutRowPtr, RowPtr);
                    foundRow = true;
                }
            }
        }
        
        *(bool*)Result = foundRow;
    }
};
