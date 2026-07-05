# Changelog

All notable changes to **asyncio.library** in the 68k lineage and related forks, through **39.3** (amigazen project ToolKit release).

Format: versions are listed **newest first** within each lineage. Dates are as recorded in `Rev.h`, AmigaGuide history, or fork readmes unless noted approximate.

---

## [39.3] — 2026-07-05 (amigazen ToolKit release)

**Lineage:** 68k · `github/asyncio.library/`  

### Added

- CLib37x-style split: `StartUp.c` (FuncTab, InitTab) + `LibInit.c` (RomTag, version strings)
- `src/asyncio_funcs.h` — LVO prototypes matching shared-library entry points (`AS_LVO` / `AS_REG`)
- `src/compiler.h` — NDK 3.2 `__ASM__`, `__REG__`, `__SAVE_DS__` wrappers
- Canonical `SDK/SFD/asyncio_lib.sfd` with `tools/genheaders.sh` and `tools/sync_include.sh`
- `SDK/Include_h/` as canonical public headers; `include/` mirror for builds
- **AsyncTest** unit harness (`src/unittests/`) — fourteen suites, expect/actual output, full 13-LVO coverage
- Expanded `SDK/Autodocs/asyncio.doc` — `PeekAsync` in TOC, **WARNING** and **BUGS** for developer pitfalls (read/write mode mismatch, SeekAsync return semantics, peek buffer limits)

### Changed

- `smakefile` — ToolKit include paths, `headers` target, integrated `unittests` build
- Removed unused `DATESTAMP` from `Rev.h`

### Notes

- **ABI unchanged:** thirteen public LVOs, bias 30, `AsyncFile` opaque to applications
- Behaviour matches Holmgren **39.2** core; no new public functions

---

## [39.3] — 2025-07-31 (68k ToolKit preparation)

**Lineage:** 68k · `AsyncIO/` working tree  

### Added

- SAS/C + NDK 3.2 build refactor
- SFD file for modern prototype generation
- Updated SMakefile
- Initial unit tests (`test_asyncio.c`)

### Changed

- Source layout and build system modernised ahead of amigazen project GitHub release

---

## [50.3] — 2005-12-07 (AmigaOS 4)

**Lineage:** AmigaOS 4 · `AsyncIO_os4/`  

### Added

- Native AmigaOS 4 shared library (`RTF_NATIVE`)
- IDLTool-generated skeleton (2003) filled out to working implementation
- Interface-based OS4 API model (separate from 68k LVO numbering)

### Notes

- **Not binary-compatible** with 68k 39.x or this amigazen tree
- Requires `dos.library` v50+
- Autodoc subset predates 68k v39 `PeekAsync` documentation

---

## [40.6] — 2001-06-19 (WarpOS PPC)

**Lineage:** WarpOS · `asyncioppc/`  
**Maintainer:** Achim Stegemann

### Added

- WarpOS **PPC** variants of all functions (`OpenAsyncPPC`, `ReadAsyncPPC`, …)
- StormC 3.0 and VBCC include paths
- Full source in archive

### Fixed

- PPC write path: dummy `SPrintF` calls must remain (40.6 readme)

### Notes

- Library version **40.x** on disk; **68k LVO table remains V39-compatible**
- **Do not mix** 68k and PPC entry points on the same `AsyncFile` from the wrong CPU
- Email-ware; no copyright claimed by author
- Based on Holmgren **39.2** sources

---

## [40.0] — 2000-05-10 (WarpOS PPC initial)

**Lineage:** WarpOS · `asyncioppc/`  

Initial WarpOS port; PPC function table added alongside 68k V39 API.

---

## [39.2a] — 1997 (archive re-pack)

**Lineage:** 68k · `AsyncIO_orig/`, `archive/AsyncIO/`  

### Added

- Missing `AsyncIO.guide` and distribution icons

### Notes

- No code changes from 39.2 — documentation and archive completeness only

---

## [39.2] — 1997-09-02 (re-release 1997-11-09 build)

**Lineage:** 68k · Holmgren archive  
**Maintainer:** Magnus Holmgren

### Fixed

- `ReadLineAsync()` — EOF when last line lacks newline; buffer overrun by one byte
- `SeekAsync()` — experimental recovery after seek past EOF (more DOS-compatible)
- `ReadCharAsync()` / `WriteCharAsync()` — false success on error paths

### Changed

- Archive re-release corrected version string only (separate upload)

---

## [39.1] — 1997-07-27

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren

### Fixed

- `SeekAsync()` — corrected “David Eaves” fix (previous fix was wrong)
- `FGets(Len)Async()` — use full buffer capacity (off-by-one byte)
- `PeekAsync()` / `FGetsLenAsync()` — `ASIO_NOEXTERNALS` build tweaks

### Changed

- Cleaned `asyncio.doc`; technical notes moved to AmigaGuide
- Release archives use “real” version numbers
- SAS/C autoinit: `__asiolibversion` reference behaviour clarified
- DMakeFile object sub-drawers

---

## [39] — 1997-04-27 (Release 9)

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren

### Added

- `FGetsAsync()`, `FGetsLenAsync()` — Michael B. Smith (with minor Holmgren edits)
- `PeekAsync()`
- `__asiolibversion` for SAS/C autoinit

### Changed

- Library version bumped to **39**
- StormC include fixes — Alexander Kazik

---

## [38] — 1997-01-28 (Release 8)

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren

### Added

- `ReadLineAsync()` completed

### Fixed

- `SeekAsync()` EOF seek bug — David Eaves report

### Changed

- Library version bumped to **38**

---

## [37.2] — 1996-01-07 (Release 7)

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren

### Added

- E module support
- SAS/C 6.56 support

### Changed

- Library version **37.2**

---

## [37.1] — 1995-11-10 (Release 6)

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren

### Fixed

- `SeekAsync()` bug

### Changed

- First **shared** `asyncio.library` on `LIBS:`
- Library version **37.1**
- Include and documentation cleanup

---

## [Release 4] — 1995-09-13

**Lineage:** 68k shared library  
**Maintainer:** Magnus Holmgren

### Added

- Shared library build (`asyncio.library` on disk)
- Non-regargs link library variant
- `ASIO_NOEXTERNALS` compile option
- SnoopDos compatibility workaround (`mp_Node.ln_Name = NULL`)

---

## [Release 3] — 1995-08-12

**Lineage:** 68k  
**Maintainer:** Magnus Holmgren (took over from Martin Taillefer)

### Added

- Link library distribution
- `OpenAsyncFromFH()`
- `ASIO_NOEXTERNALS` (no Exec/DOS externals in objects)
- Half-size double buffers (same total RAM, better behaviour)
- Memory allocation fallback (reduce buffer on `AllocVec` failure)

### Fixed

- `SeekAsync()` performance and garbage-data issues
- Inspiration from Olaf Barthel (and Taillefer Release 3 overlap)

---

## [Release 2] — 1994-02-16

**Lineage:** 68k original  
**Author:** Martin Taillefer

### Fixed

- Consecutive `SeekAsync()` calls without intervening I/O
- `WriteAsync()` garbage after failed disk-full retry

---

## [Release 1] — 1994-03-23

**Lineage:** 68k original  
**Author:** Martin Taillefer

### Added

- Initial public release: double-buffered async file I/O link library

### Fixed

- `SeekAsync()` in-buffer packet handling
- Buffer allocation size (`bufferSize*2` → `bufferSize`)
- `MEMF_PUBLIC` allocation
- `Seek()` argument order
- `AsyncFile` typedef

---

## Lineage index

| Version | Date | Tree / fork |
|---------|------|-------------|
| 39.3 | 2026-07 | `github/asyncio.library/` (amigazen) |
| 39.3 | 2025-07 | `AsyncIO/` |
| 50.3 | 2005-12 | `AsyncIO_os4/` |
| 40.6 | 2001-06 | `asyncioppc/` |
| 40.0 | 2000-05 | `asyncioppc/` |
| 39.2a | 1997 | `AsyncIO_orig/`, `archive/` |
| 39.2 | 1997-09 | 68k Holmgren |
| 39.1 | 1997-07 | 68k Holmgren |
| 39 | 1997-04 | 68k Holmgren |
| 38 | 1997-01 | 68k Holmgren |
| 37.2 | 1996-01 | 68k Holmgren |
| 37.1 | 1995-11 | 68k Holmgren |
| Release 4 | 1995-09 | 68k Holmgren |
| Release 3 | 1995-08 | 68k Holmgren |
| Release 2 | 1994-02 | Taillefer |
| Release 1 | 1994-03 | Taillefer |

---

[39.3]: https://github.com/amigazen/asyncio.library
