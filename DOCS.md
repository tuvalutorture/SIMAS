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

#### - jumpnv (CMAS only)
* jump to a label. this is a conditional jump
* OPERAND 1: name of the label
* OPERAND 2: name of a variable. if false, will jump to the label

#### - label
* define a label
* OPERAND 1: name of the label

### Math
Note: ALL math ops require both operands going in must be NUM type, and OPERAND 1 must be `num` if you wish to retain SIMASJS compatibility. 

#### - add
* add the value of OPERAND 2 and 3 - the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first addend, being a variable name
* OPERAND 3: the second addend, optionally being a variable name

#### - sub
* performs operation of OPERAND 2 minus OPERAND 3 - the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the subtrahend, being a variable name
* OPERAND 3: the minuend, optionally being a variable name

#### - mul
* performs operation of OPERAND 2 multiply OPERAND 3 - the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first factor, being a variable name
* OPERAND 3: the second factor, optionally being a variable name

#### - div
* performs operation of OPERAND 2 divide OPERAND 3 - the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
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
Note: All comparison operators will OVERWRITE the FIRST variable passed in. Please make sure to `COPY` any variables you don't want to be overwritten.

#### - and
* performs logical operation AND on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - or
* performs logical operation OR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - nand (CMAS only)
* performs logical operation AND on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - nor (CMAS only)
* performs logical operation OR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first boolean variable, being a variable name
* OPERAND 3: the second boolean variable, optionally being a variable name

#### - xor (CMAS only)
* performs logical operation XOR on OPERAND 2 and 3; the value will be assigned to OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
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

#### - gt
* greater than comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - gte
* greater than or equal to comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - st
* smaller than comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

#### - ste
* smaller than or equal to comparison operator. value will be assigned to the variable at OPERAND 2
* OPERAND 1: the data type to assign to OPERAND 2
* OPERAND 2: the first value, being a variable name
* OPERAND 3: the second value, optionally being a variable name

### Lists

#### - list
* list operations. lists are a special data structure that allows you to have multiple values in a single container. 
* Indexing, starting at 1, can be done both via constant numbers and variables. (CMAS only feature)
* OPERAND 1: the operation to run
* OPERAND 2: the name of the list to perform the operation on
* All of the list operations:
    * `list new`
        * creates a new list
    * `list appv`
        * append a variable to a list
        * OPERAND 3: data type of the variable (this operand is ignored in CMAS, but must be there to retain structure & compatibility)
        * OPERAND 4: name of the variable
    * `list appc`
        * append a constant to a list
        * OPERAND 3: data type of the constant
        * OPERNAD 4: the constant
    * `list upv`
        * update the list item at a specific index with a variable
        * OPERAND 3: the index, starting from 1
        * OPERAND 4: the data type of the variable (this operand is ignored in CMAS, but must be there to retain structure & compatibility)
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
    * `list copy` (also can be `COPYL`)
        * copies a list's contents into another list
        * OPERAND 3: the destination list to recieve the data of OPERAND 2.
    * `list alias`
        * assign an ALIAS to a list
        * OPERAND 3: the name of the alias

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
Note: Unlike labels, functions must be defined prior to use.

#### - fun
* define a function.
* OPERAND 1: The name of the function
* OPERAND 2: The amount of arguments accepted by the function. 
* Arguments will appear as $[arg number], starting from 1.

#### - call
* call a function
* OPERAND 1: The name of the function of which to execute
* OPERAND 2 (and every even-numbered operand hereon): Only needed if function takes args. The type of data that the arg passed is. V for a pre-existing variable, L is for a pre-existing list, 
B for a boolean constant, N for a NUM constant, P for a POINTER to a variable, A for an ALIAS to a list, and S for a string constant.
* OPERAND 3 (and every odd-numbered operand hereon): Only needed if function takes args. The data of the passed argument.

#### - end fun
* signals the end of a function (must be included)

#### - ret
* breaks out of a function
* Important: you MUST call this to break a function, else it will repeat endlessly.
* OPERAND 1: Optional, the type of data being returned. V for a pre-existing variable, L is for a pre-existing list, B for a boolean constant, N for a NUM constant, and S for a string constant.
* OPERAND 2: Optional, but required if OPERAND 1 is present, and is the data being returned. This is shown in a $\[function name\] variable.

### Pointers / Aliases
In SIMAS, sometimes you might want to use a POINTER instead of copying a new var, such as if you wanted to pass it into a function to modify it.

POINTERS are not their own type, rather, they are simply a variable that when used, will reference another pointer. If you modify a POINTER's value, it will reflect in the original variable, and vice versa. <br>
ALIASES are like POINTERS, but for lists instead of variables. The same rules apply here. <br>
While POINTERS and ALIASES may share the same namespaces with their respective types, they cannot be promoted to their own variables / lists, and likewise variables / lists cannot be converted to pointers / aliases. <br>
However, you CAN create a POINTER to a POINTER, as well as an ALIAS to an ALIAS. 

**Important note:** A POINTER or ALIAS ***CANNOT*** be made to a local var / list ($1, $2, etc.) or a returned var / list ($functionName).

#### ptr
* assign a POINTER to a variable
* OPERAND 1: name of source variable
* OPERAND 2: name of pointer

#### alias
* assign an ALIAS to a list
* see LIST ALIAS

### Misc instructions

#### - @ (comment)
* all code after @ in the current line is ignored. comments must also end in a semicolon.

#### - not
* negation logical operator
* OPERAND 1: name of the variable to be negated (overwritten with a BOOL)

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

#### - import
* import another SIMAS file
* Note: this also includes all variable names and labels, so make sure to not redefine variables, list names, functions, etc. You may also only import a file once.
* Note 2: import file pathing is relative to where you are RUNNING SIMAS from, NOT where your SIMAS file is.
* OPERAND 1: the file path