## push

**The Petite Un\*x Shell.**

![push-c64](https://user-images.githubusercontent.com/1909551/113136556-ca459300-9223-11eb-81ac-aa7be3cb86ae.png)

### About

**push** is a very small, yet feature rich shell for embedded
systems. It is built on top of a stripped down version of the
[linenoise](https://github.com/antirez/linenoise) line editing
library.

**push** thus inherits *TAB completion*, *history* and *hints*
(suggestions at the right of the prompt as you type) while
providiing rich line editing features like cursor movment, inserting
text at the current cursor position, jumping to the first (HOME) or
last (END) position of the input buffer, deleting the last word and
more.

### Platforms

**push** is intended to be cross platform. Currently it builds on
GNU/Linux (gcc) for easy testing and development, and can be cross
compiled for:

* [C64](https://wikipedia.org/wiki/Commodore_64) with [cc65](https://www.cc65.org)
* [ZX Spectrum](https://wikipedia.org/wiki/ZX_Spectrum) with [z88dk](https://z88dk.org)
* [Atari800](https://wikipedia.org/wiki/Atari_800) with [cc65](https://www.cc65.org)
* [Spectrum Next](https://www.specnext.com/about/) with [z88dk](https://z88dk.org)
* ~[MEGA65](https://mega65.org) with [kickc](https://gitlab.com/camelot/kickc)~

While it builds successfully for all those target platforms, it is only
really working well on Linux and C64 (and most recently on ZX-Spectrum).
The main reason is lacking/broken support for some features from the C
runtime libraries on some platforms, most important CONIO and heap
management. Hopefully over time the toolchains will provide those APIs.

### Disclaimer

Please keep in mind, that **push**, at its current state, is an experiment
(proof-of-concept) and not of much practical use. Nevertheless it is fun to
see the `test` command do its work, so give it a shot.

### Compilation

You need the toolchain you want to build with installed parallel to the
**push** source like this:

```
cc65/  kickc/  push/  z88dk/
```

The Makefile in `./push/src` contains various targets for compiling and
testing:

* **make** (build linux binary)
* **make c64** (build C64 prg)
* **make m65** (build MEGA65 prg)
* **make zx** (build zx spectrum tap)
* **make zx64** (build zx spectrum tap with 64 columns)
* **make zxn** (build spectrum next tap)
* **make c64emu** (start prg in vice)
* **make m65emu** (start prg in xemu)
* **make zxemu** (start tap in zesarux)
* **make zx64emu** (start 64 columns tap in zesarux)

### Screenshots

#### C64

![C64-1](https://user-images.githubusercontent.com/1909551/113136210-5e632a80-9223-11eb-8d3c-99e896990f25.png)
![C64-2](https://user-images.githubusercontent.com/1909551/113136236-6a4eec80-9223-11eb-944a-c02050215299.png)
![C64-4](https://user-images.githubusercontent.com/1909551/113136267-78047200-9223-11eb-8e1f-762ee5b33faa.png)

##### Linux

![push-linux](https://user-images.githubusercontent.com/1909551/113136405-9a968b00-9223-11eb-828b-0295aa947608.png)

### License

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
