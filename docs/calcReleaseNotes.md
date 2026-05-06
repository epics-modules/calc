---
layout: default
title: Release Notes
nav_order: 2
---

# calc Release Notes

## Release 3-7-5

- All autoconverted op files updated

## Release 3-7-4

- Converted adl files to bob and edl
- IOC shell files now located in calcApp/iocsh and are installed to top-level iocsh folder

## Release 3-7-3

- Db files now all get installed to top-level db folder

## Release 3-7-2

- EPICS v7 compatability changes
- Errors in OCAL expression now set OVAL to -1 instead of VAL

## Release 3-7-1

- aCalcout now posts monitors and archive monitors for AVAL and OAV if one or more their elements changes by more than MDEL or ADEL.

## Release 3-7 (12/5/2017)

- seq and sseq record displays now expose the SELL, SELN, and SELM fields, and related .req files autosave those fields.
- Made sCalc and aCalc header files public.
- Modified sseqRecord_settings.req to restore DOn before STRn, because the sseq record modifies both when either is written, and this messes up runtime restore (manual restore or configMenu restore). Test: set STRn to "1!", save and restore. Without the fix, STRn gets restored as "1".
- Modified the scalcout record's treatment of null characters in strings received from DBF_CHAR and DBF_UCHAR PVs. Previously, one or more trailing "\000" fragments were appended (by epicsStrSnPrintEscaped) to the string. These fragments are now stripped.
- Added iocsh directory, and populated with scripts callable from st.cmd
- userArrayCalcs10more_settings.req had incorrect Enable PV
- fix verious .adl files for better translation with adl2ui
- remake all ui files with adl2ui from caqtdm 4.1.3
- userStringCalcs10.db: Update SDIS links to include individual and global enable/disable
- Added acalc and scalc unit tests
- array calcs monitor and display allocated memory
- sCalcoutRecord.c: strcpy -> strNcpy; strcmp -> strncmp; paddr->field_size = STRING_SIZE changed to paddr->field_size = 1 in cvt_dbaddr()
- Changed order of STR* and DO* in sCalcout request file, so manual restore works (string value overrides numeric value).
- Fixes to compile with Visual Studio 2010
- aCalcTest.cpp: Restored powl, atan2l, sqrtl to pow, atan2, sqrt; change first arg to double to eliminate errors on Windows.
- Update CONFIG to use CONFIG_SITE.

## Release 3-6-1

- Fixed aCalcoutRecord.html, sCalcoutRecord.html, and transformRecord.html so that they specify the correct size of calc-expression fields.

## Release 3-6

- Restored yySseq.db, added yySseq_settings.req.

## Release 3-5

- usercalcs now have a global enable, a usercalc-type enable, and individual enables.
- calc.iocsh (in iocBoot/iocCalc) is now the preferred way to load usercalcs.
- transformRecord: any nonwhitespace character is now permitted in a variable name
- sseq_settings.req is deprecated. Use sseqRecord_settings.req instead.

## Release 3-4-2-1

- Not all userCalc displays use $(C); some use $(S). Specify both

## Release 3-4-2

- /userXXX10more_bare.adl specified macro C, but sseq displays need S

## Release 3-4-2

- calc was adding editSeq.dbd to calcSupport.dbd even when SNCSEQ wasn't defined in RELEASE
- sseq record: previously, sseq could segfault when .WAIT=="After\<n\>". Thanks to Matt Pearson for finding and characterizing the problem.

## Release 3-4-1

- scalcout record: conversion from numeric expression result to string now honors the PREC field.
- editSseq: new support for editing sseq and seq records at run time. editSseq allows user to move a sseqRecord *line* up or down, or to swap two lines. (A *line* is the set (DLYn, DOLn, STRn, DOn, LNKn, WAITn).)
- calc now requires the seq module by default. If the seq module is not available, the definition of SNCSEQ must be commented out of configure/RELEASE, and calc will then build everything but editSseq. This version of calc will build with either seq-2-1-18 or seq-2-2-1.
- calc.dbd: new file, which is exactly the same as calcSupport.dbd.

## Release 3-4

- acalcout record: new functions, better treatment of subranges:
  - IXMAX: index of largest array element
  - IXMIN: index of smallest array element
  - IXZ: index of first zero crossing in array
  - IXNZ: index of first nonzero array element
  - FITQ, FITMQ: fit to quadratic and optionally return fit coefficients. (Replacements for deprecated FITPOLY, FITMPOLY.)
  - CAT: concatenate array subranges, or an array subrange and a double.
  - operators `^` and `**` reimplemented to permit noninteger exponents.
  - acalcout expressions now maintain array extents on the stack, so, for example, the expression `avg(aa[1,10])` really is an average of just `aa[1,10]`. Previously, `avg(aa[1,10])` actually performed `sum(aa[1,10])/arraySize`. Now it does `sum(aa[1,10])/10`.
- scalcout and transform records: operators `^` and `**` reimplemented to permit noninteger exponents.
- use Perl's ExtUtils::Command to concatenate .dbd files, instead of local cat.pl. Left old stuff in place, for now, in case someone has a problem with the new method.
- Minor improvements in interp
- When an scalcout fails to fetch a string field, give an informative error message.

## Release 3-3

- The transform record now supports expressions with user-specified variable names. For example, instead of the expression "a-b/2", you could arrange to write "$center+$size/2". Variable names are specified using the CMTx fields. The sizes of calc fields have been increased from 80 to 120 to better accommodate variable names.
- Make it easier to add 10 more usercalcs by writing, e.g., userCalcouts10more.db
- scalcout: fix for enum fields in R2-9 broke the code that checks if links are ok. This is now fixed.

## Release 3-2

- Added CUM function to calculate running sum of array elements. Example: if AA=(1,2,3), CUM(AA) = (1,3,6). This function is useful for integrating multichannel scaler arrays of motor steps into relative motor positions.
- Previously, when fetching array values from input links, the number of elements requested was not initialized before each call to dbGetLink(). If dbGetLink() reported a smaller number of elements, that number was used in subsequent calls to dbGetLink().
- Previously, the arguments of the @ and @@ operators were not thoroughly range checked. Also, a subexpression like "@-a:=2" was executed as "@a:=-2" because the := operator failed to move operators of >= priority out to the postfix buffer.
- Added display files for caQtDM

# calc Release Notes

## Release 3-1

- Previously, only a single acalcout record could evaluate an expression at a time, because aCalcPerform() was not reentrant, because it used a global value stack. Now aCalcPerform allocates a new value stack for each expression evaluation, using EPICS' freeList. aCalcPerform manages an array of EPICS freeLists, selecting an appropriately sized list if possible, making a new one if an unclaimed list entry exists, commandeering the least recently used list with no outstanding allocations, or reporting failure. As a consequence, the iocshell variable `aCalcArraySize` is no longer needed and no longer exists.
- Previously, acalcout expression evaluation always occurred in the record-processing thread, regardless of how long expression evaluation was expected to take. (On solaris, evaluations with million-element arrays can easily take over a second.) Now, acalcout records queue expression evaluation to a separate thread, for array sizes larger than `aCalcAsyncThreshold` (default: 10000).
- Previously, acalcout records reported NUSE as the array size on the initial connection by a link or channel-access client. Now the size can be reported either as `NELM` (the default), or as `NUSE`, depending on the value of the record's `SIZE` field, which can be either "NELM" or "NUSE". "NELM" always works, but is in some cases less efficient than "NUSE". If "NUSE" is selected, the link or channel must be closed and reestablished when the record's `NUSE` field increases.
- arrayCalc plot displays (MEDM files only, for now) now use NUSE to specify the array size. This requires an additional macro (C, the record name) to be supplied when launching those displays.
- All userCalcs of one kind (i.e., userTransforms10.db, userArrayCalcs10.db, etc.) have since EPICS 3.14 been in the same lock set, because they connected via NPP links to a common enable PV. Now, userCalcs are enabled via CA links.
- Previously, the acalcout record calculated OVAL and OAV from CALC, instead of from OCAL.
- In calc R3-0, the array operators `fitmpoly`, `nderiv`, `nsmoo` were broken because they had incorrect stack-effect numbers.
- In calc R3-0, for both acalcout and scalcout records, stores to input-variable fields specified using the "@" and "@@" operators were wrongly coded for some expressions.
- Previously, the expression "a?1;b:=0:0" produced a evaluation-time error if a==0. Actually, it *is* an error to have a store expression not at top level and not within parentheses, but postfix doesn't catch it, and anyway it's pretty easy to do the expected thing in this case. Note that it's not possible to do the expected thing in the case "a?1:0;b:=0", because we can't know whether "(a?1:0);b:=0" or "a?1:(0;b:=0)" was intended.
- Previously, the sseq record used database access to get the properties of a local PV it linked to, even if it was making a CA link to that PV. Also, the sseq record failed to update the status of an input link that had changed from a CA link to a DB link.
- Previously, userCalc collections were enabled by a record whose value was undefined until the enable state was changed. Now the enables are made to be defined in the database file: field(UDF, "0"), field(STAT, "NO_ALARM"), field(SEVR, "NO_ALARM")

## Release 3-0

- In all .adl files displaying calc expressions, the expression widget has been modified to have '$' appended to PV name, and to have the attribute 'format=string'. **The trailing '$' character will confound MEDM versions earlier than 3-1-7, which assume '$' begins a macro substitution.**
- Modified scalcout, transform, swait, and acalcout records to use 80-character expression fields. Modified associated autosave-request files to handle these fields by appending '$' to the field name. **Note that autosave R5-0 or higher is required to save and restore these fields.**
- Modified interp.c and interpNew.\* software to support interpolating an array of values to produce an array of results. The interpolation of scalars and arrays uses the same interpolation tables and polynomial order.
- The sseq record (and related databases, etc) was moved from the std module to the calc module, along with all of its databases, request files, and display files. **Command files that load userStringSeqs10.db must be modified to reflect its new location.**
- The sseq record now notifies the user if a WAITn field is inconsistent with the link attribute specified for the associated LNKn field. (If WAITn, then LNKn must be a CA link.)
- The unary operator "NOT" should always have meant logical NOT, but actually has been implemented as unary negative. Now, "NOT" and "!" both mean logical NOT.
- Rewrote string and array calcs following Andrew Johnson's new model for the calc software in EPICS base. This model does a more robust job of handling functions with a variable number of arguments, and permits expressions to store values to input variables. I departed slightly from the model to permit store operations at other than top level, which allows them to be made conditional. This permitted the addition of the looping function "UNTIL". String and array calcs now export the variables sCalcLoopMax and aCalcLoopMax, respectively, to limit the number of iterations of UNTIL loops.
- **WARNING! NON-BACKWARD-COMPATIBLE CHANGE:** Fixed the array calc {} (subrange in place) operator to treat array indices as documented - exactly as the [] (subrange) operator treats them. Previously, for example, aa{0,n} was not the same as aa[0,n], but instead performed aa{0,n-1}.
- Renamed aCalcoutRecord's NEW field as NEWM, to avoid conflict with c++ reserved word
- Added python program test_aCalc.py to test array calcs. The program uses pyepics to talk to an acalcout record. Modified test_sCalc.py to use pyepics.

## Release 2-9

- Dropped backward compatibility with genSub record
- sCalcout record:
  - now treats arrays of DBF_CHAR, DBF_UCHAR as strings
  - now checks CA string-input link by calling dbGetLink(), to defend against failure resulting from enum strings not having arrived before first call.
  - fixes for 64-bit arch (Thanks to Lewis Muir and Andrew Johnson)
  - Some additions to test_sCalc.py
  - Previously, if the OCAL expression evaluation produced an error, the record set SVAL, instead of OSV, to '\*\*\*ERROR\*\*\*'.
- Added .opi display files for CSS-BOY
- Added some missing logical and bitwise operators to calcExamples.\* displays
- Added BURT array-load example to interp.README
- Added interpNew, a better version of interp.

## Release 2-8

- Modified subAve.c (which supports userAve10.db) to add two algorithms: a line fit, selected by setting the sub record's .F field to 1, and a modified line fit, in which the calculated slope is multiplied by the correlation coefficient (.F ==2). These algorithms fit recorded values, seen as a function of the time they were recorded, to a line, and return the line evaluated at the time the most recent input value was recorded. This is nearly the same as taking the average, if the input signal's true value is constant. But if the input signal varies with time, a line fit can yield a better estimate of the true current value. This is particularly useful when the smoothed value is needed in a PID loop.
- subAve.c had fixed 10,000 element buffer. Now it starts with 100 elements and grows as needed to as many as 100,000 elements.
- userAve10.db, userAve.adl - modifications to add subAve.c's new line-fit algorithm. Modifications to allow clients easily to acquire an averaged value (for example, in a scan) by writing to an "acquire" PV. Clients that use ca_put_callback to start acquisition will receive a callback when the specified number of readings have been averaged.
- Modified sCalc support to add the function nrndm, a pseudorandom number normally (i.e., bell curve) distribed about zero, with a standard deviation of one. This affects sCalcout and transform records.
- Fixed a bug in the sCalcoutRecord (thanks to Lewis Muir for finding and diagnosing the problem). The record was not in all cases checking that an input link had one of the supported types before before using it to fill the associated value field. As a result, a link to a DBF_CHAR or DBF_UCHAR PV would clear the following link's value field.
- Fixes for EPICS 3.14.11, notably, adding errlog.h
- Added FLNK to swait_settings.req (the 'userCalc' record)
- Added a simple soft function generator (sine square triangle), FuncGen\*
- sCalc's max() and min() functions fixed for 64-bit architecture. This fixes sCalcoutRecord, transformRecord, and swaitRecord.

## Release 2-7

- Instead of the genSub record, the calc module now uses the aSub record in EPICS base (3.14.10 and higher). Thanks to Eric Norum for detailing the required changes.
- transform record exports transformRecordDebug, and calcSupport.dbd names it.
- new .adl files yyArrayCalc\*.adl
- CalcRecord\*.adl: deleted link-valid fields, which don't exist for calc record.
- sCalcPerform now returns an error if the sscanf function fails to convert.
- the sCalcout record now sets VAL==-1 and SVAL == "\*\*\*ERROR\*\*\*" if sCalcPerform returns an error.

## Release 2-6-7

- Use new linkHelp displays (from std module) in displays that have links

## Release 2-6-6

- The aCalcout record was not handling alarms correctly, Peter Mueller fixed it.
- calcDocs.html now describes the databases, autosave-request files, and MEDM-display files in the calc module.
- Added a set of user-transform record that are individually enabled/disabled.

## Release 2-6-5

- The sCalcout record now supports the function, "len()", which returns the length of its argument (converted to string, if necessary).
- Made the build sensitive to the modules in configure/RELEASE, so people don't have to get a copy of sscan and gensub just to use, say, the sCalcout record. calcSupport.dbd and libcalc will include whatever calc was able to build, and this is determined by what modules are named in configure/RELEASE.
- For sCalcout and aCalcout records, fixed check of outlink (if link to link field, or if .WAIT, then outlink attribute must be CA).

## Release 2-6-4

- The transform record can now use both the input link and the expression to find a new value for a value field. This is enabled by the COPT (calc option) field. With the default value of COPT, "Conditional", the presence of the input link INPA precludes evaluation of the expression CLCA. This is the way the transform record has always worked in the past. With COPT="Always", all expressions are evaluated, whether or not the corresponding input link exists. (As before, all input links execute before any calculations are performed.)
- Minor changes to some display and documentation files.

## Release 2-6-3

- array calcs reworked to allocate arrays only as needed. aCalcPerform allocates no stack space until it is called. Then, it allocates 20 arrays of length aCalcArraySize (a volatile int, settable from the shell) or the arraySize argument specified by the first caller, whichever is larger. aCalcoutRecord allocates an NELEM-size array for each variable AA-LL, when that variable is first read from or written to.
- swaitRecord's .CALC field defaults to "0" instead of "", because "" will not be legal in EPICS 3.13.9.

## Release 2-6-2

- Added the following functions for the aCalcout record
  - **DERIV** - take derivative (with respect to element number)
  - **FITPOLY** - fit to 2nd order polynomial
  - **FITMPOLY** - masked fit to 2nd order polynomial
  - **NSMOO** - apply smoothing operator a specified number of times
- reworked medm displays that describe calc expressions
- reworked medm displays for aCalcout record
- fixed swaitRecord docs

## Release 2-6-1

- sCalcout and aCalcout records were not calling device support's init_record() routine
- Added FWHM function to array calc support.

## Release 2-6

- Added array calc support. This generalizes EPICS standard calc support in much the same way that the string calc support did, but with arrays instead of strings.
- Added a python program to test string-calc software.
- Interp support uses different genSub-record fields, and has acquired a mode switch and a success indicator. See calc/documentation/interp.README.
- sCalcout record has two new functions:
  - TR_ESC() translates escape sequences into the characters they represent
  - ESC() translates selected unprintable characters into escape sequences.

## Release 2-4

This version is intended to build with EPICS base 3.14.7.

- interp.c, interp.db, interp.adl - no longer supports spline interpolation. Array length no longer limited to 2000 elements. Added range checking. Now the number of elements must be specified at load time, with the macro, N. Polynomial order is now forced to be in [0..10].
- All databases have been converted to vdct.
- Swait-record documentation added
- New string operators '-|' and '|-' (delete first/last occurrence of substring).
- Added arrayTest.c, a gensub-based support for producing unique arrays at runtime, e.g., for verifying that sscanRecord/saveData are storing data exactly as it was acquired. Also added arrayTest.db and arrayTest.adl.
- Complete list of stringCalc functions, operators, and constants, with examples, is available as a set of medm displays, starting with the top-level display "calcExamples.adl". All medm displays that display calc expressions call up these doc displays.
- stringCalc record now uses device support. Supplied soft device support allows the user to specify whether or not to wait for completion of the operation started by the OUT link.
- EPICS base no longer contains the code required to check calc expressions for the sCalcout and transform records at boot time or DCT time. (At run time, the expressions are checked by sCalcPostfix.)
- [cvs log](cvsLog.txt)

## Release 2-3

This is the first release of the synApps calc module. Version numbering for this module begins with 2.3 because this module was split from version 2.2 of the std module, and I wanted to retain the CVS histories of module contents.

This version is intended to build with EPICS base 3.14.5.

Differences from software as previously released in std 2.2:

- interp.c, interp.db, interp.adl - now supports spline and polynomial interpolation.
- subAve.c - registry stuff

---

Suggestions and Comments to:
[Tim Mooney](mailto:mooney@aps.anl.gov) (mooney@aps.anl.gov)
Last modified: December 22, 2004
