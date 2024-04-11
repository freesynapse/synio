Lexer
    Ärvda klasser av Lexer för varje filtyp som går (nu bara C/C++). Basklassen () 
    innehåller funktionspekare till parsing och highlighting, som i basfallet inte gör något.

Tabs -- om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)

UX
    -- highlight för rad, annan nyans av grå än select (Config::HIGHLIGHT_CURRENT_LINE)

Command.h -- använda?

Undo buffer
    För att få till redo skulle man kunna ha en cirkulär buffer, nu stack för undo