# Muze Language and Compiler

This is the repository for Dr. Meehan's Muze language project.

## Setup

This project relies on some C libraries, so make sure you clone submodules too:
```bash
git clone --recurse-submodules git@gitlab.cs.wwu.edu:covinga/language-design-research.git
```

## Build

From the root of the repository, run:

```bash
make
```

This will build all necessary libraries and create an executable called `muzec`.

## Usage

First, make sure you set the `MUZE_STDLIB_PATH` environment variable so that `muzec` can link in all of the runtime code:

```bash
export MUZE_STDLIB_PATH=/path/to/repo/lib/libs.o
```

You'll also need to set the `MUZE_LD_SCRIPT_PATH` environment variable so that `muzec` can properly setup the type graph for the runtime environment

```bash
make lib/libs.o # If you already ran 'make' then you can skip this line
export MUZE_LD_SCRIPT_PATH=/path/to/repo/config/conf.ld
```

Then run the executable:

```bash
muzec -o hello hello.mz
```
