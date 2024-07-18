
# POPUPS #
--  Idé: Alla popups skulle kunna ha en callback till en funktion i konstruktorn, som 
    anropas när exemeplvis en fil väljs, man trycker Yes eller att man stänger 
    fönstret (eller trycker No), som ansvarar för att delete pekaren och fixa cleanup etc.
    Vet dock inte riktigt hur funktionspekare funkar, får kolla på det i misc/synio_dev.
    -- funkar i Listbox och FileExplorer.


# MISC #
--  Synio.h|.cpp : byt från en std::vector<file_buffer_entry_t> till std::list, för att 
    kunna flytta nya/switchade filer till 'toppen'. <-- THIS!

--  WindowManager-klass, nu ligger detta under Synio.

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

--  Tabs-rad högst upp (som i vscode) så man kan se vilka buffers som är öppna?


# UNDO BUFFER #
--  För multicursor support, lägg till en flag eller liknande som markerar att det är 
    en multi-cursor edit. Man kan peek:a på nästa i stacken och se om det är detta och 
    exekvera tills det inte är multicursor längre, typ.

--  När man ångrar enstaka chars, så skulle man kunna ha att efter de två första undos att
    alla kommande enstaka chars (genom peek i stacken) ångras (alltså hela ord),
    eventuellt till nästa space/enter etc.


# COMMAND_WINDOW #
--  CommandID stack, så att man kan stapla kommandon på varandra (ex save-temp-buffer
    efter close-buffer) och att man återgår till föregående när det ena är klart. Vid
    command_complete_() skulle man kunna kolla om stacken är tom, ex.
    
--  Prefix-tree.
    Ändra i command_window att m_utilMLBuffer alltid visas men uppdateras för varje 
    input vilka ändelser som är aktuella. Nu måste man trycka på <tab> för att få 
    frame autocompletions (funktionalitet finns i FileExplorerWindow::autocompleteInput
    och ::showCompletions).

--  (samma som ovan). Fixa showCompletions och tabComplete i CommandWindow som i 
    FileExplorerWindow.
