// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ObjectInfo.h"


namespace ZZ::Rewind
{

struct ZZREWINDRUNTIME_API FVariantValue
{
	FVariantValue()
		: Type(EValueType::None)
		, Vector()
	{
	}
	
	enum class EValueType : uint8
	{
		None,
		
		Bool,
		Int32,
		Float,
		
		Vector,
		Vector2D,
		Transform,

		Object,
		String
	} Type;

	struct FTransformEntry
	{
		FVector Location;
		FRotator Rotation;
		FVector Scale;
	};
	
	union
	{
		bool Bool;
		int32 Int32;
		float Float;
		
		FVector Vector;
		FVector2D Vector2D;
		FTransformEntry Transform;
	};

	TObjectInfo<UObject> Object;
	FString String;

	FVariantValue(bool Value)			:Type(EValueType::Bool), Bool(Value) {}
	FVariantValue(int32 Value)			:Type(EValueType::Int32), Int32(Value) {}
	FVariantValue(float Value)			:Type(EValueType::Float), Float(Value) {}
	
	FVariantValue(FVector Value)		:Type(EValueType::Vector), Vector(Value) {}
	FVariantValue(FVector2D Value)		:Type(EValueType::Vector2D), Vector2D(Value) {}
	FVariantValue(FTransform Value)		:Type(EValueType::Transform)
	{
		Transform.Location = Value.GetLocation();
		Transform.Rotation = Value.GetRotation().Rotator();
		Transform.Scale = Value.GetScale3D();
	}
	
	FVariantValue(UObject* Value)		:Type(EValueType::Object), Bool(false), Object(Value) {}
	template<typename T>
	FVariantValue(TObjectInfo<T> Value)		:Type(EValueType::Object), Bool(false), Object(Value) {}
	FVariantValue(FString Value)	:Type(EValueType::String), Bool(false), String(Value) {}

	bool operator==(const FVariantValue& Other) const
	{
		if (Type == Other.Type)
		{
			switch (Type)
			{
			case EValueType::Bool:		return Bool == Other.Bool;
			case EValueType::Int32:		return Int32 == Other.Int32;
			case EValueType::Float:		return Float == Other.Float;
			case EValueType::Vector:	return Vector == Other.Vector;
			case EValueType::Vector2D:	return Vector2D == Other.Vector2D;
			case EValueType::Transform:	return Transform.Location == Other.Transform.Location
												&& Transform.Rotation == Other.Transform.Rotation
												&& Transform.Scale == Other.Transform.Scale;
			case EValueType::Object:	return Object == Other.Object;
			case EValueType::String:	return String == Other.String;
			case EValueType::None:		return true;
			}
		}
		return false;
	}
};


}