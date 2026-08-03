#pragma once

#include <print>
#include <iostream>

#ifdef DEBUG
#define LOG(fmt, ...) std::println(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) std::println(std::cerr, fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#define LOG_ERROR(fmt, ...)
#endif