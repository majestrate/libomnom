#pragma once
#define LogDebug(ctx, ...) ctx->lmq().log(lokimq::LogLevel::debug, __FILE__, __LINE__, __VA_ARGS__)
#define LogInfo(ctx, ...) ctx->lmq().log(lokimq::LogLevel::info, __FILE__, __LINE__, __VA_ARGS__)
#define LogWarn(ctx, ...) ctx->lmq().log(lokimq::LogLevel::warn, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(ctx, ...) ctx->lmq().log(lokimq::LogLevel::error, __FILE__, __LINE__, __VA_ARGS__)
