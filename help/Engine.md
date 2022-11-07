# Engine

The engine takes command line arguments of the form: `esh [ >= 0 engine flags ] [ path to main source file ] [ >= 0 #option values ]`. The engine flags are from the following:

- `--start=...` Set the function to call on the main source file. By default, this is `Start`.
- `--evaluate` or `-e` The "path to main source file" is instead interpreted as a expression to evaluate.
- `--output-overview` Don't execute the script. Instead, output an overview of the types and functions in it.
- `--no-colored-output`, `--colored-output` Override the default for whether the output using ANSI terminal color escape sequences.
- `--log=...` Log the specified actions. See below for a list of action categories.
- `--trace=...` Log the specified actions and include back traces. See below for a list of action categories.
- `--ask=...` Ask before processing any of the specified actions. See below for a list of action categories.
- `--error-ask=...` Ask the user what to do if one of the specified actions produces an error. See below for a list of action categories.
- `--error-stop=...` Stop the script immediately if one of the specified actions produces an error. See below for a list of action categories.

The action categories available for the `--log`, `--trace`, `--ask`, `--error-ask` and `--error-stop` categories are:

- `e` Enumerating directories.
- `r` Reading files.
- `w` Writing, creating and updating files and directories.
- `d` Deleting files and directories.
- `s` Querying properties of files and directories.
- `v` Accessing or updating the environment variables.
- `x` *Not implemented yet.* Executing shell commands.

The following flags are intended for debugging the engine itself and therefore are likely not useful for most users:

- `--debug-bytecode=...` Set the bytecode debugging level.
- `--no-base-module` Don't include the base module. Helpful when tracking down lexing or parsing bugs.
- `--want-completion-confirmation` Used by the test runner to check if the engine crashes.
