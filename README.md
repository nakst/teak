# esh

A small scripting language to replace Unix shell scripts.

*Please note that the language is in active development and is not yet stable.*

## Building

If you haven't built the interpreter before, first run `gcc -o esh esh.c -pthread -ldl` (Linux/macOS) or `cl.exe esh.c` (Windows).

Then run `./esh build.esh`. For a debug build, run `./esh build.esh debug=true`. (For Windows, don't include the `./` at the start of the commands.)

Run `./esh build.esh runTests=true` to run all the tests.

## Running examples

For example, run `./esh examples/hello_world.esh`.

## Documentation

The following files document and explain the language, its tooling and the included modules.

- `examples/basic_usage.esh`: A script containing a whirlwind tour of the language's syntax.
- `help/Base Module.md`: Documentation for the base module.
- `help/Engine.md`: Describes the command line arguments for the scripting engine.
- `help/Creating Modules.md`: Describes how to make modules.

## Rationale

Why did I make another programming language? Because I felt like there wasn't a good statically typed language to compete with Unix shell scripts. That is, something lightweight and designed for simple file operations and launching other processes. If you want something dynamically typed, you're probably better off with Python, for example.

## Roadmap

- Make more documentation, examples and test cases.
- Add features listed at the top of esh.c.
- Make a REPL so people can use it as a command line shell?
