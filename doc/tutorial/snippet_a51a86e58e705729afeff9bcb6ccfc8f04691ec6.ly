% Automatically generated by BMC, the braille music compiler
\version "2.14.2"
\include "articulate.ly"
\header {
  tagline = ""
}
music =
  <<
    \new Staff {
      \time 1/4
      c,,4 | % 1
      c,4 | % 2
      c4 | % 3
      c'4 | % 4
      c''4 | % 5
      c'''4 | % 6
      c''''4 | % 7
    }
  >>

\score {
  \music
  \layout { }
}
\score {
  \unfoldRepeats \articulate \music
  \midi { }
}