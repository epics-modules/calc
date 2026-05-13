---
layout: default
title: Home
nav_order: 1
---

# calc module Documentation

## Records

**aCalcoutRecord** - [aCalcoutRecord.md](aCalcoutRecord.md) -- The **aCalcout** record is much like the **calcout** record in EPICS base, but it supports array fields and calculations, and its output link can wait for completion.

**sCalcoutRecord** - [sCalcoutRecord.md](sCalcoutRecord.md) -- The **sCalcout** record is much like the **calcout** record in EPICS base, but it supports string fields and calculations, and its output link can wait for completion.

**transformRecord** - [transformRecord.md](transformRecord.md) -- The **transform** record is sort of like 16 **calcout** records collected together into a single record. The 16 calc expressions use a common set of 16 value fields, but each calc expression writes to its own value field.

**sseqRecord** - [sseqRecord.md](sseqRecord.md) -- The **sseq** record is similar to the seq record in EPICS base, but it can write either to string or numeric fields, it can wait for completion after executing each output link, and the sequence can be aborted.

**swaitRecord** - [swaitRecord.md](swaitRecord.md) -- The **swait** record is much like the **calcout** record in EPICS base, but it uses an older form of retargetable links, and its output link waits for completion. The **swait** record is not recommended for new development; use a **calcout** record instead.

## Databases, Displays, and Configuration Files

Each record type in the calc module comes with a set of "user" databases
(pre-configured record instances for run-time programming), MEDM display
files, and autosave request files. The databases follow a naming convention:
`user<Type>s10.db` provides 10 instances with enable logic,
`user<Type>s10more.db` provides 10 additional instances, and
`user<Type>.db` provides a single parameterized instance.
All user databases share a three-level enable architecture
(global, per-type, and per-record).

### User Calcs (swait)

A *userCalc* is a **swait** record configured for run-time programming,
with enable switches, autosave support, and comprehensive displays.

> Although the **swait** record is not recommended for new development,
> it is retained because many users are accustomed to its link programming
> model (PV names only, no link attributes like NPP/NMS).

| File | Type | Description |
|------|------|-------------|
| userCalcs10.db | Database | 10 user calc records with enable logic |
| userCalcs10more.db | Database | 10 additional user calcs |
| userCalcN.db | Database | Single parameterized user calc |
| userCalcN_noDisable.db | Database | Single user calc without disable mechanism |
| waitRecN.db | Database | Single plain swait record |
| userCalc.adl | Display | Compact user calc display |
| userCalc_full.adl | Display | Detailed user calc display |
| userCalcMeter.adl | Display | User calc with meter widget |
| userCalc_help.adl | Display | User calc help display |
| userCalcs10.adl | Display | Collection of 10 user calcs |
| userCalcs20.adl | Display | Collection of 20 user calcs |
| userCalcs40.adl | Display | Collection of 40 user calcs |
| yyWaitRecord.adl | Display | Plain swait record display |
| yyWaitRecord_full.adl | Display | Detailed plain swait display |
| swait_settings.req | Autosave | Settings for a single swait record |
| userCalcN_settings.req | Autosave | Settings for a single user calc |

### User CalcOuts (calcout)

Like a *userCalc*, but based on the **calcout** record from EPICS base.

| File | Type | Description |
|------|------|-------------|
| userCalcOuts10.db | Database | 10 user calcout records with enable logic |
| userCalcOuts10more.db | Database | 10 additional user calcouts |
| userCalcOut.adl | Display | Compact user calcout display |
| userCalcOut_full.adl | Display | Detailed user calcout display |
| userCalcOuts10.adl | Display | Collection of 10 user calcouts |
| yyCalcoutRecord.adl | Display | Plain calcout record display |
| yyCalcoutRecord_full.adl | Display | Detailed plain calcout display |
| calcout_settings.req | Autosave | Settings for a single calcout record |

### User Array Calcs (acalcout)

Like a *userCalc*, but based on the **aCalcout** (array calcout) record.

| File | Type | Description |
|------|------|-------------|
| userArrayCalcs10.db | Database | 10 user array calc records with enable logic |
| userArrayCalcs10more.db | Database | 10 additional user array calcs |
| userArrayCalc.adl | Display | Compact user array calc display |
| userArrayCalc_full.adl | Display | Detailed user array calc display |
| userArrayCalcPlot.adl | Display | Single array calc with plot |
| userArrayCalcPlots10.adl | Display | 10 array calc plots |
| userArrayCalc_plot.adl | Display | Array calc plot display |
| userArrayCalcs10.adl | Display | Collection of 10 user array calcs |
| arrayPlot8.adl | Display | 8-channel array plot |
| acalcout_settings.req | Autosave | Settings for a single acalcout record |

### User String Calcs (scalcout)

Like a *userCalc*, but based on the **sCalcout** (string calcout) record.

| File | Type | Description |
|------|------|-------------|
| userStringCalcs10.db | Database | 10 user string calc records with enable logic |
| userStringCalcs10more.db | Database | 10 additional user string calcs |
| userStringCalc.adl | Display | Compact user string calc display |
| userStringCalc_full.adl | Display | Detailed user string calc display |
| userStringCalc_demo.adl | Display | String calc demo display |
| userStringCalcs10.adl | Display | Collection of 10 user string calcs |
| yysCalcoutRecord.adl | Display | Plain sCalcout record display |
| yysCalcoutRecord_demo.adl | Display | Plain sCalcout demo display |
| yysCalcoutRecord_full.adl | Display | Detailed plain sCalcout display |
| scalcout_settings.req | Autosave | Settings for a single scalcout record |

### User Transforms (transform)

Like a *userCalc*, but based on the **transform** record.

| File | Type | Description |
|------|------|-------------|
| userTransform.db | Database | Single parameterized user transform |
| userTransforms10.db | Database | 10 user transforms with enable logic |
| userTransforms10more.db | Database | 10 additional user transforms |
| userTransforms20.db | Database | 20 user transforms (deprecated) |
| transforms10.db | Database | 10 individually-enabled transforms |
| userTransform.adl | Display | Compact user transform display |
| userTransform_full.adl | Display | Detailed user transform display |
| userTransforms10.adl | Display | Collection of 10 user transforms |
| userTransforms20.adl | Display | Collection of 20 user transforms |
| Transform.adl | Display | Compact individually-enabled transform |
| Transform_full.adl | Display | Detailed individually-enabled transform |
| Transforms10.adl | Display | Collection of 10 individually-enabled transforms |
| yyTransform.adl | Display | Plain transform record display |
| yyTransform_full.adl | Display | Detailed plain transform display |
| transform_settings.req | Autosave | Settings for a single transform record |

### User String Sequences (sseq)

| File | Type | Description |
|------|------|-------------|
| userStringSeqs10.db | Database | 10 user string sequence records |
| userStringSeqs10more.db | Database | 10 additional user string sequences |
| yySseq.db | Database | Single string sequence record |
| editSseq.db | Database | Support records for the editSseq SNL program |
| userStringSeq\*.adl | Display | User string sequence displays |
| yySseq\*.adl | Display | Plain sseq record displays |
| sseqRecord_settings.req | Autosave | Settings for a single sseq record |

### User Averages (sub)

These databases provide sets of **sub** records programmed to calculate
the average of M values read via an input link. The records can
alternatively fit values (as a function of time) to a line, returning
the line evaluated at the time of the most recent reading -- useful in
PID loops where the true signal varies with time.

The records operate in two modes:

- **CONTINUOUS**: Running average/fit -- after M values, output the
  result of the most recent M values whenever a new value arrives.
- **ONE-SHOT**: Acquire M values, output the result, and stop until
  restarted.

The record is in MAJOR alarm until M readings have been accumulated.

| File | Type | Description |
|------|------|-------------|
| userAve10.db | Database | 10 user averaging records |
| userAve10more.db | Database | 10 additional user averaging records |
| userAve.adl | Display | Compact user average display |
| userAve10.adl | Display | Collection of 10 user averages |
| userAve_settings.req | Autosave | Settings for a single user average |

### Interpolation

Array interpolation using the **aSub** record, supporting linear and
polynomial (Lagrange) interpolation of one or two dependent variables
against an independent variable. Tables can be loaded via autosave,
written by CA clients, or built up point-by-point at run time.
See [Array Interpolation](interpNew.md) for details.

| File | Type | Description |
|------|------|-------------|
| interpNew.db | Database | Interpolation database (preferred) |
| interp.db | Database | Legacy interpolation database |
| interpNew.adl | Display | Interpolation display |
| interp.adl | Display | Legacy interpolation display |
| interpNew_settings.req | Autosave | Interpolation table save/restore |
| interp_settings.req | Autosave | Legacy interpolation save/restore |

### Calc and Calcout Record Support

Display files and autosave support for the **calc** and **calcout**
records from EPICS base, which do not ship with these files.

| File | Type | Description |
|------|------|-------------|
| CalcRecord.adl | Display | Compact calc record display |
| CalcRecord_full.adl | Display | Detailed calc record display |
| calcout_settings.req | Autosave | Settings for a calcout record |

### Expression Help Displays

MEDM displays documenting the expressions supported by **calc**,
**calcout**, **sCalcout**, and **aCalcout** records, with examples.

| File | Description |
|------|-------------|
| calcExamples.adl | Top-level expression examples |
| calcAlgebraicExamples.adl | Algebraic operator examples |
| calcArrayExamples.adl | Array operator examples |
| calcBitwiseExamples.adl | Bitwise operator examples |
| calcMiscExamples.adl | Miscellaneous operator examples |
| calcRelationalExamples.adl | Relational operator examples |
| calcStringExamples.adl | String operator examples |
| calcTrigExamples.adl | Trigonometric function examples |

### Other Databases

| File | Type | Description |
|------|------|-------------|
| arrayTest.db | Database | aSub record generating test arrays `VALA` and `VALB` |
| FuncGen.db | Database | Software function generator (sine, square, triangle, ramp) |
| userCalcGlobalEnable.db | Database | Global enable switch for all user calc types |

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
