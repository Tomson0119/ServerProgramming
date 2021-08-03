#pragma once

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <ranges>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <exception>
#include <assert.h>