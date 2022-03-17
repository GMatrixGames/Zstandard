// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Misc/ICompressionFormat.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#if WITH_EDITOR
#include "Settings/ProjectPackagingSettings.h"
#endif

#include "ZStandardPluginPCH.h"

DEFINE_LOG_CATEGORY_STATIC(ZStandardCompression, Log, All);

#define DEFAULT_COMPRESSION_LEVEL 10
#define LIBZSTD_VERSION 1

struct FZStandardCustomCompressor : ICompressionFormat
{
	int Level;

	FZStandardCustomCompressor(int level)
	{
		Level = level;
	}

	virtual ~FZStandardCustomCompressor() override
	{
	}

	virtual FName GetCompressionFormatName() override
	{
		return TEXT("zstd");
	}

	virtual uint32 GetVersion() override
	{
		return LIBZSTD_VERSION;
	}

	virtual FString GetDDCKeySuffix() override
	{
		return FString::Printf(TEXT("zstd_CL_%d_v%d"), Level, LIBZSTD_VERSION);
	}

	virtual bool Compress(void* CompressedBuffer, int32& CompressedSize, const void* UncompressedBuffer,
	                      const int32 UncompressedSize, const int32 CompressionData, ECompressionFlags Flags) override
	{
		if (const int32 Result = ZSTD_compress(CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize,
		                                       Level); Result > 0)
		{
			if (Result > GetCompressedBufferSize(UncompressedSize, CompressionData))
			{
				FPlatformMisc::LowLevelOutputDebugStringf(
					TEXT("%d < %d"), Result, GetCompressedBufferSize(UncompressedSize, CompressionData));
				// we cannot safely go over the BufferSize needed!
				return false;
			}
			CompressedSize = Result;
			return true;
		}
		return false;
	}

	virtual bool Uncompress(void* UncompressedBuffer, int32& UncompressedSize, const void* CompressedBuffer,
	                        const int32 CompressedSize, int32 CompressionData) override
	{
		if (const int32 Result = ZSTD_decompress(UncompressedBuffer, UncompressedSize, CompressedBuffer, CompressedSize)
			; Result > 0)
		{
			UncompressedSize = Result;
			return true;
		}

		return false;
	}

	virtual bool DoesOwnWorthDecompressingCheck() override
	{
		return false;
	}

	virtual int32 GetCompressedBufferSize(int32 UncompressedSize, int32 CompressionData) override
	{
		return ZSTD_compressBound(UncompressedSize);
	}
};

#define ZSTD_LEVEL_OPTION_STRING TEXT("-ZstdLevel=")

class FZStandardPluginModuleInterface : public IModuleInterface
{
	virtual void StartupModule() override
	{
		if (FString CommandLine = FCommandLine::Get(); CommandLine.Contains(
			ZSTD_LEVEL_OPTION_STRING, ESearchCase::IgnoreCase))
		{
			int32 Level;
			FParse::Value(FCommandLine::Get(), *FString(ZSTD_LEVEL_OPTION_STRING).ToLower(), Level);
			CompressionFormat = new FZStandardCustomCompressor(FMath::Clamp(Level, ZSTD_minCLevel(), ZSTD_maxCLevel()));
		}
		else
		{
			CompressionFormat = new FZStandardCustomCompressor(DEFAULT_COMPRESSION_LEVEL);
		}

		IModularFeatures::Get().RegisterModularFeature(COMPRESSION_FORMAT_FEATURE_NAME, CompressionFormat);
	}

	virtual void ShutdownModule() override
	{
		IModularFeatures::Get().UnregisterModularFeature(COMPRESSION_FORMAT_FEATURE_NAME, CompressionFormat);
		delete CompressionFormat;
		CompressionFormat = nullptr;
	}

	ICompressionFormat* CompressionFormat = nullptr;
};

IMPLEMENT_MODULE(FZStandardPluginModuleInterface, ZStandardPlugin);
