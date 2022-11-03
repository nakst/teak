# Engine

The engine takes command line arguments of the form: `esh [ >= 0 engine flags ] [ path to main source file ] [ >= 0 #option values ]`. The engine flags are from the following:

- `--start=...` Set the function to call on the main source file. By default, this is `Start`.
- `--evaluate` or `-e` The "path to main source file" is instead interpreted as a expression to evaluate.
- `--output-overview` Don't execute the script. Instead, output an overview of the types and functions in it.

The following flags are intended for debugging the engine itself and therefore are likely not useful for most users:

- `--debug-bytecode=...` Set the bytecode debugging level.
- `--no-base-module` Don't include the base module. Helpful when tracking down lexing or parsing bugs.
- `--want-completion-confirmation` Used by the test runner to check if the engine crashes.
