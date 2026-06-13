# ICU4ooRexx

ICU4ooRexx is an ooRexx library that wraps the ICU4C C/C++ libraries.

ICU stands for "International Components for Unicode".  
CLDR stands for "Common Locale Data Repository".  

ICU homepage:  [https://icu.unicode.org][ICU]  
ICU4C API:      [https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/][ICU4C_API]  
CLDR homepage:  [https://cldr.unicode.org][CLDR]  
Wikipedia:      [https://en.wikipedia.org/wiki/International_Components_for_Unicode][Wikipedia]

## Additional resources:

- [Diary][diary]
- [Instructions to build ICU4ooRexx ][build_icu4oorexx]


## ICU4C, not ICU4X

ICU4ooRexx uses ICU4C instead of ICU4X.

### ICU4X

ICU4X was evaluated, but it does not currently provide ready-to-use
libraries suitable for this project. In particular, using ICU4X would
require installing Rust and using Cargo, which is not desirable in
this context.  

If ICU4X eventually provides libraries in a form similar to ICU4C,
this decision may be reconsidered.

Update: since ICU4X 2.1.0 (Oct 28, 2025), prebuilt binaries have been provided.
To use the C/C++ FFI layer, you must extract the C++ header files yourself from
the source archive.

ICU4X is gaining traction in environments where binary size and
sandbox safety matter more than feature completeness, for example:

- embedded systems
- WebAssembly (WASM)
- mobile runtimes
- sandboxed environments

### ICU4C

For a language runtime like ooRexx, ICU4C is currently the most practical
choice.

ooRexx uses two complementary Unicode libraries. The lightweight utf8proc
library provides core Unicode text processing directly in the runtime, while
ICU4C provides extended Unicode functionality when needed:

- The utf8proc library is statically embedded for core Unicode text
  processing:
  
    - Unicode normalization (NFC/NFD/NFKC/NFKD)
    - Locale-agnostic case mapping and case folding
    - Grapheme cluster segmentation
    - Character property inspection
    - Filtering or removal of characters (e.g., accents)

- ICU4C provides extended Unicode functionality when needed:

    - Locale-aware case mapping
    - Collation
    - Transliteration
    - Break iterators (word/sentence/line)
    - Bidirectional text
    - Regular expressions
    - Formatting (numbers, dates, etc.)

  Loaded optionally via:
  
  ```REXX
  ::requires "ICU4ooRexx.cls"
  ```


## ICU installation

ICU releases:  [https://github.com/unicode-org/icu/releases][ICU_releases]


### Windows-ARM64

Declare the ICU_ROOT environment variable, update PATH, INCLUDE and LIB.  
For example:
```Shell
set ICU_ROOT=E:\ICU4C\icu4c-78.2-WinARM64-MSVC2022
set PATH=%ICU_ROOT%\binARM64;%PATH%
set INCLUDE=%ICU_ROOT%\include\unicode;%INCLUDE%
set LIB=%ICU_ROOT%\libARM64;%LIB%
```

### Windows-X86_64

Declare the ICU_ROOT environment variable, update PATH, INCLUDE and LIB.  
For example:
```Shell
set ICU_ROOT="E:\ICU4C\icu4c-78.2-Win64-MSVC2022"
set PATH "%ICU_ROOT%\bin64;%PATH%"
set INCLUDE="%ICU_ROOT%\include\unicode;%INCLUDE%"
set LIB="%ICU_ROOT%\lib64;%LIB%"
```

### Windows-WSL

Follow the instructions of your package manager. 
``` 
libicu-dev  
libicu<version>
```


### macOS

Assuming you installed ICU4C with Homebrew.

```Shell
brew info icu4c

icu4c is keg-only, which means it was not symlinked into /opt/homebrew,
because macOS provides libicucore.dylib (but nothing else).
```

Follow the instructions of your package manager.

```Shell
export ICU_ROOT="/opt/homebrew/opt/icu4c"
export PATH="$ICU_ROOT/bin:$PATH"
export PATH="$ICU_ROOT/sbin:$PATH"
export DYLD_LIBRARY_PATH="$ICU_ROOT/lib:$DYLD_LIBRARY_PATH"
export CPPFLAGS="-I$ICU_ROOT/include $CPPFLAGS"
export LDFLAGS="-L$ICU_ROOT/lib $LDFLAGS"
export PKG_CONFIG_PATH="$ICU_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH"
```

[CLDR]: https://cldr.unicode.org
[diary]: https://github.com/jlfaucher/icu4oorexx/blob/main/_diary.txt "Diary"
[build_icu4oorexx]: https://github.com/jlfaucher/builder/blob/master/build-ICU4ooRexx.txt
[ICU]: https://icu.unicode.org
[ICU_releases]: https://github.com/unicode-org/icu/releases
[ICU4C_API]: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/
[Wikipedia]: https://en.wikipedia.org/wiki/International_Components_for_Unicode
