-- Demonstrates u_charName and u_charFromName from ICU4ooRexx
--
-- Run:
--   rexx charnames.rex

-- Show static and dynamic ICU version
say
say "ICU4C compile-time version:" .ICU4ooRexx~U_ICU_VERSION
say "ICU4C runtime version:" .ICU4ooRexx~u_getVersion
say


-- Some codepoints to look up
codepoints = -
    65, -           -- LATIN CAPITAL LETTER A
    97, -           -- LATIN SMALL LETTER A
    "00E9"~x2d, -   -- LATIN SMALL LETTER E WITH ACUTE  (é)
    "03A9"~x2d, -   -- GREEK CAPITAL LETTER OMEGA        (Ω)
    "2603"~x2d, -   -- SNOWMAN                           (☃)
    "1F600"~x2d, -  -- GRINNING FACE                     (😀)
    "10FFFD"~x2d    -- last assigned private-use codepoint

say
say "Codepoint -> Name:"
say "-------------------"
do cp over codepoints
    name = .ICU4ooRexx~u_charName(cp)
    if name == "" then name = "(no name)"
    say .RexxUnicode~D2U(cp)~left(10) ':' name
end
say

-- Using the name selector
cp = "0001"~x2d
say "Using U_UNICODE_CHAR_NAME selector: "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_UNICODE_CHAR_NAME)    -- Unicode character name (Name property)
say "Using U_EXTENDED_CHAR_NAME selector:"  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_EXTENDED_CHAR_NAME)   -- Standard or synthetic character name
say "Using U_CHAR_NAME_ALIAS selector:   "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_CHAR_NAME_ALIAS)      -- Corrected name from NameAliases.txt
say
cp = "0022"~x2d
say "Using U_UNICODE_CHAR_NAME selector: "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_UNICODE_CHAR_NAME)    -- Unicode character name (Name property)
say "Using U_EXTENDED_CHAR_NAME selector:"  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_EXTENDED_CHAR_NAME)   -- Standard or synthetic character name
say "Using U_CHAR_NAME_ALIAS selector:   "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp, .ICU4ooRexx~U_CHAR_NAME_ALIAS)      -- Corrected name from NameAliases.txt
say

-- Using the convenience methods
cp = "2448"~x2d
say "Using u_charName:        "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~u_charName(cp)           -- Unicode character name (Name property)
say "Using h_extendedCharName:"  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~h_extendedCharName(cp)   -- Standard or synthetic character name
say "Using h_charNameAlias:   "  .RexxUnicode~D2U(cp) ":" .ICU4ooRexx~h_charNameAlias(cp)      -- Corrected name from NameAliases.txt
say


-- Reverse: name -> codepoint
say
say "Name -> Codepoint:"
say "-------------------"
names = -
    "SNOWMAN", -
    "GRINNING FACE", -
    "LATIN SMALL LETTER E WITH ACUTE", -
    "NO SUCH CHARACTER NAME" -

do nm over names
    cp = .ICU4ooRexx~u_charFromName(nm)
    name = ('"'nm'"')~left(35)
    if cp < 0 then
        say name "-> not found"
    else
        say name "->" .RexxUnicode~D2U(cp) "("cp")"
end
say


-- Reverse: extended name -> codepoint
say
say "ExtendedName -> Codepoint:"
say "---------------------------"
extendedNames = -
    "<private use area-10FFFD>", -
    "<private use area-10FFFE>", -
    "<noncharacter-10FFFE>"

do nm over extendedNames
    cp = .ICU4ooRexx~h_charFromExtendedName(nm)
    name = ('"'nm'"')~left(35)
    if cp < 0 then
        say name "-> not found"
    else
        say name "->" .RexxUnicode~D2U(cp) "("cp")"
end
say


-- Reverse: name alias -> codepoint
say
say "NameAlias -> Codepoint:"
say "------------------------"
nameAliases = -
    "LATIN CAPITAL LETTER GHA", -
    "MICR ON US SYMBOL", -
    "MICR DASH SYMBOL"

do nm over nameAliases
    cp = .ICU4ooRexx~h_charFromNameAlias(nm)
    name = ('"'nm'"')~left(35)
    if cp < 0 then
        say name "-> not found"
    else
        say name "->" .RexxUnicode~D2U(cp) "("cp")" '"'.ICU4ooRexx~u_charName(cp)'"'
end
say


say
say "Extended names algorithmically generated:"
say "------------------------------------------"
/*
An extended name is either the standard name if defined, or a code point label algorithmically generated.
Extended names are strictly unique across the entire codespace.
Code point labels are constructed by using a lowercase prefix derived from the code point type,
followed by a hyphen-minus and then a 4- to 6-digit hexadecimal representation of the code point.
    Control         <control-NNNN>
    Reserved        <reserved-NNNN>
    Noncharacter    <noncharacter-NNNN>
    Private-Use     <private-use-NNNN>
    Surrogate       <surrogate-NNNN>
U+0008 label is <control-0008>
*/
previousExtendedCharName = ""
previousCodepointType = ""
do cp = 0 to .ICU4ooRexx~maxCodepoint
    extendedCharName = .ICU4ooRexx~h_extendedCharName(cp)
    if extendedCharName~startsWith("<"), extendedCharName~endsWith(">") then do
        parse var extendedCharName "<" codepointType "-" codepointHex ">"
        if codepointType \= previousCodepointType then do
            if previousCodepointType \== "" then say previousExtendedCharName
            call charout , extendedCharName || "..."
        end
        previousExtendedCharName = extendedCharName
        previousCodepointType = codepointType
    end
end
if previousExtendedCharName \== "" then say previousExtendedCharName
say


say
say "Characters with a name alias:"
say "------------------------------"
do cp = 0 to .ICU4ooRexx~maxCodepoint
    charName = .ICU4ooRexx~u_charName(cp)
    charNameAlias = .ICU4ooRexx~h_charNameAlias(cp)
    if charNameAlias \== "" then do
        say .RexxUnicode~D2U(cp) '"'charName'"' "->" '"'charNameAlias'"'
    end
end


say
say "Search using loose names:"
say "--------------------------"
--signal end_loose_names -- takes time
do cp = 0 to .ICU4ooRexx~maxCodepoint
    charName = .ICU4ooRexx~u_charName(cp)
    looseCharName = " "charName" "

    extendedCharName = .ICU4ooRexx~h_extendedCharName(cp)
    looseExtendedCharName = " "extendedCharName" "

    charNameAlias = .ICU4ooRexx~h_charNameAlias(cp)
    looseCharNameAlias = " "charNameAlias" "

    -- A character name can be empty
    if charName \== "" then do
        cpFound = .ICU4ooRexx~h_charFromLooseName(charName)
        if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "charName" '"'charName'". Got' cpFound

        cpFound = .ICU4ooRexx~h_charFromLooseName(looseCharName)
        if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "looseCharName" '"'looseCharName'". Got' cpFound
    end

    -- A character extended name is never empty.
    cpFound = .ICU4ooRexx~h_charFromLooseName(extendedCharName)
    if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "extendedCharName" '"'extendedCharName'". Got' cpFound

    cpFound = .ICU4ooRexx~h_charFromLooseName(looseExtendedCharName)
    if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "looseExtendedCharName" '"'looseExtendedCharName'". Got' cpFound

    -- A character name alias can be empty
    if charNameAlias \== "" then do
        cpFound = .ICU4ooRexx~h_charFromLooseName(charNameAlias)
        if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "charNameAlias" '"'charNameAlias'". Got' cpFound

        cpFound = .ICU4ooRexx~h_charFromLooseName(looseCharNameAlias)
        if cp \== cpFound then say "*** h_charFromLooseName failed for" cp "looseCharNameAlias" '"'looseCharNameAlias'". Got' cpFound
    end
end
end_loose_names:
say "done (ok if nothing displayed)."


say
say "Error messages:"
say "----------------"
    testCase = 1
    signal on syntax
    1: say "1:"; say .ICU4ooRexx~u_charName(-1, 1000)                                   -- Argument 1 must be in the range 0 to 4294967295; found "-1".
    2: say "2:"; say .ICU4ooRexx~u_charName(.ICU4ooRexx~UCHAR_MAX_VALUE + 1, 1000)      -- Argument 2 must be in the range 0 to 255; found "1000".
    3: say "3:"; say .ICU4ooRexx~u_charName(.ICU4ooRexx~UCHAR_MAX_VALUE + 1, 100)       -- [ICU4ooRexx] Codepoint 0x110000 is out of Unicode range (must be 0x0000..0x10FFFF).
    4: say "4:"; say .ICU4ooRexx~u_charName(0, 100)                                     -- [ICU4ooRexx] NameChoice must be 0 (U_UNICODE_CHAR_NAME) or 2 (U_EXTENDED_CHAR_NAME) or 3 (U_CHAR_NAME_ALIAS); found 100.

    5: say "5:"; say .ICU4ooRexx~u_charFromName(.object, 1000)                          -- Argument 1 must have a string value.
    6: say "6:"; say .ICU4ooRexx~u_charFromName("<control-0000>", 1000)                 -- Argument 2 must be in the range 0 to 255; found "1000".
    7: say "7:"; say .ICU4ooRexx~u_charFromName("<control-0000>", 100)                  -- [ICU4ooRexx] NameChoice must be 0 (U_UNICODE_CHAR_NAME) or 2 (U_EXTENDED_CHAR_NAME) or 3 (U_CHAR_NAME_ALIAS); found 100.

    8: say "8:"; say .ICU4ooRexx~h_UAX44_LM2(.object)                                   -- Argument 1 must have a string value.
    9: say "9:"; say .ICU4ooRexx~h_UAX44_LM2("x"~copies(1000))                          -- [ICU4ooRexx] Name length must be lesser than 256.
    10: say "10:"; say .ICU4ooRexx~h_UAX44_LM2("père")                                  -- [ICU4ooRexx] Name must be an ASCII string; found "père".
    11: say "11:"; say .ICU4ooRexx~h_UAX44_LM2("père" || "x"~copies(250))               -- [ICU4ooRexx] Name must be an ASCII string; found "pèrexxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx".

    12:

exit

syntax:
    call sayCondition condition("O")
    testCase += 1
    signal on syntax
    signal value testCase


::routine sayCondition
    use strict arg condition
    if condition == .nil then return

    if condition~condition <> "SYNTAX" then .error~say(condition~condition)
    if condition~description <> .nil, condition~description <> "" then .error~say(condition~description)

    -- For SYNTAX conditions
    if condition~message <> .nil then .error~say(condition~message)
    else if condition~errortext <> .nil then .error~say(condition~errortext)
    if condition~code <> .nil then .error~say("Error code=" condition~code)


::requires "ICU4ooRexx.cls"
::requires "rxunicode.cls" -- for D2U
