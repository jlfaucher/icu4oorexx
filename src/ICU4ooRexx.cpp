/*******************************************************************************
* ICU4ooRexx.cpp
* An ooRexx interface library to ICU4C
*
* Binding:
* Both a routine-based interface (mirroring ICU4C C API names)
* and a method-based interface are provided.
*******************************************************************************/

#include "config.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>          // include first so all subsequent inclusions are no-ops
#endif

#include <stdio.h>
#include <stdarg.h>

#include "unicode/uchar.h"      // u_charName, u_charFromName, UChar32, ...
#include "unicode/uvernum.h"    // U_ICU_VERSION
#include "oorexxapi.h"


/*******************************************************************************
*
* Helpers
*
*******************************************************************************/

// From a routine
// Raise an ooRexx Incorrect_method_user_defined exception
static void raiseException(RexxCallContext* c, const char* fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
}


// From a method
// Raise an ooRexx Incorrect_method_user_defined exception
static void raiseException(RexxMethodContext* c, const char* fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
}


/*******************************************************************************
*
* Routine interface
* Function names mirror the ICU4C C API.
*
*******************************************************************************/

#if 0
RexxRoutine2(RexxStringObject,
             ICU4C_u_charName,
             uint32_t,          codepoint,
             OPTIONAL_uint8_t,  nameChoice)
#endif


/**
 * Returns the Unicode name of a codepoint as an ooRexx String.
 *
 * ICU4ooRexx_u_charName(codepoint [, nameChoice])
 *
 * @param codepoint
 *   The Unicode codepoint whose name should be returned.
 *
 * @param nameChoice (optional)
 *   Selects which Unicode name to return:
 *   - 0 = U_UNICODE_CHAR_NAME (default) — Unicode character name (Name property).
 *   - 2 = U_EXTENDED_CHAR_NAME - Standard or synthetic character name.
 *   - 3 = U_CHAR_NAME_ALIAS — Corrected name from NameAliases.txt
 *
 * @return
 *   The Unicode name, or an empty string if the codepoint has no name
 *   (e.g. control characters, surrogates, or unassigned codepoints).
 *
 * @par Example
 * @code
 *   say .ICU4ooRexx~u_charName("1F600"~x2d)                           -- "GRINNING FACE"
 *   say .ICU4ooRexx~u_charName(65)                                    -- "LATIN CAPITAL LETTER A"
 *   say .ICU4ooRexx~u_charName(0)                                     -- ""                   NULL has no Unicode Name
 *   say .ICU4ooRexx~u_charName(0, .ICU4ooRexx~U_EXTENDED_CHAR_NAME)   -- "<control-0000>"     NULL has an extended name
 * @endcode
 */
RexxMethod2( RexxStringObject,
             ICU4ooRexx_u_charName,
             uint32_t,          codepoint,
             OPTIONAL_uint8_t,  nameChoice)
{
    // Validate codepoint range: Unicode is U+0000 .. U+10FFFF
    if (codepoint > 0x10FFFF) {
        raiseException(context,
            "u_charName: codepoint 0x%X is out of Unicode range "
            "(must be 0x0000..0x10FFFF)", codepoint);
        return context->NullString();
    }

    UCharNameChoice choice = U_UNICODE_CHAR_NAME;
    if (argumentExists(2)) {
        if (nameChoice != U_UNICODE_CHAR_NAME &&
            nameChoice != U_EXTENDED_CHAR_NAME &&
            nameChoice != U_CHAR_NAME_ALIAS) {
            raiseException(context,
                "nameChoice must be %d (U_UNICODE_CHAR_NAME) "
                "or %d (U_EXTENDED_CHAR_NAME) "
                "or %d (U_CHAR_NAME_ALIAS); found %d",
                (int)U_UNICODE_CHAR_NAME, (int)U_EXTENDED_CHAR_NAME, (int)U_CHAR_NAME_ALIAS,
                nameChoice);
            return context->NullString();
        }
        choice = (UCharNameChoice)nameChoice;
    }

    char        buf[256];
    UErrorCode  err = U_ZERO_ERROR;
    int32_t     len = u_charName((UChar32)codepoint, choice,
                                 buf, (int32_t)sizeof(buf), &err);

    if (U_FAILURE(err) || len == 0)
        return context->NullString();   // no name for this codepoint

    return context->String(buf);
}


#if 0
RexxRoutine2(int32_t,
             ICU4C_u_charFromName,
             CSTRING,           name,
             OPTIONAL_uint8_t,  nameChoice)
#endif


/**
 * Returns the codepoint of a Unicode name as an ooRexx integer.
 *
 * ICU4ooRexx_u_charFromName(name [, nameChoice])
 *
 * @param name
 *   The name whose Unicode codepoint should be returned.
 *
 * @param nameChoice (optional)
 *   Selects which Unicode name to return:
 *   - 0 = U_UNICODE_CHAR_NAME (default) — Unicode character name (Name property).
 *   - 2 = U_EXTENDED_CHAR_NAME - Standard or synthetic character name.
 *   - 3 = U_CHAR_NAME_ALIAS — Corrected name from NameAliases.txt
 *
 * @return
 *   the codepoint (as an ooRexx integer) whose Unicode name is
 *   exactly 'name', or -1 if not found.
 *
 * @par Example
 * @code
 *   say .ICU4ooRexx~u_charFromName("GRINNING FACE")                                       -- 128512
 *   say .ICU4ooRexx~u_charFromName("LATIN CAPITAL LETTER A")                              -- 97
 *   say .ICU4ooRexx~u_charFromName("<control-0000>")                                      -- -1
 *   say .ICU4ooRexx~u_charFromName("<control-0000>", .ICU4ooRexx~U_EXTENDED_CHAR_NAME)    -- 0
 * @endcode
 */
RexxMethod2( int32_t,
             ICU4ooRexx_u_charFromName,
             CSTRING,           name,
             OPTIONAL_uint8_t,  nameChoice)
{
    UCharNameChoice choice = U_UNICODE_CHAR_NAME;
    if (argumentExists(2)) {
        if (nameChoice != U_UNICODE_CHAR_NAME &&
            nameChoice != U_EXTENDED_CHAR_NAME &&
            nameChoice != U_CHAR_NAME_ALIAS) {
            raiseException(context,
                "nameChoice must be %d (U_UNICODE_CHAR_NAME) "
                "or %d (U_EXTENDED_CHAR_NAME) "
                "or %d (U_CHAR_NAME_ALIAS); found %d",
                (int)U_UNICODE_CHAR_NAME, (int)U_EXTENDED_CHAR_NAME, (int)U_CHAR_NAME_ALIAS,
                nameChoice);
            return -1;
        }
        choice = (UCharNameChoice)nameChoice;
    }

    UErrorCode err = U_ZERO_ERROR;
    UChar32 cp = u_charFromName(choice, name, &err);

    if (U_FAILURE(err) || cp < 0)
        return -1;

    return (int32_t)cp;
}


#if 0
RexxRoutine0(RexxStringObject,
             ICU4C_U_ICU_VERSION)
#endif


/**
 * ICU4ooRexx_U_ICU_VERSION()
 *
 * Returns a string identifying the ICU4C version this binding was built
 * against, e.g. "73.2"
 *
 * Note: this reports the compile-time version (from uvernum.h).
 */
RexxMethod0( RexxStringObject,
             ICU4ooRexx_U_ICU_VERSION)
{
    char buf[U_MAX_VERSION_STRING_LENGTH];
    snprintf(buf, sizeof(buf), U_ICU_VERSION);
    return context->String(buf);
}


#if 0
RexxRoutine0(RexxStringObject,
             ICU4C_u_getVersion)
#endif


/**
 * ICU4ooRexx_u_getVersion()
 *
 * Returns a string identifying the ICU4C version this binding is dynamically
 * liked against, e.g. "73.2"
 *
 * Note: this reports the runtime version (returned by u_getVersion()).
 */
RexxMethod0( RexxStringObject,
             ICU4ooRexx_u_getVersion)
{
    UVersionInfo versionArray;
    u_getVersion(versionArray);
    char buf[U_MAX_VERSION_STRING_LENGTH];
    u_versionToString(versionArray, buf);
    return context->String(buf);
}


/*******************************************************************************
*
* Package definition
*
*******************************************************************************/

// ICU4C API routines wrappers
RexxRoutineEntry ICU4ooRexx_routines[] = {
    // REXX_TYPED_ROUTINE(ICU4C_U_ICU_VERSION,         ICU4C_U_ICU_VERSION),
    // REXX_TYPED_ROUTINE(ICU4C_u_getVersion,          ICU4C_u_getVersion),

    // REXX_TYPED_ROUTINE(ICU4C_u_charName,            ICU4C_u_charName),
    // REXX_TYPED_ROUTINE(ICU4C_u_charFromName,        ICU4C_u_charFromName),

    REXX_LAST_ROUTINE()
};


// ICU4C API methods wrappers
RexxMethodEntry ICU4ooRexx_methods[] = {
    REXX_METHOD(ICU4ooRexx_U_ICU_VERSION,         ICU4ooRexx_U_ICU_VERSION),
    REXX_METHOD(ICU4ooRexx_u_getVersion,          ICU4ooRexx_u_getVersion),

    REXX_METHOD(ICU4ooRexx_u_charName,            ICU4ooRexx_u_charName),
    REXX_METHOD(ICU4ooRexx_u_charFromName,        ICU4ooRexx_u_charFromName),

    REXX_LAST_METHOD()
};


// When you modify the package version, update also the constants in ICU4ooRexx.cls
RexxPackageEntry ICU4ooRexx_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,         // works with any ooRexx >= 4.0.0
    "ICU4ooRexx",                   // package name (must match library name)
    "0.0.1",                        // package version (keep it aligned with the version in ICU4ooRexx.cls)
    NULL,                           // no load function needed
    NULL,                           // no unload function needed
    ICU4ooRexx_routines,            // exported routines
    ICU4ooRexx_methods              // exported methods
};

OOREXX_GET_PACKAGE(ICU4ooRexx);
