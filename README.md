# The SIMAS Programming Language
*Created by: Turrnut*<br>
**Brought to C by tuvalutorture.**<br>

License: GPLv3, which is the same as the [original SIMAS repo, which is also licensed under GPLv3](https://github.com/turrnut/simas).

**SIMAS**, which is an acronym for **SIM**ple **AS**sembly, is a semi-statically typed, high level procedural programming language  
with a syntax that is inspired by the Assembly programming language.<br>
In SIMAS, each line starts with an instruction, optionally followed by one or more operands, just like Assembly.<br>
To run a SIMAS program, simply run ```./simas <filename(s)>```. <br>
Additionally, you can use the ```-d``` flag as the last argument to get more info.
You can also run just ```./simas``` for the SIMAS command line.

An demo program, Jeremy Simulator (```jeremy.simas```), is included to showcase CMAS' features & capability.

**Additional Notes** <br>

SIMAS is case-sensitive, although instructions and data types are not. <br>

If you want to be polite to SIMAS, you can add `PLEASE` (case-insensitive) and a space character
in front of any instruction. However, SIMAS will ignore your politeness by ignoring `PLEASE`. <br>

For example, `PLEASE PRINTC Hello!;` and `PRINTC Hello!;` does the same thing.<br>

All occurences of `\n` within string constants (such as PRINTC) will be replaced with a new line.

## DOCUMENTATION 
### DATA TYPES 
#### - bool : a boolean value.
#### - num  : a number, can be an integer or a float
#### - str  : a string of characters
### INSTRUCTIONS

### Control Flow

#### - jump
* jump to a label. this is an unconditional jump
* OPERAND 1: name of the label

#### - jumpv
* jump to a label. this is a conditional jump
* OPERAND 1: name of the label
* OPERAND 2: name of a variable. if true, will jump to the label

#### - jumpnv
* jump to a label. this is a conditional jump
* OPERAND 1: name of the label
* OPERAND 2: name of a variable. if false, will jump to the label

#### - label
* define a label
* OPERAND 1: name of the label

### Math

#### - add
* add the value of OPERAND 2 and 3 (must be `num`) the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first addend, being a variable name
* OPERAND 3: the second addend, optionally being a variable name

#### - sub
* performs operation of OPERAND 2 minus OPERAND 3 (must be `num`) the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the subtrahend, being a variable name
* OPERAND 3: the minuend, optionally being a variable name

#### - mul
* performs operation of OPERAND 2 multiply OPERAND 3 (must be `num`) the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the first factor, being a variable name
* OPERAND 3: the second factor, optionally being a variable name

#### - div
* performs operation of OPERAND 2 divide OPERAND 3 (must be `num`) the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3
* OPERAND 2: the dividend, being a variable name
* OPERAND 3: the divisor, optionally being a variable name

### File I/O

#### - read
* read from a file
* OPERAND 1: path to the file
* OPERAND 2: name of a variable to store the data of the file in

#### - write
* write to a file after erasing all of its contents
* OPERAND 1: path of the file
* OPERAND 2: text to write

#### - writev
* write to a file after erasing all of its contents. same as `write` but writing variables
* OPREAND 1: path of the file
* OPERAND 2: name of the variable whose contents will be written to the file.

### Comparison

#### - and
* performs logical operation AND on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - or
* performs logical operation OR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - nand
* performs logical operation AND on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - nor
* performs logical operation OR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - xor
* performs logical operation XOR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `bool`) 
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - eqc
* equal to comparison operator. value will be assigned to the variable at OPERAND 2
* ATTENTION: please use this ONLY when OPERAND 2 is a variable name and OPERAND 3 is a constant
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: a constant

#### - eqv
* equal to comparison operator. value will be assigned to the variable at OPERAND 2
* ATTENTION: please use this ONLY whend dealing with two variables
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: name of second variable

#### - gt
* greater than comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `num`)
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - gte
* greater than or equal to comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `num`)
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - neqc
* not equal to comparison operator. value will be assigned to the variable at OPERAND 2.
* ATTENTION: please use this ONLY when OPERAND 2 is a variable name and OPERAND 3 is a constant
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: a constant

#### - neqv
* not equal to comparison operator. value will be assigned to the variable at OPERAND 2.
* ATTENTION: please use this ONLY whend dealing with two variables
* OPERAND 1: data type of OPERANDS 2 and 3
* OPERAND 2: name of first variable
* OPERAND 3: name of second variable

#### - st
* smaller than comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `num`)
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - ste
* smaller than or equal to comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type of both OPERAND 2 and 3 (must be `num`)
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

### Lists

#### - list
* list operations. lists are a special data structure that allows you to have multiple values in a single container. Indexing can be done both via constant numbers and variables.
* OPERAND 1: type of operation
* OPERAND 2: name of the list
* All of the list operations:
    * `list new`
        * creates a new list
    * `list appv`
        * append a variable to a list
        * OPERAND 3: data type of the variable
        * OPERAND 4: name of the variable
    * `list appc`
        * append a constant to a list
        * OPERAND 3: data type of the constant
        * OPERNAD 4: the constant
    * `list upv`
        * update the list item at a specific index with a variable
        * OPERAND 3: the index, starting from 1
        * OPERAND 4: the data type of the variable
        * OPERAND 5: name of the variable
    * `list upc`
        * update the list item at a specific index with a constant
        * OPERAND 3: the index, starting from 1
        * OPERAND 4: the data type of the constant
        * OPERAND 5: the constant
    * `list del`
        * delete an item from the list
        * OPERAND 3: the index of the item, starting from 1
    * `list acc`
        * access an item from a list and store it in a variable
        * OPERAND 3: the index, starting from 1
        * OPERAND 4: the name of the variable that you want to store the value in.
    * `list show`
        * print out the entire list to the standard output
    * `list dump`
        * writes a list to a file
        * OPERAND 3: file name
    * `list load`
        * loads a list from a file
        * OPERAND 3: the name of the list you want to store to
        * OPERAND 4: file name
    * `list len`
        * returns the number of elements in a list
        * OPERAND 3: the name of the variable to store the value in

### Console I/O

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
* This is useful in cases where you may need to print something right after using ```print```.

### Functions

#### - fun
* define a function.
* OPERAND 1: The name of the function
* OPERAND 2: The amount of arguments accepted by the function.

#### - call
* call a function
* OPERAND 1: The name of the function of which to execute
* OPERAND 2 (and every even-numbered operand hereon): Only needed if function takes args. The type of data that the arg passed is. V for a pre-existing variable, B for a boolean constant, N for a NUM constant, and S for a string constant.
* OPERAND 3 (and every odd-numbered operand hereon): Only needed if function takes args. The data of the passed argument.

#### - end
* ends a block 
* OPERAND 1: The type of block you are ending (currently only accepts "FUN" for function).

#### - ret
* breaks out of a function
* Important: you MUST call this to break a function, else it will repeat endlessly.
* OPERAND 1: Optional, the type of data being returned. V for a pre-existing variable, B for a boolean constant, N for a NUM constant, and S for a string constant.
* OPERAND 2: Optional, but required if OPERAND 1 is present, and is the data being returned. This is shown in a $\[function name\] variable.

### Misc instructions

#### - @ (comment)
* all code after @ in the current line is ignored. comments must also end in a semicolon.

#### - not
* negation logical operator
* OPERAND 1: name of the variable to be negated (must be a bool)

#### - conv
* Convert to a different data type
* OPERAND 1: name of variable
* OPERAND 2: target data type

#### - copy
* copy a variable's value to another
* OPERAND 1: the name of the variable copying from
* OPERAND 2: the name of the variable copying to

#### - quit
* quits the program

#### - set
* assign a value to a variable.
* OPERAND 1: the type of value. If the operand here is "in", then the value of the user input will be stored at this variable, with `str` type
* OPERAND 2: the name of the variable
* OPERAND 3: the value you wish to assign, if not using "in" as OPERAND 1

#### - type
* get the type of a variable
* OPERAND 1: the variable name
* OPERAND 2: the type of OPERAND 1 will be assigned to this variable as a string