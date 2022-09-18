// Copyright 2022, Aquanox.

#include "CoreMinimal.h"
#include "SubsystemBrowserUtils.h"
#include "Tests/SubsystemBrowserTestSubsystem.h"
#include "Misc/AutomationTest.h"

constexpr int BasicTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

//
// IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPropertyCountTest, "Subsystem.Util.PropertyCounter", BasicTestFlags);
//
// bool FPropertyCountTest::RunTest(const FString& Parameters)
// {
// 	UClass* SBClass = USubsystemBrowserTestSubsystem::StaticClass();
//
// 	auto PropertyCounts = FSubsystemBrowserUtils::GetClassPropertyCounts(SBClass);
// 	TestTrueExpr(PropertyCounts.NumTotal == 23);
// 	TestTrueExpr(PropertyCounts.NumEditable == 21);
// 	TestTrueExpr(PropertyCounts.NumVisible == 2);
// 	TestTrueExpr(PropertyCounts.NumHidden == 2);
// 	TestTrueExpr(PropertyCounts.NumConfig == 6);
//
// 	return true;
// }
