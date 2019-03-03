# Features

Turns this:

![Screenshot of a CSV file](./images/csv.jpg)

into this:

![Screenshot of the resulting htaccess file](./images/htaccess.jpg)

# Compile

The project has a Makefile included. If you run `make` (or your OS' equivalent, e.g. `mingw32-make` on Windows) in the root directory of the project, you will receive a compiled file in a subfolder called `bin`.

# Usage

The basic usage is as follows:

```
./csv_to_htaccess <csv path> [--prefix path/to/prefix] [--suffix path/to/suffix]
```

The CSV has to have the following columns (with no header line):

1. **FROM** - the path to redirect from. Can be a full URL, the domain part will be removed.
2. **TO** - the path to redirect to. Can be a full URL, will be removed if it matches the FROM URL.
3. (optional) **STATUS_CODE** - The program will use Status Code 302 for every redirect, unless specified here.

The columns 4 and up are ignored by the program, you can freely use them for e.g. comments.

The resulting file will be placed in the same directory and will follow this schema:

```
# Contents of your prefix file, or empty

# generated redirects

# contents of your suffix file, or empty
```

The pre- and suffix files have to be text files. Binary files lead to undefined behavior.

# Support

No external documentation exists. If you encounter a problem, please raise a Github issue.