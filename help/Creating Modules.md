# Creating Modules

You can call into C code from scripts using modules and the `#extcall` system.

## Basic example

Start by creating a script file for the module. When another script wants to use it, they can `#import` the script file. 

At the top of the script file, put `#library` followed by the name of the shared library that the native code will be stored in. For example, `#library "my_module";` will load `lmy_module.so` on Linux and `my_module.dll` on Windows.

Next, put the prototypes of the functions you will implement in the native code. Instead of a function body, put `#extcall;`. For example, here's the script we'll be starting with:

```java
#library "my_module";
void Hello() #extcall;
```

Now we need to write the native code for the `Hello` function. The scripting engine will look for the function `ScriptExtHello` in our library. It will pass one parameter, a pointer to the engine's internal data structures, and will expect a return value of a `bool`. You also need to include the native interface header file. Here's the contents of `my_module.c`:

```c
#include <stdio.h>
#include <stdbool.h>

#include "modules/native_interface.h"

bool ScriptExtHello(struct ExecutionContext *context) {
	printf("Hello!\n");
	return true; // true indicates success.
}
```

And that's all we need to implement this function. We can now compile the native code into a shared library as follows:

```sh
gcc -o lmy_module.so -shared my_module.c -fPIC
```

Finally, let's call into the module with a test script.

```java
#import "my_module.esh" mod;

void Start() {
	mod.Hello();
}
```

Make sure to run the scripting engine from the same directory containing the shared library, or it won't be able to find it. Running the test script should result in `Hello!` being output.

## Parameters and return values

Let's add a new function that takes two integers as parameters and return their sum. First, the prototype:

```java
int Sum(int x, int y) #extcall;
```

The native implementation of the function starts like before:

```c
bool ScriptExtSum(struct ExecutionContext *context) {
	return true;
}
```

But this time we need to read the function parameters from the engine and set the return value. Declare the following functions; these are implemented by the scripting engine.

```c
bool ScriptParameterInt64(struct ExecutionContext *context, int64_t *output);
bool ScriptReturnInt(struct ExecutionContext *context, int64_t input);
```

To read the parameters to the function, we simply call `ScriptParameterInt64` for each, and it will return the parameters from left to right. If it returns `false`, then we'll need to return `false` from our function to allow the error to propagate. To set the return value, we call `ScriptReturnInt`. Make sure that you only set the return value once!

```c
bool ScriptExtSum(struct ExecutionContext *context) {
    int64_t left, right;
    if (!ScriptParameterInt64(context, &left )) return false;
    if (!ScriptParameterInt64(context, &right)) return false;
    int64_t result = left + right;
    if (!ScriptReturnInt(context, result)) return false;
    return true;
}
```

Adding the following to our test script,

```java
	int x = mod.Sum(2, 8);
	Log("%x%");
```

We see that `10` is output, as expected.

### Integers and booleans

Integers and booleans can be read and returned with the following functions:

```c
bool ScriptParameterBool(struct ExecutionContext *context, bool *output);
bool ScriptParameterInt32(struct ExecutionContext *context, int32_t *output);
bool ScriptParameterInt64(struct ExecutionContext *context, int64_t *output);
bool ScriptParameterUint32(struct ExecutionContext *context, uint32_t *output);
bool ScriptParameterUint64(struct ExecutionContext *context, uint64_t *output);
bool ScriptReturnInt(struct ExecutionContext *context, int64_t input);
```

### Floating point values

Floating point values can be read and returned with the following functions:

```c
bool ScriptParameterDouble(struct ExecutionContext *context, double *output);
bool ScriptReturnDouble(struct ExecutionContext *context, double input);
```

### Strings

Strings can be read and returned with the following functions:

```c
// The returned pointer must be freed using free().
bool ScriptParameterCString(struct ExecutionContext *context, char **output);

// The returned pointer is valid until you return from the extcall function.
bool ScriptParameterString(struct ExecutionContext *context, const void **output, size_t *outputBytes);

// The pointer you pass in only needs to remain valid for the call to ScriptReturnString.
bool ScriptReturnString(struct ExecutionContext *context, const void *input, size_t inputBytes);
```

### Errors

If you're implementing a function that returns a type wrapped in an `err[...]` box, exactly one of the following functions should be called before returning.

```c
// After setting the return value as normal, call this function to box it in an error.
bool ScriptReturnBoxInError(struct ExecutionContext *context);

// If you want to return an error, call this function.
// Do this instead of calling ScriptReturnInt or whatever.
// The message pointer you pass in only needs to remain valid for the call to ScriptReturnError.
bool ScriptReturnError(struct ExecutionContext *context, const char *message);
```

## Handle types

In the module script, you can define `handletype`s, which can be used for storing opaque value needed by the native code. There is no way to produce values of `handletype` in scripts; they must originate from native code. That said, a `handletype` variable can be set to `null`.

```java
handletype Object;

// Inherit from Object. A SpecialObject can then be used in place of a normal Object, but not vice-versa.
handletype SpecialObject : Object; 
```

The native code can read handle parameters using:

```c
bool ScriptParameterHandle(struct ExecutionContext *context, void **output);
```

This returns the pointer associated with the handle when it was created. If the handle was `null`, `NULL` is returned. To create and return a handle, using `ScriptReturnHandle`:

```c
bool ScriptReturnHandle(struct ExecutionContext *context, void *handleData, void (*close)(void *));
```

You must specify a function to call when the handle is deallocated. Deallocation is done whenever the garbage collector deems it necessary, so don't expect this to happen in any sort of timely manner; but it will certainly be called if the script exits normally. If the `handleData` is set to `NULL`, then a `null` will be returned to the script, and the `close` callback will be unused.

```c
// TODO Document this.
intptr_t ScriptCreateHandle(struct ExecutionContext *context, void *handleData, void (*close)(void *));
```

## The Initialise function

When the module is imported, if an `Initialise` function is defined it will be called. It will be called before any other functions in the module are executed; and it will be called after the `Initialise` functions of any modules it imports are executed. For example, 

```java
#library "my_module";
void Hello() #extcall;
int Sum(int x, int y) #extcall;

void Initialise() { 
    Log("This will be printed before anything else!"); 
}
```

## Calling back into the script

```c
// TODO Document this.
bool ScriptRunCallback(struct ExecutionContext *context, intptr_t functionPointer, int64_t *parameters, bool *managedParameters, size_t parameterCount);
```

## References to heap objects

```c
// TODO Document these.

bool ScriptParameterHeapRef(struct ExecutionContext *context, intptr_t *output);
bool ScriptHeapRefClose(struct ExecutionContext *context, intptr_t index);
bool ScriptReturnHeapRef(struct ExecutionContext *context, intptr_t index);

bool ScriptStructReadInt32(ExecutionContext *context, intptr_t index, uintptr_t fieldIndex, int32_t *output);
```

## An quicker way to read parameters

```c
// TODO Document this.
bool ScriptParameterScan(struct ExecutionContext *context, const char *cFormat, ...);
```
