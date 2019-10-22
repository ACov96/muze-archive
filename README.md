# Language Design Research Project

This is the repository for Dr. Meehan's language design project. The language doesn't have a name yet but will be updated once a name has been decided.

## Build

From the root of the repository, run:

```bash
make
```

This will create an executable called `muzec`.

## Usage

First, make sure you set the `MUZE_STDLIB_PATH` environment variable so that `muzec` can link in all of the runtime code:

```bash
export MUZE_STDLIB_PATH=/path/to/repo/lib/stdlib.o
```

Then run the executable:

```bash
muzec -o hello hello.mz
```
