#pragma once
#include <cstdlib>
#include <tinyfiledialogs.h>

// Certain function can emit an "EXIT" signal that's propagated until the program exits.
enum class Signal : bool {
	CONTINUE,
	EXIT
};

#ifndef NDEBUG
#define GTREF_VERSION_SUFFIX "d"
#else
#define GTREF_VERSION_SUFFIX ""
#endif

#ifdef _WIN32
#define GTREF_PLATFORM " (Windows)"
#elif __linux__
#define GTREF_PLATFORM " (Linux)"
#else
#define GTREF_PLATFORM ""
#endif

#ifndef NDEBUG
#define GTREF_STRINGIFY(x) #x
#define GTREF_TO_STRING(x) GTREF_STRINGIFY(x)
#define GTREF_ASSERT(cond)                                                                                             \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			tinyfd_messageBox("Assertion Failed - gtrefc", "At " __FILE__ ":" GTREF_TO_STRING(__LINE__) ":\n" #cond,   \
							  "ok", "error", 0);                                                                       \
			std::abort();                                                                                              \
		}                                                                                                              \
	} while (0)
#else
#define GTREF_ASSERT(cond)                                                                                             \
	do {                                                                                                               \
	} while (0)
#endif