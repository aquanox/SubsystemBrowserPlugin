// Copyright 2022, Aquanox.

#include "SubsystemBrowserUtils.h"
#include "SubsystemBrowserTestSubsystem.h"
#include "Misc/AutomationTest.h"

#ifdef WITH_SB_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPropertyCountTest, "SubsystemBrowser.Util.PropertyCounter",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter);

bool FPropertyCountTest::RunTest(const FString& Parameters)
{
	UClass* SBClass = USubsystemBrowserTestSubsystem::StaticClass();

	auto PropertyCounts = FSubsystemBrowserUtils::GetClassFieldStats(SBClass);
	TestEqual("PropertyCounts.NumProperties", PropertyCounts.NumProperties, 27);
	TestEqual("PropertyCounts.NumEditable", PropertyCounts.NumEditable,  24);
	TestEqual("PropertyCounts.NumVisible", PropertyCounts.NumVisible, 2);
	TestEqual("PropertyCounts.NumConfig", PropertyCounts.NumConfig, 7);
	TestEqual("PropertyCounts.NumCallable", PropertyCounts.NumCallable,  2);

	return true;
}

#endif
