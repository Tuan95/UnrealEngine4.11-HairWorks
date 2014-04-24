// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*==============================================================================
	MaterialExpressionParticleDirection: Exposes the direction of a particle to
		the material editor.
==============================================================================*/

#pragma once

#include "MaterialExpressionParticleDirection.generated.h"

UCLASS(HeaderGroup=Material, collapsecategories, hidecategories=Object)
class UMaterialExpressionParticleDirection : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()


	// Begin UMaterialExpression Interface
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex, int32 MultiplexIndex) OVERRIDE;
	virtual void GetCaption(TArray<FString>& OutCaptions) const OVERRIDE;
	// End UMaterialExpression Interface
};



