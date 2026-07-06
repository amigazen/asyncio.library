# asyncio.library

Classic Amiga applications that read or write large files one syscall at a time leave the CPU idle while the file system and device drivers fill buffers over DMA. Every `Read()` waits; every `Write()` waits again. For scanners, archivers, database tools, and anything streaming megabytes from hard disk or CD-ROM, that idle time dominates.

**asyncio.library** — originally by Martin Taillefer, maintained by Magnus Holmgren from 1995 — solves this with **double-buffered asynchronous file I/O**: while your program processes one buffer, the file system prefetches the next via Exec packets. The API mirrors familiar DOS patterns (`Open`, `Read`, `Write`, `Seek`, line-oriented helpers) but returns quickly because work continues in the background.

This **39.3** release from the **amigazen project** is a BSD-licensed refactor of the classic 68k library for **NDK 3.2** and the **ToolKit** standard: canonical **SFD** headers, CLib39x-style startup (`StartUp.c` + `LibInit.c`), modern `__ASM__` / `__REG__` / `__SAVE_DS__` LVO definitions, expanded autodocs with **WARNING** and **BUGS** sections, and the **AsyncTest** harness covering all thirteen public LVOs. The function table, `AsyncFile` layout, and application-facing ABI remain those of Holmgren's **39.x** line — not the separate WarpOS **40.x** or AmigaOS 4 **50.x** forks.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** is using modern software development tools and methods to update and rerelease classic Amiga open source software. Projects include a new AWeb, a new Amiga Python 2, and the ToolKit project — a universal SDK for Amiga development. All *amigazen project* releases are guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

This **39.3** release is still Martin Taillefer's original code, updated over the years by Magnus Holmgren and others. The amigazen project has refactored the build system, SDK headers, startup layout, autodocs, and tests for NDK 3.2 and ToolKit without replacing the core library implementation. It is redistributed under the BSD 2-Clause License (see [LICENSE.md](LICENSE.md)).

The amigazen project philosophy is based on openness:

*Open* to anyone and everyone — *Open* source and free for all — *Open* your mind and create!

PRs for all projects are gratefully received at [GitHub](https://github.com/amigazen/). While the focus now is on classic 68k software, it is intended that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## History

**asyncio.library** began in **1994** when **Martin Taillefer** published double-buffered file I/O for the Amiga — a link library (and later shared library) that kept DMA devices fed while application code worked on already-arrived data. In **August 1995** **Magnus Holmgren** took over maintenance after reporting SeekAsync bugs to Taillefer; Holmgren's releases through **1997** established the stable **37.x → 39.x** API that most 68k programs still target today.

Independent ports followed on other Amiga platforms without sharing version numbers with the 68k line:

| Era | Version | Lineage | Maintainer | Notes |
|-----|---------|---------|------------|-------|
| 1994 | Release 1–2 | 68k original | Martin Taillefer | First public releases; SeekAsync and buffer fixes |
| 1995–96 | 37.1–37.2 | 68k shared library | Magnus Holmgren | Shared `asyncio.library`, `OpenAsyncFromFH`, no-externals build |
| 1997 | 38 | 68k | Magnus Holmgren | SeekAsync EOF fix; `ReadLineAsync` completed |
| 1997 | 39 | 68k | Magnus Holmgren + Michael B. Smith | `FGetsAsync`, `FGetsLenAsync`, `PeekAsync`; `__asiolibversion` |
| 1997 | 39.1–39.2 | 68k | Magnus Holmgren | SeekAsync and line/char I/O bug fixes; past-EOF seek recovery |
| 1997 | 39.2a | 68k archive | Magnus Holmgren | Added missing `AsyncIO.guide` and icons only |
| 2000–01 | 40.0–40.6 | **WarpOS PPC** | Achim Stegemann | Parallel `*PPC` LVOs; 68k V39 API unchanged on 68k side |
| 2003–05 | 50.3 | **AmigaOS 4** | Hyperion / Amiga Inc. | Native OS4 library; new interface numbering |
| 2025 | 39.3 | 68k ToolKit prep | amigazen project | SAS/C + NDK 3.2, SFD, SMakefile, first unit tests |
| 2026 | 39.3 | **amigazen release** | amigazen project | CLib37x startup split, canonical SDK headers, AsyncTest, autodoc warnings |

See [CHANGELOG.md](CHANGELOG.md) for per-release detail.

## The double-buffer pattern

asyncio.library implements **overlapped read-ahead** (and write-behind) at the file-handle level:

| Phase | What happens |
|-------|----------------|
| **Open (read)** | First read packet dispatched to the handler while the app prepares |
| **ReadAsync** | Copies from the filled buffer; sends a packet to refill the other half |
| **SeekAsync** | Repositions within buffered data when possible; otherwise flushes and re-syncs |
| **PeekAsync** | Inspects buffered bytes without advancing the logical read position |
| **Close** | Waits for pending packets; flushes partial write buffers on write handles |

Unlike calling `Read()` in a loop, the file system can DMA into the inactive buffer during your processing window. Buffer size is typically **8192** bytes (two half-buffers, rounded to device block size).

**What asyncio is not:** a general async exec scheduler, a socket library, or a `MODE_READWRITE` handle — each `AsyncFile` is **read-only** or **write/append-only**. See `SDK/Autodocs/asyncio.doc` **WARNING** sections for mode-mismatch behaviour.

## About asyncio.library

`asyncio.library` exposes **thirteen public LVOs** for buffered file I/O:

| Function | Role |
|----------|------|
| `OpenAsync` | Open by path (`MODE_READ`, `MODE_WRITE`, `MODE_APPEND`) |
| `OpenAsyncFromFH` | Wrap an existing DOS `BPTR` |
| `CloseAsync` | Flush, wait, close |
| `SeekAsync` | `MODE_START`, `MODE_CURRENT`, `MODE_END` |
| `ReadAsync` / `WriteAsync` | Block transfer |
| `ReadCharAsync` / `WriteCharAsync` | Single-byte I/O |
| `ReadLineAsync` / `WriteLineAsync` | Line records |
| `FGetsAsync` / `FGetsLenAsync` | fgets-style lines with optional length |
| `PeekAsync` | Non-destructive buffer preview |

### Fork compatibility

| Fork | Version | Relationship to this tree |
|------|---------|----------------------------|
| **68k Holmgren / amigazen** | 39.x | **Target ABI** — this release |
| **WarpOS (`asyncioppc`)** | 40.6 | Same 68k LVOs plus separate `*PPC` entry points; do not mix CPU sides |
| **AmigaOS 4 (`AsyncIO_os4`)** | 50.3 | Different library version and interface model; not binary-compatible |







## Contact

- At GitHub https://github.com/amigazen/asyncio.library/
- On the web at http://www.amigazen.com/ (Amiga browser compatible)
- Or email toolkit@amigazen.com

## Acknowledgements

**asyncio.library** was originally written by **Martin Taillefer** (1994). **Magnus Holmgren** maintained the 68k line from 1995 through **39.2**, with contributions and inspiration from **Olaf Barthel**, line I/O from **Michael B. Smith**, and SeekAsync reports from **David Eaves**. The **WarpOS** port (**40.x**) is by **Achim Stegemann**; the **AmigaOS 4** port (**50.3**) is by **Hyperion Entertainment** / **Amiga Inc.** This **39.3** ToolKit release is by the **amigazen project** (2025–2026). See [LICENSE.md](LICENSE.md) for copyright and redistribution terms.

*Amiga* is a trademark of **Amiga Inc**.
