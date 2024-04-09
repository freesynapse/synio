Lexer

    Ärvda klasser av Lexer   för varje filtyp som går (nu bara C/C++). Basklassen () ;;; innehåller
    funktionspekare till parsing och highlighting, som i basfallet inte gör något.

Tabs -- om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)

Input
    I default-fallet: ha ny switch där bara tillåtna tecken finns med? Är det så man ska göra?

Command.h -- använda?

Undo buffer
    För att få till redo skulle man kunna ha en cirkulär buffer, men vi börjar med en stack 
    för undo.