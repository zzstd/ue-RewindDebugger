// Copyright 2024-2026 zz studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"



namespace ZZ::Rewind
{

/*
 *  Intended for frequently sampled frame data.
 *	For example, Position, Time, Pose, etc
 *		
 *	Consecutive equal values are deduplicated by comparing each value with the previous recorded value, 
 *	so T must support operator==.
 *	
 *	
*/ 
template<typename T>
class TFrameData
{
	static_assert(
		TModels_V<CEqualityComparable, T>,
		"TFrameData<T> requires T to be equality comparable."
	);
	
	TArray<T> Data;
	TArray<int32> IndexMap;
	TOptional<int32> FirstFrame;
	TOptional<int32> LastFrame;
	
	void FillGap(int32 Frame)
	{
		if (!FirstFrame.IsSet())
		{
			FirstFrame = Frame;
		}

		if (LastFrame.IsSet())
		{
			const int32 GapCount = Frame - LastFrame.GetValue() - 1;
			if (GapCount > 0)
			{
				const int32 StartIndex = IndexMap.AddUninitialized(GapCount);
				FMemory::Memset(IndexMap.GetData() + StartIndex, INDEX_NONE, GapCount * sizeof(int32));
			}
		}
	}
public:
	void Append(int32 Frame, const T& Value)
	{
		FillGap(Frame);
		
		int32 ValueIndex;
		// Deduplicate consecutive values
		if (!Data.IsEmpty() && Data.Last() == Value)
		{
			ValueIndex = Data.Num() - 1;
		}
		else
		{
			ValueIndex = Data.Add(Value);
		}
		
		IndexMap.Add(ValueIndex);
		LastFrame = Frame;
	}

	const T* Find(int32 Frame) const
	{
		if (!FirstFrame.IsSet())
		{
			return nullptr;
		}

		const int32 Offset = Frame - FirstFrame.GetValue();
		if (!IndexMap.IsValidIndex(Offset))
		{
			return nullptr;
		}

		const int32 ValueIndex = IndexMap[Offset];
		return ValueIndex == INDEX_NONE ? nullptr : &Data[ValueIndex];
	}
	
	void Clear() noexcept
	{
		Data.Empty();
		IndexMap.Empty();
		FirstFrame.Reset();
		LastFrame.Reset();
	}
};
	
/*
 * Intended for values that remain unchanged for long periods
 * For example, Asset, etc
 * 
 * Adjacent ranges are merged by comparing each value with the previous recorded value, 
 * so T must support operator==.
 */
template<typename T>
class TRangeData
{
	static_assert(
		TModels_V<CEqualityComparable, T>,
		"TRangeData<T> requires T to be equality comparable."
	);
	
	struct FInfo
	{
		T Value;
		int32 StartFrame;
		int32 EndFrame;
		bool InRange(int32 Frame) const
		{
			return StartFrame <= Frame && Frame <= EndFrame;
		}
	};
	TArray<FInfo> Data;
	
	mutable TOptional<int32> CachedFindIndex;
public:
	void Append(int32 Frame, const T& Value)
	{
		if (!Data.IsEmpty() && Data.Last().Value == Value)
		{
			Data.Last().EndFrame = Frame;
		}
		else
		{
			Data.Add({ Value, Frame, Frame });
		}
	}
	
	const T* Find(int32 Frame) const
	{
		if (CachedFindIndex.IsSet())
		{
			const int32 CachedIndex = CachedFindIndex.GetValue();
			if (Data.IsValidIndex(CachedIndex) && Data[CachedIndex].InRange(Frame))
			{
				return &Data[CachedIndex].Value;
			}
		}
		
		for (int32 i = 0; i < Data.Num(); i++)
		{
			if (Data[i].InRange(Frame))
			{
				CachedFindIndex = i;
				return &Data[i].Value;
			}
		}
	
		return nullptr;
	}
	
	void Clear() noexcept
	{
		Data.Empty();
		CachedFindIndex.Reset();
	}
};
}