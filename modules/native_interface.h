#ifdef _WIN32
#define LIBRARY_EXPORT __declspec(dllexport)
#else
#define LIBRARY_EXPORT
#endif

struct ExecutionContext;
typedef bool (*ScriptExtFunction)(struct ExecutionContext *context);
typedef void (*ScriptCloseHandleFunction)(struct ExecutionContext *context, void *handleData);

typedef struct ScriptNativeInterface {
	bool (*CreateHandle)(struct ExecutionContext *context, void *handleData, ScriptCloseHandleFunction close, intptr_t *handle);
	bool (*CreateString)(struct ExecutionContext *context, const void *input, size_t inputBytes, intptr_t *handle);
	bool (*CreateStruct)(struct ExecutionContext *context, int64_t *fields, bool *managedFields, size_t fieldCount, intptr_t *index);
	void (*HeapRefClose)(struct ExecutionContext *context, intptr_t index);
	bool (*ListCount)(struct ExecutionContext *context, intptr_t listIndex, size_t *output);
	bool (*ParameterBool)(struct ExecutionContext *context, bool *output);
	bool (*ParameterCString)(struct ExecutionContext *context, char **output);
	bool (*ParameterDouble)(struct ExecutionContext *context, double *output);
	bool (*ParameterHandle)(struct ExecutionContext *context, void **output);
	bool (*ParameterHeapRef)(struct ExecutionContext *context, intptr_t *index);
	bool (*ParameterInt32)(struct ExecutionContext *context, int32_t *output);
	bool (*ParameterInt64)(struct ExecutionContext *context, int64_t *output);
	bool (*ParameterScan)(struct ExecutionContext *context, const char *cFormat, ...);
	bool (*ParameterString)(struct ExecutionContext *context, const void **output, size_t *outputBytes);
	bool (*ParameterUint32)(struct ExecutionContext *context, uint32_t *output);
	bool (*ParameterUint64)(struct ExecutionContext *context, uint64_t *output);
	bool (*ReturnBoxInError)(struct ExecutionContext *context);
	bool (*ReturnDouble)(struct ExecutionContext *context, double input);
	bool (*ReturnError)(struct ExecutionContext *context, const char *message);
	bool (*ReturnHandle)(struct ExecutionContext *context, void *handleData, ScriptCloseHandleFunction close);
	bool (*ReturnHeapRef)(struct ExecutionContext *context, intptr_t index);
	bool (*ReturnInt)(struct ExecutionContext *context, int64_t input);
	bool (*ReturnString)(struct ExecutionContext *context, const void *input, size_t inputBytes);
	bool (*ReturnStruct)(struct ExecutionContext *context, int64_t *fields, bool *managedFields, size_t fieldCount);
	bool (*ReturnStructInl)(struct ExecutionContext *context, size_t fieldCount, ...); // TODO Remove this!
	bool (*RunCallback)(struct ExecutionContext *context, intptr_t functionPointer, int64_t *parameters, bool *managedParameters, size_t parameterCount, int64_t *returnValue, bool managedReturnValue);
	bool (*StructReadBool)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, bool *output);
	bool (*StructReadDouble)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, double *output);
	bool (*StructReadHandle)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, void **output);
	bool (*StructReadHeapRef)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, intptr_t *index);
	bool (*StructReadInt32)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, int32_t *output);
	bool (*StructReadInt64)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, int64_t *output);
	bool (*StructReadString)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, const void **output, size_t *outputBytes);
	bool (*StructReadUint32)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, uint32_t *output);
	bool (*StructReadUint64)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, uint64_t *output);
	bool (*StructWriteDouble)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, double value);
	bool (*StructWriteHandle)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, void *value, ScriptCloseHandleFunction close);
	bool (*StructWriteHeapRef)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, intptr_t value);
	bool (*StructWriteInt)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, int64_t value);
	bool (*StructWriteString)(struct ExecutionContext *context, intptr_t structOrListIndex, uintptr_t indexWithin, const void *value, size_t valueBytes);
} ScriptNativeInterface;

#ifdef SCRIPT_ENGINE

typedef void (*ScriptSetNativeInterfacePointerFunction)(const ScriptNativeInterface *s);
extern const ScriptNativeInterface _scriptNativeInterface;

#else

#define ScriptCreateHandle(...)       (scriptNativeInterface->CreateHandle      (__VA_ARGS__))
#define ScriptCreateString(...)       (scriptNativeInterface->CreateString      (__VA_ARGS__))
#define ScriptCreateStruct(...)       (scriptNativeInterface->CreateStruct      (__VA_ARGS__))
#define ScriptHeapRefClose(...)       (scriptNativeInterface->HeapRefClose      (__VA_ARGS__))
#define ScriptListCount(...)          (scriptNativeInterface->ListCount         (__VA_ARGS__))
#define ScriptParameterBool(...)      (scriptNativeInterface->ParameterBool     (__VA_ARGS__))
#define ScriptParameterCString(...)   (scriptNativeInterface->ParameterCString  (__VA_ARGS__))
#define ScriptParameterDouble(...)    (scriptNativeInterface->ParameterDouble   (__VA_ARGS__))
#define ScriptParameterHandle(...)    (scriptNativeInterface->ParameterHandle   (__VA_ARGS__))
#define ScriptParameterHeapRef(...)   (scriptNativeInterface->ParameterHeapRef  (__VA_ARGS__))
#define ScriptParameterInt32(...)     (scriptNativeInterface->ParameterInt32    (__VA_ARGS__))
#define ScriptParameterInt64(...)     (scriptNativeInterface->ParameterInt64    (__VA_ARGS__))
#define ScriptParameterScan(...)      (scriptNativeInterface->ParameterScan     (__VA_ARGS__))
#define ScriptParameterString(...)    (scriptNativeInterface->ParameterString   (__VA_ARGS__))
#define ScriptParameterUint32(...)    (scriptNativeInterface->ParameterUint32   (__VA_ARGS__))
#define ScriptParameterUint64(...)    (scriptNativeInterface->ParameterUint64   (__VA_ARGS__))
#define ScriptReturnBoxInError(...)   (scriptNativeInterface->ReturnBoxInError  (__VA_ARGS__))
#define ScriptReturnDouble(...)       (scriptNativeInterface->ReturnDouble      (__VA_ARGS__))
#define ScriptReturnError(...)        (scriptNativeInterface->ReturnError       (__VA_ARGS__))
#define ScriptReturnHandle(...)       (scriptNativeInterface->ReturnHandle      (__VA_ARGS__))
#define ScriptReturnHeapRef(...)      (scriptNativeInterface->ReturnHeapRef     (__VA_ARGS__))
#define ScriptReturnInt(...)          (scriptNativeInterface->ReturnInt         (__VA_ARGS__))
#define ScriptReturnString(...)       (scriptNativeInterface->ReturnString      (__VA_ARGS__))
#define ScriptReturnStruct(...)       (scriptNativeInterface->ReturnStruct      (__VA_ARGS__))
#define ScriptReturnStructInl(...)    (scriptNativeInterface->ReturnStructInl   (__VA_ARGS__))
#define ScriptRunCallback(...)        (scriptNativeInterface->RunCallback       (__VA_ARGS__))
#define ScriptStructReadBool(...)     (scriptNativeInterface->StructReadBool    (__VA_ARGS__))
#define ScriptStructReadDouble(...)   (scriptNativeInterface->StructReadDouble  (__VA_ARGS__))
#define ScriptStructReadHandle(...)   (scriptNativeInterface->StructReadHandle  (__VA_ARGS__))
#define ScriptStructReadHeapRef(...)  (scriptNativeInterface->StructReadHeapRef (__VA_ARGS__))
#define ScriptStructReadInt32(...)    (scriptNativeInterface->StructReadInt32   (__VA_ARGS__))
#define ScriptStructReadInt64(...)    (scriptNativeInterface->StructReadInt64   (__VA_ARGS__))
#define ScriptStructReadString(...)   (scriptNativeInterface->StructReadString  (__VA_ARGS__))
#define ScriptStructReadUint32(...)   (scriptNativeInterface->StructReadUint32  (__VA_ARGS__))
#define ScriptStructReadUint64(...)   (scriptNativeInterface->StructReadUint64  (__VA_ARGS__))
#define ScriptStructWriteDouble(...)  (scriptNativeInterface->StructWriteDouble (__VA_ARGS__))
#define ScriptStructWriteHandle(...)  (scriptNativeInterface->StructWriteHandle (__VA_ARGS__))
#define ScriptStructWriteHeapRef(...) (scriptNativeInterface->StructWriteHeapRef(__VA_ARGS__))
#define ScriptStructWriteInt(...)     (scriptNativeInterface->StructWriteInt    (__VA_ARGS__))
#define ScriptStructWriteString(...)  (scriptNativeInterface->StructWriteString (__VA_ARGS__))

const ScriptNativeInterface *scriptNativeInterface;
LIBRARY_EXPORT void ScriptSetNativeInterfacePointer(const ScriptNativeInterface *s) { scriptNativeInterface = s; }

#endif
