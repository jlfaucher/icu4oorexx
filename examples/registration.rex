say "If rxunicode.cls is not loaded before ICU4ooRexx.cls, the automatic registration is not done."
.context~package~loadPackage("ICU4ooRexx.cls")
.context~package~loadPackage("rxunicode.cls")
say ".RexxUnicode~ICU4ooRexxIsRegistered =" .RexxUnicode~ICU4ooRexxIsRegistered
say ".RexxUnicode~character('€')~name =" .RexxUnicode~character('€')~name

say "Registering ICU4ooRexx"
ICU4ooRexxClass = .context~package~findPublicClass("ICU4ooRexx")
.RexxUnicode~registerICU4ooRexx(ICU4ooRexxClass~package)
say ".RexxUnicode~ICU4ooRexxIsRegistered =" .RexxUnicode~ICU4ooRexxIsRegistered
say ".RexxUnicode~character('€')~name =" .RexxUnicode~character('€')~name
