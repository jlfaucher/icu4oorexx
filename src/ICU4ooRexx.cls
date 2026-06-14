-- ICU4ooRexx_lib.cls
-- ooRexx interface to ICU4C (International Components for Unicode)
-- Keep it compatible with ooRexx 4.2


-- If rxunicode.cls has been loaded by the user, register ICU4ooRexx.
-- This will make ICU4ooRexx available to the rxunicode package.
if .RexxUnicode~isa(.Class), .RexxUnicode~hasMethod("registerICU4ooRexx") then do
    .RexxUnicode~registerICU4ooRexx(.context~package)
end


--------------------------------------------------------------------------------
-- ICU4ooRexx constants

::class "ICU4ooRexx" public

-- When you modify these constants, update also the package version in ICU4ooRexx.cpp
::constant majorVersion 0
::constant minorVersion 0
::constant patchVersion 1
::constant version "0.0.1"

::method "U_ICU_VERSION" class unguarded external 'library ICU4ooRexx ICU4ooRexx_U_ICU_VERSION' -- ICU4C compile-time version
::method "u_getVersion" class unguarded external 'library ICU4ooRexx ICU4ooRexx_u_getVersion' -- ICU4C runtime version
::method "u_getUnicodeVersion" class unguarded external 'library ICU4ooRexx ICU4ooRexx_u_getUnicodeVersion' -- Supported Unicode version

-- UCharNameChoice constants (from unicode/uchar.h)
::constant U_UNICODE_CHAR_NAME  0   -- Official Unicode Name property (default)
::constant U_EXTENDED_CHAR_NAME 2   -- Standard or synthetic character name.
::constant U_CHAR_NAME_ALIAS    3   -- Corrected name from NameAliases.txt

::method "u_charName" class unguarded external 'library ICU4ooRexx ICU4ooRexx_u_charName'

::method "u_charFromName" class unguarded external 'library ICU4ooRexx ICU4ooRexx_u_charFromName'


--------------------------------------------------------------------------------
-- Routine interface (names mirror the ICU4C C API)

-- The routine names are quoted to avoid this kind of error with ooRexx 4.2:
--     Error 90.999:  Unable to find external routine "ICU4C_U_GETVERSION"
-- Other possible workaround for ooRexx 4.2:
-- declare the routine name after the library name:
--     ::routine ICU4C_u_getVersion public external 'library ICU4ooRexx ICU4C_u_getVersion'


-- ::routine "ICU4C_U_ICU_VERSION" public external 'library ICU4ooRexx' -- ICU4C compile-time version
-- ::routine "ICU4C_u_getVersion" public external 'library ICU4ooRexx' -- ICU4C runtime version

-- ::routine "ICU4C_u_charName" public external 'library ICU4ooRexx'
-- ::routine "ICU4C_u_charFromName" public external 'library ICU4ooRexx'
