# esh

A small scripting language to replace Unix shell scripts.

## Building

For the first build, run `./build.sh`.

For subsequent builds, run `./esh build.esh`. For a debug build, run `./esh build.esh debug=true`.

## Running examples

For example, run `./esh examples/hello_world.esh`.

## Documentation

See `examples/basic_usage.esh` for a whirlwind tour of the language's syntax.

TODO Make an example demonstrating the standard library.

TODO Make formal documentation.

TODO Discuss: handle types, the Initialise function, modules, extension libraries.

## Rationale

Why did I make another programming language? Because I felt like there wasn't a good statically typed language to compete with Unix shell scripts. That is, something lightweight and designed for simple file operations and launching other processes. If you want something dynamically typed, you're probably better off with Python, for example.

## Roadmap

- Make documentation.
- Make examples.
- Make test cases.
- Add features listed at the top of esh.c.
- Make a REPL so people can use it as a command line shell?

## Standard library

*Please keep in mind that some of these functions may be modified slightly as the language in not yet stable.*

### Strings and characters — conversions

```
// Extract the byte value (0-255) of the unit length string.
int CharacterToByte(str x);

// Convert string to uppercase or lowercase.
// These treat the strings as ASCII, mapping between [a-z] and [A-Z] only.
str StringToLowerRaw(str s);
str StringToUpperRaw(str s);

// Remove whitespace characters from the start and end of the string.
// The whitespace characters are: space, tab, carriage return and newline (ASCII).
str StringTrim(str string);
```

### Strings and characters — testing

```
// Returns whether the first byte in the string is a alphanumeric ASCII character.
// That is, whether it is in one of the ranges [a-z], [A-Z], [0-9].
bool CharacterIsAlnum(str c);

// Returns whether the haystack strings contains the needle somewhere in it.
bool StringContains(str haystack, str needle);

// Returns whether the string s starts with the string x.
bool StringStartsWith(str s, str x);

// Returns whether the string s ends with the string x.
bool StringEndsWith(str s, str x);
```

### Strings and characters — joining and splitting

```
// Returns a range of the bytes in the string.
str StringSlice(str x, int start, int end);

// Concatenate the list of strings into a single string.
// The string k is used as a delimiter between joined strings.
// If trailing is true, k is appended at the end, as well as between strings.
str StringJoin(str[] strings, str k, bool trailing);

// Split the strings into an list of strings.
// The character string should be a single byte, giving the delimeter character.
// If includeEmptyString is false, then empty strings are not included in the output list.
str[] StringSplitByCharacter(str string, str character, bool includeEmptyString);
```

### Strings and characters — file system paths

```
// If the input string does not end with a directory separator, then one is appended.
str PathEnsureTrailingSlash(str x);

// if the input string ends with a directory separator, one is removed.
str PathTrimTrailingSlash(str x);
```

### Logging

```
// Log a message in the default color.
void Log(str x); 

// Log a message in the error color.
void LogError(str x); 

// Log a message in the info color.
void LogInfo(str x); 

// Open a group of log messages.
// Groups can be nested.
void LogOpenGroup(str title);

// Close the most recently opened log group.
void LogClose();
```

### Text formatting

These functions return strings that can be concatenated with other strings to add formatting when using the logging functions.

For example, `Log("Hello, %TextBold()%world%TextPlain()%.")` will display the word "world" in bold.

```
str TextBold();
str TextColorError();
str TextColorHighlight();
str TextMonospaced();
str TextPlain();
str TextWeight(int i); // e.g. 700 = bold
```

### File system — directory enumeration

```
// Get the list of names of children of the directory at the given path.
err[str[]] DirectoryEnumerate(str path);

// Same as above, but recurse into directories.
// To get a full path for one of the outputs, you need to prepend the input path.
// For example, 
// 	for str relativePath in DirectoryEnumerateRecursively(path):default(new str[]) {
//		str absolutePath = PathEnsureTrailingSlash(path) + relativePath;
//		Log(absolutePath);
//	}
err[str[]] DirectoryEnumerateRecursively(str path);

// Same as above, but the output list is filtered.
// If prependSourceDirectory is true, then PathEnsureTrailingSlash(sourceDirectory) is prepended to each element in the output list.
// The filter is a list of strings giving what names should be matched in each subdirectory.
// For example, [ "b*", "*.txt" ] would match all files with names ending in ".txt" that reside in directories whose names begin with "b".
// If filterWildcard is not -1, then arbitrarily many directories can exist between filter[filterWildcard] and filter[filterWildcard + 1].
// TODO Explain this better.
err[str[]] DirectoryEnumerateFiltered(str sourceDirectory, str[] filter, int filterWildcard, bool prependSourceDirectory);
```

### File system — files

```
// Read the file at the given path into a string.
err[str] FileReadAll(str path);

// Replace the file at the given path with the contents of the string x.
// (If the file did not already exist, it is created.)
err[void] FileWriteAll(str path, str x);

// Append the contents of the string x at the end of the file at the given path.
// (If the file did not already exist, it is created.)
err[void] FileAppend(str path, str x);

// Create the file if it does not already exist, and update its modification timestamp.
err[void] FileTouch(str path);

// Get the size of the file in bytes at the given path.
err[int] FileGetSize(str path);

// Copy a file from the source path to the destination path.
// You must specify the destination name; giving a directory as the destination will not work!
err[void] FileCopy(str source, str destination);
```

### File system — copying

```
// Copy the directory at source and all of the files it contains recursively to the destination directory.
err[void] PathCopyRecursively(str source, str destination);

// Copy the single file or directory in the source directory into the destination directory.
// That is, the source path is sourceDirectory + item, and the destination path is destinationDirectory + item.
// If the source path points to a directory, then a directory is created at the destination path.
// If the source path points to a file, then that file will be copied to the destination path.
err[void] PathCopyInto(str sourceDirectory, str item, str destinationDirectory);

// The same as PathCopyInto, except it is repeated for each of the items in the list.
err[void] PathCopyAllInto(str sourceDirectory, str[] items, str destinationDirectory);

// The same as PathCopyFilteredInto, but the list of items is constructed using DirectoryEnumerateFiltered.
err[void] PathCopyFilteredInto(str sourceDirectory, str[] filter, int filterWildcard, str destinationDirectory);
```

### File system — deleting

```
// Delete the file or directory at the given path.
// If a directory is specified, then it must be empty.
err[void] PathDelete(str x);

// The same as PathDelete, except it is repeated for each of the items in the list.
void PathDeleteAll(str[] x);

// The same as PathDeleteAll, but the list of items is constructed using DirectoryEnumerateFiltered.
void PathDeleteAllFiltered(str sourceDirectory, str[] filter, int filterWildcard);

// Delete the directory at the given path and all the files and directories it contains recursively.
err[void] PathDeleteRecursively(str path);
```

### File system — detecting files

```
// Return true if an item exists at the given path.
bool PathExists(str x);

// Return true if an item exists at the given path and it is a directory.
bool PathIsDirectory(str source);

// Return true if an item exists at the given path and it is a file.
bool PathIsFile(str source);

// Return true if an item exists at the given path and it is a symbolic link.
bool PathIsLink(str source);
```

### File system — creating directories

```
// Create a directory at the given path.
err[void] PathCreateDirectory(str x);

// Create a directory at the given path, and any leading directories as necessary.
err[void] PathCreateLeadingDirectories(str path);
```

### File system — default prefix

```
// Get the current prefix that is being appended to all non-absolute paths.
str PathGetDefaultPrefix();

// Set the prefix to match the directory that contains the main source file.
// (That is, the one the script interpreter was passed when it was created.)
err[void] PathSetDefaultPrefixToScriptSourceDirectory();
```

### File system — other

```
// Remove the last component from the path.
str PathGetLeadingDirectories(str path);

// Move the item from the source path to the destination path.
// You must specify the destination name; giving a directory as the destination will not work!
err[void] PathMove(str source, str destination);
```

### System — shell commands

These functions are only available on platforms that have a command line shell.

```
// Execute the specified command, returning true if it succeeded.
// Its working directory will be the same as the default path prefix of the script.
bool SystemShellExecute(str x);

// Execute the specified command, returning true if it succeeded, with the specified working directory.
bool SystemShellExecuteWithWorkingDirectory(str wd, str x);

// Execute the specified command, returning its output as a string.
str SystemShellEvaluate(str x);

// Enable or disable logging of each command that is run.
void SystemShellEnableLogging(bool x);
```

### System — console

These functions are only available on platforms that use a terminal emulator.

```
// Get a line of input from the user.
str ConsoleGetLine();

// Write to the stderr output stream.
void ConsoleWriteStderr(str x);

// Write to the stdout output stream.
void ConsoleWriteStdout(str x);
```

### System — environment variables

These functions are only available on platforms that support environment variables.

```
err[str] SystemGetEnvironmentVariable(str name);
err[void] SystemSetEnvironmentVariable(str name, str value);
```

### System — information

```
// Get the host platform name, such as "Linux" or "Essence".
str SystemGetHostName();

// Returns the number of processors on the system, or the ideal number of jobs that it should run in parallel.
int SystemGetProcessorCount();

// Returns true if the script is running with additional permissions.
bool SystemRunningAsAdministrator();
```

### Integer mathematics

```
// Return the absolute value of x.
int IntegerAbsolute(int x);

// Return the larger (closer to plus infinity) value of a and b.
int IntegerMaximum(int a, int b);

// Return the smaller (closer to minus infinity) value of a and b.
int IntegerMinimum(int a, int b);

// Clamp x between min and max.
// min must be less than or equal to max.
int IntegerClamp(int x, int min, int max);

// Returns r such that 0 <= r < y, and (x - r) is a multiple of y.
// y >= 1. Works with negative x.
int IntegerModulo(int x, int y);

// Count the number of set bits in the integer.
int IntegerCountOneBits(int x);

// Count the number of consecutive clear bits in the integer, starting from the least significant bit.
int IntegerCountLeastSignificantZeroBits(int x);

// Count the number of consecutive set bits in the integer, starting from the least significant bit.
int IntegerCountLeastSignificantOneBits(int x);

// Count the number of consecutive clear bits in the integer, starting from the most significant bit.
int IntegerCountMostSignificantZeroBits(int x);

// Count the number of consecutive set bits in the integer, starting from the most significant bit.
int IntegerCountMostSignificantOneBits(int x);

// Get a randomized integer between min and max (inclusive).
int RandomInt(int min, int max);
```

### Other

```
// Set the path to use for storing and loding #persist variables.
bool PersistRead(str path);

// Pause execution of the active coroutine for the specified number of milliseconds.
void SystemSleepMs(int ms);

// Generate a UUID as a string.
str UUIDGenerate();
```
