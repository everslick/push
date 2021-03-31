## push

### About

**push** is a very small, yet feature rich shell for embedded
systems. It is built on top of a stripped down version of the
line editing library [linenoise](https://github.com/antirez/linenoise).

**push** thus inherits *TAB completion*, *history* and *hints*
(suggestions at the right of the prompt as you type). Additionally
it provides rich line editing feautures like cursor movment, inserting
text on the current cursor position, jumping to the first (HOME) or
last (END) position in the input line, deleting the last word and more.

### Platforms

**push** is intended to be cross platform. Currently it builds on GNU/Linux
(gcc) for easy testing and development, and can be cross compiled for:

* [C64](wikipedia.org/wiki/Commodore_64) with [cc65](cc65.org)
* [MEGA65](mega65.org) with [kickc](gitlab.com/camelot/kickc)
* [ZX Spectrum](wikipedia.org/wiki/ZX_Spectrum) with [z88dk](z88dk.org)
* [Spectrum Next](www.specnext.com/about/) with [z88dk](z88dk.org)

While it builds successfully for all those target platforms, it is only
really working well on linux and C64. The main reason is lacking/broken
support for some features from the C runtime libraries on some platforms,
most important CONIO and heap management. Hopefully over time the toolchains
will provide those APIs.

### Disclaimer

Please keep in mind, that **push** is, at its current state, an experiment
(proof-of-concept) and not of any practical use, e.g. it lacks the ability
to launch other programs, what could be considered a slight hinderence for
widspread adoption.

### Compilation

You need the toolchain you want to build with installed parallel to the
push source like this:

```
cc65/  kickc/  push/  z88dk/
```

The Makefile contains various targets for compiling and testing:

* **make** (build linux binary)
* **make c64** (build C64 prg)
* **make m65** (build MEGA65 prg)
* **make zx** (build zx spectrum tap)
* **make zxn** (build spectrum next tap)
* **make c64emu** (start prg in vice)
* **make m65emu** (start prg in xemu)
* **make zxemu** (start tap in zesarux)
