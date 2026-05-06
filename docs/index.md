# calc module Documentation

[Release Notes](calcReleaseNotes.md)

## Records

**aCalcoutRecord** - [aCalcoutRecord.md](aCalcoutRecord.md)
: The **aCalcout** record is much like the **calcout** record in EPICS base, but it supports array fields and calculations, and its output link can wait for completion.

**sCalcoutRecord** - [sCalcoutRecord.md](sCalcoutRecord.md)
: The **sCalcout** record is much like the **calcout** record in EPICS base, but it supports string fields and calculations, and its output link can wait for completion.

**transformRecord** - [transformRecord.md](transformRecord.md)
: The **transform** record is sort of like 16 **calcout** records collected together into a single record. The 16 calc expressions use a common set of 16 value fields, but each calc expression writes to its own value field.

**sseqRecord** - [sseqRecord.md](sseqRecord.md)
: The **sseq** record is similar to the seq record in EPICS base, but it can write either to string or numeric fields, it can wait for completion after executing each output link, and the sequence can be aborted.

**swaitRecord** - [swaitRecord.md](swaitRecord.md)
: The **swait** record is much like the **calcout** record in EPICS base, but it uses an older form of retargetable links, and its output link waits for completion. The **swait** record is not recommended for new development; use a **calcout** record instead.

## EPICS Databases, MEDM display files, and autosave-request files

**arrayTest.db, arrayTest.adl**
: This database contains an **aSub** record that maintains two array fields: "$(P)arrayTest.VALA", and "$(P)arrayTest.VALB". The "VALB" field contains double values from 0 to $(N)-1, where $(N) is a macro supplied to the `dbLoadRecords()` command that loads the database. The "VALA" field contains double values from M to M+$(N)-1, where M is supplied at run time to the field "$(P)arrayTest_start".

**CalcRecord.adl, CalcRecord_full.adl**
: The **calc** record, in EPICS base, does not come with display files. But, frequently, one would like to be able to display and interact with a **calc** record embedded in a database. The files serve that purpose.

**calcExamples.adl, calcAlgebraicExamples.adl, calcArrayExamples.adl, calcBitwiseExamples.adl, calcMiscExamples.adl, calcRelationalExamples.adl, calcStringExamples.adl, calcTrigExamples.adl**
: These files document the expressions evaluated by **calc**, **calcout**, **sCalcout**, and **aCalcout** records.

**calcout_settings.req**
: The **calcout** record, in EPICS base, does not come with an autosave-request file.

**yyCalcoutRecord.adl, yyCalcoutRecord_full.adl**
: The **calcout** record, in EPICS base, does not come with display files. But, frequently, one would like to be able to display and interact with a **calcout** record embedded in a database. The files serve that purpose.

**interpNew.db, interpNew_settings.req, interpNew.adl**
: This is a new version of interp, that requires the additional macro, Q, and that is easier to use. [Details](interpNew.md)

**interp.db, interp_settings.req, interp.adl**
: This database implements array interpolation, and hosts three arrays: an independent variable array, X, whose PV name is "$(P)interp.VALA"); and two dependent variable arrays, Y1(X) and Y2(X), whose PV names are, respectively, "$(P)interp.VALB", and "$(P)interp.VALC".

  The arrays can be loaded at boot time, using autosave; written to at run time by a CA client; or built up element by element at run time, by setting the MODE switch ("$(P)interp_mode") to "Add Entry".

  To interpolate, one sets the mode switch to "Interpolate", and writes a number to the X-input field, "$(P)interp_x". This causes an underlying **aSub** record to interpolate the Yi arrays to find values that would correspond with that value of X, to display them as "$(P)interp_y1" and "$(P)interp_y2", and, optionally, to write them to the external PVs named in the link fields "$(P)interp_y1.OUT", and "$(P)interp_y2.OUT".

  The interpolation performed is a Lagrange interpolation, where the degree of the polynomial can be specified by writing to "$(P)interp.F". The order must be in the range 1..10. Currently the database doesn't visibly enforce these limits, but silently clips to them if they are violated.

  To add an element to the arrays, one sets the mode switch to "Add Element", writes Yi values to "$(P)interp_y1" and "$(P)interp_y2", and then writes the corresponding X value to "$(P)interp_x". The number of entries, "$(P)interp_n" will then increase by one.

  The arrays can be cleared by setting the mode switch to "Clear All" and writing anything to X.

  For more information, see [interp.README](interp.README).

**userAve10.db, userAve10_settings.req, userAve.adl, userAve10.adl, userAve10more.db, userAve10more_settings.req, userAve_settings.req**
: These files implement, manage, and display sets of 10 **sub** records programmed (thanks to Frank Lenkszus) to calculate the average of M values, where M is given by "$(P)userAve$(N).A", which are read via the input link "$(P)userAve$(N).INPB". Alternatively, the **sub** record can be made to fit recorded values, seen as a function of time, to a line, and to return the line evaluated at the time of the most recently recorded value. This is very similar to an average, if the true value is constant. If the true value varies with time, the line fit produces a better estimate of the signal's true value than an average.

  These records are intended for use in run-time programming. With either algorithm, the records can operate in two ways:

  - **Running average/fit** ("$(P)userAve$(N)_mode" == "CONTINUOUS"): After M values have been recorded, output the average (line-fit result) of the most recent M value whenever a new value is read.

  - **One-shot average/fit** ("$(P)userAve$(N)_mode" == "ONE-SHOT"): Clear the output, read M values, output the average (line-fit result) of those values, and ignore new values until a restart command is received ("$(P)userAve$(N).C is set to 1).

  In either mode, the record uses its alarm field to indicate whether the output value it is displaying ("$(P)userAve$(N).VAL") is valid (i.e., is the result of M readings). Until M readings have been read, the record will be in alarm (STAT=="SOFT", SEVR=="MAJOR"). After M readings have been treated, STAT=="NO_ALARM", SEVR=="NO_ALARM".

**userCalcs10.db, userCalcs10_settings.req, userCalcs10more.db, userCalcs10more_settings.req, swait_settings.req, userCalc.adl, userCalcMeter.adl, userCalc_full.adl, userCalc_help.adl, userCalcs10.adl, userCalcs20.adl, userCalcs40.adl, userCalcN.db, userCalcN_noDisable.db, userCalcN_settings.req**
: A *userCalc* is simply a **swait** record implemented for use in run-time programming. "Implemented" means that a database containing several uncommitted **swait** records, with an overall enable switch, is loaded; corresponding entries are made in autosave files to save and restore selected field values; and reasonably comprehensive MEDM displays are provided so that a user can exercise most of the records' features.

  > Although the **swait** record is not recommended for new development, it is retained in this case because many users have become accustomed to the way it behaves -- in particular, to the way its links are programmed: they take only a PV name, without the link attributes (e.g., NPP, NMS) that standard EPICS links have.

**userCalcOuts10.db, userCalcOuts10_settings.req, userCalcOuts10more.db, userCalcOuts10more_settings.req, userCalcOut.adl, userCalcOut_full.adl, userCalcOuts10.adl**
: Just like a *userCalc*, but based on the **calcout** record, instead of the **swait** record.

**userArrayCalcs10.db, userArrayCalcs10_settings.req, userArrayCalcs10more.db, userArrayCalcs10more_settings.req, acalcout_settings.req, userArrayCalc.adl, userArrayCalcPlot.adl, userArrayCalcPlots10.adl, userArrayCalc_full.adl, userArrayCalc_plot.adl, userArrayCalcs10.adl, arrayPlot8.adl**
: Just like a *userCalc*, but based on the **aCalcout** (array calcout) record, instead of the **swait** record.

**userStringCalcs10.db, userStringCalcs10_settings.req, userStringCalcs10more.db, userStringCalcs10more_settings.req, scalcout_settings.req, userStringCalc.adl, userStringCalc_demo.adl, userStringCalc_full.adl, userStringCalcs10.adl**
: Just like a *userCalc*, but based on the **aCalcout** (array calcout) record, instead of the **swait** record.

**yysCalcoutRecord.adl, yysCalcoutRecord_demo.adl, yysCalcoutRecord_full.adl**
: MEDM displays for a plain **sCalcout** record, i.e., one that is not also a userStringCalc.

**userTransform.db, userTransforms10.db, userTransforms10_settings.req, userTransforms10more.db, userTransforms10more_settings.req, userTransform.adl, userTransform_full.adl, userTransforms10.adl, userTransforms20.adl, userTransforms20.db (deprecated), userTransforms20_settings.req (deprecated)**
: Just like a *userCalc*, but based on the **transform** record, instead of the **swait** record.

**Transform.adl, Transform_full.adl, Transforms10.adl, transforms10.db, transforms10_settings.req, transform_settings.req**
: Just like a userTransform, but individually enabled/disabled.

**yyTransform.adl, yyTransform_full.adl**
: MEDM displays for a plain **transform** record, i.e., one that is not also a userTransform.

**waitRecN.db, yyWaitRecord.adl, yyWaitRecord_full.adl**
: MEDM displays for a plain **swait** record, i.e., one that is not also a userCalc, and a database to load a single **swait** record. These files are retained mostly for backward compatibility.

**userStringSeqs10.db, userStringSeqs10_settings.req, userStringSeqs10more_settings.req, userStringSeqs10more.db, userStringSeq\*.adl**
: Ten string-sequence records.

**yySseq.db, yySseq\*.adl**
: One string-sequence record.

## How to build and use

- Edit `configure/RELEASE` to specify the paths to **EPICS base**, and to the **sscan** and **seq** modules. If you don't have **sscan**, or don't want it to be used by the **calc** module, comment the definition out, and the **calc** module will adjust its build accordingly. Same for **seq**. The only thing **calc** can't build without **sscan** is the swait record, which is retained in **calc** mostly for historical reasons. The only thing **calc** can't build without **sscan** is the editSseq program and database, which support run-time editing of sseq and seq records.

  > If **calc** was built with **sscan** or **seq**, modules that depend on calc should also be built with those modules.

  > The swait record is the original "userCalc" record, and many users at APS have become accustomed to the way it works. The calcout record from EPICS base, with "Async Soft Channel" device support, is a reasonable substitute for the swait record,

- Run Gnu Make to build.

- Note that the **calc** module is not useful on its own, but only produces code that can be used by other modules. (A good example of a module that uses the calc module is synApps' **xxx** module.) An application that uses **calc** must specify the path to the calc module in its configure/RELEASE file; it must add

  ```
  include calc.dbd
  ```

  to its `xxxInclude.dbd` file, and it must link with `libcalc`, by adding a line like

  ```
  xxx_LIBS += calc
  ```

  to xxxApp/src/Makefile.

---

Suggestions and Comments to:
Tim Mooney (mooney@aps.anl.gov)
