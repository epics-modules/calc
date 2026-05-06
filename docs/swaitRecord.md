# The SWAIT record

Ned D. Arnold, Tim Mooney
Advanced Photon Source
Argonne National Laboratory

## Contents

1. [Introduction](#1-introduction)
2. [Scan Parameters](#2-scan-parameters)
3. [Read Parameters](#3-read-parameters)
4. [Expression-related Parameters](#4-expression-related-parameters)
   - [4.1. Operands](#41-operands)
   - [4.2. Algebraic Operators](#42-algebraic-operators)
   - [4.3. Trigonometric Operators](#43-trigonometric-operators)
   - [4.4. Relational Operators](#44-relational-operators)
   - [4.5. Logical Operators](#45-logical-operators)
   - [4.6. Bitwise Operators](#46-bitwise-operators)
   - [4.7. Parentheses and Comma](#47-parentheses-and-comma)
   - [4.8. Conditional Expression](#48-conditional-expression)
   - [4.9. Examples](#49-examples)
5. [Desired Output Parameters](#5-desired-output-parameters)
6. [Write Parameters](#6-write-parameters)
7. [Operator Display Parameters](#7-operator-display-parameters)
8. [Alarm Parameters](#8-alarm-parameters)
9. [Monitor Parameters](#9-monitor-parameters)
10. [Run-time Parameters](#10-run-time-parameters)

---

## 1. Introduction

This chapter describes the capabilities and use of the SWAIT record, a variant of the WAIT record, written by Ned Arnold, which was modified to use recDynLinkPutCallback so that it actually does wait for the operation its output link triggers to finish before executing its forward link.

The SWAIT record is nearly obsolete, because most of its once-unique capabilities are now included in the calcout record. The only thing the calcout record does not (yet?) do is to wait for completion of the processing started by its output link before executing its forward link.

The SWAIT record, is derived from the Calc record with the following additional features:

- reassignable PV links
- an output link
- a desired output link
- an output event number to post
- and several options as to when it will execute the output link and event posting.

The SWAIT record also has the capability to process as a result of an input changing (via CA monitors), and it can be used to do *conditional* processing within the database. Its fields fall into the following categories:

- scan parameters
- read parameters
- expression-related parameters
- desired output parameters
- write parameters
- operator parameters
- monitor parameters
- run-time and simulation mode parameters

---

## 2. Scan Parameters

The SWAIT record has the standard fields for specifying under what circumstances the record will be processed. In addition to the standard scan mechanisms available to all records (periodic, passive, event, etc.), the SWAIT record can be specified to process as a result of one of its input values changing (using Channel Access monitors). The scan mechanism choice `I/O Intr` enables this feature for the record instance as a whole, and the fields INAP - INLP control the behavior of each input link INPA - INPL, respectively. Thus, if .SCAN == "I/O Intr" and .INAP == "Yes", then the record will process every time a new value is posted for the field specified by the input INPA.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor |
|-------|---------|------|-----|---------|--------|--------|------------------|
| INAP | Input A ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INBP | Input B ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INCP | Input C ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INDP | Input D ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INEP | Input E ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INFP | Input F ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INGP | Input G ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INHP | Input H ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INIP | Input I ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INJP | Input J ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INKP | Input K ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |
| INLP | Input L ProcessOnChange | Menu | Yes | No | Yes | Yes | Yes |

> **Note: Because of the event-driven nature of this feature, it is quite easy to configure a database that results in an infinite loop which uses all available CPU time. If the SWAIT record is set to process as a result of a channel changing and the processing of the SWAIT record causes the channel to change again, an infinite loop will result. This is not by itself a bad thing, but if there is no throttle on the rate at which the SWAIT record causes itself to process, its activity will use all of the available CPU time. The symptom will be an apparent loss of all channel access connections (lower priority tasks) even though the shell responds normally. Using the `spy` vxWorks utility will confirm the predicament by showing 0% free CPU time. If the condition persists for more than a few tens of seconds, some system queue is likely to overflow, and a crate reboot may be required to recover.**
>
> **You can safely use the SWAIT record in such a self-sustaining loop by setting the record's output-delay (.ODLY) field to, say, .02 (seconds). The delay value must be larger than a single system-clock tick (typically, .01667) or it will have no effect.**

The Application Developer's Guide explains how the rest of these fields are used.

---

## 3. Read Parameters

The read parameters for the SWAIT record consist of 12 string fields--INAN, INBN, . . . INLN. The fields can contain PV names or constants. If they are constants, the corresponding value field for the link (A-L) will be initialized with the constant's value and can be changed via dbPuts. The SWAIT record does not support direct links to hardware.

These input "links" can be modified at run time. These are ASCII fields which have a special processing routine attached to them. When changed, the routine is called. The record will use the new link the next time the record is processed.

A consequence of reassignable links is that one cannot force the processing of any specified records prior to retrieving the data from them (i.e. there is no .PP flag). This should be considered when designing a database using the SWAIT record.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor |
|-------|---------|------|-----|---------|--------|--------|------------------|
| INAN | Input Link A Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INBN | Input Link B Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INCN | Input Link C Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INDN | Input Link D Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INEN | Input Link E Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INFN | Input Link F Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INGN | Input Link G Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INHN | Input Link H Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| ININ | Input Link I Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INJN | Input Link J Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INKN | Input Link K Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |
| INLN | Input Link L Name | STRING[40] | Yes | 0 | Yes | Yes | Yes |

---

## 4. Expression-related Parameters

The expression related fields of the SWAIT record are identical to those of the CALCOUT record. The two main fields are the CALC and RPCL fields. The CALC field contains the infix expression which the record routine will use when it processes the record. The resulting value of the expression is placed in the VAL field. The write parameters of the record can write this value if configured to.

The CALC field is actually converted to opcode and stored as a postfix expression, in the RPCL field. It is this expression which is actually used to calculate VAL. CALC can be changed at run-time, and the special record routine calls a function to convert it to the postfix expression. The flag field, CLCV, indicates whether or not the expression is a valid one. It will be true (non-zero) if the expression is invalid, in which case an error message is generated.

The postfix expression is evaluated most efficiently during run-time. The range of expressions supported by the calculation record are separated into operands, algebraic operations, trigonometric operations, relational operations, logical operations, parentheses and commas, and the question mark operator.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| CALC | Calculation | DBF_STRING | Yes | 0 | Yes | Yes | Yes | Yes |
| RPCL | Reverse Polish | DBF_NOACCESS | No | 0 | No | No | N/A | No |

### 4.1. Operands

The expression can use the values retrieved from the INP*x* links as operands, these values being stored in the A-L fields. The values to be used in the expression are simply referenced by the field letter. For instance, the value obtained from the INPA link is stored in field A and the value obtained from INPB is stored in field B. The field names can be included in the expression which will operate on their respective values, as in A+B. In addition, the RNDM unary function can be included as an operand to generate a random number between 0 and 1.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor |
|-------|---------|------|-----|---------|--------|--------|------------------|
| A | Input Values A | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| B | Input Values B | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| C | Input Values C | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| D | Input Values D | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| E | Input Values E | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| F | Input Values E | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| G | Input Values G | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| H | Input Values H | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| I | Input Values I | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| J | Input Values J | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| K | Input Values K | DOUBLE | No | 0 | Yes | Yes/No | Yes |
| L | Input Values L | DOUBLE | No | 0 | Yes | Yes/No | Yes |

### 4.2. Algebraic Operators

- ABS: Absolute value (unary)
- SQR: Square root (unary)
- MIN: Minimum (binary function)
- MAX: Maximum (binary function)
- CEIL: Ceiling (unary)
- FLOOR: Floor (unary)
- LOG: Log base 10 (unary)
- LOGE: Natural log (unary)
- EXP: Exponential function (unary)
- ^ : Exponential (binary)
- \*\* : Exponential (binary)
- \+ : Addition (binary)
- \- : Subtraction (binary)
- \* : Multiplication (binary)
- / : Division (binary)
- % : Modulo (binary)
- NOT: Negate (unary)

### 4.3. Trigonometric Operators

- SIN: Sine
- SINH: Hyperbolic sine
- ASIN: Arc sine
- COS: Cosine
- COSH: Hyperbolic cosine
- ACOS: Arc cosine
- TAN: Tangent
- TANH: Hyperbolic tangent
- ATAN: Arc tangent

### 4.4. Relational Operators

- \>= : Greater than or equal to
- \> : Greater than
- <= : Less than or equal to
- < : Less than
- \# : Not equal to
- = : Equal to

### 4.5. Logical Operators

- && : And
- || : Or
- ! : Not

### 4.6. Bitwise Operators

- | : Bitwise Or
- & : Bitwise And
- OR : Bitwise Or
- AND: Bitwise And
- XOR: Bitwise Exclusive Or
- ~ : One's Complement
- << : Left shift
- \>\> : Right shift

### 4.7. Parentheses and Comma

The open and close parentheses are supported. Nested parentheses are supported.

The comma is supported when used to separate the arguments of a binary function.

### 4.8. Conditional Expression

The "C" question mark operator is supported. The format is:

```
(condition)? True result : False result
```

### 4.9. Examples

#### Algebraic

```
A + B
```

- Result is A + B

#### Relational

```
(A + B) < (C + D)
```

- Result is 1 if (A+B) < (C+D)
- Result is 0 if (A+B) >= (C+D)

#### Question Mark

```
(A+B)<(C+D)?E:F
```

- Result is E if (A+B) < (C+D)
- Result is F if (A+B) >= (C+D)

```
(A+B)<(C+D)?E
```

- Result is E if (A+B) < (C+D)
- Result is unchanged if (A+B) >= (C+D)

#### Logical

```
A&B
```

- Causes the following to occur:
  - Convert A to integer
  - Convert B to integer
  - Bit-wise And A and B
  - Convert result to floating point

---

## 5. Desired Output Parameters

Using the choices in the Data Output field (DOPT) explained in the next section, the SWAIT record can be configured to write as its output the result of its calculation (the value in VAL) or it can be configured to write a desired output value, fetched from an input link. The desired output fields for the SWAIT record are similar to the desired output parameters of other output records.

In the desired output location (DOLN) field, the user specifies a link from which the record will retrieve the value that it writes (if the DOPT field specifies DOLD). The value is retrieved and placed in the desired output location data field (DOLD). If DOLN specifies no link, a flag field indicates that the DOLN field is invalid. When the DOLN field is invalid, the value of DOLD can be set via dbPuts at run-time.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| DOLN | Desired Output Location | STRING [40] | Yes | Null | Yes | Yes | No | No |
| DOLD | Desired Output Location Data | DOUBLE | Yes | 0 | Yes | Yes | Yes | No |

---

## 6. Write Parameters

The SWAIT record can write either the result of its calculation (VAL) or the value retrieved by the DOLN field. If the user specifies `Use VAL` in the Data Option (DOPT) field, then the value in VAL will be written. If the user specifies `Use DOL`, the value retrieved from the DOLN link and contained in the DOLD field will be written. The DOPT field can be changed during run-time.

The Output Link Name (OUTN) specifies where the record is to write the output value.

The SWAIT record does not have to write its output every time the record processes. This allows "downstream" processing of records to be done conditionally. The record has *output execution* options which can be specified in the OOPT field. These options are as follows:

**`Every Time`**
: Output link is written to every time the record processes.

**`On Change`**
: Output link is written to if the result of the calculation (VAL) is different than the previous time the record was processed.

**`When Zero`**
: Output link is written to if the result of the calculation is zero.

**`When Non-zero`**
: Output link is written to if the result of the calculation is not zero.

**`Transition To Zero`**
: Output link is written to if the result of the calculation is zero and the previous value was not zero.

**`Transition To Non-zero`**
: Output link is written to if the result of the calculation is not zero and the previous value was zero.

**`Never`**
: Output link is never written to.

The SWAIT record can also post an event. If a non-zero value is entered into the OEVT (Output Event) field, the record will "post an event" (using the entered number as the event number) whenever the output link is executed. This is a way of initiating several other records to process as a result of a calculation.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| OOPT | Output Option | MENU | Yes | 0 | Yes | Yes | No | No |
| OUTN | Output Link Name | STRING [40] | Yes | Null | Yes | Yes | No | No |
| DOPT | Data Option | MENU | Yes | 0 | Yes | Yes | No | No |
| OEVT | Output Event | USHORT | Yes | 0 | Yes | Yes | No | No |

---

## 7. Operator Display Parameters

These parameters are used to present meaningful data to the operator. They display the value and other parameters of the SWAIT record either textually or graphically.

The HOPR and LOPR fields set the upper and lower display limits for the VAL field. The `get_graphic_double` retrieves these fields.

The PREC field determines the floating point precision with which to display VAL. It is used whenever the `get_precision` record support routine is called.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| HOPR | High Operating Range | FLOAT | Yes | 0 | Yes | Yes | No | No |
| LOPR | Low Operating Range | FLOAT | Yes | 0 | Yes | Yes | No | No |
| PREC | Display Precision | SHORT | Yes | 0 | Yes | Yes | No | No |
| NAME | Record Name | STRING [29] | Yes | 0 | Yes | No | No | No |
| DESC | Description | STRING [29] | Yes | Null | Yes | Yes | No | No |

---

## 8. Alarm Parameters

The wait record has the alarm parameters common to all record types. The Application Developer's Guide lists other fields related to alarms that are common to all record types.

---

## 9. Monitor Parameters

These parameters are delta values that implement a deadband on the value-change monitors for the VAL field. Monitors are sent when the value field exceeds the last monitored field (MLST, for instance) by the appropriate delta. If these fields have a value of zero, everytime the value changes, a monitor will be triggered; if they have a value of -1, everytime the record is processed, monitors are triggered. The ADEL field is used for archive monitors and the MDEL field for all other types of monitors. See the Application Developer's Guide for a complete explanation of monitors.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| ADEL | Archive Deadband | DOUBLE | Yes | 0 | Yes | Yes | No | No |
| MDEL | Monitor, i.e. value change, Deadband | DOUBLE | Yes | 0 | Yes | Yes | No | No |

---

## 10. Run-time Parameters

These parameters are used by the run-time code for processing the SWAIT record. They are not configurable. They represent the current state of the record. The record support routines use some of them to process the record or to implement monitors.

The OVAL field is used to implement monitors on the VAL field. If VAL differs from OVAL (taking into account the deadband) then monitors for VAL are triggered.

The LA-LL fields are used to implement monitors for the A-L field, the fields which hold the values retrieved from the input links. They hold the values retrieved from INPA-INPL the last time the record was processed. For instance, when A, the current value retrieved from INPA, does not equal LA, the last value retrieved, then monitors for A are triggered.

The DOLV and CLCV are flag fields. The DOLV flag is used to indicate if the process variable specified in the Desired Output Location (DOLN) field is a valid name. If it is not, the desired output value is not retrieved from DOLN. The CLCV flag is used to indicate if the SWAIT record's CALC expression is a mathematically valid one.

The CBST field contains a pointer to a record private structure. It is not of concern to the user.

| Field | Summary | Type | DCT | Initial | Access | Modify | Rec Proc Monitor | PP |
|-------|---------|------|-----|---------|--------|--------|------------------|----|
| CBST | Callback Structure | NOACCESS | No | Null | No | No | No | No |
| OVAL | Old Value | DOUBLE | No | 0 | Yes | Yes | No | No |
| DOLA | Desired Output Location Address | NOACCESS | No | Null | No | No | No | No |
| DOLV | Desired Output Location Valid | LONG | No | 0 | Yes | Yes | Yes | No |
| ALST | Archive Last Value | DOUBLE | No | 0 | Yes | No | No | No |
| MLST | Monitor Last Value | DOUBLE | No | 0 | Yes | No | No | No |
| CLCV | Calculation String Valid | LONG | No | 0 | Yes | Yes | Yes | No |
| LA | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LB | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LC | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LD | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LE | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LF | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LG | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LH | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LI | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LJ | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LK | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
| LL | Previous Input | DOUBLE | No | 0 | Yes | Yes | No | No |
