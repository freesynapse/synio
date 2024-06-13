
I nuläget så att mycket av funktionaliteten för fönster ligger i själva fönster-klassen,
inte optimalt kanske, men kan fö.n. inte komma på varför, så får fortsätta så för nu..

Implementera C-x-typ kommandon som i emacs. Skriptspråk?
    - nej, snarare att ha kommandon som kan visas i en separat hjälp-fil

--  Tabs: om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)?

--  Flera bufferfönster som man kan switcha mellan, stänga med C-x C-w

--  FileBufferWindow: om man har fått autocomplete på ex '[' (= '[]') och står på closing
    bracket och trycker ']' så ska det inte infogas någon ny, utan bara moveCursor(1, 0).

--> Prefix-tree.
    Ändra i command_window att m_utilMLBuffer alltid visas men uppdateras för varje 
    input vilka ändelser som är aktuella. Nu måste man trycka på <tab> för att få 
    frame autocompletions.

--> I FileBufferWindow måste och copybuffern bli global, så att man kan kopiera mellan
    fönster. Static klass CopyBuffer?
    CopyBuffer ska läsa och skriva till X11s generella buffer.

--> FileExplorerWindow, under CommandWindow, som listar alla filer i aktuellt katalog.
    Ska ha en rad för att skriva också, och att man kan bläddra i listan. State för detta,
    bläddra eller skriva (hoppa med tab, ctrl+tab). Inputfältet uppdateras automatiskt 
    när man bläddrar. Om content i inputfältet när <tab>, så ska innehållet selekteras, 
    så att det är lätt att ändra. Tror att LineBufferWindow inheritance borde vara bäst
    (typ som PoC för ListboxWindow).    
    FileExplorerWindow, i sin tur, äger en YesNoDialogWindow, för ex att skriva över 
    befintlig fil osv. PoC ListboxWindow har logik för att hantera de valen.

--  ListBox window, med eller utan prompt för att skriva och söka i ett prefix-tree.
    Som default tas som argument en std::vector<std::string> som utgör alternativen.
    I ::redraw() vore det fräsigt att kunna markera hela raden, kanske nytt försök med 
    detta i ncurses_colors.h|.cpp.
