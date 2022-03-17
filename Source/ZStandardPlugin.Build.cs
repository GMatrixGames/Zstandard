// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ZStandardPlugin : ModuleRules
{
	public ZStandardPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "Private/ZStandardPluginPCH.h";

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Analytics"
			}
		);

		PublicIncludePaths.AddRange(
			new[]
			{
				Path.Combine(ModuleDirectory, "ThirdParty"),
				Path.Combine(ModuleDirectory, "ThirdParty/common"),
				Path.Combine(ModuleDirectory, "ThirdParty/compress"),
				Path.Combine(ModuleDirectory, "ThirdParty/decompress"),
				Path.Combine(ModuleDirectory, "ThirdParty/deprecated"),
				Path.Combine(ModuleDirectory, "ThirdParty/dictBuilder"),
				Path.Combine(ModuleDirectory, "ThirdParty/legacy")
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		PublicDefinitions.AddRange(new[]
		{
			"ZSTD_MULTITHREAD=1",
			"ZSTD_LEGACY_SUPPORT=5"
		});

		bEnableUndefinedIdentifierWarnings = false;
	}
}