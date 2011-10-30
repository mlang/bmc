BMC -- Braille Music Compiler
-----------------------------

BMC aims to become a system for parsing and generating braille music code.


History
-------

A predecessor to this project is FreeDots[1], a Java-based program for
converting MusicXML Documents to Braille Music code, as well as offering
playback and some interactive editing capabilities.

 [1] http://code.google.com/p/freedots/

FreeDots has been very helpful in providing a general understanding of the
various aspects of braille music code.  However, it was only designed to cover
one way of conversion, namely from some music notation source (like MusicXML) to
braille music code.  A full system for working with braille music code should
eventually cover both directions.


Dependencies
------------

Being a C++ program, BMC naturally uses the Standard Template Library (STL).

In addition to that some Boost[2] C++ Libraries are employed.
Currently however, to make the Boost dependencies as
easy to handle as possible, we are only using Boost header-only libraries[3].
This means that the boost include directory is enough to build BMC.
There is no need to link any Boost Library files to the final executable.

 [2] http://www.boost.org/
 [3] http://www.boost.org/doc/libs/1_47_0/?view=filtered_header-only


Parsing
-------

So BMC was started to begin completely a new, this time, with the more complex
task of parsing braille music code.  Braille music code was invented originally
around 1880, and was later refined by several comittees.  Its aim was naturally
to be unambiguous and allow verbatim transcriptions from visual music notation,
but it was never particularily designed to be read by a computer.  This aspect
of the history shows up in the more complex tricks that
we will want to handle to provide a parser that is as intelligent as possible.

We are parsing braille music code to eventually convert it to some other
representation which we will want to use in our final program.
For instance, a user of BMC might want to listen to a correctly parsed
Braille Music Document.  So conversion to MIDI (partly implemented already)
is one of the obvious things we will want to do.  On the other hand,
we also want to be able to convert the given braille music to visual music
notation, by going via some format supported by music notation software.
Interesting candidates seem to be MusicXML and LilyPonds.

So considering this, we are actually trying to build a cross-compiler
for braille music code to possibly several other music notation formats.
Therefore one important aspect of our program is going to be the so-called
intermediate representation (IR in this document).  It can be thought
of as music in the most abstract sense possible, not attached to
a particular type of notation, be it tactile or visual.  However, it
needs to preserve enough cues from its input to allow for a smooth
conversion to other formats.  As we will see below, strictly parsing
a string of braille music code into a data structure representing braille music
symbols is not enough.  Fundamental concepts like the ambiguity of note values,
or more complex things like unrolling of simile signs and calculation of
actual octaves of notes and chords will require (possibly several) steps of
processing of the parsed data to progressively calculate all the information
required to fully represent the actually intended musical content.

How all of this should be done is still rather open, because the necessary
algorithms have partly not been explored yet.  We do the disambiguation of
note values already, which allows for a rather approximated playback
codepath which is very useful for debugging and first steps of
experimentation with the codebase.  We also calculate actual octaves
already, which makes the playback alot more useable.  But, for instance,
we do not have any kind of accidental handling yet, neither
key signatures nor accidental placement is honored.


The Intermediate Representation
-------------------------------

Some particular rules of braille music code deserve special attention
when designing a data structure to correctly represent fully parsed and
processed braille music.  In particular,

 * Note groupings are commonly used in well-written braille music code to
   increase readablility.  While they might be computable in some cases,
   the actual exact placement of note groupings can be important.  Additionally,
   note groupings could be seen as a kind of beaming, which is otherwise absent
   in braille music altogether.  Therefore the exact use of note groupings,
   when detected by the disambiguation routines, should be recorded and
   preserved in the intermediate representation. so that it can be
   regenerated in the case of printing to braille, or used to provide
   beaming for MusicXML documents for instance.

 * Braille repetititon: In addition to the usual way of indicating repeats with
   special barlines in visual music notation, braille music code provides
   several means to indicate repetition of note material either across several
   measures in a section, or particular voices of a measure or even parts of a
   voice of a measure.  These methods are used to minimize the amount of
   characters required to fully represent a complete piece of music, and also to
   make certain structure in music clear immediately, which helps a lot during
   the process of memorisation of music.
   However, for the purpose of our project this means that the parsed
   input string does not necessarily have an unique character for every note
   in the presented score.  Certain characters and/or syntactic constructs
   will effectively constitute placeholders for note material presented already
   eralier in the braille music document.
   Our intermediate representation will therefore have to deal with
   this situation, how exactly is not clear.  For instance, the processing
   step after parsing the braille music code first will have to do
   unrolling of the actually parsed material in any case.  For conversion
   to performance data like MIDI, unrolling will be necessary as well.
   So one might think that the intermediate representation should be a totally
   unrolled view of all the note material, removing all the special cases
   introduced by braille music code.  However, it would be desireable to
   preserve the compressions employed in the original input.


The identity transformation
---------------------------

One interesting special case of conversion is the transformation
from braille music code to braille music code.  While this might seem
academic at first sight, it is not.  Braille music code employs very complicated
hyphenation rules, the rules governing the splitting of very long measures
across two (or several) lines of braille.  So to be able to
reformat braille music for a different page width requires
it to be parsed completely first, and then printed again just
with a different line width.  This feature would be very desireable
for braille display users, since a lot of readily-available braille
music code documents currently are formatted for 32 (or less) characters
per line, while a typical braille display has 40 or even 60 or 80 characters.


Generating
----------

A logical step given the description above is to convert from a sensible
Music notation format *to* braille msuci code, something that our predecessor
project FreeDots already does, it converts MusicXML to braille music code.  For
now, integrating this into BMC is delayed until the parsing part works nicely,
but it is something planned as the final sugar-topping, so it should be kept
in mind.


User Interface(s)
-----------------

By the nature of BMC (and actually also its name), the basic interface should be
a command-line tool, behaving roughly like a normal compiler (for instance,
error reporting format) to support easy integration into existing editing
environments (like GNU Emacs).

However, to support the greatest possible user base (which is mostly blind
people and people working with them) BMC should also have a graphical user
interface (GUI) basically resembling a simple editor.  In that GUI, the user
will open files containing braille music code or create new documents entering
music manually.  Upon a sepcial keypress or invokation of a menu item the
program will try to parse the current document and either report errors sensibly
or provide some sort of confirmation.  Once the document is successfully parsed,
several more options will be available, like starting playback or jumping to a
particular position in the document (like a particular measure of the music).
The document could now also be converted to a visual notation via some external
program like Lilypond, and eventually displayed alongside the braille music code.

Since the user base is definitely relying on accessibility being available on
their respective operating systems, some extra care has to be taken when
implementing the graphical user interface.  Some seemingly obvious choices are
not possible.  For instance, the program is currently being developed under
GNU/Linux.  A natural choice on Linux for an accessible GUI is GTK, which
features well tested accessibility since a few years now.  GTK is actually also
ported to Microsoft Windows, but it is not accessible on that platform.  We are
aiming to support as many users as possible, so we do have to plan/provide a
port to MS Windows.  Some way come up with wxWindows as a possible candidate
since it features a GUI toolkit for Linux and Windows with the same API on the
different systems.  Unfortuantely, the accessibility of wxWindows is not equally
accessible on both platform (tested around 2009).

It looks like we will have to solve this "the hard way": Implement natively
accessible interfaces for both platforms separately: On for GTK on Linux and one
for MFC (or whatever is best for C++) on Windows.

To avoid substantial different behaviour on the two platforms, it might be
desireable to develop some kind of common GUI layer which provides most of our
application logic, and only a rather thin layer for the particular toolkits on
the particular platforms below.  We want to minimize the bugs introduced by
duplicated code.


Coding Standard
---------------

The primary programming language for BMC is C++.
Templates and meta programming are allowed and encouraged.
In addition to the Standard Library, Boost is used to make a few hard jobs a lot
easier, and encourage good coding style.  If in doubt, check with the Standard
Library and Boost conding standards.

* C++11
  The upcoming C++11[4] standard is a nice enhancement of the existing language.
  Given the fact that GCC supports alot of the new standard already, and
  Microsofts Visual Studio is apparently also following at least somewhat, it
  was decided to allow some C++11 constructs into the codebase already.
  The BMC codebase is totally fresh anyways, and the actual task at hand
  seems so involved that we do not think it likely that we're going to be
  "finished" in the foreseeable future.  It is therefore very much likely that
  C++11 will be well adopted by most noteworthy compilers, by the time BMC is
  reaching any sort of production quality maturity.  Therefore we think we
  should be fine with C++11 already, and we can have a lot of fun learning its
  features along the way, not needing to limit us to the more verbose older ways
  of C++.
  That said, if C++11 presents a problem to a porter, C++03 is definitely
  preferable and should just be patched.  rvalue references, range based for
  loops and all the other nice features of C++11 are just that, nice.
  They are syntactic sugar, in a sense.  Portability takes preference.

  That said, here is a (possibly incomplete) list of C++11 features used by BMC:
  * Rvalue references and move constructors[5]: This is probably the greatest
    improvement to C++ performance-wise.  Nevertheless it is currently only used
    to implement move-semantics for the FluidSynth wrapper class (to fullfil a
    requrement of the threading API).
    Additionally, disambiguate.hpp makes some use of the STL member function
    emplace_back() to increase performance.

   [5] http://en.wikipedia.org/wiki/C++11#Rvalue_references_and_move_constructors

  * Range-based for-loop[6]: This is really just syntactic sugar to make code
    more concise and readable (very desireable).  It is only sporadically
    used throughout the code.  The macro BOOST_FOREACH[7] is a good drop-in
    replacement for a native range-based for-loop if it presents any problems.
    However, tests have shown that BOOST_FOREACH is slower compared to
    native range-based for-loops.

   [6] http://en.wikipedia.org/wiki/C++11#Range-based_for-loop
   {7] http://www.boost.org/doc/libs/1_47_0/doc/html/foreach.html

  * Right angle bracket[8]: Template code tends to look ugly if the programmer
    needs to put spaces between two consecutive angle brackets.  Fortunately,
    C++11 solves this ugly gotcha of C++.  Some places of the code might
    make use of this.  If its a problem, simply insert the dreaded space.

   [8] http://en.wikipedia.org/wiki/C++11#Right_angle_bracket

  * Explicitly defaulted and deleted special member functions[9]: A useful
    feature to make it more clear which member functions are actually created
    by the compiler implicitly.  This is used in the FluidSynth class, since that
    class is already explicitly designed to be move-aware, which is already
    a C++11 only functionality.

   [9] http://en.wikipedia.org/wiki/C++11#Explicitly_defaulted_and_deleted_special_member_functions

  * Threading facilities[10]: The playback code makes use of std::thread and
    std::chrono.  If the Standard Library of a certain platform lacks these
    C++11 improvements, you might be able to use boost::thread[11] and
    boost::chrono[12] as a (temporary) replacement.

   [10] http://en.wikipedia.org/wiki/C++11#Threading_facilities
   [11] http://www.boost.org/doc/libs/1_47_0/doc/html/thread.html
   [12] http://www.boost.org/doc/libs/1_47_0/doc/html/chrono.html


* const qualifier: C++ is ambiguous regarding the placement of const qualifiers.
  const can either be placed before or after the variable or argument type.
  I prefer const being placed after the variable or arguments type, i.e.

   GOOD:   std::vector<int> const v;
   BAD:    const std::vector<int> v;


Getting the source
------------------

Note that the following description includes a simple way of getting a current
(and tested) version of Boost trunk, for your convenience.  If you already have
a recent enough (1.47.0) copy of Boost on your system you can of course skip the
"svn export".

 $ git clone http://github.com/mlang/bmc
 $ cd bmc
 $ svn export http://svn.boost.org/svn/boost/trunk/boost@75166 boost


Building
--------

 $ autoconf
 $ ./configure -q --with-boost-root-directory=.
 $ make


TODO
----

* Port current codebase to Windows (at least VS2010)
  * Port the build system, or replace with portable on (CMake?)
  * Figure out how to mimmick the FluidSynth functionality currently used under
    Linux under Windows.  Ideally, create a common class for realtime MIDI
    playback which is platform independent, and implement FluidSynth (Linux) and
    Windows backends on top of that.
  * Make the command-line part (bmc) and the testsuite (test) work on Windows.
    Make sure the testsuite runs without errors.

