#ifdef _WIN32
#define LIBRARY_EXPORT __declspec(dllexport)
#else
#define LIBRARY_EXPORT
#endif

struct ExecutionContext;
typedef bool (*ScriptExtFunction)(struct ExecutionContext *context);

typedef struct ScriptNativeInterface {
	bool (*ParameterBool)(struct ExecutionContext *context, bool *output);
	bool (*ParameterCString)(struct ExecutionContext *context, char **output);
	bool (*ParameterDouble)(struct ExecutionContext *context, double *output);
	bool (*ParameterHandle)(struct ExecutionContext *context, void **output);
	bool (*ParameterInt32)(struct ExecutionContext *context, int32_t *output);
	bool (*ParameterInt64)(struct ExecutionContext *context, int64_t *output);
	bool (*ParameterString)(struct ExecutionContext *context, const void **output, size_t *outputBytes);
	bool (*ParameterUint32)(struct ExecutionContext *context, uint32_t *output);
	bool (*ParameterUint64)(struct ExecutionContext *context, uint64_t *output);
	bool (*ReturnBoxInError)(struct ExecutionContext *context);
	bool (*ReturnDouble)(struct ExecutionContext *context, double input);
	bool (*ReturnError)(struct ExecutionContext *context, const char *message);
	bool (*ReturnHandle)(struct ExecutionContext *context, void *handleData, void (*close)(void *));
	bool (*ReturnInt)(struct ExecutionContext *context, int64_t input);
	bool (*ReturnString)(struct ExecutionContext *context, const void *input, size_t inputBytes);
	bool (*RunCallback)(struct ExecutionContext *context, intptr_t functionPointer, int64_t *parameters, bool *managedParameters, size_t parameterCount);
} ScriptNativeInterface;

#ifdef SCRIPT_ENGINE
typedef void (*ScriptSetNativeInterfacePointerFunction)(const ScriptNativeInterface *s);
extern const ScriptNativeInterface _scriptNativeInterface;
#else
#define ScriptParameterBool(...)    (scriptNativeInterface->ParameterBool   (__VA_ARGS__))
#define ScriptParameterCString(...) (scriptNativeInterface->ParameterCString(__VA_ARGS__))
#define ScriptParameterDouble(...)  (scriptNativeInterface->ParameterDouble (__VA_ARGS__))
#define ScriptParameterHandle(...)  (scriptNativeInterface->ParameterHandle (__VA_ARGS__))
#define ScriptParameterInt32(...)   (scriptNativeInterface->ParameterInt32  (__VA_ARGS__))
#define ScriptParameterInt64(...)   (scriptNativeInterface->ParameterInt64  (__VA_ARGS__))
#define ScriptParameterString(...)  (scriptNativeInterface->ParameterString (__VA_ARGS__))
#define ScriptParameterUint32(...)  (scriptNativeInterface->ParameterUint32 (__VA_ARGS__))
#define ScriptParameterUint64(...)  (scriptNativeInterface->ParameterUint64 (__VA_ARGS__))
#define ScriptReturnBoxInError(...) (scriptNativeInterface->ReturnBoxInError(__VA_ARGS__))
#define ScriptReturnDouble(...)     (scriptNativeInterface->ReturnDouble    (__VA_ARGS__))
#define ScriptReturnError(...)      (scriptNativeInterface->ReturnError     (__VA_ARGS__))
#define ScriptReturnHandle(...)     (scriptNativeInterface->ReturnHandle    (__VA_ARGS__))
#define ScriptReturnInt(...)        (scriptNativeInterface->ReturnInt       (__VA_ARGS__))
#define ScriptReturnString(...)     (scriptNativeInterface->ReturnString    (__VA_ARGS__))
#define ScriptRunCallback(...)      (scriptNativeInterface->RunCallback     (__VA_ARGS__))
const ScriptNativeInterface *scriptNativeInterface;
LIBRARY_EXPORT void ScriptSetNativeInterfacePointer(const ScriptNativeInterface *s) { scriptNativeInterface = s; }
#endif
