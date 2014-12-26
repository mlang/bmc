======================
Braille Music Tutorial
======================

A basic introduction to braille music code with examples in braille,
visual notation and as sound files.  You can touch, view, and hear them.

.. contents::
   :local:

Basic signs
===========

Before we can dive into braille music notation we will have to learn a few
basic signs and rules.

Notes
-----

The signs used for notes follow a certain system.  The note name
is encoded in the upper 4 braille dots.  Braille letter d through j are
used to indicate the note name.  Dots 3 and 6 can be added
to indicate the note value.

.. _eighth note: http://en.wikipedia.org/wiki/Eighth_note
.. _hundred twenty-eighth note: http://en.wikipedia.org/wiki/Hundred_twenty-eighth_note

If none of dot 3 or 6 are present, a `eighth note`_ (quaver) or
`hundred twenty-eighth note`_ (semihemidemisemiquaver) is given.

+------+-------+---------+-----------------+
| Sign | Dots  |Note name| Possible values |
+======+=======+=========+=================+
| ⠙    |1 4 5  | C       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠑    |1 5    | D       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠋    |1 2 4  | E       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠛    |1 2 4 5| F       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠓    |1 2 5  | G       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠊    |2 4    | A       | 1/8 or 1/128    |
+------+-------+---------+-----------------+
| ⠚    |2 4 5  | B       | 1/8 or 1/128    |
+------+-------+---------+-----------------+

.. _half note: http://en.wikipedia.org/wiki/Half_note) (minim
.. _thirty-second note: http://en.wikipedia.org/wiki/Thirty-second_note

If dot 3 is added, we are dealing with either a `half note`_ (minim) or
`thirty-second note`_ (demisemiquaver).

+------+---------+---------+-----------------+
| Sign |  Dots   |Note name| Possible values |
+======+=========+=========+=================+
| ⠝    |1 3 4 5  | C       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠕    |1 3 5    | D       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠏    |1 2 3 4  | E       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠟    |1 2 3 4 5| F       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠗    |1 2 3 5  | G       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠎    |2 3 4    | A       | 1/2 or 1/32     |
+------+---------+---------+-----------------+
| ⠞    |2 3 4 5  | B       | 1/2 or 1/32     |
+------+---------+---------+-----------------+

.. _quarter note: http://en.wikipedia.org/wiki/Quarter_note
.. _sixty-fourth note: http://en.wikipedia.org/wiki/Sixty-fourth_note

If dot 6 is added, we are dealing with a `quarter note`_ (crotchet) or
`sixty-fourth note`_ (hemidemisemiquaver).

+------+---------+---------+-----------------+
| Sign |  Dots   |Note name| Possible values |
+======+=========+=========+=================+
| ⠹    |1 4 5 6  | C       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠱    |1 5 6    | D       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠫    |1 2 4 6  | E       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠻    |1 2 4 5 6| F       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠳    |1 2 5 6  | G       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠪    |2 4 6    | A       | 1/4 or 1/64     |
+------+---------+---------+-----------------+
| ⠺    |2 4 5 6  | B       | 1/4 or 1/64     |
+------+---------+---------+-----------------+

.. _whole note: http://en.wikipedia.org/wiki/Whole_note
.. _sixteenth note: http://en.wikipedia.org/wiki/Sixteenth_note

Finally, if both dot 3 and dot 6 are added, we have a `whole note` (semibreve) or
`sixteenth note`_ (semiquaver).

+------+-----------+---------+-----------------+
| Sign |   Dots    |Note name| Possible values |
+======+===========+=========+=================+
| ⠽    |1 3 4 5 6  | C       | 1 or 1/16 C     |
+------+-----------+---------+-----------------+
| ⠵    |1 3 5 6    | D       | 1 or 1/16 D     |
+------+-----------+---------+-----------------+
| ⠯    |1 2 3 4 6  | E       | 1 or 1/16 E     |
+------+-----------+---------+-----------------+
| ⠿    |1 2 3 4 5 6| F       | 1 or 1/16 F     |
+------+-----------+---------+-----------------+
| ⠷    |1 2 3 5 6  | G       | 1 or 1/16 G     |
+------+-----------+---------+-----------------+
| ⠮    |2 3 4 6    | A       | 1 or 1/16 A     |
+------+-----------+---------+-----------------+
| ⠾    |2 3 4 5 6  | B       | 1 or 1/16 B     |
+------+-----------+---------+-----------------+

For people coming from a cultural background where notes are named like
C, D, E, F, G, A and B (english speaking countries) or C, D, E, F, G, A, H
(german speaking countries) you might have a hard time at first to read note
names.  However, there is a trick for memorizing.  Observe that the braille
letters used to indicate note names are shifted up by one letter.  So the note
C is written with the braille letter d, the note D is written with the braille
letter e and so on.
The two exceptions here are the note names A and B (or H).  A is written
with the braille letter i, and B (or H) is written with the braille letter j.

Rests
-----

There are four signs used to show rest values.  As with note values,
each sign can have two meanings.

+------+-------+---------------+
| Sign | Dots  | Rest value    |
+======+=======+===============+
| ⠍    |1 3 4  |whole or 16th  |
+------+-------+---------------+
| ⠥    |1 3 6  |half or 32th   |
+------+-------+---------------+
| ⠧    |1 2 3 6|quarter or 64th|
+------+-------+---------------+
| ⠭    |1 3 4 6|eighth or 128th|
+------+-------+---------------+

Meter and barlines
------------------

Measures (bars) divide a piece into groups of beats, and the time signatures
specify those groupings.

In braille music code, measures of music are separated with a space or newline.

Each part of a piece of music needs to end with the final barline sign (⠣⠅).

The time signature is often expressed as a fraction.  In braille we write
a number sign (⠼) followed by digits in the upper and lower position.
If no time signature is specified, 4/4 is assumed.

The following example specifies a time signature of 6/8 and puts 6 eighth
notes on the third line of the five line stave.

.. braille-music::

   ⠼⠋⠦⠀⠐⠚⠚⠚⠚⠚⠚⠣⠅

The time signature is essential for value ambiguity resolution.  Since all note
and rest values in braille music code do have two potential meanings, the
time signature which specifies the duration of a measure is required to
determine the exact values of all notes and rests inside a measure.

Octave signs
------------

Contrary to visual music notation where clefs determine the pitch of a note,
braille music uses octave signs to indicate the exact pitch of a note.

An octave sign is placed directly in front of a note sign, without any other
signs in between.
The following example shows octave signs 1 through 7 placed in front of a
quarter C.

.. braille-music::

   ⠼⠁⠲⠀⠈⠹⠀⠘⠹⠀⠸⠹⠀⠐⠹⠀⠨⠹⠀⠰⠹⠀⠠⠹⠣⠅

There has to be an octave sign in front of the first note of a piece.
For all following notes these rules apply:

If the following note goes a second or third up or down it does not need an octave sign.

.. braille-music::

   ⠼⠋⠦ ⠐⠙⠋⠓⠪⠙⠀⠑⠛⠊⠚⠑⠛⠀⠋⠓⠚⠙⠊⠛⠀⠑⠚⠓⠛⠑⠚⠀⠐⠳⠛⠱⠋⠀⠝⠄⠣⠅

If it goes a fourth or fifth up or down it receives an octave sign only if it is
in a different octave than the previous note.

Compare the following two examples.

.. braille-music::

   ⠼⠉⠲ ⠐⠓⠊⠚⠓⠨⠙⠐⠓⠀⠨⠑⠐⠓⠨⠏⠣⠅

----

.. braille-music::

   ⠼⠉⠲ ⠐⠙⠑⠋⠙⠛⠙⠀⠓⠙⠐⠎⠣⠅

With an interval of a sixth or greater to the previous note, it always receives an octave sign.

.. braille-music::

   ⠼⠋⠦ ⠐⠙⠋⠓⠊⠐⠙⠐⠊⠀⠐⠙⠣⠐⠚⠐⠙⠐⠊⠐⠙⠐⠊⠀⠗⠄⠣⠅

These rules are demonstrated in the following example.

.. braille-music::

   ⠨⠏⠱⠹⠀⠪⠨⠕⠹⠀⠺⠨⠏⠱⠀⠝⠻⠫⠀⠱⠗⠻⠀⠫⠹⠨⠪⠨⠹⠀⠚⠙⠑⠋⠛⠓⠊⠚⠀⠝⠥⠣⠅

Value distinction
-----------------

Value ambiguity is usually not a problem, because knowledge of the current
time signature will make it easy in most cases to determine to which of the two
values a note (or rest) belongs.
Where confusion is likely to arise (e.g. in the case of the half followed
immediately by a 32th), one of the value signs must be placed between them.

.. braille-music::

   ⠼⠉⠲⠀⠨⠝⠠⠣⠂⠝⠕⠏⠟⠓⠣⠅

When a piece begins with an incomplete measure (pickup or
[anacrusis](http://en.wikipedia.org/wiki/Anacrusis)) and there is any
doubt as to the value of the first note or rest, the sign ⠠⠣⠂ is used if the
note or rest belongs to the smaller of the two values.

.. braille-music::

   ⠠⠣⠂⠸⠷⠀⠐⠙⠭⠚⠭⠙⠭⠑⠭⠀⠋⠭⠛⠭⠕⠣⠅

The clef
========

Since braille music code uses octave signs instead of clefs to indicate the
exact pitch of a note, clef signs are optional.

When a blind user communicates with a visual music notation reader about a piece
which is available in braille and visual notation, it can be important to
understand the implications of clefs.  To make it clear why placement of clefs
can be important, we need to take a small detour and explain how visual
music notation actually works.

The five-line staff
-------------------

The staff (or stave, in British English) is the fundamental latticework of
modern visual music notation, upon which symbols are placed.  The five stave
lines and four intervening spaces correspond to pitches of the diatonic
scale - which pitch is meant by a given line or space is defined by the clef.

Ledger lines
~~~~~~~~~~~~

Used to extend the stave to pitches that fall above or below it. Such ledger
lines are placed behind the note heads, and extend a small distance to each
side.  Multiple ledger lines can be used when necessary to notate pitches even
farther above or below the staff.

Clef sign
---------

A clef (French: clef; “key”) is a musical symbol used to indicate the pitch of
written notes.  Placed on one of the lines at the beginning of the stave, it
indicates the name and pitch of the notes on that line.  This line serves as a
reference point by which the names of the notes on any other line or space of
the stave may be determined.

There are three types of clef used in modern music notation: F, C, and G.  Each
type of clef assigns a different reference note to the line on which it is
placed.

The treble or G clef identifies the second line up on the five line
stave as the note G above middle C.

The bass or F clef identifies the fourth line up on the five line stave
as the note F below middle C. 

Finally, the C clef identifies the third line up on the five line stave
as middle C.

.. |GClef| image:: http://upload.wikimedia.org/wikipedia/commons/f/ff/GClef.svg
           :alt: G-Clef

.. |CClef| image:: http://upload.wikimedia.org/wikipedia/commons/b/b0/CClef.svg
           :alt: C-Clef

.. |FClef| image:: http://upload.wikimedia.org/wikipedia/commons/c/c5/FClef.svg
           :alt: F-Clef

+------+---------+--------+----+----+
| Sign | Symbol  | Name   |Note|Line|
+====+=+=========+========+====+====+
| ⠜⠌⠇  | |Gclef| | treble | G4 | 2  |
+------+---------+--------+----+----+
| ⠜⠬⠇  | |Cclef| | alto   | C4 | 3  |
+------+---------+--------+----+----+
| ⠜⠼⠇  | |Fclef| | bass   | F3 | 4  |
+----+-----------+--------+----+----+

Observe that the second cell of a clef sign is an interval sign which matches
the line a particular clef symbol is usually place on.

If a clef symbol is placed on an unusual stave line this can be indicated
with an additional octave sign directly after the second cell in a clef sign.
In this case, the octave sign is used to indicate a stave line.

+------+---------+------------+---------+
| Sign | Symbol  | Name       |Note|Line|
+======+=========+============+====+====+
| ⠜⠌⠈⠇ | |Gclef| |french      | G4 | 1  |
+------+---------+------------+---------+
| ⠜⠬⠈⠇ | |Cclef| |soprano     | C4 | 1  |
+------+---------+------------+---------+
| ⠜⠬⠘⠇ | |Cclef| |mezzosoprano| C4 | 2  |
+------+---------+------------+---------+
| ⠜⠬⠐⠇ | |Cclef| |tenor       | C4 | 4  |
+------+---------+------------+---------+
| ⠜⠬⠨⠇ | |Cclef| |baritone    | C4 | 5  |
+------+---------+------------+---------+
| ⠜⠼⠸⠇ | |Fclef| |baritone    | F3 | 3  |
+------+---------+------------+---------+
| ⠜⠼⠨⠇ | |Fclef| |subbass     | F3 | 5  |
+------+---------+------------+---------+

Notes representing a pitch outside of the scope of
the five line stave can be represented using ledger lines, which
provide a single note with additional lines and spaces.

The use of three different clefs makes it possible to write music for all
instruments and voices, even though some sound much higher or lower than others.
This would be difficult to do with only one clef, since the modern stave has
only five lines, and the number of pitches that can be represented on the stave,
even with ledger lines, is not nearly equal to the number of notes the orchestra
can produce.

The use of different clefs for different instruments and voices allows each part
to be written comfortably on the stave with a minimum of ledger lines.  To this
end, the G-clef is used for high parts, the C-clef for middle parts, and the
F-clef for low parts.

Consider the following example which does not use any clef signs in braille.
As a result, a treble (G) clef is used in both measures in visual music
notation.

.. braille-music::

   ⠣⠼⠋⠦⠀⠐⠛⠛⠊⠙⠄⠮⠙⠀⠘⠛⠓⠊⠺⠄⠣⠅

Since the treble clef is default, the first measure fits nicely on the five-line
staff.  However, the second measure requires up to 6 ledger lines below the
stave to denote the low pitches in visual music notation.

If we add a bass (F) clef sign in braille at the beginning of the second measure
the visual transcription now fits on the stave without ledger lines.

.. braille-music::

   ⠣⠼⠋⠦⠀⠐⠛⠛⠊⠙⠄⠮⠙⠀⠜⠼⠇⠘⠛⠓⠊⠺⠄⠣⠅

Differences between braille and visual notation
-----------------------------------------------

Braille music code has no equivalent for stave lines and ledger lines, nor
does it has a need for clefs.  This is because the diatonic pitch of a note
is encoded into the note sign and octave signs are used to indicate the actual
pitch of notes.  Clef signs are usually omitted in braille music code since
they are not relevant to braille music readers most of the time.

However, if a braille music reader needs to get a clear understanding on how
music is visually presented, they need to know which clef is currently in
effect.  For instance, a very low pitch played while a treble (G) clef is active
might be unsuspicious to a braille music reader, while it will generate many
ledger lines below the stave in visual music notation.

It is common practice to use a treble clef in the right hand stave of a keyboard
part and a bass clef in the left hand stave.  If a passage of music differs
significantly in pitch from what is common for a particular clef, braille music
code writers should consider adding clef signs as appropriate to ensure that
automatic conversion to visual notation can produce a pleasant result.

Accidentals
===========

Just like lines and spaces in five-line stave notation, the pitches of
braille music notes correspond to the
[diatonic scale](http://en.wikipedia.org/wiki/Diatonic_scale).  They can
essentially be viewed as the white keys of a piano keyboard.  To reach pitches
that are a half step away from the diatonic pitches, accidentals are used.

+------+-------------+
| Sign | Description |
+======+=============+
| ⠡    |natural      |
+------+-------------+
| ⠣    |flat (b)     |
+------+-------------+
| ⠩    |sharp (#)    |
+------+-------------+
| ⠣⠣   |double flat  |
+------+-------------+
| ⠩⠩   |double sharp |
+------+-------------+

Key signature
-------------

Following the clef, the key signature on a stave indicates the key of the piece
by specifying that certain notes are flat or sharp throughout the piece, unless
otherwise indicated.

Rhythmic note groups
====================

Beaming
-------

A [beam](http://en.wikipedia.org/wiki/Beam_%28music%29) in visual music notation
is a thick line frequently used to connect multiple consecutive eighth notes
(quavers), or notes of shorter value (indicated by two or more beams), and
occasionally rests.  Beamed notes or rests are groups of notes and rests
connected by a beam; the use of beams is called beaming.

This kind of grouping is also used, with certain restrictions, in braille music
code.  Three or more 16th notes (but also smaller values) can be grouped in
braille music.  One group of notes should not be part of two different beats.
The first note is written with the real value of the group, while the remaining
notes are written as 8th notes.

.. braille-music::

   ⠼⠙⠲⠀⠐⠽⠑⠋⠛⠷⠊⠚⠙⠵⠙⠚⠊⠷⠛⠋⠑⠣⠅

Note grouping is also legal if it is started with a rest of the same value as
the other notes in the group.
However, rests are not allowed to appear anywhere else in a note group in
braille music code.

.. braille-music::

   ⠼⠉⠲⠀⠍⠐⠋⠛⠓⠍⠛⠓⠊⠍⠓⠊⠚⠣⠅

Tuplets
-------

A [tuplet](http://en.wikipedia.org/wiki/Tuplet) is a grouping of notes
with irregular time.

The simplest of tuplets, the triplet, has two possible signs in braille.
THe shorter version is used in combination with the 3-character sign if
triplets are nested.

.. braille-music::

   ⠨⠹⠆⠋⠛⠓⠆⠛⠸⠒⠄⠯⠿⠯⠑⠹⠣⠅

Polyphony
=========

Intervals
---------

If two or more notes sound at the same time and have the same value (duration)
intervals are used.

In upper registers, or in the right hand of a keyboard piece,
only the highest note is written as a normal note sign, all following notes
of the chord are written with interval signs downward.

.. braille-music:: intervals-rh.bmc

In lower registers or in the left hand of a keyboard piece, the lowest
note is wirtten as a normal note and all others are expressed with interval
signs, ordered upward.

.. braille-music:: intervals-lh.bmc

If there are several interval signs following a note sign there is no need to
place octave signs in between except the intervals are separated by an octave or more.

.. braille-music::

   ⠼⠉⠲⠀⠨⠳⠤⠼⠴⠳⠔⠬⠒⠻⠬⠒⠔⠀⠏⠄⠬⠴⠬⠣⠅

The pitches of the written notes determine if a chord needs to be prefixed with
an octave sign or not.

.. braille-music:: chords-lh.bmc

in-accord
---------

If not all simultaneously played notes of a part of a measure do have the same
rhythm they are written as separate voices with same duration.
If the voices span a coplete measure, the full measure in-accord sign is used.

An octave sign needs to be placed in front of the first note of a part
separated by in-accord signs, and the following measure needs to have an
octave sign on the first note no matter if it is in-accord or not.

.. braille-music::

   ⠨⠽⠣⠜⠐⠫⠱⠫⠻⠀⠐⠾⠣⠜⠐⠳⠻⠫⠱⠣⠅

If only one part of a measure requires an in-accord sign, a partial measure sign
is used in combination with a partial measure in-accord sign.

In this example the first measure splits into two voices at the third beat.

.. braille-music::

   ⠐⠳⠓⠨⠙⠨⠅⠨⠟⠐⠂⠨⠹⠚⠊⠀⠨⠏⠄⠴⠧⠣⠅

A measure can contain full measure in-accord and partial measure in-accord signs
at the same time.

.. braille-music::

  ⠐⠓⠋⠨⠅⠐⠊⠄⠷⠛⠯⠿⠷⠛⠋⠑⠐⠂⠐⠙⠊⠐⠑⠄⠽⠚⠮⠾⠣⠜⠸⠷⠣⠅

Slurs and ties
==============

Ties and slurs are curved lines connecting notes in visual music notation.
While they are visually quite similar they represent difference concepts.

Tie
---

A tie is a curved line connecting the heads of two notes of the same pitch and
name, indicating that they are to be played as a single note with a duration
equal to the sum of the individual notes' note values.

If notes of a chord are tied the tie sign is placed directly after the
corresponding note or interval sign.

.. braille-music::

   ⠜⠌⠇⠐⠺⠬⠈⠉⠙⠼⠑⠈⠉⠴⠱⠒⠈⠉⠙⠴⠭⠣⠅

Slur
----

A slur in visual notation connects several notes to indicate that they should
be played as close together as possible.

The single slur sign (⠉) is used if at most four notes are slurred together.
It is placed after every note except the last.

.. braille-music::

   ⠼⠉⠲ ⠭⠐⠓⠉⠋⠭⠭⠨⠙⠉⠀⠣⠚⠉⠊⠭⠨⠛⠉⠑⠉⠡⠚⠉⠀⠹⠥⠣⠅

If more then four notes are slurred together, doubling can be used.
The first note of a phrase receives a doubled slur sign, and the note before
the last note in the phrase receives a single slur sign.

.. braille-music::

   ⠼⠃⠲⠀⠐⠳⠉⠉⠊⠚⠀⠹⠑⠋⠀⠻⠋⠑⠀⠫⠉⠙⠭⠣⠅

