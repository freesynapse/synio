--- What we have and what we need ---

1.  I Lexer: istället för att gå vidare till nästa rad med m_line = m_line->next borde 
    finnas flaggor för m_inMComment och m_inMString. Varje rad ska läsas 'separat', och om 
    m_inMComment == true ska bara letas efter token "*/" (och motsvarande för mstring).


2.  Vid öppning av ny .c/.cpp/.h/.hpp-fil görs en parse av hela filen, med syntax HL.

3.  Nya SYN_COLOR_-entries för MCOMMENT och MSTRING, på så sätt finns denna information
    lagrad i line_t->content[line_t->len - 1] gratis.
    Inline-funktion i synio_colors för att få reda på vilken SYN_COLOR_ som är i sista
    char i en line_t.

3.  Vid editering av rad kollas om föregående rad var del av en mcomment eller mstring
    (se ovan), och i så fall vid parsing av innevarande rad kan m_inMComment sättas till 
    true vid parsing av den raden. 
    --  Här behövs en ny funktion i Lexer för att parsa en rad. 
        Eller: parse_lines(line_t *_start_line, 
                           line_t *_end_line=NULL, 
                           bool _in_mcomment=false, ...), som för hela filen 
        blir parse_lines(m_lineBuffer.m_head, m_lineBuffer.m_tail), ex.
        Om man gör så kan man ha en enda rad som parse_lines(m_currentLine, m_currentLine).
    (forts.) m_inMComment är false kan man bara parsa raden som valigt. Om man då skulle 
    stöta på en "/*" i den editerade raden måste man parsa därifrån, tills denna stängs
    med "*/".

    