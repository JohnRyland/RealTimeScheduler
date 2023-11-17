
# GenericMake
### Simple generic Makefile based build/project system

Copyright (c) 2021-2022, John Ryland

All rights reserved.


## Introduction

This is a copy/fork of https://github.com/JohnRyland/GenericMake.git with specific
modifications for building the operating system and to bundle with the source of
the operating system to not require any additional dependancies.

This is a very light and simple build system which builds on top of standard make.

It has some similarities to qmake in that most of the project specific details of
what and how to build the project are inside a .pro project file. The syntax is also
similar, with `VAR=value` pairs. Because it is still just essentially a makefile though,
anything you can do in make you can put in to the .pro file too.


## Example usage

Here is a simple example of a HelloWorld project which will build main.cpp and compile
it to a HelloWorld executable:

```
PROJECT   = HelloWorld
TARGET    = HelloWorld 
SOURCES   = main.cpp
```


## Getting started

In the example above, typically you would name this file *HelloWorld.pro* inside your
HelloWorld project directory. Then copy or symlink 'Bootstrap.mak' from this repo
as *Makefile* in to your project directory. Now you can run 'make' and it should build
the project for you.


## How to use

When you copy *Bootstrap.mak* file in to your project as *Makefile*, make will be able
to execute a small amount of bootstrap code which will clone this repo (if it needs to)
and then include the cloned Generic.mak file. This means that if you run `make purge`
and then run `make` again, you will be on the latest version. The purge target will
wipe away the clone and running `make` will fetch the latest copy.

An alternative way you can use this in your projects is to copy the file *Generic.mak* in
to your project and rename it to *Makefile*. This will fix the version of GenericMake used by
the project until it is manually updated by copying a new version over the top. You may also
need to copy over some of the directories from here.


## More detailed usage

### Example

Here is a more complicated example:

```
PROJECT   = Maths3D
TARGET    = Maths3D
VERSION   = 1.0.0
BRIEF     = "Maths for Computer Graphics"
LOGO      = docs/logo.svg
DOCS      = docs/README.md

SOURCES   = src/maths3d.cpp tests/tests.cpp examples/examples.pro
INCLUDES  = includes
LIBRARIES = m
CFLAGS    = -Wall -ffast-math -O2
CXXFLAGS  = -std=c++11 -fno-exceptions

MODULES   = https://github.com/JohnRyland/TestFramework.git

# Add some additional tests to check the generated code
.build/optimization_test.S: tests/optimization_test.cpp includes/maths3d.h
	$(CXX) $(CXX_FLAGS) $< -S -o $@

check_code_gen: .build/optimization_test.S
	@echo checking the compiler is optimizing to use inverse sqrt instruction
	grep rsqrtss .build/optimization_test.S > /dev/null

test: check_code_gen
```

Note that headers are not explicitly listed. That is because header dependencies
are automatically determined by the build system.

Note that you are not restricted to only putting key-value pairs in the .pro files.
You can add additional make rules if you wish. In the above example, it adds
an additional dependency to the *test* target which also needs to be run when building
the test target (when you execute `make test`).

Note the *BRIEF*, *LOGO* and *DOCS* keys. These are used in the generation of documentation.
Any markdown file in DOCS are converted to PDF using pandoc and an included template.
The template can be a user supplied one by setting the *PANDOC_TEMPLATE* value. The *BRIEF*
and *LOGO* values are used by the doxygen documentation generation. This build system
gathers the list of files to pass to doxygen and generates a Doxyfile and then runs
doxygen.

Also notice in the *SOURCES* list is another .pro file. It can build sub-projects too,
and when it does this, it will gather a list of all the source files from the sub-projects
to pass to *doxygen*.


### Targets

This is a list of the makefile targets that can be used:

 - run: Will build if required and then run the target executable.
 - todos: Prints a list of 'TODO's found in the code.
 - purge: Removes the .modules, .build and bin directories.
 - pdfs: Generates PDFs from all the markdown files in DOCS.
 - build: Will build the target executable as well as documentation.
 - doxygen: Builds the doxygen documentation.
 - coverage: Runs the test build and generates a coverage report.
 - package: Packages up the PDFs and target executable.
 - release: Builds a release build type.
 - debug: Builds a debug build type and then runs in debug mode.
 - profile: Build a profile build type and profiles.
 - test: Builds a test build type and runs the tests.
 - verify: Runs the tests and checks the test results.
 - help: Prints help on usage.
 - info: Prints info about the project and detected settings.
 - clean: Removes the generated files (note this is for release build type)


### Variables

This is a list of variables in .pro files which are used:

 - PROJECT: Name of the project
 - VERSION: Version of the project
 - BRIEF: Short description about the project (optional)
 - LOGO: Logo/icon associated with the project (optional)
 - TARGET: Name of the executable (optional)
 - CFLAGS: C compiler flags (optional)
 - LFLAGS: Linker flags (optional)
 - CXXFLAGS: C++ compiler flags (optional)
 - INCLUDES: List of include paths to search for headers (optional)
 - LIBRARIES: List of libraries to link (optional)
 - PANDOC_FLAGS: Flags to pass to pandoc (optional)
 - PANDOC_TEMPLATE: TeX template file to use instead of the default (optional)
 - SOURCES: List of C, C++ and PRO files to build (optional)
 - DOCS: List of MD file to generate in to PDFs (optional)
 - MODULES: External dependancies which will be retrieved and built first (optional)


### Unit tests

There is integration to run unit tests. It can be made to work with the TestFramework
project.

 - https://github.com/JohnRyland/TestFramework.git

This can be included as a module like this:

```
MODULES   = https://github.com/JohnRyland/TestFramework.git
```

Adding this to your .pro file will fetch this project in to a .modules directory.
Then targets which link against this can be built and run. The test framework tries to
have some compatibilty with *google-test* and *u-test* in both source and usage. Similar
to these test frameworks is a way to discover a list of tests. Running the test
program with the command line option of *--list-tests* will output a list of the
included unit tests. The build system when building the *verify* target will do this
to get a list of tests, and will run each one by using the *--filter* option to
only execute one unit test at a time. And it uses the *--output* option to request
an XML report output. These XML test run reports are output in to a testing directory
in the .build folder.

The build system hasn't been tested with *google-test* or *u-test* yet, so integration with
these might not currently work, but it expected that it would not be too difficult to
adapt it to support these.


## Markdown to PDF

In the subdirectory named *pandoc* in this repo is an example template written in *tex*
and a background PDF. These are used by the *pandoc* tool to be able to convert a
markdown file in to a nice looking PDF.

For more ideas on using markdown to create PDF documentation using pandoc and templates
see this project which has many examples:

 - https://github.com/Wandmalfarbe/pandoc-latex-template/

For making the background file, I used *inkscape* to draw some shapes and saved this
as a SVG file. Because SVG is XML, it would be relatively simple to dynamically change
colors or transform the background from a build system if required. To convert the
SVG to PNG for passing to *pandoc*, I used this command:

```
rsvg-convert -f pdf background.svg -o background.pdf
```

Setting *PANDOC_TEMPLATE* in your .pro file allows you to use an alternative to the supplied
*template.tex* file which is what currently includes *background.pdf* as the background.


##  Editor integration

There are some special makefile targets which are provided for editor integration.

These targets can be called by an editor's settings, such as the VimSettings here:

 - https://github.com/JohnRyland/VimSettings.git

It detects if the makefile contains these special targets by running

```
make vim_project_support
```

The special targets then help to tell vim where to search for includes, other files in the
project, debugging options etc. It can produce a project file tree the editor can use to
help navigate the project. Possibly other editors might be able to be configured to do
something similar.

