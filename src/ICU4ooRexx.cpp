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
#include <string.h>
#include <ctype.h>

#include <cstdint>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "unicode/uchar.h"      // u_charName, u_charFromName, UChar32, ...
#include "unicode/uvernum.h"    // U_ICU_VERSION
#include <unicode/utypes.h>
#include <unicode/ustring.h>

#include "oorexxapi.h"

// strcasecmp is called _stricmp by MSVC
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

// ICU4C doesnt define the maximum buffer size needed for u_charName.
#define NAME_MAX_SIZE 256
// A loose name is not an ICU4C concept
#define LOOSE_NAME_MAX_SIZE 256


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
    c->RaiseException1(Rexx_Error_Incorrect_call_user_defined, c->String(buf));
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



/*******************************************************************************
*
* Class interface
* Method names mirror the ICU4C C API.
*
*******************************************************************************/

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
            "Codepoint 0x%X is out of Unicode range "
            "(must be 0x0000..0x10FFFF)", codepoint);
        return context->NullString();
    }

    UCharNameChoice choice = U_UNICODE_CHAR_NAME;
    if (argumentExists(2)) {
        if (nameChoice != U_UNICODE_CHAR_NAME &&
            nameChoice != U_EXTENDED_CHAR_NAME &&
            nameChoice != U_CHAR_NAME_ALIAS) {
            raiseException(context,
                "NameChoice must be %d (U_UNICODE_CHAR_NAME) "
                "or %d (U_EXTENDED_CHAR_NAME) "
                "or %d (U_CHAR_NAME_ALIAS); found %d",
                (int)U_UNICODE_CHAR_NAME, (int)U_EXTENDED_CHAR_NAME, (int)U_CHAR_NAME_ALIAS,
                nameChoice);
            return context->NullString();
        }
        choice = (UCharNameChoice)nameChoice;
    }

    char        buf[NAME_MAX_SIZE];
    UErrorCode  status = U_ZERO_ERROR;
    int32_t     len = u_charName((UChar32)codepoint, choice,
                                 buf, (int32_t)sizeof(buf), &status);

    if (U_FAILURE(status) || len == 0)
        return context->NullString();   // no name for this codepoint

    return context->String(buf);
}


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
 *   The codepoint (as an ooRexx integer) whose Unicode name is
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
                "NameChoice must be %d (U_UNICODE_CHAR_NAME) "
                "or %d (U_EXTENDED_CHAR_NAME) "
                "or %d (U_CHAR_NAME_ALIAS); found %d",
                (int)U_UNICODE_CHAR_NAME, (int)U_EXTENDED_CHAR_NAME, (int)U_CHAR_NAME_ALIAS,
                nameChoice);
            return -1;
        }
        choice = (UCharNameChoice)nameChoice;
    }

    UErrorCode status = U_ZERO_ERROR;
    UChar32 codepoint = u_charFromName(choice, name, &status);

    if (U_FAILURE(status) || codepoint < 0)
        return -1;

    return (int32_t)codepoint;
}


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


/**
 * ICU4ooRexx_u_getVersion()
 *
 * Returns a string identifying the ICU4C version this binding is dynamically
 * linked against, e.g. "73.2"
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


/**
 * ICU4ooRexx_u_getUnicodeVersion()
 *
 * Returns a string identifying the Unicode version supported by ICU4C.
 */
RexxMethod0( RexxStringObject,
             ICU4ooRexx_u_getUnicodeVersion)
{
    UVersionInfo versionArray;
    u_getUnicodeVersion(versionArray);
    char buf[U_MAX_VERSION_STRING_LENGTH];
    u_versionToString(versionArray, buf);
    return context->String(buf);
}


/*******************************************************************************
*
* Helpers
*
*******************************************************************************/

/*
 * UAX44-LM2 normalization for Unicode character name matching.
 * https://unicode.org/reports/tr44/#UAX44-LM2
 *
 * Transforms 'input' (a NUL-terminated UTF-8/ASCII character name)
 * into the normalized form used for loose matching:
 *   1. Remove medial hyphens (except in "HANGUL JUNGSEONG O-E")
 *   2. Remove all whitespace and underscores
 *   3. Fold to lowercase
 *
 * 'buf' must be a writable buffer of at least strlen(input)+1 bytes.
 * If 'input' is too large, NULL is returned.
 * The result is written into 'buf'. Returns 'buf'.
 *
 * Note: character names are ASCII by specification, so no ICU string
 * machinery is needed beyond what is shown here.
 */
static char *UAX44_LM2(const char *input, size_t len, char *buf, size_t buf_size, int &error)
{
    /* --- Step 0: buf~space ------------------------------------------------ */

    /* copy input and collapse runs of spaces (simulate
       ooRexx buf~space which normalises internal whitespace to
       single spaces and strips leading/trailing spaces). */

    if (input == NULL || buf == NULL) {
        error = 1;
        return NULL; // NULL argument (internal)
    }

    if (len > buf_size - 1) {
        error = 2;
        return NULL; // input too large
    }

    /* collapse whitespace into single spaces, trim ends */
    size_t w = 0;
    int in_space = 1; /* start as 1 to trim leading spaces */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (c >= 128) {
            error = 3; // not ASCII
            return NULL;
        }
        if (c == ' ' || c == '\t') {
            if (!in_space && w > 0)
                buf[w++] = ' ';
            in_space = 1;
        } else {
            buf[w++] = c;
            in_space = 0;
        }
    }
    /* trim trailing space */
    if (w > 0 && buf[w-1] == ' ')
        w--;
    buf[w] = '\0';

    /* --- Step 1: remove medial hyphens ------------------------------------ */

    /* Special-case: if the (space-normalised) name is exactly
       "HANGUL JUNGSEONG O-E" (case-insensitive) leave it untouched
       for this step. We compare before lowercasing. */
    int is_oe = (strcasecmp(buf, "HANGUL JUNGSEONG O-E") == 0);

    if (!is_oe) {
        w = 0;
        len = strlen(buf);
        for (size_t i = 0; i < len; i++) {
            unsigned char c = (unsigned char)buf[i];
            if (c == '-') {
                /* medial = preceded and followed by a letter */
                int prev_alpha = (i > 0)        && isalpha((unsigned char)buf[i-1]);
                int next_alpha = (i + 1 < len)  && isalpha((unsigned char)buf[i+1]);
                if (prev_alpha && next_alpha) {
                    /* drop the hyphen (replace with nothing) */
                    continue;
                }
            }
            buf[w++] = c;
        }
        buf[w] = '\0';
    }

    /* --- Step 2: remove all whitespace and underscores. ------------------- */
    len = strlen(buf);
    w = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)buf[i];
        if (c == ' ' || c == '\t' || c == '_')
            continue;
        buf[w++] = c;
    }
    buf[w] = '\0';

    /* --- Step 3: fold to lowercase. --------------------------------------- */
    for (size_t i = 0; buf[i]; i++)
        buf[i] = (char)tolower((unsigned char)buf[i]);

    return buf;
}


static const char *normalizeName(const char *looseName, size_t looseName_size, char *buf, size_t buf_size, RexxMethodContext* context)
{
    int error = 0;

    char *normalizedName = UAX44_LM2(looseName, looseName_size, buf, buf_size, error);

    if (error != 0) {
        if (context != NULL) {
            if      (error == 1) raiseException(context, "Internal error: NULL argument");
            else if (error == 2) raiseException(context, "Name length must be lesser than %d", buf_size);
            else if (error == 3) raiseException(context, "Name must be an ASCII string; found \"%s\"", looseName);
            else                 raiseException(context, "Unknown error %i", error);
        }
        return NULL;
    }

    return normalizedName;
}


/**
 * UAX44-LM2 normalization for Unicode character name matching.
 * https://unicode.org/reports/tr44/#UAX44-LM2
 *
 * ICU4ooRexx_h_UAX44_LM2(looseName)
 *
 * @param looseName
 *   The name to normalize according to the loose matching rule UAX44-LM2.
 *   Must be an ASCII string.
 *
 * @return
 *   The normalized string
 *
 * @par Example
 * @code
 *   say .ICU4ooRexx~h_UAX44_LM2("ZERO WIDTH SPACE")      -- "zerowidthspace"
 *   say .ICU4ooRexx~h_UAX44_LM2("Zero_Width-Space")      -- "zerowidthspace"
 *   say .ICU4ooRexx~h_UAX44_LM2("   ZeroWidth Space ")   -- "zerowidthspace"
 * @endcode
 */
RexxMethod1( RexxStringObject,
             ICU4ooRexx_h_UAX44_LM2,
             RexxStringObject,  looseName)
{
    size_t looseName_size = context->StringLength(looseName);
    const char *looseName_data = context->StringData(looseName);

    char buf[LOOSE_NAME_MAX_SIZE];
    const char *normalizedName = normalizeName(looseName_data, looseName_size, buf, sizeof(buf), context);

    if (normalizedName == NULL) return context->NullString();
    return context->String(normalizedName);
}


#define NEW_LOOSENAME_IMPLEMENTATION 1

#if NEW_LOOSENAME_IMPLEMENTATION

struct NormalizedName
{
    uint64_t normalizedNameHash;
    UChar32 codepoint;
};

static std::vector<NormalizedName> normalizedNames; // sorted by hash

#else

static std::unordered_map<std::string, UChar32> normalizedNames;

#endif


#if NEW_LOOSENAME_IMPLEMENTATION
static uint64_t fnv1aHash(const char *s)
{
    uint64_t hash = 0xcbf29ce484222325ULL;
    constexpr uint64_t prime = 0x100000001b3ULL;
    if (s != NULL) {
        while (*s) {
            unsigned char c = (unsigned char)*s;
            hash ^= c;
            hash *= prime;
            s++;
        }
    }
    return hash;
}
#endif


// Callback used by u_enumCharNames
static UBool U_CALLCONV initializeNormalizedNamesCallback(
    void* context,
    UChar32 codepoint,
    UCharNameChoice nameChoice,
    const char* name,
    int32_t length)
{
    char buf[LOOSE_NAME_MAX_SIZE];
    const char *normalizedName = normalizeName(name, length, buf, sizeof(buf), NULL); // NULL ==> don't raise exception

#if NEW_LOOSENAME_IMPLEMENTATION
    if (normalizedName != NULL) {
        uint64_t normalizedNameHash = fnv1aHash(normalizedName);
        normalizedNames.push_back({normalizedNameHash, codepoint});
    }

    // Add the name alias, if not empty
    {
        char buf[NAME_MAX_SIZE];
        const char *nameAlias = buf;
        UErrorCode status = U_ZERO_ERROR;
        int32_t len = u_charName(codepoint, U_CHAR_NAME_ALIAS, buf, sizeof(buf), &status);
        if (U_SUCCESS(status) && len != 0) {
            char buf[LOOSE_NAME_MAX_SIZE];
            const char *normalizedNameAlias = normalizeName(nameAlias, strlen(nameAlias), buf, sizeof(buf), NULL);
            if (normalizedNameAlias != NULL) {
                // printf("[ICU4ooRexx] push_back codepoint %i '%s'\n", codepoint, normalizedNameAlias);
                uint64_t normalizedNameAliasHash = fnv1aHash(normalizedNameAlias);
                normalizedNames.push_back({normalizedNameAliasHash, codepoint});
            }
        }
    }
#else
    if (normalizedName != NULL) normalizedNames[normalizedName] = codepoint;
    // TODO: add all the aliases
#endif
    return true; // keep going
}


static UErrorCode initializeNormalizedNames()
{
    static UErrorCode status = U_ZERO_ERROR;
    if (status != 0) return status; // A previous call failed, return the last failed status
    if (normalizedNames.empty() == false) return U_ZERO_ERROR; // Already initialized

    u_enumCharNames(
        0, UCHAR_MAX_VALUE + 1, // full Unicode range
        initializeNormalizedNamesCallback,
        NULL,  // context not used
        U_EXTENDED_CHAR_NAME, // extended character names
        &status
    );

#if NEW_LOOSENAME_IMPLEMENTATION
    std::sort(normalizedNames.begin(), normalizedNames.end(),
        [](const NormalizedName &nn1, const NormalizedName &nn2) { return nn1.normalizedNameHash < nn2.normalizedNameHash; });
#endif

    return status;
}


#if NEW_LOOSENAME_IMPLEMENTATION
// The initialization is made when this library is loaded
static struct NormalizedNamesInitializer
{
    NormalizedNamesInitializer()
    {
        UErrorCode status = initializeNormalizedNames();
        if (status != 0) {
            // can't raiseException because context not available
            fprintf(stderr, "[ICU4ooRexx] Could not initialize the mapping from normalized names to codepoints");
        }
    };
} normalizedNamesInitializer;
#endif


#if NEW_LOOSENAME_IMPLEMENTATION
static UChar32 lookup(const char *looseName, size_t looseName_size)
{
    char bufLooseName[LOOSE_NAME_MAX_SIZE];
    const char *normalizedLooseName = normalizeName(looseName, looseName_size, bufLooseName, sizeof(bufLooseName), NULL);

    uint64_t hash = fnv1aHash(normalizedLooseName);
    // binary search
    auto it = std::lower_bound(normalizedNames.begin(), normalizedNames.end(), hash,
        [](const NormalizedName &nn, uint64_t hash){ return nn.normalizedNameHash < hash; });

    // Handles collisions.
    // On a hash hit, call u_charName() with U_EXTENDED_CHAR_NAME on the candidate codepoint,
    // re-normalize it, and confirm it matches normalizedLooseName.
    // If no match with U_EXTENDED_CHAR_NAME, test with U_CHAR_NAME_ALIAS.
    // For info, I checked ALL the names of Unicode 17 with a script, there is ZERO collision.
    int count = 0;
    while (it != normalizedNames.end() && it->normalizedNameHash == hash)
    {
        char bufName[NAME_MAX_SIZE];
        char bufLooseName[LOOSE_NAME_MAX_SIZE];

        // Compare with U_EXTENDED_CHAR_NAME
        const char *extendedName = bufName;
        UErrorCode status = U_ZERO_ERROR;
        int32_t len = u_charName(it->codepoint, U_EXTENDED_CHAR_NAME, bufName, sizeof(bufName), &status);
        if (U_SUCCESS(status) && len != 0) {
            const char *normalizedExtendedName = normalizeName(extendedName, strlen(extendedName), bufLooseName, sizeof(bufLooseName), NULL);
            if (strcmp(normalizedLooseName, normalizedExtendedName) == 0) {
                // if (count > 0) printf("[ICU4ooRexx] looseName '%s' found after %i collisions\n", looseName, count);
                return it->codepoint;
            }
        }

        // Compare with U_CHAR_NAME_ALIAS
        const char *nameAlias = bufName;
        status = U_ZERO_ERROR;
        len = u_charName(it->codepoint, U_CHAR_NAME_ALIAS, bufName, sizeof(bufName), &status);
        if (U_SUCCESS(status) && len != 0) {
            const char *normalizedNameAlias = normalizeName(nameAlias, strlen(nameAlias), bufLooseName, sizeof(bufLooseName), NULL);
            if (strcmp(normalizedLooseName, normalizedNameAlias) == 0) {
                // if (count > 0) printf("[ICU4ooRexx] looseName '%s' found after %i collisions\n", looseName, count);
                return it->codepoint;
            }
        }

        ++it;
        count++;
    }
    // if (count > 0) printf("[ICU4ooRexx] looseName '%s' NOT found after %i collisions\n", looseName, count);
    return -1;
}
#endif


/**
 * Returns the codepoint of a Unicode loose name as an ooRexx integer.
 *
 * ICU4ooRexx_h_charFromLooseName(looseName)
 *
 * @param looseName
 *   The loose name whose Unicode codepoint should be returned.
 *
 * @return
 *   The codepoint (as an ooRexx integer) whose Unicode name is matched
 *   according to the loose matching rule UAX44-LM2, or -1 if not found.
 *
 * @par Example
 * @code
 *   say .ICU4ooRexx~h_charFromLooseName("ZERO WIDTH SPACE")        -- 8203
 *   say .ICU4ooRexx~h_charFromLooseName("Zero_Width-Space")        -- 8203
 *   say .ICU4ooRexx~h_charFromLooseName("   ZeroWidth Space ")     -- 8203
 * @endcode
 */
RexxMethod1( int32_t,
             ICU4ooRexx_h_charFromLooseName,
             RexxStringObject,  looseName)
{
#if NEW_LOOSENAME_IMPLEMENTATION
    size_t looseName_size = context->StringLength(looseName);
    const char *looseName_data = context->StringData(looseName);

    UChar32 codepoint = lookup(looseName_data, looseName_size);

    return (int32_t)codepoint;
#else
    // The initialization is made at the first call
    // The next calls receive the same value as the first call
    UErrorCode status = initializeNormalizedNames();
    if (status != 0) {
        raiseException(context, "Could not initialize the mapping from normalized names to codepoints");
        return -1; // The initialization failed
    }

    size_t looseName_size = context->StringLength(looseName);
    const char *looseName_data = context->StringData(looseName);

    char buf[LOOSE_NAME_MAX_SIZE];
    const char *normalizedName = normalizeName(looseName_data, looseName_size, buf, sizeof(buf), context);

    UChar32 codepoint = -1;
    if (normalizedName != NULL) {
        auto it = normalizedNames.find(normalizedName);
        if (it != normalizedNames.end()) codepoint = it->second;
    }

    return (int32_t)codepoint;
#endif
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
    REXX_METHOD(ICU4ooRexx_U_ICU_VERSION,                   ICU4ooRexx_U_ICU_VERSION),
    REXX_METHOD(ICU4ooRexx_u_getVersion,                    ICU4ooRexx_u_getVersion),
    REXX_METHOD(ICU4ooRexx_u_getUnicodeVersion,             ICU4ooRexx_u_getUnicodeVersion),

    REXX_METHOD(ICU4ooRexx_u_charName,                      ICU4ooRexx_u_charName),
    REXX_METHOD(ICU4ooRexx_u_charFromName,                  ICU4ooRexx_u_charFromName),
    REXX_METHOD(ICU4ooRexx_h_UAX44_LM2,                     ICU4ooRexx_h_UAX44_LM2),
    REXX_METHOD(ICU4ooRexx_h_charFromLooseName,             ICU4ooRexx_h_charFromLooseName),

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
