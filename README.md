# Language overview

Z2 is a statically-typed high-level OOP with functional elements general-purpose performance-centric natively-compiled systems-programming language.

It is a multi-paradigm language capable of multiple styles, but it designed to be particularly good at a programming style which uses heavy ownership, the "[everything belongs somewhere](everything%20belongs%20somewhere)" rule, RAII, values and references, copy and move semantics in order to offer automatic deterministic of most resources, not just memory.

It is a language in the C++ family and can be considered an intentionally smaller, simple to learn and master version of it, eliminating many of the more obscure elements and designed to be highly practical, something you can pick up and use.

It has a relatively small but powerful and practical standard library, made up of several classes which have the right balance between safety and performance. One can write bindings and make use of any C library, even limited interoperability with C++ is supported.

It has a compiler that is also a build tool that builds your packages and employs several techniques in order to provide [fast compilation times](fast%20compilation%20times), something that is a real focus.

It offers a cross-platform editor, called ZIDE, which can be considered a minimal goldens standard, offering a few valuable features out of the box. ZIDE predates LSP and before that, choosing to offer language support to and IDE meant picking one specific one and writing integration plugins for your language. Thus it was decided it is easier not to pick favourites and create a new editor that "just works" for people to get started. But Z2 is still  a command line compiler tool and one can easily invoke it form anything from a `makefile` to your favourite editor/IDE.

## Assortment of features

- it is a statically typed language that compiles to dependency free native platform binaries
- it is closely related to C++, but also draws inspiration from C#, Delphi and Ruby
- it tries to solve the same problems as C++ using similar constructs, but is a smaller easier to learn more sanitized language
- it systematically tries to address all the perceived issues of C++, while still maintaining a comparable feature set
- it is a practical language, with each feature being a consequence of some programming problem and solution that was encountered in the past
- it is performance centric and has the goal to be just as fast and in some cases even faster than C++, while addressing most of the safety concerns
- it is a system programming language, allowing you to get close to the metal, do system calls and has no mandatory overhead to its features
- it is high level, but tries to implement all the high level features efficiently and with "zero cost abstractions"
- the compiler is also the main build tool and arbitrarily large projects can be compiled with a single call to the compiler
- the compiler has a single front-end that compiles source code into an intermediate form and an open set of backend compilers that take that intermediate form and generate native platform binaries
- it uses a modern module system for building, where projects are divided into multiple packages with dependency management, each package is compiled only once and is designed to minimize backend compilations (these are slow) at the expense of more frontend compilations (these are fast)
- the main backend compiler uses transpilation to C++/C. This is the most practical and general way to get integrated into and existing platform and leverage to power of both new and old compilers, like Clang, GCC and MSVC, which are all supported out of the box.

## Getting started

Getting started with Z2 is easy, all you need is to grab one of the [pre-compiled releases](https://github.com/MasterZean/z2cr/releases) from [GitHub](https://github.com/MasterZean/z2cr).

You can check out [installing and getting started](docs/Installing%20and%20getting%20started.md) guide, which is step one in a series language tour.

Alternatively, you can build from source. Building from source isn't particularly difficult, but you need to install and configure a few dependencies, thus it is harder to get started with, making [compiling from sources]() a more advanced topic that needs to have its own section in the documentation.


