#include "LBHudWidget.h"

FText ULBHudWidget::FormatCompactNumber(int64 Value)
{
	if (Value >= 1000000)
	{
		return FText::FromString(FString::Printf(TEXT("%.1fM"), Value / 1000000.0));
	}
	if (Value >= 1000)
	{
		return FText::FromString(FString::Printf(TEXT("%.1fK"), Value / 1000.0));
	}
	return FText::AsNumber(Value);
}
