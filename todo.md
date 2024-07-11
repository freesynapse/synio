
# LISTBOX_WINDOW #
--  ListBox window, med eller utan prompt för att skriva och söka i ett prefix-tree.
    Som default tas som argument en std::vector<std::string> som utgör alternativen.
    I ::redraw() vore det fräsigt att kunna markera hela raden, kanske nytt försök med 
    detta i ncurses_colors.h|.cpp. <== THIS!!


# POPUPS #
--  Idé: Alla popups skulle kunna ha en callback till en funktion i konstruktorn, som 
    anropas när exemeplvis en fil väljs, man trycker Yes eller att man stänger 
    fönstret (eller trycker No), som ansvarar för att delete pekaren och fixa cleanup etc.
    Vet dock inte riktigt hur funktionspekare funkar, får kolla på det i misc/synio_dev.

    --> PoC i /misc/synio_dev/dunction_callbacks/. Lite trixigt med std::placeholders::
        men annars borde funka.


# MISC #
--  Fixa så att __debug_str inte är __debug utan en permanent del av line_t, och som 
    auto-allokeras på samma sätt som CHTYPE.

--  Yes/No dialog: global (eller static class) som tar en callback func, som anropas om
    Yes.

--  Implementera C-x-typ kommandon som i emacs. Skriptspråk?
    - nej, snarare att ha kommandon som kan visas i en separat hjälp-fil

--  I nuläget så att mycket av funktionaliteten för fönster ligger i själva fönster-
    klassen, inte optimalt kanske, men kan fö.n. inte komma på varför, så får fortsätta 
    så för nu..

--  Flera bufferfönster som man kan switcha mellan, stänga med C-x C-w

--  Tabs: om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)?

--  Merge FileExplorerWindow och ListboxWindow; funktionaliteten väldigt lika för input 
    osv.

# FILE_BUFFER_WINDOW #
--  I FileBufferWindow måste och copybuffern bli global, så att man kan kopiera mellan
    fönster. Static klass CopyBuffer?
    CopyBuffer ska läsa och skriva till X11s generella buffer.

--  FileBufferWindow: om man har fått autocomplete på ex '[' (= '[]') och står på closing
    bracket och trycker ']' så ska det inte infogas någon ny, utan bara moveCursor(1, 0).


# COMMAND_WINDOW #
--  Prefix-tree.
    Ändra i command_window att m_utilMLBuffer alltid visas men uppdateras för varje 
    input vilka ändelser som är aktuella. Nu måste man trycka på <tab> för att få 
    frame autocompletions (funktionalitet finns i FileExplorerWindow::autocompleteInput
    och ::showCompletions).
--  (samma som ovan). Fixa showCompletions och tabComplete i CommandWindow som i 
    FileExplorerWindow.
