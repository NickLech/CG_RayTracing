#include <utility>

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE __attribute__((always_inline))
#endif

#define DECLARE_ENUM_LOGICAL_OPS(enum_name) \
	static enum_name operator|(enum_name l, enum_name r) { return enum_name(std::to_underlying(l) | std::to_underlying(r)); } \
	static enum_name operator&(enum_name l, enum_name r) { return enum_name(std::to_underlying(l) & std::to_underlying(r)); } \
	static enum_name operator^(enum_name l, enum_name r) { return enum_name(std::to_underlying(l) ^ std::to_underlying(r)); } \
	static enum_name operator~(enum_name v) { return enum_name(~std::to_underlying(v)); } \
	static bool to_bool(enum_name v) { return std::to_underlying(v) != 0; }