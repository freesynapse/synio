
I nuläget så att mycket av funktionaliteten för fönster ligger i själva fönster-klassen,
inte optimalt kanske, men kan fö.n. inte komma på varför, så får fortsätta så för nu..

Implementera C-x-typ kommandon som i emacs. Skriptspråk?
    - nej, snarare att ha kommandon som kan visas i en separat hjälp-fil

--  Tabs: om stöts på ändra till spaces (tar bort TABS_OR_SPACES?)?

--> CLI: om tomt ny fil, om fil som inte finns skapa den, men om inte sparas försvinner.

--  Flera bufferfönster som man kan switcha mellan, stänga med C-x C-w

--  Dialogrutor? Yes/No eller filnamn t.ex.

--  FileBufferWindow: om man har fått autocomplete på ex '[' (= '[]') och står på closing
    bracket och trycker ']' så ska det inte infogas någon ny, utan bara moveCursor(1, 0).