// Copyright 2022, Aquanox.

#pragma once

/**
 * Customized SceneOutliner sorting helper
 */
namespace SubsystemBrowser
{

template<typename ItemType, typename PrimaryKeyType, typename SecondaryKeyType = int32>
struct FSortHelper : FNoncopyable
{
	typedef TFunction<PrimaryKeyType(ItemType)> FPrimaryFunction;
	typedef TFunction<SecondaryKeyType(ItemType)> FSecondaryFunction;

	FSortHelper() = default;

	/** Sort primarily by the specified function and mode. Beware the function is a reference, so must be valid for the lifetime of this instance. */
	FSortHelper& Primary(FPrimaryFunction&& Function, EColumnSortMode::Type SortMode)
	{
		PrimarySortMode = SortMode;
		PrimaryFunction = MoveTemp(Function);
		return *this;
	}

	/** Sort secondarily by the specified function and mode. Beware the function is a reference, so must be valid for the lifetime of this instance. */
	FSortHelper& Secondary(FSecondaryFunction&& Function, EColumnSortMode::Type SortMode)
	{
		SecondarySortMode = SortMode;
		SecondaryFunction = MoveTemp(Function);
		return *this;
	}

	/** Sort the specified array using the current sort settings */
	void Sort(TArray<ItemType>& Array)
	{
		TArray<FSortPayload> SortData;
		const auto End = Array.Num();
		for (int32 Index = 0; Index < End; ++Index)
		{
			const auto& Element = Array[Index];

			PrimaryKeyType PrimaryKey = PrimaryFunction(Element);

			SecondaryKeyType SecondaryKey;
			if (SecondarySortMode != EColumnSortMode::None)
			{
				SecondaryKey = SecondaryFunction(Element);
			}

			SortData.Emplace(Index, MoveTemp(PrimaryKey), MoveTemp(SecondaryKey));
		}

		SortData.Sort([&](const FSortPayload& One, const FSortPayload& Two)
		{
			if (PrimarySortMode == EColumnSortMode::Ascending && One.PrimaryKey != Two.PrimaryKey)
				return One.PrimaryKey < Two.PrimaryKey;
			else if (PrimarySortMode == EColumnSortMode::Descending && One.PrimaryKey != Two.PrimaryKey)
				return One.PrimaryKey > Two.PrimaryKey;

			if (SecondarySortMode == EColumnSortMode::Ascending)
				return One.SecondaryKey < Two.SecondaryKey;
			else if (SecondarySortMode == EColumnSortMode::Descending)
				return One.SecondaryKey > Two.SecondaryKey;

			return false;
		});

		TArray<ItemType> NewArray;
		NewArray.Reserve(Array.Num());

		for (const auto& Element : SortData)
		{
			NewArray.Add(Array[Element.OriginalIndex]);
		}
		Array = MoveTemp(NewArray);
	}

private:

	EColumnSortMode::Type 	PrimarySortMode = EColumnSortMode::None;
	EColumnSortMode::Type 	SecondarySortMode = EColumnSortMode::None;

	FPrimaryFunction	PrimaryFunction;
	FSecondaryFunction	SecondaryFunction;

	/** Aggregated data from the sort methods. We extract the sort data from all elements first, then sort based on the extracted data. */
	struct FSortPayload : FNoncopyable
	{
		int32 OriginalIndex;

		PrimaryKeyType PrimaryKey;
		SecondaryKeyType SecondaryKey;

		FSortPayload(int32 InOriginalIndex, PrimaryKeyType&& InPrimaryKey, SecondaryKeyType&& InSecondaryKey)
			: OriginalIndex(InOriginalIndex)
			, PrimaryKey(MoveTemp(InPrimaryKey))
			, SecondaryKey(MoveTemp(InSecondaryKey)) {}

		FSortPayload(FSortPayload&& Other) { (*this) = MoveTemp(Other); }
		FSortPayload& operator=(FSortPayload&& rhs)
		{
			OriginalIndex = rhs.OriginalIndex;
			PrimaryKey = MoveTemp(rhs.PrimaryKey);
			SecondaryKey = MoveTemp(rhs.SecondaryKey);
			return *this;
		}
	};
};

}
