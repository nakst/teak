# esh

A small scripting language to replace Unix shell scripts.

## Building

Run `./build.sh`, or `./esh build.esh` if you've already built it.

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
