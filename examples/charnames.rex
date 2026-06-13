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


-- Using the name selector
say
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


::requires "ICU4ooRexx.cls"
::requires "rxunicode.cls" -- for D2U
