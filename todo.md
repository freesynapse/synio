Lexer

    Ärvda klasser av Lexer   för varje filtyp som går (nu bara C/C++). Basklassen () ;;; innehåller
    funktionspekare till parsing och highlighting, som i basfallet inte gör något.

Tabs -- om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)

Input
    I default-fallet: ha ny switch där bara tillåtna tecken finns med? Är det så man ska göra?

Command.h -- använda?


Undo buffer:
Varje åtgärd lagras, med en UndoItemType (enum class); LINES eller STRING. Structen 
undo_item_t består av en vector av copy_line_t (precis som m_copyBuffer) och en start-
och end-position (för att hålla reda på var det var). Eftersom bara det senaste kommandot
kan ångras kommer cursorns position innan det kommandot vara valid för det state som 
man kan ångra i, och man borde inte behöva hålla bättre koll än så. Utöver 

Möjliga actions:

    --  Undo insertion string (single char eller char *)
    --  Undo deletion string (single char eller char *)
    --  Undo insertion lines (som cut(), ta bort lines)
    --  Undo deletion lines (som paste(), infoga lines) -- detta bygger på att alla lines
        redan måste vara selected, finns inget annat sätt att ta bort mer än en rad.
    
    Special cases
    --  Undo Insert/remove leading tab, då går det inte att använda kod för paste().
        Kanske skulle det vara lättast att modifiera koden för removeLeadingTab så 
        att istället för att kolla alla line_t* i en Selection (m_selection) skulle
        man skriva en generisk funktion som tar två radnummer och sen om det är 
        selected utökar selektionen.
        --> Lägger till TABS i UndoItemType för att ha detta som en egen entitet.

Börjar med add/del lines, eftersom den koden är klar (i cut() och paste()). Först LINES,
LINES_ADD (som i paste). Allt som borde behövas för att ångra (alltså ta bort igen) är
start_pos och end_pos.


För att få till redo skulle man kunna ha en cirkulär buffer, men vi börjar med en stack 
för undo.