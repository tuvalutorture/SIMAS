# The SIMAS Programming Language
*Created by: Turrnut*<br>
**Brought to C by tuvalutorture.**<br>

License: [Creative Commons Attribution Non-Commercial](https://creativecommons.org/licenses/by-nc/4.0/) (applies to all CODE in this repository)
This README is adapted from a significant chunk of the [original SIMAS repo, which is licensed under GPLv3, henceforth this readme is also under the GPL v3 license](https://github.com/turrnut/simas).

**SIMAS**, which is an acronym for **SIM**ple **AS**sembly, is a semi-statically typed, high level procedural programming language  
with a syntax that is inspired by the Assembly programming language.<br>
In SIMAS, each line starts with an instruction, optionally followed by one or more operands, just like Assembly.<br>
To run a SIMAS program, simply run ```./simas <filename>```.

**Additional Notes** <br>

SIMAS is case-sensitive, although instructions and data types are not. <br>

If you want to be polite to SIMAS, you can add `PLEASE` (case-insensitive) and a space character
in front of any instruction. However, SIMAS will ignore your politeness by ignoring `PLEASE`. <br>

For example, `PLEASE PRINTC Hello!;` and `PRINTC Hello!;` does the same thing.<br>

All occurences of `\n` will be replaced with a new line, and `\\` with `\`.

**Differences** <br>

In this version of SIMAS, which can also be called CMAS (C SIMAS),  
there are a few key differences between CMAS and SIMASJS (the main fork).  

Differences:
* CMAS does not support Lists or Functions at this moment.
* CMAS does not require a compilation step.
* CMAS runs atop a C based interpreter, while SIMASJS is JavaScript based.
* For most things, CMAS requires you to create a var AHEAD of use (though math ops and COPY will automatically create one of the desired name if it doesn't exist)

> [!NOTE]
> This is a REIMPLEMENTATION of the majority of SIMAS features. This may behave differently or not support certain commands.<br>
> Also, as of now, SIMAS/CMAS has *ONLY* been confirmed to work on Mac (no testing has been done on other platforms).

> [!IMPORTANT]
> When doing file I/O (READ, WRITE, WRITEV), you MUST give an EXPLICIT path to the file, or else it will fail.

## DOCUMENTATION 
### DATA TYPES 
#### - bool : a boolean value.
#### - num  : a number, can be an integer or a decimal
#### - str  : a string of characters
### INSTRUCTIONS
#### - @
* all code after @ in the current line is ignored, until a semicolon is seen. this is a comment feature

#### - add
* add the value of OPERAND 2 and 3 (must be `num`) the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first addend, optionally being a variable name
* OPERAND 3: the second addend, optionally being a variable name

#### - and
* performs logical operation AND on OPERAND 2 and 3; the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, optionally being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - conv
* Convert to a different data type
* OPERAND 1: name of variable
* OPERAND 2: target data type

#### - copy
* copy a variable's value to another
* OPERAND 1: the name of the variable copying from
* OPERAND 2: the name of the variable copying to

#### - div
* performs operation of OPERAND 2 divide OPERAND 3 (as of now can only handle num) the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the dividend, optionally being a variable name
* OPERAND 3: the divisor, optionally being a variable name

#### - eqc
* equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* ATTENTION: please use this ONLY when OPERAND 2 is a variable name and OPERAND 3 is a constant
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: a constant

#### - eqv
* equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* ATTENTION: please use this ONLY whend dealing with two variables
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: name of second variable

#### - gt
* greater than comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first value, optionally being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - gte
* greater than or equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first value, optionally being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - jump
* jump to a label. this is an unconditional jump
* OPERAND 1: name of the label

#### - jumpv
* jump to a label. this is a conditional jump
* OPERAND 1: name of the label
* OPERAND 2: name of a variable. if true, will jump to the label

#### - label
* define a label
* labels MUST be defined in a line prior to the line in which it is used
* OPERAND 1: name of the label

#### - mul
* performs operation of OPERAND 2 multiply OPERAND 3 (as of now can only handle num) the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first factor, optionally being a variable name
* OPERAND 3: the second factor, optionally being a variable name

#### - neqc
* not equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* ATTENTION: please use this ONLY when OPERAND 2 is a variable name and OPERAND 3 is a constant
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: a constant

#### - neqv
* not equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* ATTENTION: please use this ONLY whend dealing with two variables
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: name of second variable

#### - not
* negation logical operator
* OPERAND 1: name of the variable to be negated

#### - or
* performs logical operation OR on OPERAND 2 and 3; the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, optionally being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - print
* print something to the console.
* OPERAND 1: The name of the variable which the information is stored

#### - printc
* print a constant to the console.
* OPERAND 1: The constant being printed

#### - println
* print a new line

#### - prints
* print a space

#### - quit
* quits the program

#### - read
* read from a file
* OPERAND 1: path to the file
* OPERAND 2: name of a variable to store the data of the file in

#### - set
* assign a value to a variable.
* OPERAND 1: the type of value. If the operand here is "in", then the value of the user input will be stored at this variable, with `str` type
* OPERAND 2: the name of the variable
* OPERAND 3: the value you wish to assign, if not using "in" as OPERAND 1

#### - st
* smaller than comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first value, optionally being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - ste
* smaller than or equal to comparison operator. value will be assigned to the variable at OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first value, optionally being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - sub
* performs operation of OPERAND 2 minus OPERAND 3 (as of now can only handle num) the value will be assigned to OPERAND 2, if it is a variable name
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the subtrahend, optionally being a variable name
* OPERAND 3: the minuend, optionally being a variable name

#### - write
* write to a file after erasing all of its contents
* OPERAND 1: path of the file
* OPERAND 2: text to write

#### - writev
* write to a file after erasing all of its contents. same as `write` but writing variables
* OPREAND 1: path of the file
* OPERAND 2: name of the variable whose contents will be written to the file.
