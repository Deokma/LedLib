#ifdef __cplusplus
extern "C" {
#endif

#ifdef SIMPLEBLE_WRAPPER_EXPORTS
#define SIMPLEBLE_WRAPPER_API __declspec(dllexport)
#else
#define SIMPLEBLE_WRAPPER_API __declspec(dllimport)
#endif

	SIMPLEBLE_WRAPPER_API void startSimpleBLE(const char* data);  // Updated function signature to accept data

#ifdef __cplusplus
}
#endif

#include <common/utils.hpp>
extern int simple_write(SimpleBLE::Peripheral& peripheral, const std::string& data);