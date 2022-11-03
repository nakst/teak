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

Now we need to write the native code for the `Hello` function. The scripting engine will look for the function `ScriptExtHello` in our library. It will pass one parameter, a pointer to the engine's internal data structures, and will expect a return value of a `bool`. Here's the contents of `my_module.c`:

```c
#include <stdio.h>
#include <stdbool.h>

bool ScriptExtHello(void *engine) {
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
bool ScriptExtSum(void *engine) {
	return true;
}
```

But this time we need to read the function parameters from the engine and set the return value. Declare the following functions; these are implemented by the scripting engine.

```c
bool ScriptParameterInt64(void *engine, int64_t *output);
bool ScriptReturnInt(void *engine, int64_t input);
```

To read the parameters to the function, we simply call `ScriptParameterInt64` for each, and it will return the parameters from left to right. If it returns `false`, then we'll need to return `false` from our function to allow the error to propagate. To set the return value, we call `ScriptReturnInt`. Make sure that you only set the return value once!

```c
bool ScriptExtSum(void *engine) {
    int64_t left, right;
    if (!ScriptParameterInt64(engine, &left )) return false;
    if (!ScriptParameterInt64(engine, &right)) return false;
    int64_t result = left + right;
    if (!ScriptReturnInt(engine, result)) return false;
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
bool ScriptParameterBool(void *engine, bool *output);
bool ScriptParameterInt32(void *engine, int32_t *output);
bool ScriptParameterInt64(void *engine, int64_t *output);
bool ScriptParameterUint32(void *engine, uint32_t *output);
bool ScriptParameterUint64(void *engine, uint64_t *output);
bool ScriptReturnInt(void *engine, int64_t input);
```

### Floating point values

Floating point values can be read and returned with the following functions:

```c
bool ScriptParameterDouble(void *engine, double *output);
bool ScriptReturnDouble(void *engine, double input);
```

### Strings

Strings can be read and returned with the following functions:

```c
// The returned pointer must be freed using free().
bool ScriptParameterCString(void *engine, char **output);

// The returned pointer is valid until you return from the extcall function.
bool ScriptParameterString(void *engine, void **output, size_t *outputBytes);

// The pointer you pass in only needs to remain valid for the call to ScriptReturnString.
bool ScriptReturnString(void *engine, const void *input, size_t inputBytes);
```

### Errors

If you're implementing a function that returns a type wrapped in an `err[...]` box, exactly one of the following functions should be called before returning.

```c
// After setting the return value as normal, call this function to box it in an error.
bool ScriptReturnBoxInError(void *engine);

// If you want to return an error, call this function.
// Do this instead of calling ScriptReturnInt or whatever.
// The message pointer you pass in only needs to remain valid for the call to ScriptReturnError.
bool ScriptReturnError(void *engine, const char *message);
```

## Handle types

In the module script, you can define `handletype`s, which can be used for storing opaque value needed by the native code. There is no way to produce values of `handletype` in scripts; they must originate from native code.

```java
handletype Object;

// Inherit from Object. A SpecialObject can then be used in place of a normal Object, but not vice-versa.
handletype SpecialObject : Object; 
```

The native code can read handle parameters using:

```c
bool ScriptParameterHandle(void *engine, void **output);
```

This returns the pointer associated with the handle when it was created. To create and return a handle, using `ScriptReturnHandle`:

```c
bool ScriptReturnHandle(void *engine, void *handleData, void (*close)(void *));
```

You must specify a function to call when the handle is deallocated. Deallocation is done whenever the garbage collector deems it necessary, so don't expect this to happen in any sort of timely manner; but it will certainly be called if the script exits normally.

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
