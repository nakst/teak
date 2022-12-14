# Base Module

The base module is automatically imported into all scripts. You don't need to use a prefix to use it.

*Please keep in mind that some of these functions may be modified slightly as the language is not yet stable.*

## Strings and characters — conversions

```c
// Convert string to uppercase or lowercase.
// These treat the strings as ASCII, mapping between [a-z] and [A-Z] only.
str StringToLowerRaw(str s);
str StringToUpperRaw(str s);

// Remove whitespace characters from the start and end of the string.
// The whitespace characters are: space, tab, carriage return and newline (ASCII).
str StringTrim(str s);

// Parse the string as a floating point number.
// The error string "INVALID_FORMAT" indicates it was not a floating point number.
err[float] StringParseFloat(str s);

// *Deprecated* Use int str:byte(int index) instead.
// Extract the byte value (0-255) of the one byte string.
// If the string is longer than a byte, the first byte is used.
// If the string is empty, -1 is returned.
int CharacterToByte(str x);

// *Deprecated* Use str int:str() instead.
// Convert the byte value (0-255) to a one byte string.
str StringFromByte(int x);
```

## Strings and characters — UTF-8

The functions `StringUTF8Advance`, `StringUTF8Retreat` and `StringUTF8Decode` take a byte offset into the string. `-1` is used as a silent error value when something goes wrong. So it is always safe to do, for example, `StringUTF8Decode(x, StringUTF8Advance(x, position))`. Even if the `StringUTF8Advance` fails, it will return `-1` which informs `StringUTF8Decode` to simply return a Unicode replacement character.

```c
// Count the number of UTF-8 codepoints in the string.
// If the string cannot be traversed, -1 is returned.
// (This is not a substitute for a proper validation.)
int StringUTF8Count(str x);

// Get the starting byte index of the first UTF-8 codepoint after the codepoint starting at byte i.
// If i is the last codepoint in the string, x:len() is returned.
// If i == -1, then -1 is returned.
// If the string cannot be traversed, -1 is returned.
// If i < -1 || i >= x:len(), the script will crash.
int StringUTF8Advance(str x, int i);

// Get the starting byte index of the first UTF-8 codepoint before the codepoint starting at byte i.
// If i == x:len(), the starting byte index of the last UTF-8 codepoint in the string is returned.
// If i == -1, then -1 is returned.
// If the string cannot be traversed, -1 is returned.
// If i == 0 || i < -1 || i > x:len(), the script will crash.
int StringUTF8Retreat(str x, int i);

// Decode the UTF-8 codepoint starting at byte i.
// If i == -1 or the codepoint is invalid, 0xFFFD is returned.
int StringUTF8Decode(str x, int i);

// Encode a value from 0 to 0x1FFFFF as a UTF-8 string.
str StringUTF8Encode(int value);
```

## Strings and characters — testing

Important: A string is an immutable, ordered collection of bytes. A "character" refers to a one byte string. Various character functions will assume that this byte represents an ASCII character; this is usually the behaviour you want because UTF-8 strings encode the codepoint range 0-127 as a single byte with that value.

```c
// Returns whether the one byte string is a alphanumeric ASCII character.
// That is, whether it is in one of the ranges [a-z], [A-Z], [0-9].
bool CharacterIsAlnum(str c);

// Returns whether the one byte string is a whitespace ASCII character.
// That is, whether it is one of ' ', '\r', '\n' or '\t'.
bool CharacterIsSpace(str c);

// Returns whether the one byte string is a digit ASCII character.
// That is, whether it is in the range [0-9].
bool CharacterIsDigit(str c);

// Returns whether the string s starts with the string x.
bool StringStartsWith(str s, str x);

// Returns whether the string s ends with the string x.
bool StringEndsWith(str s, str x);
```

## Strings and characters — searching

```c
// Returns whether the haystack string contains the needle somewhere in it.
bool StringContains(str haystack, str needle);

// Returns the first index of the needle string in the haystack string.
// If it is not found, -1 is returned.
// The needle must be non-empty.
// startAt gives the first index to check.
// If reverse is false, each iteration moves the checked index one byte forwards.
// If reverse is true, each iteration moves the checked index one byte backwards.
// If the index goes (or starts) outside the bounds [0, haystack:len()-1], -1 is returned.
int StringFind(str haystack, str needle, int startAt, bool reverse);

// Finds the starting index in the haystack string of the first occurence of the needle string.
// If it is not found, -1 is returned. The needle must be non-empty.
// Equivalent to StringFind(haystack, needle, 0, false).
int StringFindFirst(str haystack, str needle);

// Finds the starting index in the haystack string of the last occurence of the needle string.
// If it is not found, -1 is returned. The needle must be non-empty.
// Equivalent to StringFind(haystack, needle, haystack:len() - 1, true).
int StringFindLast(str haystack, str needle);
```

## Strings and characters -- replacing

```c
// Replace the first instance of the needle string in the haystack string with the with string.
// Or, if it is not found, return the haystack string unchanged.
str StringReplaceFirst(str haystack, str needle, str with);

// Replace the last instance of the needle string in the haystack string with the with string.
// Or, if it is not found, return the haystack string unchanged.
str StringReplaceLast(str haystack, str needle, str with);

// Replace all instances of the needle string in the haystack string with the with string.
str StringReplaceAll(str haystack, str needle, str with);

// Replace all instances of the needle string in the haystack string with the with string.
// This also returns the number of replacements made.
tuple[str, int] StringReplaceAllWithCount(str haystack, str needle, str with);
```

## Strings and characters — joining and splitting

```c
// *Deprecated* Use str str:slice(int start, int end) instead.
// Returns a range of the bytes in the string.
str StringSlice(str x, int start, int end);

// Concatenate the list of strings into a single string.
// The string k is used as a delimiter between joined strings.
// If trailing is true, k is appended at the end, as well as between strings.
str StringJoin(str[] strings, str k, bool trailing);

// Split the string into an list of strings.
// The character string should be a single byte, giving the delimeter character.
// If includeEmptyString is false, then empty strings are not included in the output list.
str[] StringSplitByCharacter(str string, str character, bool includeEmptyString);

// Split the string into two parts, the string before the first instance of the needle,
// and the string after the first instance of the needle. If the needle is not found,
// the second part will be the empty string. The needle must not be the empty string.
// For example, StringSplitFirst("a,b,c,d", ",") returns "a" and "b,c,d".
tuple[str, str] StringSplitFirst(str string, str needle);

// Identical to StringSplitFirst except the last instance of the needle is used.
// For example, StringSplitLast("a,b,c,d", ",") returns "a,b,c" and "d".
tuple[str, str] StringSplitLast(str string, str needle);

// If the string starts with the given prefix, it is removed. Otherwise, the original string is returned.
str StringRemoveOptionalPrefix(str s, str prefix);

// If the string ends with the given suffix, it is removed. Otherwise, the original string is returned.
str StringRemoveOptionalSuffix(str s, str suffix);

// Repeats the string multiple times. n >= 0.
str StringRepeat(str s, int n);
```

## Strings and characters — file system paths

```c
// If the input string does not end with a directory separator, then one is appended.
str PathEnsureTrailingSlash(str x);

// if the input string ends with a directory separator, one is removed.
str PathTrimTrailingSlash(str x);
```

## Logging

```c
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

## Text formatting

These functions return strings that can be concatenated with other strings to add formatting when using the logging functions.

For example, `Log("Hello, %TextBold()%world%TextPlain()%.")` will display the word "world" in bold.

```c
str TextBold();
str TextColorError();
str TextColorHighlight();
str TextMonospaced();
str TextPlain();
str TextWeight(int i); // e.g. 700 = bold
```

## File system — directory enumeration

```c
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

## File system — files

```c
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

## File system — copying

```c
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

## File system — deleting

```c
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

## File system — detecting files

```c
// Return true if an item exists at the given path.
bool PathExists(str x);

// Return true if an item exists at the given path and it is a directory.
bool PathIsDirectory(str source);

// Return true if an item exists at the given path and it is a file.
bool PathIsFile(str source);

// Return true if an item exists at the given path and it is a symbolic link.
bool PathIsLink(str source);
```

## File system — creating directories

```c
// Create a directory at the given path.
// It is not considered an error if the directory already exists.
err[void] PathCreateDirectory(str x);

// Create a directory at the given path, and any leading directories as necessary.
err[void] PathCreateLeadingDirectories(str path);
```

## File system — default prefix

```c
// Get the current prefix that is being appended to all non-absolute paths.
// By default on Unix and Windows, this is the "current directory".
str PathGetDefaultPrefix();

// Set the prefix to match the directory that contains the main source file.
// (That is, the one the script engine was passed when it was invoked.)
err[void] PathSetDefaultPrefixToScriptSourceDirectory();

// Append the default prefix to the path if necessary,
// and possibly also perform some normalization depending on the system.
str PathToAbsolute(str x);
```

## File system — other

```c
// Remove the last component from the path.
str PathGetLeadingDirectories(str path);

// Move the item from the source path to the destination path.
// You must specify the destination name; giving a directory as the destination will not work!
err[void] PathMove(str source, str destination);
```

## File system — errors

You may encounter some of the following error strings when using file system functions:

- `"OPERATION_BLOCKED"` Another application is accessing the file.
- `"DIRECTORY_NOT_EMPTY"` The directory cannot be operated on because it is not empty.
- `"FILE_TOO_LARGE"` The underlying file system does not support files this large.
- `"DRIVE_FULL"` The drive is out of free space.
- `"FILE_ON_READ_ONLY_VOLUME"` The file cannot be modified because the file system is mounted as read-only.
- `"INCORRECT_NODE_TYPE"` An attempt was made to perform a directory operation on a file, or a file operation on a directory.
- `"FILE_DOES_NOT_EXIST"` The item does not exist.
- `"ALREADY_EXISTS"` The item already exists.
- `"INSUFFICIENT_RESOURCES"` The system does not have enough resources to perform the operation. Typically, this indicates an out-of-memory condition.
- `"PERMISSION_NOT_GRANTED"` You do not have permission to perform the operation on the file.
- `"VOLUME_MISMATCH"` Two items specified for the operation reside on different file system volumes, which the operation does not allow.
- `"HARDWARE_FAILURE"` The operation could not complete because of a hardware malfunction.
- `"PATH_NOT_TRAVERSABLE"` A non-final directory in the path did not exist, or the drive did not exist.

## System — shell commands

These functions are only available on platforms that have a command line shell.

```c
// Execute the specified command, returning true if it succeeded.
// Its working directory will be the same as the default path prefix of the script.
bool SystemShellExecute(str x);

// Execute the specified command, returning true if it succeeded, with the specified working directory.
bool SystemShellExecuteWithWorkingDirectory(str wd, str x);

// Execute the specified command, returning its output as a string.
str SystemShellEvaluate(str x);

// Execute the specified command, returning its output as a string.
// If the command fails (the command returns a non-zero exit status), the boolean returned is false.
tuple[str, bool] SystemShellEvaluate2(str x);

// Enable or disable logging of each command that is run.
void SystemShellEnableLogging(bool x);
```

## System — console

These functions are only available on platforms that use a terminal emulator.

```c
// Get a line of input from the user.
str ConsoleGetLine();

// Write to the stderr output stream.
void ConsoleWriteStderr(str x);

// Write to the stdout output stream.
void ConsoleWriteStdout(str x);
```

## System — environment variables

These functions are only available on platforms that support environment variables.

```c
err[str] SystemGetEnvironmentVariable(str name);
err[void] SystemSetEnvironmentVariable(str name, str value);
```

## System — information

```c
// Get the host platform name, such as "Linux" or "Essence".
str SystemGetHostName();

// Returns the number of processors on the system, or the ideal number of jobs that it should run in parallel.
int SystemGetProcessorCount();

// Returns true if the script is running with additional permissions.
bool SystemRunningAsAdministrator();
```

## Integer mathematics

```c
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

## Floating point mathematics

```c
// Apply x to the unique linear map that sends fromStart to toStart and fromEnd to toEnd.
float MathLinearRemap(float x, float fromStart, float fromEnd, float toStart, float toEnd);

// Return the sign of x.
// That is, 1.0 for positive x, -1.0 for negative x, 0.0 for 0.0.
float MathSign(float x);

// Return the absolute value of x.
float FloatAbsolute(float x);

// Return the larger (closer to plus infinity) value of a and b.
float FloatMaximum(float x, float y);

// Return the smaller (closer to minus infinity) value of a and b.
float FloatMinimum(float x, float y);

// Clamp x between min and max.
// min must be less than or equal to max.
float FloatClamp(float x, float min, float max);

// Clamp x between 0.0 and 1.0.
float FloatClamp01(float x);

// Get a randomized float between min and max (inclusive).
float RandomFloat(float min, float max);

// Get a randomized float between 0.0 and 1.0 (inclusive).
float RandomFloat01();
```

## Other

```c
// Set the path to use for storing and loding #persist variables.
bool PersistRead(str path);

// Pause execution of the active coroutine for the specified number of milliseconds.
void SystemSleepMs(int ms);

// Immediately terminate the current process with the provided exit code.
void SystemExit(int exitCode);

// Generate a UUID as a string.
str UUIDGenerate();
```
