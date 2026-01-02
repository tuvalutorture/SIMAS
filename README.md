# The SIMAS Programming Language
*Originally reated by: Turrnut*<br>
**Brought to C by tuvalutorture.**<br>

License: GPLv3, which is the same as the [original SIMAS repo, which is also licensed under GPLv3](https://github.com/turrnut/simas).

**SIMAS**, which is an acronym for **SIM**ple **AS**sembly, is a dynamically typed, high level procedural programming language  
with a syntax that is inspired by the Assembly programming language.<br>
In SIMAS, each line starts with an instruction, optionally followed by one or more operands, just like Assembly.<br>
To run a SIMAS program, simply run ```./simas <filename(s)>```. <br>
Additionally, you can use the ```-d``` flag as the last argument to get more info.
You can also run just ```./simas``` for the SIMAS command line.

An demo program, Jeremy Simulator (```jeremy.simas```), is included to showcase CMAS' features & capability.

**Additional Notes** <br>

SIMAS is case-sensitive, although instructions and data types are not. <br>

If you want to be polite to SIMAS, you can add `PLEASE` (case-insensitive) and a space character
in front of any instruction (this can be done as many times as you wish). However, SIMAS will ignore your politeness by ignoring `PLEASE`. <br>

For example, `PLEASE PRINTC Hello!;` and `PRINTC Hello!;` does the same thing.<br>

#### Extra stuffs to keep in mind:
* All occurences of `\n` within string constants (such as PRINTC) will be replaced with a new line. This also applies to:
    * `\\` - One backslash
    * `\t` - Tab character
    * `\r` - Carriage return (for files and such)
* Operations like `WRITE` or `PRINTC` do *not* need quotes to wrap string literals or file pathing.
* All statements, including function definitions or comments **must** end in semicolons.
* Accessing arguments in functions starts with `$` followed by a number (starting from 1). Ex. `$1`, `$2`, etc.
* Lists are 1-indexed (starts from 1).
* Names starting with `$` are reserved, meaning you may not create a variable, list, function, pointer, or alias starting with `$`, nor can you create a pointer/alias to a reserved name.

<br>

**A word of caution** <br>
As of writing this, SIMASJS is not under active development. I will try my best to mark CMAS-only features,
but be warned that most CMAS code may not run correctly on other implementations. <br>

CMAS tries to achieve maximal parity with SIMASJS, and as such supports (nearly) all SIMASJS instructions & behaviour,
but also has its own superset of features & instructions.<br>

#### Known CMAS-only features (excluding instructions):
* Type coersion - For operations like `ADD`, not all operands need to be `num` type, as their values will be coerced into `num`-compatible values.
* Type assignment - For operations like `ADD`, rather than OPERAND 1 always being `num`, you can set it to any type, and the resulting value of the operation
will be coerced into that new type & assigned to OPERAND 2.
* Indexing w/ vars - When indexing a list, CMAS supports indexing using pre-existing vars. This is not supported in SIMASJS as of writing this.
* ***VERY IMPORTANT!*** Lists passed by value - When calling / returning a function, lists in CMAS are passed by VALUE, not reference. This is different from SIMASJS,
and MUST be kept in mind if attempting to write cross-compatible programs.
* CMAS is whitespace-insensitive for tokenisation, so instructions may be split by multiple spaces, lines, etc. This differs from SIMASJS, which requires EXACTLY one space per token.
* CMAS supports calling functions from within functions, which is NOT supported by SIMASJS and will often lead to an infinite loop of execution.
* ***IMPORTANT!*** Function arguments start at $1 - In SIMASJS, however, they start at $0.

# Links
For instructions & datatypes, check <a href="DOCS.md">here.</a>