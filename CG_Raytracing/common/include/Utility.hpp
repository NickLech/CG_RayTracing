#pragma once

#include <cstdint>

namespace cg_raytracing {
	namespace common {
		/// <summary>
		/// Marks code following this function 
		/// as 'unreachable'. Use for possible
		/// optimization and/or remove useless
		/// compiler warnings
		/// </summary>
		static void Unreachable() {
#if defined(_MSC_VER)
			__assume(false);
#elif defined(__GNUC__) || defined(__clang__)
			__builtin_unreachable();
#else
#error "Unrecognized compiler"
#endif // _MSVC_VER
		}

		/// <summary>
		/// Probably causes an int3 to occurr,
		/// which sends a SIGTRAP to all threads.
		/// If a debugger is attached, execution
		/// will break at the level of this call
		/// </summary>
		void DebugBreak();
	}
}
