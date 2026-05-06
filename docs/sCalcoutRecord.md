---
layout: default
title: sCalcout Record
nav_order: 4
---

# scalcout - String Calculation Output Record


## Contents

1. [Introduction](#1-introduction)
2. [Scan Parameters](#2-scan-parameters)
3. [Read Parameters](#3-read-parameters)
4. [Expressions](#4-expressions)
   - [4.1. Operands](#41-operands)
   - [4.2. Algebraic Functions/Operators](#42-algebraic-functionsoperators)
   - [4.3. Trigonometric Functions](#43-trigonometric-functions)
   - [4.4. Relational Operators](#44-relational-operators)
   - [4.5. Logical Operators](#45-logical-operators)
   - [4.6. Bitwise Operators](#46-bitwise-operators)
   - [4.7. Special Characters](#47-special-characters)
   - [4.8. Conditional Expression](#48-conditional-expression)
   - [4.9. String Functions/Operators](#49-string-functionsoperators)
   - [4.10. Array Operators](#410-array-operators)
   - [4.11. Miscellaneous Operators](#411-miscellaneous-operators)
   - [4.12. Examples](#412-examples)
      - [Algebraic](#algebraic)
      - [Relational](#relational)
      - [Question Mark](#question-mark)
      - [Logical](#logical)
      - [String](#string)
      - [Argument Array](#argument-array)
      - [Store](#store)
      - [Loop](#loop-until)
5. [Output Parameters](#5-output-parameters)
6. [Operator Display Parameters](#6-operator-display-parameters)
7. [Alarm Parameters](#7-alarm-parameters)
8. [Monitor Parameters](#8-monitor-parameters)
9. [Run-time Parameters](#9-run-time-parameters)
10. [Record Support Routines](#10-record-support-routines)
    - [init_record](#init_record)
    - [process](#process)
    - [special](#special)
    - [get_value](#get_value)
    - [get_units](#get_units)
    - [get_precision](#get_precision)
    - [get_graphic_double](#get_graphic_double)
    - [get_control_double](#get_control_double)
    - [get_alarm_double](#get_alarm_double)
11. [Record Processing](#11-record-processing)
    - [11.1. process()](#111-process)
    - [11.2. execOutput()](#112-execoutput)
12. [Sample Database](#12-sample-database)

---


## 1. Introduction


The String Calculation Output or "scalcout" record is derived from the Calcout
record in EPICS base, and extends it by supporting string expressions in
addition to numeric expressions.  The record has 12 string fields (AA...LL)
and 12 double fields (A...L) which are used as input variables for the
expression.  It calls an extended version of the EPICS calculation engine that
accepts string arguments, supports a variety of string functions, and produces a
string result and a numeric result.


If the expression involves any string operators or operands, the calc engine
produces a string result, and converts it to double, using atof(); otherwise, it
produces a numeric result and converts it to string, using cvtDoubleToString()
from EPICS base.


When writing to a string PV (any of DBF_STRING, DBF_ENUM, DBF_MENU,
DBF_DEVICE, DBF_INLINK, DBF_OUTLINK, DBF_FWDLINK) the record (actually, device
support) writes its string value (SVAL or OSV).  When writing to any other kind
of PV, the record writes its numeric value (VAL or OVAL).


To write successfully to a DBF_MENU or DBF_ENUM (for example, the VAL field
of a `bo` or `mbbo` record) the record's string value must
be one of the possible strings for the PV, or it must an integer specifying the
string number [0..N] for the PV.  For example, when writing to a `bo`
record whose ZNAM is "No" and whose ONAM is "Yes", the string value must be one
of the following: "No", "Yes", "0", and "1".  To ensure that numeric values are
converted to integers, set the precision (the PREC field) to zero.


---


## 2. Scan Parameters


The scalcout record has the standard fields for specifying under what
circumstances the record will be processed.  See the EPICS Record Reference
Manual for these fields and their use.


---


## 3. Read Parameters


The read parameters for the scalcout record consist of 24 input links:
12 to numeric fields (INPA -> A, INPB -> B, . . . INPL -> L); and 12 to
string fields (INAA -> AA, INBB -> BB, ...INLL -> LL). The fields can be
database links, channel access links, or constants. If they are links,
they must specify another record's field. If they are constants, they will
be initialized with the value they are configured with and can be changed
via `dbPuts`. These fields cannot be hardware addresses. In addition,
the scalcout record contains the fields INAV, INBV, . . . INLV, which indicate
the status of the links to numeric fields, and the fields IAAV, IBBV, .
. . ILLV, which indicate the status of the links to string fields.
These fields indicate whether or not the specified PV was found and a link
to it established. See [Section 6, *Operator Display
Parameters*](#6-operator-display-parameters) for an explanation of these fields.


From most types of PV's, the scalcout record's string input links fetch data
as strings, and depend on EPICS to convert a PV's native value to string.  But
when an scalcout record's string input link names a PV whose data type is an
array of DBF_CHAR or DBF_UCHAR, the record fetches up to 39 array elements from
the array in their native form, translates the result using
epicsStrSnPrintEscaped(), strips any trailing "\000" string fragments, and
truncates the result to fit into a 40-character string.


See the EPICS Record Reference Manual for information on how to specify database links.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor |
| --- | --- | --- | --- | --- | --- | --- | --- |
| INPA | Input Link A | INLINK | Yes | 0 | Yes | Yes | N/A |
| INPB | Input Link B | INLINK | Yes | 0 | Yes | Yes | N/A |
| ... | ... | ... | ... | ... | ... | ... | ... |
| INPL | Input Link L | INLINK | Yes | 0 | Yes | Yes | N/A |
| INAA | Input Link AA | INLINK | Yes | 0 | Yes | Yes | N/A |
| INBB | Input Link BB | INLINK | Yes | 0 | Yes | Yes | N/A |
| ... | ... | ... | ... | ... | ... | ... | ... |
| INLL | Input Link LL | INLINK | Yes | 0 | Yes | Yes | N/A |


---


## 4. Expressions


Like the Calc record, the scalcout record has a CALC field into which
you can enter an expression for the record to evaluate when it processes.  The resulting numeric value will be placed in the VAL field, and the resulting string value will be placed in the SVAL field.  VAL can then be used by the OOPT field (see [Section 5, *Output Parameters*](#5-output-parameters)) to determine whether or not to write to the output link or post an output event. Either VAL and SVAL can also be written to the output link.  (If you elect to write an output value, the record will choose between VAL and SVAL, depending on the data type of the field at the other end of the output link.)


The CALC expression is actually converted to opcodes and stored in
postfix notation in the RPCL field.  It is this expression which is actually
used to calculate VAL.  The postfix expression is evaluated more efficiently at
run-time than an infix expression would be.  When CALC is changed at run-time,
the record-support routine `special()` will call a function to check
it, and convert it to postfix notation.


The record also has a second set of calculation-related fields described in [Section 5, *Output Parameters.*](#5-output-parameters)


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| CALC | Calculation | STRING[80] | Yes | 0 | Yes | Yes | Yes | No |
| VAL | Value | DOUBLE | No | 0 | Yes | Yes | Yes | No |
| RPCL | Reverse Polish | NOACCESS | No | 0 | No | No | N/A | No |
| SVAL | String value | STRING (40) | No | 0 | Yes | Yes | Yes | No |


Expressions supported by the string calculation record can involve operands,
algebraic operators and functions, trigonometric functions, relational
operators, logical operators, string operators and functions, parentheses
and commas, and the conditional '?:' operator. The expression can consist
of any of these operators, as well as any of the values from the input
fields, which are the operands.


Grouping of operands with parentheses is important not only to
specify the order in which operations are to be done, but also to distinguish
operands from operators whose names are spelled out, such as the logical
operator 'AND'.  The expression 'AANDB', which could have meant 'A AND B',
will be misinterpreted by the parser, which will interpret 'AA' as the name
of a string variable.


### 4.1. Operands


The expression can use as operands the values retrieved from the input links,
and literal constants such as "abc", "1.5", and "23". (Numeric constants in
expressions must be either floating-point numbers, or integers in base ten.
Numbers like "0x03" will not be understood by the expression parser.)  Values
retrieved from the input links are stored in the A-L, and AA-LL fields. The
values to be used in the expression are simply referenced by the field name. For
example, the value obtained from the INPA link is stored in the field A, and the
value obtained from INPB is stored in field B. The field names can be included
in the expression which will operate on their respective values, as in "A+B".


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| A | Input Value A | DOUBLE | No | 0 | Yes | Yes/No* | Yes | Yes |
| B | Input Value B | DOUBLE | No | 0 | Yes | Yes/No* | Yes | Yes |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| L | Input Value L | DOUBLE | No | 0 | Yes | Yes/No* | Yes | Yes |
| AA | Input string AA | STRING (40) | No | 0 | Yes | Yes/No* | Yes | Yes |
| BB | Input string BB | STRING (40) | No | 0 | Yes | Yes/No* | Yes | Yes |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| LL | Input string LL | STRING (40) | No | 0 | Yes | Yes/No* | Yes | Yes |


\* If a valid input link is associated with this field, then it may
not be modified.


There are a few special operands not associated with input fields, but
defined by the record (more exactly, defined by the calc engine the record uses
to evaluate expressions).  All but `RNDM` are constants.


| Operand | Description |
| --- | --- |
| PI | 3.141592654 |
| D2R | Degrees to radians (PI/180) |
| R2D | Radians to degrees (1/D2R) |
| S2R | Arc seconds to radians (D2R/3600) |
| R2S | Radians to arc seconds (1/S2R) |
| RNDM | Random number between 0 and 1. |


### 4.2. Algebraic Functions/Operators


| Op | Description | Example |
| --- | --- | --- |
| ABS | Absolute value (one-argument function) | `ABS(A)` |
| SQRT | Square root (one-argument function) (SQR is deprecated) | `SQRT(A)` |
| MIN | Minimum (two-or-more-argument function) | `MIN(A,B,C)` |
| MAX | Maximum (two-or-more-argument function) | `MAX(A,B,C)` |
| CEIL | Ceiling (one-argument function) | `CEIL(A)` |
| FLOOR | Floor (one-argument function) | `FLOOR(A)` |
| INT | Nearest integer (one-argument function) | `INT(A)` |
| NINT | Nearest integer (one-argument function) | `NINT(A)` |
| LOG | Log base 10 (one-argument function) | `LOG(A)` |
| LN | Natural logarithm (one-argument function) | `LN(A)` |
| LOGE | Deprecated synonym for 'LN' | `LOGE(A)` |
| EXP | Exponential function (unary) | `EXP(A)` |
| ^ | Exponential (binary) (Same as '\*\*'.) | `A^B` |
| \*\* | Exponential (binary) (Same as '^'.) | `A**B` |
| + | Addition (binary) | `A+B` |
| - | Subtraction (binary) | `A-B` |
| \* | Multiplication (binary) | `A*B` |
| / | Division (binary) | `A/B` |
| % | Modulo (binary) | `A%B` |
| - | Negate (unary) | `-A` |
| NOT | Negate (unary) | `NOT A` |
| >? | Max (binary) | `A>?B` |
| <? | Min (binary) | `A<?B` |


### 4.3. Trigonometric Functions


| Op | Description | Example |
| --- | --- | --- |
| SIN | Sine (one-argument function) | `SIN(A)` |
| SINH | Hyperbolic sine (one-argument function) | `SINH(A)` |
| ASIN | Arc sine (one-argument function) | `ASIN(A)` |
| COS | Cosine (one-argument function) | `COS(A)` |
| COSH | Hyperbolic cosine (one-argument function) | `COSH(A)` |
| ACOS | Arc cosine (one-argument function) | `ACOS(A)` |
| TAN | Tangent (one-argument function) | `TAN(A)` |
| TANH | Hyperbolic tangent (one-argument function) | `TANH(A)` |
| ATAN | Arc tangent (one-argument function) | `ATAN(A)` |
| ATAN2 | Alternate form of arctangent (two-argument function) | `ATAN2(A,B)` |


### 4.4. Relational Operators


| Op | Description | Example |
| --- | --- | --- |
| >= | Greater than or equal to | `A>=B` |
| > | Greater than | `A>B` |
| <= | Less than or equal to | `A<=B` |
| < | Less than | `A<B` |
| != | Not equal to (same as '#') | `A!=B` |
| # | Not equal to (same as '!=') | `A#B` |
| == | Equal to (same as '=') | `A==B` |
| = | Equal to (same as '==') | `A=B` |


### 4.5. Logical Operators


| Op | Description | Example |
| --- | --- | --- |
| && | Logical AND | `A&&B` |
| \|\| | Logical OR | `A\|\|B` |
| ! | Logical NOT | `!A` |


### 4.6. Bitwise Operators


| Op | Description | Example |
| --- | --- | --- |
| \| | Bitwise OR | `A\|B` |
| OR | Bitwise OR | `A OR B` |
| & | Bitwise AND | `A&B` |
| AND | Bitwise AND | `A AND B` |
| XOR | Bitwise Exclusive OR | `A XOR B` |
| ~ | One's Complement | `~A` |
| << | Left shift | `A<<B` |
| >> | Right shift | `A>>B` |


### 4.7. Special Characters


Parentheses, and nested parentheses are permitted (sometimes required) in expressions.


The comma is required to separate the arguments of a binary function.


Quotes of two kinds (" and ') are permitted to indicate the beginning and end of a string expression.
The expression parser treats these quote characters in exactly the same way, so you can use pairs of
either.  Using one kind of quote allows you to include the other kind in a string.  Note that string calc
expressions defined in an EPICS database must use the ' character to delimit embedded strings, because
database tools will use " to delimit the entire expression.


Escape sequences understood by the Epics routines dbTranslateEscape() and
epicsStrSnEscaped() are permitted in strings handled by the scalcout record,
and the sCalc engine supplies functions that encode and decode escaped
strings.  Here's the current escape list:

` \a \b \f \n \r \t \v \\ \? \' \" \000 \xhh`


Note that '\000' represents an octal number, and may include one, two, or
three octal digits.  \xhh is a two-digit hexadecimal number.


See the Application Developer's Guide for more on this topic.


### 4.8. Conditional Expression


The C language's question mark operator is supported. The format is:


```
 <expression> ? <expression-true result> : <expression-false  result>
```


### 4.9. String Functions/Operators


Most two-argument string functions and binary string operators are "overloaded"
numeric functions and operators that perform string operations only if
both of their arguments/operands are strings.


| Op | Description | Example |
| --- | --- | --- |
| MIN | Minimum lexically (two-or-more-argument function) | `MAX('a','b','c') -> 'a'` |
| MAX | Maximum lexically (two-or-more-argument function) | `MAX('a','b','c') -> 'c'` |
| INT | Find first number in string; return nearest integer (one-argument function) | `INT('1.9') -> 1` / `INT('abc1.9') -> 1` |
| NINT | Find first number in string; return nearest integer (one-argument function) | `NINT('1.9') -> 2` / `NINT('abc1.9') -> 2` |
| + | Concatenate (binary) | `'a'+'b' -> 'ab'` |
| - | Delete first occurrence of substring (binary) | `'abca'-'a' -> 'bca'` |
| -\| | Delete first occurrence of substring (binary) | `'abca'-\|'a' -> 'bca'` |
| \|- | Delete last occurrence of substring (binary) | `'abca'\|-'a' -> 'abc'` |
| >= | Lexically greater than or equal (binary) | `'a'>='b' -> 0` |
| > | Lexically greater than (binary) | `'a'>'b' -> 0` |
| <= | Lexically less than or equal (binary) | `'a'<='b' -> 1` |
| < | Lexically less than (binary) | `'a'<'b' -> 1` |
| != | Lexically not equal to (binary) | `'a'!='b' -> 1` |
| == | Lexically equal to (binary) | `'a'=='b' -> 0` |
| >> | Right shift characters (fill behind with spaces). (binary) | `'abc'>>2 -> '  abc'` |
| << | Left shift characters. (binary) | `'abc'<<2 -> 'c'` |
| DBL | Convert argument to double (one-argument function) | `DBL('1') -> 1.0` / `DBL('abc1.23') -> 1.23` |
| STR | Convert argument to string (one-argument function) | `STR(1) -> '1.00000000'` |
| BYTE | Convert first character of string argument to double (one-argument function) | `BYTE('abc') -> 97.0` |
| PRINTF | Return string constructed from format string and string or double argument (two-argument function). (NOTE: '$P' is a synonym for PRINTF.) Allowed format indicators: %cdeEfgGiosuxX | `PRINTF("%.2f",1.23) -> '1.23'` |
| SSCANF | Return string or double parsed from string argument according to format string (two-argument function). (NOTE: '$S' is a synonym for SSCANF.) Allowed format indicators: %\*[cdeEfgGhilosuxX. Disallowed indicators: $npw | `SSCANF('V=1.25', "%*2c%lf") -> 1.25` |
| READ | Read binary data. Similar in form to SSCANF; operates on binary data. The first argument is the string from which data is to be copied; READ() converts it using dbTranslateEscape(), copies bytes into a temporary numeric variable, converts to double, and returns it. (NOTE: '$R' is a synonym for READ.) See format table below. | `READ('\x01\x02', "%hu") -> 258` |
| WRITE | Write binary data. Similar in form to PRINTF; operates on binary data. The first argument is the format string; the second is the data to be written. The buffer is processed by epicsStrSnEscaped() and returned. (NOTE: '$W' is a synonym for WRITE.) See format table below. | `WRITE('%hd', 1250) -> '\004\342'` |
| TR_ESC | Translate escape sequences into the characters they represent (one-argument function). (NOTE: '$T' is a synonym for TR_ESC.) Applies dbTranslateEscape() to its argument. Approximately the opposite of ESC(). An escaped null character will terminate the result string. | `TR_ESC("a\x62c") -> 'abc'` |
| ESC | Translate escape characters into equivalent escape sequences. (one-argument function) (NOTE: '$E' is a synonym for ESC.) Applies epicsStrSnEscaped() to its argument. Approximately the opposite of TR_ESC(). The first null character found in the input string terminates it. | `ESC("a<return>c") -> 'a\rc'` |
| CRC16 | Calculate Modbus/RTU 16-bit CRC and return it as an escaped string. | `CRC16('\x01\x03') -> '\x01\x03\x40\x21'` |
| MODBUS | Calculate Modbus/RTU 16-bit CRC and append it, as an escaped string, to the argument. | `CRC16('\x01\x03') -> '\x40\x21'` |
| LRC | Calculate Modbus/ASCII 8-bit LRC and return it as an escaped string. | `LRC('\x01\x03') -> '\xfc'` |
| AMODBUS | Calculate Modbus/ASCII 8-bit LRC and append it, as an escaped string, to the argument. Note that this doesn't do everything needed for an actual device. | `AMODBUS('\x01\x03') -> '\x01\x03\xfc'` |
| XOR8 | Calculate 8-bit XOR checksum and return it as an escaped string. | `XOR8('\x01\x03') -> '\x02'` |
| ADD_XOR8 | Calculate 8-bit XOR checksum and append it, as an escaped string, to the argument. | `ADD_XOR8('\x01\x03') -> '\x01\x03\x02'` |
| LEN | Return length of string argument. If arg is not a string, it will be converted to string. | `LEN('abc') -> 3` |
| [ | Substring | `'abcdef'[1,3] -> 'bcd'` / `'abcdef'['ab','ef'] -> 'cd'` / `'abcdef'[0,-1] -> 'abcdef'` |
| { | Substring substitution | `'abcdef'{'cd','XX'} -> 'abXXef'` |
| " | String indicator | `"abc"` |
| ' | String indicator | `'abc'` |

Functions and operators marked **(experimental)** may have names or implementations that change in a future version of this software.

**READ format table:**

| Format | Resulting behavior |
| --- | --- |
| %i %d | copy four bytes to a long signed integer |
| %hi %hd | copy two bytes to a short signed integer |
| %o %u %x %X | copy four bytes to a long unsigned integer |
| %ho %hu %hx %hX | copy two bytes to a short unsigned integer |
| %e %E %f %g %G | copy four bytes to a float |
| %le %lE %lf %lg %lG | copy eight bytes to a double |
| %c | copy one byte to a signed character |
| %\* | suppress assignment to next conversion indicator. Only one '%\*' is honored. |

*This is the only reliable way to skip past a specified number of bytes in a
binary input string.  For a non-escaped string, something like '`aa[3,5]`'
would have worked as well.  But if the string `aa` might include
escape sequences, character offsets in the escaped string will depend on the
values of the characters, because printable characters are not escaped.*

Allowed format indicators: **%\*cdeEfgGhilouxX**

Disallowed indicators: **$npw[s**

**WRITE format table:**

| Format | Resulting behavior |
| --- | --- |
| %i %d | convert to a long signed integer and write four bytes |
| %hi %hd | convert to a short signed integer and copy two bytes |
| %o %u %x %X | convert to a long unsigned integer and copy four bytes |
| %ho %hu %hx %hX | convert to a short unsigned integer and copy two bytes |
| %e %E %f %g %G | convert to a float and copy four bytes |
| %le %lE %lf %lg %lG | convert to a double and copy eight bytes |
| %c | convert to a signed character and copy one byte |

Allowed format indicators: **%cdeEfgGhilouxX**

Disallowed indicators: **$npw[s**


### 4.10. Array Operators


| Op | Description | Example |
| --- | --- | --- |
| @ | Numeric array element. Regard the numeric fields A-L as an array whose elements are numbered 0-11, and return the element whose number follows. (unary) | `@A` |
| @@ | String array element. Regard the string fields AA-LL as an array of strings whose elements are numbered 0-11, and return the element whose number follows. (unary) | `@@A` |


### 4.11. Miscellaneous Operators


| Op | Description | Examples |
| --- | --- | --- |
| := | Store value of right hand side in location specified by left hand side. (binary) | `A:=1.2` / `@7:='abc'` / `AA:='abc'` / `@@4:='abc'` |
| UNTIL | Execute expression until its value is TRUE. (binary) The total number of iterations is limited to the ioc-shell variable sCalcLoopMax, which defaults to 1000. | `until(1)` / `until(a:=a+1;b:=b-1;b<1)` |


### 4.12. Examples


---


#### Algebraic


`A + B + 10`

Result is `A + B + 10`


---


#### Relational


`(A + B) < (C + D)`

Result is `1` if `(A+B) < (C+D)`

Result is `0` if `(A+B) >= (C+D)`


---


#### Question Mark


`(A+B)<(C+D)?E:F+L+10`

Result is `E` if `(A+B) < (C+D)`

Result is `F+L+10` if `(A+B) >= (C+D)`


`(A+B)<(C+D)?E`

Result is `E` if (A+B) < (C+D)

Result is unchanged if `(A+B) >= (C+D)`


---


#### Logical


`A&B`

Causes the following to occur:

- Convert `A` to integer
- Convert `B` to integer
- Perform bit-wise `AND` of `A` and `B`
- Convert result to floating point


---


#### String


`A + "abc"` / `A + "abc1.2"`

`A + AA`  *(where AA = "abc1.2")*

Result is `A`.  The strings `"abc"` and
`"abc1.2"` will be converted implicitly to the number `0`.


`A + DBL("abc1.2")`

Result is `A + 1.2`.  Explicit conversion to a number
(by `DBL`, `INT`, or `NINT`) is more
aggressive than implicit conversion, and skips leading non-numeric characters.


`"abc" + "def"`

Result is `"abcdef"`.


`PRINTF("abc%1.2f", A)` *(where A = 1.2345)*

Result is  `"abc1.23"`.


NOTE: unlike the c-language function

printf(\<format\>,\<arg1\>,\<arg2\>,...),

this function accepts only one argument after the format string.


`SSCANF(AA, "%*3c%lf")` *(where AA = "abc1.2")*

Result is `"1.20000000"`.  (The format fragment
`"%*3c"` directs sscanf to ignore three characters.)


NOTE: unlike the c-language function

sscanf(\<source\>,\<format\>,\<dest1\>,\<dest2\>,...),

this function cannot store to a variable, but only to the value
stack.


`BYTE("ABC")`

Result is `65`, the ASCII code for "A" in decimal.


`"abcdef"[2,4]`

Result is `"cde"`.  (The first character of the string
is numbered "0".)


`"abcdef"[-2,-1]`

Result is `"ef"`.  (The last character of the string is
numbered "-1".)


`"abcdef"["ab","ef"]`

Result is `"cd"`.  (If the first argument is a string,
it returns the index of the first character *after* a successful match,
or the first character in the string if no match was found.  If the
second argument is a string, it returns the index of the last character
*before* a successful match, or the last character in the string if no
match was found.)


`A==2 ? "yes" : "no"`

Result is `"yes"` if `A == 2`; otherwise, result
is `"no"`.


`"abcdef"{"bcd","dcb"}`

Result is `"adcbef"`.


`"abcdef"{"zzz","dcb"}`

Result is `"abcdef"`.  (If no match, then no replacement.)


`"abcdef"[1,-2][1,-2]`

Result is `"cd"`


---


#### Argument array


`@0`

Result in the value of the numeric variable `A`.  ("`@0`"
is just another name for `A`.)


`@@0`

Result in the value of the string variable AA.


`@(A+B)`

Result in the value of the numeric variable whose number is given by the sum
of A and B.


---


#### Store


The "Store" operator is the only string-calc operator that does not produce a
value.  Thus, the expression `a:=0` is an incomplete and therefore illegal calc expression,
because it leaves us with nothing to write to the record's `VAL` field.


`A:=A-1;7`

Evaluate the expression `A-1`, store the result in the input
variable `A`, and set the VAL field to 7.


`@0:=A-1;7`

Same as above, because `@0` is just another name for `A`


`D:=0;@D:=A-1;7`

Same as above, because `D==0`.


`AA:="abc";7`

Overwrite the string input variable `AA` with the string 'abc', and set the VAL field to 7.


`AA:="abc";b:=0;7`

Multiple store expressions, separated by ';' terminators, are legal at top
level, but - as always - the expression as a whole must produce a value.  This expression
produces the value `7`.


`A+(AA:="abc";b:=0;7)`

Multiple store expressions are also legal within parentheses, and again the
parenthesized subexpression must produce a value.  The parenthesized
subexpression in this example produces the value `7`, which is added to
`A`.


---


#### Loop ("UNTIL")


The `UNTIL` function evaluates its expression repeatedly until the
expression returns a nonzero value, or the allowed number of iterations
`sCalcLoopMax` has been reached.  When looping is done, or aborted,
the expression value is returned.


`UNTIL(1)`

The expression 1 is evaluated, terminates the loop, and is returned.  This
do-nothing expression is equivalent to `(1)`.


`B:=10;UNTIL(B:=B-1;B<1)`

This do-almost-nothing
expression initializes B to 10, decrements it repeatedly until its value is
zero, and returns the value 1 (True).


`B:=9; AA:=''; UNTIL(AA:=AA+CC[B,B]; B:=B-1; B<0)`

If we start with `CC='abcdefghij'`, this expression loops
backwards through `CC`, appending characters to produce
`AA='jihgfedcba'`.


`B:=0;AA:='';UNTIL(AA:=AA+(CC[b,b]==','?'':CC[b,b]);B:=B+1;B>LEN(CC))`

This expression copies CC to AA, leaving out the commas.


`AA:='';B:=1;UNTIL(A:=0;C:=UNTIL(AA:=AA+(@@B)[A,A];A:=A+1;A>1);B:=B+1;B>12)`

This nested loop copies the first two characters from each of the
string-input fields BB-LL, and appends them to AA.  Note that the inner "UNTIL"
function produces a value that must be discarded (by storing it to C), because
it is part of the outer UNTIL function's expression.  That expression must
produce a single value, which the UNTIL function uses to determine when it's
done.


---


## 5. Output Parameters


These parameters specify and control the output capabilities of the
scalcout record. They determine when to write the output, where to write
it, and what the output will be. The OUT link specifies the Process Variable
to which the result will be written. The OOPT field determines the condition
that causes the output link to be written to. It's a menu field that has
six choices:


| Option | Description |
| --- | --- |
| `Every Time` | write output every time record is processed. |
| `On Change` | write output every time VAL changes, i.e., every time the result of the expression changes. |
| `When Zero` | when record is processed, write output if VAL is zero. |
| `When Non-zero` | when record is processed, write output if VAL is non-zero. |
| `Transition to Zero` | when record is processed, write output only if VAL is zero and last value was non-zero. |
| `Transition to Non-zero` | when record is processed, write output only if VAL is non-zero and last value was zero. |
| `Never` | Don't write output ever. |

The DOPT field determines what data is written to the output link when
the output is executed. The field is a menu field with two options: `Use
CALC` or `Use OCAL`. If `Use CALC` is specified, when
the record writes its output it will write the result of the expression
in the CALC record, that is, it will write the value of the VAL [SVAL]
field to a double [string] destination. If `Use OCAL` is specified,
the record will instead write the result of the expression in the OCAL
field, which is contained in the OVAL field (string result in the OSV)
field. The OCAL field is exactly like the CALC field and has the same functionality:
it can contain the string representation of an expression which is evaluated
at run-time. Thus, if necessary, the record can use the result of the CALC
expression to determine if data should be written and can use the result
of the OCAL expression as the data to write.


If the OEVT field specifies a non-zero integer and the condition
in the OOPT field is met, the record will post a corresponding event. If
the ODLY field is non-zero, the record pauses for the specified number
of seconds before executing the OUT link or posting the output event. During
this waiting period the record is "active" and will not be processed again
until the wait is over. The field DLYA is equal to 1 during the delay period.
The resolution of the delay entry is 1/60 of a second (it uses the VxWorks
`tickLib`).


The IVOA field specifies what action to take with the OUT link
if the scalcout record enters an INVALID alarm status. The options are
`Continue normally`, `Don't drive outputs`, and `Set output
to IVOV`. If the IVOA field is `Set output to IVOV`, the data
entered into the IVOV field is written to the OUT link if the record alarm
severity is INVALID.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| OUT | Output Specification | OUTLINK | Yes | 0 | Yes | Yes | N/A | No |
| OOPT | Output Execute Option | Menu | Yes | 0 | Yes | Yes | No | No |
| DOPT | Output Data Option | Menu | Yes | 0 | Yes | Yes | No | No |
| OCAL | Output Calculation | STRING[36] | Yes | Null | Yes | Yes | No | No |
| OVAL | Output Value | DOUBLE | No | 0 | Yes | Yes | Yes | No |
| OEVT | Event To Issue | SHORT | Yes | 0 | Yes | Yes | No | No |
| ODLY | Output Execution Delay | FLOAT | Yes | 0 | Yes | Yes | No | No |
| IVOV | Invalid Output Action | Menu | Yes | 0 | Yes | Yes | No | No |
| IVOA | Invalid Output Value | DOUBLE | Yes | 0 | Yes | Yes | No | No |
| OSV | Output string value | STRING[40] | NO | 0 | Yes | Yes | Yes | No |
| WAIT | Wait for completion? | Menu | Yes | "NoWait" | Yes | Yes | Yes | No |


The scalcout record now uses device support to write to the
`OUT` link. Soft device supplied with the record is selected with
the .dbd specification
```
field(DTYP,"Soft Channel")
```


This device support uses the record's `WAIT` field to determine
whether to wait for completion of processing initiated by the `OUT`
link before causing the record to execute its forward link.  The mechanism by
which this waiting for completion is performed requires that the
`OUT` link have the attribute `CA` -- i.e., the link text
looks something like


`  xxx:record.field CA NMS`


Currently, the record does not try to ensure that `WAIT` and
`OUT` are compatibly configured.  If `WAIT` == "Wait",
but the link looks like


` xxx:record.field PP NMS`


for example, then the record will not wait for completion before executing
its forward link.


---


## 6. Operator Display Parameters


These parameters are used to present meaningful data to the operator.
Some are also meant to represent the status of the record at run-time.
An example of an interactive MEDM display screen that displays the status
of the scalcout record is located here.


The EGU field contains a string of up to 16 characters which is
supplied by the user and which describes the values being operated upon.
The string is retrieved whenever the routine `get_units` is called.
The EGU string is solely for an operator's sake and does not have to be
used.


The HOPR and LOPR fields only refer to the limits of the VAL,
HIHI, HIGH, LOW, and LOLO fields. PREC controls the precision of the VAL
field.


The INAV-INLV and IAAV-ILLV fields indicate the status of the
link to the PVs specified in the INPA-INPL and INAA-INLL fields, respectively.
The fields can have three possible values:


The OUTV field indicates the status of the OUT link. It has the same possible
values as the INAV-INLV fields.


The CLCV and OLCV fields indicate the validity of the expression
in the CALC and OCAL fields, respectively. If the expression is invalid,
the field is set to one.


The DLYA field is set to one during the delay interval specified
in ODLY.


See the EPICS Record Reference Manual, for more on the record name (NAME) and
description (DESC) fields.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| EGU | Engineering Units | STRING [16] | Yes | Null | Yes | Yes | No | No |
| PREC | Display Precision | SHORT | Yes | 0 | Yes | Yes | No | No |
| HOPR | High Operating Range | FLOAT | Yes | 0 | Yes | Yes | No | No |
| LOPR | Low Operating Range | FLOAT | Yes | 0 | Yes | Yes | No | No |
| INAV | Link Status of INPA | Menu | No | 1 | Yes | No | No | No |
| INBV | Link Status of INPB | Menu | No | 1 | Yes | No | No | No |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| INLV | Link Status of INPL | Menu | No | 1 | Yes | No | No | No |
| OUTV | OUT PV Status | Menu | No | 0 | Yes | No | No | No |
| CLCV | CALC Valid | LONG | No | 0 | Yes | Yes | No | No |
| OCLV | OCAL Valid | LONG | No | 0 | Yes | Yes | No | No |
| DLYA | Output Delay Active | USHORT | No | 0 | Yes | No | No | No |
| NAME | Record Name | STRING [29] | Yes | 0 | Yes | No | No | No |
| DESC | Description | STRING [29] | Yes | Null | Yes | Yes | No | No |
| IAAV | Link Status of INAA | Menu | No | 1 | Yes | No | No | No |
| IBBV | Link Status of INBB | Menu | No | 1 | Yes | No | No | No |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| ILLV | Link Status of INLL | Menu | No | 1 | Yes | No | No | No |


---


## 7. Alarm Parameters


The possible alarm conditions for the scalcout record are the SCAN,
READ, Calculation, and limit alarms. The SCAN and READ alarms are called
by the record support routines. The Calculation alarm is called by the
record processing routine when the CALC expression is an invalid one, upon
which an error message is generated.


The following alarm parameters which are configured by the user
define the limit alarms for the VAL field and the severity corresponding
to those conditions.


The HYST field defines an alarm deadband for each limit. See the
EPICS Record Reference Manual for a complete explanation of alarms
and these fields.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HIHI | Hihi Alarm Limit | FLOAT | Yes | 0 | Yes | Yes | No | Yes |
| HIGH | High Alarm Limit | FLOAT | Yes | 0 | Yes | Yes | No | Yes |
| LOW | Low Alarm Limit | FLOAT | Yes | 0 | Yes | Yes | No | Yes |
| LOLO | Lolo Alarm Limit | FLOAT | Yes | 0 | Yes | Yes | No | Yes |
| HHSV | Severity for a Hihi Alarm | Menu | Yes | 0 | Yes | Yes | No | Yes |
| HSV | Severity for a High Alarm | Menu | Yes | 0 | Yes | Yes | No | Yes |
| LSV | Severity for a Low Alarm | Menu | Yes | 0 | Yes | Yes | No | Yes |
| LLSV | Severity for a Lolo Alarm | Menu | Yes | 0 | Yes | Yes | No | Yes |
| HYST | Alarm Deadband | DOUBLE | Yes | 0 | Yes | Yes | No | No |


---


## 8. Monitor Parameters


These parameters are used to determine when to send monitors for the
value fields. The monitors are sent when the value field exceeds the last
monitored field by the appropriate deadband, the ADEL for archiver monitors
and the MDEL field for all other types of monitors. If these fields have
a value of zero, every time the value changes, monitors are triggered;
if they have a value of -1, every time the record is scanned, monitors
are triggered.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ADEL | Archive Deadband | DOUBLE | Yes | 0 | Yes | Yes | No | No |
| MDEL | Monitor, i.e. value change, Deadband | DOUBLE | Yes | 0 | Yes | Yes | No | No |


---


## 9. Run-time Parameters


These fields are not configurable using a configuration tool and none
are modifiable at run-time. They are used to process the record.


The LALM field is used to implement the hysteresis factor for
the alarm limits.


The LA-LL fields are used to decide when to trigger monitors for
the corresponding fields. For instance, if LA does not equal the value
for A, monitors for A are triggered. The MLST and MLST fields are used
in the same manner for the VAL field.


| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| LALM | Last Alarmed Value | DOUBLE | No | 0 | Yes | No | No | No |
| ALST | Archive Last Value | DOUBLE | No | 0 | Yes | No | No | No |
| MLST | Monitor Last Value | DOUBLE | No | 0 | Yes | No | No | No |
| LA | Previous Input Value for A | DOUBLE | No | 0 | Yes | No | No | No |
| LB | Previous Input Value for B | DOUBLE | No | 0 | Yes | No | No | No |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| LL | Previous Input Value for A | DOUBLE | No | 0 | Yes | No | No | No |
| LAA | Previous Input Value for AA | STRING[40] | No | 0 | Yes | No | No | No |
| LBB | Previous Input Value for BB | STRING[40] | No | 0 | Yes | No | No | No |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |
| LLL | Previous Input Value for LL | STRING[40] | No | 0 | Yes | No | No | No |


---


## 10. Record Support Routines


#### init_record

For each constant input link, the corresponding value field is initialized
with the constant value if the input link is CONSTANT or a channel access
link is created if the input link is PV_LINK.


A routine postfix is called to convert the infix expression in
CALC and OCAL to reverse polish notation. The result is stored in RPCL
and ORPC, respectively.


#### process

See section 11.


#### special

This is called if CALC or OCAL is changed. special calls sCalcPostfix.


#### get_value

Fills in the values of struct valueDes so that they refer to VAL.


#### get_units

Retrieves EGU.


#### get_precision

Retrieves PREC.


#### get_graphic_double

Sets the upper display and lower display limits for a field. If the field
is VAL, HIHI, HIGH, LOW, or LOLO, the limits are set to HOPR and LOPR,
else if the field has upper and lower limits defined they will be used,
else the upper and lower maximum values for the field type will be used.


#### get_control_double

Sets the upper control and the lower control limits for a field. If the
field is VAL, HIHI, HIGH, LOW, or LOLO, the limits are set to HOPR and
LOPR, else if the field has upper and lower limits defined they will be
used, else the upper and lower maximum values for the field type will be
used.


#### get_alarm_double

Sets the following values:

- upper_alarm_limit = HIHI
- upper_warning_limit = HIGH
- lower_warning_limit = LOW
- lower_alarm_limit = LOLO


---


## 11. Record Processing


### 11.1. process()

The `process()` routine implements the following algorithm:

1. Fetch all arguments.

2. Call routine sCalcPerform(), which calculates VAL from the postfix version
   of the expression given in CALC. If sCalcPerform() returns success, UDF
   is set to FALSE.

3. Check alarms. This routine checks to see if the new VAL causes the alarm
   status and severity to change. If so, NSEV, NSTA and LALM are set. It also
   honors the alarm hysteresis factor (HYST). Thus the value must change by
   at least HYST before the alarm status and severity changes.

4. Determine if the Output Execution Option (OOPT) is met. If it is met,
   either execute the output link (and output event) immediately (if ODLY
   = 0), or schedule a callback to do so after the specified interval. See the
   explanation for the `execOutput()` routine below.

5. Check to see if monitors should be invoked.

   - Alarm monitors are invoked if the alarm status or severity has changed.
   - Archive and value change monitors are invoked if ADEL and MDEL conditions
     are met.
   - Monitors for A-L and AA-LL are checked whenever other monitors are invoked.
   - NSEV and NSTA are reset to 0.

6. If no output delay was specified, scan forward link if necessary, set
   PACT FALSE, and return.


### 11.2. execOutput()

1. If DOPT field specifies the use of OCAL, call the routine sCalcPerform
   for the postfix version of the expression in OCAL. Otherwise, use VAL.

2. If the Alarm Severity is INVALID, follow the option as designated by
   the field IVOA.

3. If the Alarm Severity is not INVALID or IVOA specifies "Continue Normally",
   call device support to write the value of OVAL to device or PV specified by
   the OUT link, and post the event in OEVT (if non-zero).

4. If an output delay was implemented, process the forward link.


### 12. Sample Database


Here's a sample database fragment that illustrates how to include a constant
literal string within a calc expression.  At run time, you could enter the
expression below either as


```
AA+printf(' %.3f',A)
```


or as


```
AA+printf(" %.3f",A)
```


But in a database, you don't have that freedom, because the entire expression
is the value of an EPICS field, which must be enclosed in double quotes:


> ```
> record(scalcout, "$(P)SRS810:$(N):SetPhas") {
>     field(DESC, "Create Phase Set Str")
>     field(CALC, "AA+printf(' %.3f',A)")
>     field(OUT, "$(P)SRS810:$(N):WritePhas.AOUT  PP MS")
>     field(PREC, "2")
>     field(AA, "PHAS")
> }
> ```


---

Tim Mooney
