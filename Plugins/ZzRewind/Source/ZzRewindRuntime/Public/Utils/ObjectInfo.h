// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


namespace ZZ::Rewind
{

class FRewindItem;
	
/* 
 *	TWeakObjectPtr + ObjectName
 */
template<typename T>
class TObjectInfo
{
public:
	~TObjectInfo() = default;

	TWeakObjectPtr<T> WeakObject;
	
	FString ObjectName;
	
	FString ObjectPath;

	TWeakObjectPtr<UClass> ObjectClass;
	
	uint64 ObjectID = 0; // @TODO

	TObjectInfo(T* Object = nullptr)
		: WeakObject(Object)
	{
		if (UObject* ThisObj = Cast<UObject>(Object))
		{
			ObjectName = ThisObj->GetName();
			ObjectPath = ThisObj->GetPathName();
			ObjectClass = ThisObj->GetClass();
		}
	}

	template<typename T2>
	TObjectInfo(TObjectInfo<T2> Object)
		: WeakObject(Object.WeakObject)
		, ObjectName(Object.ObjectName)
		, ObjectPath(Object.ObjectPath)
		, ObjectClass(Object.ObjectClass)
		, ObjectID(Object.ObjectID)
	{ }
	
	TObjectInfo(TObjectPtr<T> Object) : TObjectInfo(Object.Get()) {}
	TObjectInfo(TWeakObjectPtr<T> Object) : TObjectInfo(Object.Get()) {}

	bool operator==(const TObjectInfo& Other) const
	{
		if (WeakObject.IsValid() || Other.WeakObject.IsValid())
		{
			return WeakObject == Other.WeakObject;
		}

		return ObjectName == Other.ObjectName && ObjectPath == Other.ObjectPath;
	}
};

class FZzRewindObjectMap
{
public:
	uint64 RegisterObject(UObject* InObj);

	TMap<uint64, TWeakPtr<FRewindItem>> Map;
};

}