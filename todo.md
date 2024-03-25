Lexer

    Ärvda klasser av Lexer   för varje filtyp som går (nu bara C/C++). Basklassen () ;;; innehåller
    funktionspekare till parsing och highlighting, som i basfallet inte gör något.

Tabs -- om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)
CTRL+BACKSPACE och CTRL+DEL

Input
    I default-fallet: ha ny switch där bara tillåtna tecken finns med? Är det så man ska göra?

//          2.  Also fix for backspace, so that when pressed and all whitespace,
//              a whole TABs worth of spaces are removed per keypress.
