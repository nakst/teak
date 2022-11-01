/* generated by build.esh */
"// Logging:\n"
"\n"
"void Log(str x) #extcall;\n"
"void LogInfo(str x) { Log(TextColorHighlight() + x); }\n"
"void LogError(str x) { Log(TextColorError() + \"Error: \" + x); }\n"
"\n"
"void LogOpenGroup(str title) #extcall;\n"
"// TODO LogOpenList, LogOpenTable, etc.\n"
"void LogClose() #extcall;\n"
"\n"
"str TextColorError() #extcall;\n"
"str TextColorHighlight() #extcall;\n"
"str TextWeight(int i) #extcall;\n"
"str TextMonospaced() #extcall;\n"
"str TextPlain() #extcall;\n"
"str TextBold() { return TextWeight(700); }\n"
"\n"
"// String operations:\n"
"\n"
"str StringSlice(str x, int start, int end) #extcall;\n"
"int CharacterToByte(str x) #extcall;\n"
"str StringFromByte(int x) #extcall;\n"
"\n"
"bool StringContains(str haystack, str needle) {\n"
"\tfor int i = 0; i <= haystack:len() - needle:len(); i += 1 {\n"
"\t\tbool match = true;\n"
"\t\tfor int j = 0; j < needle:len(); j += 1 { if haystack[i + j] != needle[j] match = false; }\n"
"\t\tif match { return true; }\n"
"\t}\n"
"\treturn false;\n"
"}\n"
"\n"
"str StringTrim(str string) {\n"
"\tint start = 0;\n"
"\tint end = string:len();\n"
"\twhile start != end && (string[start] == \" \" || string[start] == \"\\t\" || string[start] == \"\\r\" || string[start] == \"\\n\") { start += 1; }\n"
"\twhile start != end && (string[end - 1] == \" \" || string[end - 1] == \"\\t\" || string[end - 1] == \"\\r\" || string[end - 1] == \"\\n\") { end -= 1; }\n"
"\treturn StringSlice(string, start, end);\n"
"}\n"
"\n"
"str[] StringSplitByCharacter(str string, str character, bool includeEmptyString) {\n"
"\tassert character:len() == 1;\n"
"\tstr[] list = new str[];\n"
"\tint x = 0;\n"
"\tfor int i = 0; i < string:len(); i += 1 {\n"
"\t\tif string[i] == character {\n"
"\t\t\tif (includeEmptyString || x != i) { list:add(StringSlice(string, x, i)); }\n"
"\t\t\tx = i + 1;\n"
"\t\t}\n"
"\t}\n"
"\tif (includeEmptyString || x != string:len()) { list:add(StringSlice(string, x, string:len())); }\n"
"\treturn list;\n"
"}\n"
"\n"
"str StringJoin(str[] strings, str k, bool trailing) {\n"
"\tstr c = \"\";\n"
"\tfor int i = 0; i < strings:len(); i += 1 {\n"
"\t\tif i < strings:len() - 1 || trailing { c = c + strings[i] + k; }\n"
"\t\telse { c = c + strings[i]; }\n"
"\t}\n"
"\treturn c;\n"
"}\n"
"\n"
"bool StringEndsWith(str s, str x) { \n"
"\treturn s:len() >= x:len() && StringSlice(s, s:len() - x:len(), s:len()) == x; \n"
"}\n"
"bool StringStartsWith(str s, str x) { \n"
"\treturn s:len() >= x:len() && StringSlice(s, 0, x:len()) == x; \n"
"}\n"
"\n"
"bool CharacterIsAlnum(str c) {\n"
"\tint b = CharacterToByte(c);\n"
"\treturn (b >= CharacterToByte(\"A\") && b <= CharacterToByte(\"Z\")) || (b >= CharacterToByte(\"a\") && b <= CharacterToByte(\"z\"))\n"
"\t\t|| (b >= CharacterToByte(\"0\") && b <= CharacterToByte(\"9\"));\n"
"}\n"
"\n"
"bool CharacterIsSpace(str c) { \n"
"\treturn c == \" \" || c == \"\\t\" || c == \"\\r\" || c == \"\\n\"; \n"
"}\n"
"\n"
"bool CharacterIsDigit(str c) { \n"
"\tint b = CharacterToByte(c); \n"
"\treturn b >= CharacterToByte(\"0\") && b <= CharacterToByte(\"9\"); \n"
"}\n"
"\n"
"str CharacterToUpperRaw(str c) {\n"
"\tint b = CharacterToByte(c);\n"
"\n"
"\tif b >= CharacterToByte(\"a\") && b <= CharacterToByte(\"z\") {\n"
"\t\tstr upper = \"ABCDEFGHIJKLMNOPQRSTUVWXYZ\";\n"
"\t\treturn upper[b - CharacterToByte(\"a\")];\n"
"\t} else {\n"
"\t\treturn c;\n"
"\t}\n"
"}\n"
"\n"
"str StringToUpperRaw(str s) { \n"
"\tstr t = \"\"; \n"
"\n"
"\tfor str c in s { \n"
"\t\tt += CharacterToUpperRaw(c); \n"
"\n"
"\t} \n"
"\treturn t; \n"
"}\n"
"\n"
"str CharacterToLowerRaw(str c) {\n"
"\tint b = CharacterToByte(c);\n"
"\n"
"\tif b >= CharacterToByte(\"A\") && b <= CharacterToByte(\"Z\") {\n"
"\t\tstr lower = \"abcdefghijklmnopqrstuvwxyz\";\n"
"\t\treturn lower[b - CharacterToByte(\"A\")];\n"
"\t} else {\n"
"\t\treturn c;\n"
"\t}\n"
"}\n"
"\n"
"str StringToLowerRaw(str s) { \n"
"\tstr t = \"\"; \n"
"\n"
"\tfor str c in s { \n"
"\t\tt += CharacterToLowerRaw(c); \n"
"\t} \n"
"\n"
"\treturn t; \n"
"}\n"
"\n"
"str StringRemoveOptionalPrefix(str s, str prefix) { \n"
"\tif StringStartsWith(s, prefix) { \n"
"\t\treturn StringSlice(s, prefix:len(), s:len()); \n"
"\t} else { \n"
"\t\treturn s; \n"
"\t} \n"
"}\n"
"\n"
"str StringRemoveOptionalSuffix(str s, str suffix) { \n"
"\tif StringEndsWith(s, suffix) { \n"
"\t\treturn StringSlice(s, 0, s:len() - suffix:len()); \n"
"\t} else { \n"
"\t\treturn s; \n"
"\t} \n"
"}\n"
"\n"
"int StringFind(str haystack, str needle, int startAt, bool reverse) {\n"
"\tassert needle:len() != 0;\n"
"\tint index = startAt;\n"
"\n"
"\twhile index >= 0 && index < haystack:len() {\n"
"\t\tif index + needle:len() <= haystack:len() {\n"
"\t\t\tbool match = true;\n"
"\n"
"\t\t\tfor int j = 0; j < needle:len(); j += 1 { \n"
"\t\t\t\tif haystack[index + j] != needle[j] match = false; \n"
"\t\t\t}\n"
"\n"
"\t\t\tif match { return index; }\n"
"\t\t}\n"
"\n"
"\t\tindex += -1 if reverse else 1;\n"
"\t}\n"
"\n"
"\treturn -1;\n"
"}\n"
"\n"
"int StringFindFirst(str haystack, str needle) { return StringFind(haystack, needle, 0, false); }\n"
"\n"
"int StringFindLast(str haystack, str needle) { return StringFind(haystack, needle, haystack:len() - 1, true); }\n"
"\n"
"str StringReplaceFirst(str haystack, str needle, str with) {\n"
"\tint index = StringFindFirst(haystack, needle);\n"
"\tif index == -1 { return haystack; }\n"
"\treturn StringSlice(haystack, 0, index) + with + StringSlice(haystack, index + needle:len(), haystack:len());\n"
"}\n"
"\n"
"str StringReplaceLast(str haystack, str needle, str with) {\n"
"\tint index = StringFindLast(haystack, needle);\n"
"\tif index == -1 { return haystack; }\n"
"\treturn StringSlice(haystack, 0, index) + with + StringSlice(haystack, index + needle:len(), haystack:len());\n"
"}\n"
"\n"
"tuple[str, int] StringReplaceAllWithCount(str haystack, str needle, str with) {\n"
"\tstr result = \"\";\n"
"\tint position = 0;\n"
"\tint count = 0;\n"
"\twhile true {\n"
"\t\tint index = StringFind(haystack, needle, position, false);\n"
"\t\tif index == -1 {\n"
"\t\t\tresult += StringSlice(haystack, position, haystack:len());\n"
"\t\t\treturn result, count;\n"
"\t\t}\n"
"\t\tresult += StringSlice(haystack, position, index) + with;\n"
"\t\tposition = index + needle:len();\n"
"\t\tcount += 1;\n"
"\t}\n"
"}\n"
"\n"
"str StringReplaceAll(str haystack, str needle, str with) {\n"
"\tstr result, int count = StringReplaceAllWithCount(haystack, needle, with);\n"
"\treturn result;\n"
"}\n"
"\n"
"str StringRepeat(str s, int n) { assert n >= 0; str t = \"\"; for int i = 0; i < n; i += 1 { t += s; } return t; }\n"
"\n"
"// Miscellaneous:\n"
"\n"
"int SystemGetProcessorCount() #extcall;\n"
"bool SystemRunningAsAdministrator() #extcall;\n"
"str SystemGetHostName() #extcall;\n"
"void SystemSleepMs(int ms) #extcall;\n"
"int RandomInt(int min, int max) #extcall;\n"
"\n"
"str UUIDGenerate() {\n"
"\tstr hexChars = \"0123456789abcdef\";\n"
"\tstr result;\n"
"\tfor int i = 0; i <  8; i += 1 { result += hexChars[RandomInt(0, 15)]; }\n"
"\tresult += \"-\";\n"
"\tfor int i = 0; i <  4; i += 1 { result += hexChars[RandomInt(0, 15)]; }\n"
"\tresult += \"-4\";\n"
"\tfor int i = 0; i <  3; i += 1 { result += hexChars[RandomInt(0, 15)]; }\n"
"\tresult += \"-\" + hexChars[RandomInt(8, 11)];\n"
"\tfor int i = 0; i <  3; i += 1 { result += hexChars[RandomInt(0, 15)]; }\n"
"\tresult += \"-\";\n"
"\tfor int i = 0; i < 12; i += 1 { result += hexChars[RandomInt(0, 15)]; }\n"
"\treturn result;\n"
"}\n"
"\n"
"// File system access:\n"
"\n"
"bool PathExists(str x) #extcall;\n"
"bool PathIsFile(str source) #extcall;\n"
"bool PathIsDirectory(str source) #extcall;\n"
"bool PathIsLink(str source) #extcall;\n"
"err[void] PathCreateDirectory(str x) #extcall;\n"
"err[void] PathDelete(str x) #extcall;\n"
"err[void] PathMove(str source, str destination) #extcall;\n"
"str PathGetDefaultPrefix() #extcall;\n"
"err[void] PathSetDefaultPrefixToScriptSourceDirectory() #extcall;\n"
"err[str] FileReadAll(str path) #extcall;\n"
"err[void] FileWriteAll(str path, str x) #extcall;\n"
"err[void] FileAppend(str path, str x) #extcall;\n"
"err[void] FileCopy(str source, str destination) #extcall;\n"
"err[int] FileGetSize(str path) #extcall;\n"
"err[void] FileTouch(str path) { return FileAppend(path, \"\"); }\n"
"\t\t\t\t      \n"
"err[void] _DirectoryInternalStartIteration(str path) #extcall;\n"
"str _DirectoryInternalNextIteration() #extcall;\n"
"void _DirectoryInternalEndIteration() #extcall;\n"
"\n"
"err[void] _DirectoryInternalEnumerateChildren(str path, str prefix, str[] result, bool recurse) {\n"
"\treterr _DirectoryInternalStartIteration(path);\n"
"\tstr child = _DirectoryInternalNextIteration();\n"
"\tint start = result:len();\n"
"\n"
"\twhile child != \"\" { \n"
"\t\tif child != \".\" && child != \"..\" {\n"
"\t\t\tresult:add(child); \n"
"\t\t}\n"
"\n"
"\t\tchild = _DirectoryInternalNextIteration(); \n"
"\t}\n"
"\n"
"\t_DirectoryInternalEndIteration();\n"
"\n"
"\tint end = result:len();\n"
"\n"
"\tif recurse {\n"
"\t\tfor int i = start; i < end; i += 1 {\n"
"\t\t\tstr actual = path + \"/\" + result[i];\n"
"\n"
"\t\t\tif PathIsDirectory(actual) {\n"
"\t\t\t\t_DirectoryInternalEnumerateChildren(actual, prefix + result[i] + \"/\", result, true);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t}\n"
"\n"
"\tfor int i = start; i < end; i += 1 {\n"
"\t\tresult[i] = prefix + result[i];\n"
"\t}\n"
"\t\n"
"\treturn #success;\n"
"}\n"
"\n"
"err[str[]] DirectoryEnumerate(str path) {\n"
"\tstr[] result = new str[];\n"
"\terr[void] e = _DirectoryInternalEnumerateChildren(path, \"\", result, false);\n"
"\treterr e; return result;\n"
"}\n"
"\n"
"err[str[]] DirectoryEnumerateRecursively(str path) {\n"
"\tstr[] result = new str[];\n"
"\terr[void] e = _DirectoryInternalEnumerateChildren(PathTrimTrailingSlash(path), \"\", result, true);\n"
"\treterr e; return result;\n"
"}\n"
"\n"
"err[str[]] DirectoryEnumerateFiltered(str sourceDirectory, str[] filter, int filterWildcard, bool prependSourceDirectory) {\n"
"\tassert filter:len() > 0;\n"
"\terr[str[]] e;\n"
"\tif filterWildcard != -1 || filter:len() > 1 { e = DirectoryEnumerateRecursively(sourceDirectory); }\n"
"\telse { e = DirectoryEnumerate(sourceDirectory); }\n"
"\treterr e;\n"
"\tstr[] allItems = e:assert();\n"
"\tstr[] filteredItems = new str[];\n"
"\tfor str item in allItems {\n"
"\t\tif PathMatchesFilter(item, filter, filterWildcard) {\n"
"\t\t\tif prependSourceDirectory {\n"
"\t\t\t\tfilteredItems:add(PathEnsureTrailingSlash(sourceDirectory) + item);\n"
"\t\t\t} else {\n"
"\t\t\t\tfilteredItems:add(item);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t}\n"
"\treturn filteredItems;\n"
"}\n"
"\n"
"str PathTrimTrailingSlash(str x) { if x:len() > 0 && x[x:len() - 1] == \"/\" { return StringSlice(x, 0, x:len() - 1); } return x; }\n"
"str PathEnsureTrailingSlash(str x) { if x:len() > 0 && x[x:len() - 1] == \"/\" { return x; } return x + \"/\"; }\n"
"\n"
"err[void] PathDeleteRecursively(str path) {\n"
"\terr[str[]] e = DirectoryEnumerateRecursively(path);\n"
"\tif str[] all in e {\n"
"\t\tfor int i = 0; i < all:len(); i += 1 {\n"
"\t\t\tstr p = path + \"/\" + all[i];\n"
"\t\t\tif PathIsFile(p) || PathIsLink(p) {\n"
"\t\t\t\tPathDelete(p);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tfor int i = all:len(); i > 0; i -= 1 {\n"
"\t\t\tstr p = path + \"/\" + all[i - 1];\n"
"\t\t\tif PathIsDirectory(p) {\n"
"\t\t\t\tPathDelete(p);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\treturn PathDelete(path);\n"
"\t} else {\n"
"\t\treturn new err[void] e:error();\n"
"\t}\n"
"}\n"
"\n"
"void PathDeleteAll(str[] files) { \n"
"\tfor str file in files { \n"
"\t\tPathDelete(file); \n"
"\t} \n"
"}\n"
"\n"
"void PathDeleteAllFiltered(str sourceDirectory, str[] filter, int filterWildcard) {\n"
"\tPathDeleteAll(DirectoryEnumerateFiltered(sourceDirectory, filter, filterWildcard, true):default(new str[]));\n"
"}\n"
"\n"
"err[void] PathCopyRecursively(str source, str destination) {\n"
"\terr[str[]] e = DirectoryEnumerateRecursively(source);\n"
"\n"
"\tif str[] all in e {\n"
"\t\treterr PathCreateDirectory(destination);\n"
"\n"
"\t\tfor int i = 0; i < all:len(); i += 1 {\n"
"\t\t\tstr sourceItem = source + \"/\" + all[i];\n"
"\t\t\tstr destinationItem = destination + \"/\" + all[i];\n"
"\n"
"\t\t\tif PathIsDirectory(sourceItem) {\n"
"\t\t\t\tPathCreateDirectory(destinationItem);\n"
"\t\t\t} else {\n"
"\t\t\t\treterr FileCopy(sourceItem, destinationItem);\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\treturn #success;\n"
"\t} else {\n"
"\t\treturn new err[void] e:error();\n"
"\t}\n"
"}\n"
"\n"
"err[void] PathCopyInto(str sourceDirectory, str item, str destinationDirectory) {\n"
"\tstr sourceItem = sourceDirectory + \"/\"+ item;\n"
"\tstr destinationItem = destinationDirectory + \"/\"+ item;\n"
"\tPathCreateLeadingDirectories(PathGetLeadingDirectories(destinationItem));\n"
"\n"
"\tif PathIsDirectory(sourceItem) {\n"
"\t\treturn PathCreateDirectory(destinationItem);\n"
"\t} else {\n"
"\t\treturn FileCopy(sourceItem, destinationItem);\n"
"\t}\n"
"}\n"
"\n"
"err[void] PathCopyAllInto(str sourceDirectory, str[] items, str destinationDirectory) {\n"
"\tfor int i = 0; i < items:len(); i += 1 {\n"
"\t\treterr PathCopyInto(sourceDirectory, items[i], destinationDirectory);\n"
"\t}\n"
"\n"
"\treturn #success;\n"
"}\n"
"\n"
"err[void] PathCopyFilteredInto(str sourceDirectory, str[] filter, int filterWildcard, str destinationDirectory) {\n"
"\terr[str[]] e = DirectoryEnumerateFiltered(sourceDirectory, filter, filterWildcard, false);\n"
"\treterr e;\n"
"\treturn PathCopyAllInto(sourceDirectory, e:assert(), destinationDirectory);\n"
"}\n"
"\n"
"bool PathMatchesFilter(str path, str[] filterComponents, int _filterWildcard) {\n"
"\tstr[] pathComponents = StringSplitByCharacter(path, \"/\", false);\n"
"\tint filterWildcard = _filterWildcard;\n"
"\n"
"\tif filterWildcard == -1 { \n"
"\t\tfilterWildcard = pathComponents:len(); \n"
"\t}\n"
"\n"
"\tif filterComponents:len() > pathComponents:len() { \n"
"\t\treturn false; \n"
"\t}\n"
"\n"
"\tfor int i = 0; i < filterComponents:len(); i += 1 {\n"
"\t\tstr filterComponent = filterComponents[i];\n"
"\t\tstr pathComponent;\n"
"\t\tint wildcard = filterComponent:len();\n"
"\n"
"\t\tif i < filterWildcard { \n"
"\t\t\tpathComponent = pathComponents[i]; \n"
"\t\t} else { \n"
"\t\t\tpathComponent = pathComponents[pathComponents:len() - filterComponents:len() + i]; \n"
"\t\t}\n"
"\n"
"\t\tfor int j = 0; j < filterComponent:len(); j += 1 { \n"
"\t\t\tif filterComponent[j] == \"*\" { \n"
"\t\t\t\twildcard = j; \n"
"\n"
"\t\t\t\tif pathComponent:len() < filterComponent:len() - 1 {\n"
"\t\t\t\t\treturn false;\n"
"\t\t\t\t}\n"
"\t\t\t} \n"
"\t\t}\n"
"\n"
"\t\tfor int j = 0; j < filterComponent:len(); j += 1 {\n"
"\t\t\tif j < wildcard {\n"
"\t\t\t\tif filterComponent[j] != pathComponent[j] { return false; }\n"
"\t\t\t} else if j > wildcard {\n"
"\t\t\t\tif filterComponent[j] != pathComponent[pathComponent:len() - filterComponent:len() + j] { return false; }\n"
"\t\t\t}\n"
"\t\t}\n"
"\t}\n"
"\n"
"\treturn true;\n"
"}\n"
"\n"
"str PathGetLeadingDirectories(str path) {\n"
"\tfor int i = path:len() - 1; i >= 0; i -= 1 {\n"
"\t\tif path[i] == \"/\" {\n"
"\t\t\treturn StringSlice(path, 0, i);\n"
"\t\t}\n"
"\t}\n"
"\n"
"\treturn \"\";\n"
"}\n"
"\n"
"err[void] PathCreateLeadingDirectories(str path) {\n"
"\tfor int i = 0; i < path:len(); i += 1 {\n"
"\t\tif path[i] == \"/\" {\n"
"\t\t\tPathCreateDirectory(StringSlice(path, 0, i));\n"
"\t\t}\n"
"\t}\n"
"\n"
"\treturn PathCreateDirectory(path);\n"
"}\n"
"\n"
"// Persistent variables:\n"
"\n"
"bool PersistRead(str path) #extcall;\n"
"\n"
"// Command line:\n"
"\n"
"str ConsoleGetLine() #extcall;\n"
"void ConsoleWriteStdout(str x) #extcall;\n"
"void ConsoleWriteStderr(str x) #extcall;\n"
"err[str] SystemGetEnvironmentVariable(str name) #extcall;\n"
"err[void] SystemSetEnvironmentVariable(str name, str value) #extcall;\n"
"bool SystemShellExecute(str x) #extcall; // Returns true on success.\n"
"bool SystemShellExecuteWithWorkingDirectory(str wd, str x) #extcall; // Returns true on success.\n"
"str SystemShellEvaluate(str x) #extcall; // Captures stdout.\n"
"void SystemShellEnableLogging(bool x) #extcall;\n"
"\n"
"// Integer mathematics:\n"
"\n"
"int IntegerModulo(int x, int y) { assert y >= 1; int d = x / y; int r = x - d * y; if r >= 0 { return r; } else { return r + y; } }\n"
"int IntegerAbsolute(int x) { if x < 0 { return -x; } else { return x; } }\n"
"int IntegerMaximum(int a, int b) { if a < b { return b; } else { return a; } }\n"
"int IntegerMinimum(int a, int b) { if a > b { return b; } else { return a; } }\n"
"int IntegerClamp(int x, int min, int max) { assert min <= max; if x > max { return max; } else if x < min { return min; } else { return x; } }\n"
"\n"
"int IntegerCountLeastSignificantZeroBits(int x) {\n"
"\tint count = 0;\n"
"\twhile count != 64 && ((x & (1 << count)) == 0) { count += 1; }\n"
"\treturn count;\n"
"}\n"
"\n"
"int IntegerCountLeastSignificantOneBits(int x) {\n"
"\tint count = 0;\n"
"\twhile count != 64 && ((x & (1 << count)) != 0) { count += 1; }\n"
"\treturn count;\n"
"}\n"
"\n"
"int IntegerCountMostSignificantZeroBits(int x) {\n"
"\tint count = 0;\n"
"\twhile count != 64 && ((x & (1 << (63 - count))) == 0) { count += 1; }\n"
"\treturn count;\n"
"}\n"
"\n"
"int IntegerCountMostSignificantOneBits(int x) {\n"
"\tint count = 0;\n"
"\twhile count != 64 && ((x & (1 << (63 - count))) != 0) { count += 1; }\n"
"\treturn count;\n"
"}\n"
"\n"
"int IntegerCountOneBits(int x) {\n"
"\tint count = 0;\n"
"\tfor int i = 0; i < 64; i += 1 {\n"
"\t\tif (x & (1 << i)) != 0 { count += 1; }\n"
"\t}\n"
"\treturn count;\n"
"}\n"
"\n"
"int IntegerGCD(int _a, int _b) {\n"
"\tint a = _a;\n"
"\tint b = _b;\n"
"\tassert a != 0 || b != 0;\n"
"\tif a < 0 a = -a;\n"
"\tif b < 0 b = -b;\n"
"\twhile true {\n"
"\t\tif a < b { int t = a; a = b; b = t; }\n"
"\t\tif b == 0 { return a; }\n"
"\t\tif b == 1 { return 1; }\n"
"\t\ta = IntegerModulo(a, b);\n"
"\t}\n"
"}\n"
"\n"
"int IntegerLCM(int a, int b) { return IntegerAbsolute(a * b / IntegerGCD(a, b)); }\n"
"\n"
"tuple[int, int, int] IntegerExtendedEuclidean(int _a, int _b) {\n"
"\tassert _a != 0 || _b != 0;\n"
"\tint ma = 1; int a = _a; int mb = 1; int b = _b;\n"
"\tif a < 0 { a = -a; ma = -ma; }\n"
"\tif b < 0 { b = -b; mb = -mb; }\n"
"\tbool swap = false;\n"
"\tif a < b { int t = a; a = b; b = t; t = ma; ma = mb; mb = t; swap = true; }\n"
"\tint gcd; int ca; int cb;\n"
"\tif b == 0 { gcd = a; ca = 1; cb = 0; } \n"
"\telse if b == 1 { gcd = 1; ca = 0; cb = 1; } \n"
"\telse { int q = a / b; int r = a - q * b; gcd, ca, cb = IntegerExtendedEuclidean(r, b); cb -= ca * q; }\n"
"\tif swap { return gcd, cb * mb, ca * ma; } \n"
"\telse { return gcd, ca * ma, cb * mb; }\n"
"}\n"
"\n"
"tuple[int, int] IntegerCRT(int[] r, int[] m) {\n"
"\tassert r:len() == m:len() && r:len() != 0;\n"
"\tint M = 1;\n"
"\tint x = 0;\n"
"\tfor int mi in m { assert mi >= 1; M *= mi; }\n"
"\n"
"\tfor int i = 0; i < r:len(); i += 1 {\n"
"\t\tint mi = m[i];\n"
"\t\tint gcd, int ca, int cb = IntegerExtendedEuclidean(IntegerModulo(M / mi, mi), mi);\n"
"\t\tassert gcd == 1;\n"
"\t\tx += r[i] * ca * M / mi;\n"
"\t}\n"
"\n"
"\treturn x, M;\n"
"}\n"
"\n"
"int[] IntegerPrimeFactorization(int _x) {\n"
"\tint x = _x;\n"
"\tassert x != 0;\n"
"\tint[] r = [];\n"
"\tif x < 0 { x = -x; r:add(-1); }\n"
"\tint p = 2;\n"
"\twhile x != 1 {\n"
"\t\tif IntegerModulo(x, p) == 0 {\n"
"\t\t\tx /= p;\n"
"\t\t\tr:add(p);\n"
"\t\t} else {\n"
"\t\t\tp += 1;\n"
"\t\t}\n"
"\t}\n"
"\treturn r;\n"
"}\n"
"\n"
"// Floating point mathematics.\n"
"\n"
"float MathLinearRemap(float x, float fromStart, float fromEnd, float toStart, float toEnd) { return (x - fromStart) / (fromEnd - fromStart) * (toEnd - toStart) + toStart; }\n"
"float MathSign(float x) { return 0.0 if x == 0.0 else 1.0 if x > 0.0 else -1.0; }\n"
"float FloatAbsolute(float x) { return x if x >= 0.0 else -x; }\n"
"float FloatMaximum(float x, float y) { return x if x >= y else y; }\n"
"float FloatMinimum(float x, float y) { return x if x <= y else y; }\n"
"float FloatClamp(float x, float min, float max) { assert min <= max; return min if x <= min else max if x >= max else x; }\n"
"float FloatClamp01(float x) { return 0.0 if x <= 0.0 else 1.0 if x >= 1.0 else x; }\n"
"float RandomFloat(float min, float max) { assert min <= max; return (RandomInt(0, 1000000000):float() / 1000000000.0) * (max - min) + min; }\n"
"float RandomFloat01() { return RandomFloat(0.0, 1.0); }\n"
"\n"
"// Permutations.\n"
"\n"
"int[] PermutationFirst(int n) { int[] p = new int[]; for int i = 0; i < n; i += 1 { p:add(i); } return p; }\n"
"\n"
"bool PermutationIterate(int[] p) {\n"
"\tint lvi = -1;\n"
"\tint xiForLvi;\n"
"\tint yiForLvi;\n"
"\tfor int xi = 0; xi < p:len(); xi += 1 {\n"
"\t\tint i = p[xi];\n"
"\t\tbool odd = false;\n"
"\t\tfor int j = 0; j < p:len(); j += 1 {\n"
"\t\t\tfor int k = 0; k < j; k += 1 {\n"
"\t\t\t\tif p[j] < i && p[k] < i && p[j] < p[k] {\n"
"\t\t\t\t\todd = !odd;\n"
"\t\t\t\t}\n"
"\t\t\t}\n"
"\t\t}\n"
"\t\tint yi = xi - 1;\n"
"\t\tif odd yi += 2;\n"
"\t\tbool valid = yi >= 0 && yi < p:len() && p[yi] < i;\n"
"\t\tif valid && i > lvi {\n"
"\t\t\tlvi = i;\n"
"\t\t\txiForLvi = xi;\n"
"\t\t\tyiForLvi = yi;\n"
"\t\t}\n"
"\t}\n"
"\tif lvi == -1 {\n"
"\t\treturn false;\n"
"\t} else {\n"
"\t\tint swap = p[xiForLvi];\n"
"\t\tp[xiForLvi] = p[yiForLvi];\n"
"\t\tp[yiForLvi] = swap;\n"
"\t\treturn true;\n"
"\t}\n"
"}\n"
"\n"
"// UTF-8.\n"
"\n"
"int _StringUTF8CodepointLength(int c) {\n"
"\treturn 1 if (c & 0x80) == 0x00 else \n"
"\t       2 if (c & 0xE0) == 0xC0 else \n"
"\t       3 if (c & 0xF0) == 0xE0 else \n"
"\t       4 if (c & 0xF8) == 0xF0 else 0;\n"
"}\n"
"\n"
"int StringUTF8Advance(str x, int i) {\n"
"\tif i == -1 { return -1; }\n"
"\tint c0 = CharacterToByte(x[i + 0]);\n"
"\tint length = _StringUTF8CodepointLength(c0);\n"
"\tif length == 0 || i + length > x:len() { return -1; }\n"
"\treturn i + length;\n"
"}\n"
"\n"
"int StringUTF8Retreat(str x, int i) {\n"
"\tif i == -1 { return -1; }\n"
"\tassert i >= 1 && i <= x:len();\n"
"\tint j = i;\n"
"\twhile j >= 1 {\n"
"\t\tj -= 1;\n"
"\t\tint k = CharacterToByte(x[j]);\n"
"\t\tif (k & 0xC0) != 0x80 { return j if j + _StringUTF8CodepointLength(k) == i else -1; }\n"
"\t}\n"
"\treturn -1;\n"
"}\n"
"\n"
"int StringUTF8Count(str x) {\n"
"\tint i = 0;\n"
"\tint count = 0;\n"
"\t\n"
"\twhile i != -1 && i != x:len() { \n"
"\t\ti = StringUTF8Advance(x, i); \n"
"\t\tcount += 1; \n"
"\t}\n"
"\treturn count if i != -1 else -1;\n"
"}\n"
"\n"
"int StringUTF8Decode(str x, int i) {\n"
"\tif i == -1 { return 0xFFFD; }\n"
"\tint c0 = CharacterToByte(x[i + 0]);\n"
"\tint length = _StringUTF8CodepointLength(c0);\n"
"\tif length == 0 || i + length > x:len() { return 0xFFFD; }\n"
"\tif length == 1 { return c0; }\n"
"\tint c1 = CharacterToByte(x[i + 1]);\n"
"\tif (c1 & 0xC0) != 0x80 { return 0xFFFD; }\n"
"\tif length == 2 { return ((c0 & 0x1F) << 6) | (c1 & 0x3F); }\n"
"\tint c2 = CharacterToByte(x[i + 2]);\n"
"\tif (c2 & 0xC0) != 0x80 { return 0xFFFD; }\n"
"\tif length == 3 { return ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F); }\n"
"\tint c3 = CharacterToByte(x[i + 3]);\n"
"\tif (c3 & 0xC0) != 0x80 { return 0xFFFD; }\n"
"\tif length == 4 { return ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F); }\n"
"\tassert false;\n"
"}\n"
"\n"
"str StringUTF8Encode(int value) {\n"
"\tif value < 0x80 {\n"
"\t\treturn StringFromByte(value);\n"
"\t} else if value < 0x800 {\n"
"\t\treturn StringFromByte(0xC0 | ((value >> 6) & 0x1F)) + StringFromByte(0x80 | (value & 0x3F));\n"
"\t} else if value < 0x10000 {\n"
"\t\treturn StringFromByte(0xE0 | ((value >> 12) & 0x0F)) + StringFromByte(0x80 | ((value >> 6) & 0x3F)) + StringFromByte(0x80 | (value & 0x3F));\n"
"\t} else if value < 0x200000 {\n"
"\t\treturn StringFromByte(0xF0 | (value >> 18)) + StringFromByte(0x80 | ((value >> 12) & 0x3F)) \n"
"\t\t\t+ StringFromByte(0x80 | ((value >> 6) & 0x3F)) + StringFromByte(0x80 | (value & 0x3F));\n"
"\t} else {\n"
"\t\treturn StringUTF8Encode(0xFFFD);\n"
"\t}\n"
"}\n"
""