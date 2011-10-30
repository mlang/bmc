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
Currently however, to make the Boost dependencies as easy to handle as possible,
we are only using Boost header-only libraries[3].  This means that the boost
include directory is enough to build BMC.  There is no need to link any Boost
Library files to the final executable.

 [2] http://www.boost.org/
 [3] http://www.boost.org/doc/libs/1_47_0/?view=filtered_header-only


Parsing
-------

So BMC was started to begin completely a new, this time, with the more complex
task of parsing braille music code.  Braille music code was invented originally
around 1880, and was later refined by several comittees.  Its aim was naturally
to be unambiguous and allow verbatim transcriptions from visual music notation,
but it was never particularily designed to be read by a computer.  This aspect
of the history shows up in the more complex tricks that we will need to provide
a parser that is as intelligent as possible.

Instead of the more conservative yacc/lex approach, we are using a much more
modern parsing framework, namely Boost.Spirit[4].  Spirit is a C++ framework
for creating parsers based on templates and meta-programming.
A DSEL for EBNF-alike grammars is provided, and transformed at compile time
into the necessary code to implement the actual parser.
Using Boost.Fusion, Spirit can support a wide variety of desireable C++
data structures as synthesized attributes of its grammars.  We can make use
of STL containers and even more specific types like Boost.Variant or
Boost.Optional to construct our abstract syntax tree.

 [4] http://www.boost.org/doc/libs/1_47_0/libs/spirit/doc/html/index.html

We are using a helper-class called error_handler to save iterators into the
original input data for parsed entities such that we can report the exact
location (line and column) of error conditions during the parsing (and
during later processing).  This exact contextual information is going to
be important later on for other planned features of the project.


heterogeneous containers
------------------------

A common problem arising in the sort of data structures required to represent
parsed braille music is the heterogenuity of the underlying sequences
of objects.  Traditionally, virtual member functions and a common base class are
used in C++ to implement a sequence of objects of varying purpose.  However,
this approach is rather error-prone and requires the differing object types to
at least have something in common to allow for a meaningful interface.
Fortunately, Boost offers a very elegant solution to this type of problem, the
Boost.Variant discriminated union container class template.  Boost.Variant is
already implicitly used by Spirit to store alternative choices in the grammar.
So it just falls naturally to use Boost.Variant for problems falling in the
category of heterogeneous containers.  Using Boost.Variant implies the visitor
pattern is used to access the data structures.  This is desireable because it
results in type-safe code which eliminates a certain class of runtime bugs.  It
makes code a little bit more verbose to write, but for the better, actually.


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
   the process of memorisation.
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

One interesting special case of conversion is the transformation from braille
music code to braille music code.  While this might seem academic at first
sight, it is not.  Braille music code employs very complicated hyphenation
rules, the rules governing the splitting of very long measures across two (or
several) lines of braille.  So to be able to reformat braille music for a
different page width requires it to be parsed completely first, and then printed
again just with a different line width.  This feature would be very desireable
for braille display users, since a lot of readily-available braille music code
documents currently are formatted for 32 (or less) characters per line, while a
typical braille display has 40 or even 60 or 80 characters.


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
  The upcoming C++11[5] standard is a nice enhancement of the existing language.
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

  [5] http://en.wikipedia.org/wiki/C++11

  Here is a (possibly incomplete) list of C++11 features used by BMC:
  * Rvalue references and move constructors[6]: This is probably the greatest
    improvement to C++ performance-wise.  Nevertheless it is currently only used
    to implement move-semantics for the FluidSynth wrapper class (to fullfil a
    requrement of the threading API).
    Additionally, disambiguate.hpp makes some use of the STL member function
    emplace_back() to increase performance.

   [6] http://en.wikipedia.org/wiki/C++11#Rvalue_references_and_move_constructors

  * Range-based for-loop[7]: This is really just syntactic sugar to make code
    more concise and readable (very desireable).  It is only sporadically
    used throughout the code.  The macro BOOST_FOREACH[8] is a good drop-in
    replacement for a native range-based for-loop if it presents any problems.
    However, tests have shown that BOOST_FOREACH is slower compared to
    native range-based for-loops.

   [7] http://en.wikipedia.org/wiki/C++11#Range-based_for-loop
   {8] http://www.boost.org/doc/libs/1_47_0/doc/html/foreach.html

  * Right angle bracket[9]: Template code tends to look ugly if the programmer
    needs to put spaces between two consecutive angle brackets.  Fortunately,
    C++11 solves this ugly gotcha of C++.  Some places of the code might
    make use of this.  If its a problem, simply insert the dreaded space.

   [9] http://en.wikipedia.org/wiki/C++11#Right_angle_bracket

  * Explicitly defaulted and deleted special member functions[10]: A useful
    feature to make it more clear which member functions are actually created
    by the compiler implicitly.  This is used in the FluidSynth class, since that
    class is already explicitly designed to be move-aware, which is already
    a C++11 only functionality.

   [10] http://en.wikipedia.org/wiki/C++11#Explicitly_defaulted_and_deleted_special_member_functions

  * Threading facilities[11]: The playback code makes use of std::thread and
    std::chrono.  If the Standard Library of a certain platform lacks these
    C++11 improvements, you might be able to use boost::thread[12] and
    boost::chrono[13] as a (temporary) replacement.

   [11] http://en.wikipedia.org/wiki/C++11#Threading_facilities
   [12] http://www.boost.org/doc/libs/1_47_0/doc/html/thread.html
   [13] http://www.boost.org/doc/libs/1_47_0/doc/html/chrono.html


* const qualifier: C++ is ambiguous regarding the placement of const qualifiers.
  const can either be placed before or after the variable or argument type.
  I prefer const being placed after the variable or arguments type, i.e.

   GOOD:   std::vector<int> const v;
   BAD:    const std::vector<int> v;

* Use tabs, no spaces: To minimize indentation issues in a multi-developer
  project, we'd like to avoid the use of tabs.  Use spaces to indent.

* Write maintainable code: Given the low amount of manpower we can expect,
  the source code of BMC should be as readable and maintainable as possible.
  This implies a lot, for instance:
  * Avoid code duplication at all cost: Whenever there is duplicated code
    and/or functionality, someone editing the code will have to know about all
    the other places which also need updating.  In the case of cross-platform
    support code, they might not even have access to the particular platform
    at the moment to research and/or test the change.  Keep laptform specific
    code separate from the core program logic as much as possible.
  * Keep in mind the code will evolve: In reality, certain changes to a program
    require quite substantial modifications to the source.  These changes get
    easier to do if code is decoupled as much as possible.  Forcing yourself to
    do test-driven development can help in the decoupling.  Someone doing a
    bigger change in the future will still have to read and understand your code
    and possibly change/transform it to something else to fit the newly
    discovered requirement.  Keep this in mind and try to avoid excessive
    coupling of unrelated things into single functions/classes for instance.
    Use meaningful member names.  If in doubt, make the name verbose, instead of
    choosing a cryptic name that might be unclear to someone else.
  * Write test cases: The nature of the problem we are trying to solve with BMC
    absolutely requires to follow a test-driven development pattern.  Braille
    music code has so many special cases that we need to secure milestones
    in development with good test cases to avoid breaking acomplishments
    we've already achieved by later changes in the development process.
    This is, however, a good thing, because test-driven development has many
    positive side-effects on code quality.
    If realisticly possible, write test cases for new code you develop.  A
    framework for test cases is present and being used in the build process to
    indicate regressions as early as possible.  Make use of this safe-guard
    to avoid your code being accidentally broken in the future.
    Some special cases, like GUI behaviour for instance, are of course rather
    hard to test.  Apply common sense when deciding how much time to invest in
    developing a useful test case.
  * Use the features of C++ to avoid dangerous code:  For instance, use enums
    instead of integer constants.  Help the compiler avoid programming errors.
    Or, use a safe discriminated union and the visitor pattern instead of
    dangerous runtime type checking (as we do with Boost.Variant already).
  * Use the STL and Boost: Do not reinvent the wheel.  STL + Boost is an
    extremely powerful set of libraries which can solve a host of problems in a
    very well thought-out kind of way.  It is worth it to explore their details
    and make use of their powerful features.
  * If it makes sense, use C++11: Two examples of this guideline are the use
    of std::thread to launch playback tasks and std::chrono to handle sleeping in
    the inner playback loop.  These two new library components from C++11 just
    perfectly fit the job.  std::thread offers a nice C++ wrapper around the
    usual OS-level threading primitives, and std::chrono allows for safely typed
    real-time duration management, something basically every real-time playback
    code needs.  We could also have invented our own wrappers and helper classes
    to achieve the same thing, but why bother if a full comittee of people has
    invested years of discussion and review to come up with a very good API for
    us?


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


Description of source code components
-------------------------------------

To ease review, here is a rough overview of the various source code files which
make up this prototype:

 * Text to braille:
   The subdirectory ttb/ contains source code for the mapping of character values
   to braille dots.  The code has been borrowed from the BRLTTY project,
   and stripped down a little to avoid excessive code bloat.  It is therefore
   compatible to the format of braille tables employed by BRLTTY (on purpose).
   This part of the code is pretty well-tested and should not need to be changed.
 * Parsing:
   The files numbers.hpp, measure.hpp and score.hpp contain the toplevel
   grammar declarations.  The actual grammars are defined in the accompanying
   files numbers_def.hpp, measure_def.hpp and score_def.hpp.
   The resulting class templates are instantiated into separate translation
   units using the files numbers.cpp, measure.cpp and score.cpp.
   The files error_handler.hpp and annotation.hpp provide input location
   tracking for individual parsed entities.
   brlsym.hpp and brlsym.cpp define a few symbol tables for the purpose of
   parsing braille music code (used in the *_def.hpp files).
   ambiguous.hpp collects all the data types necessary to represent the result
   of parsing the given input (in other words, the abstract syntax tree).
   And finally, music.hpp contains basic utility types which seem common to
   musical notation in general, not being tied to a particular type of notation.
   For instance, a rational data type is created using Boost.Rational.
   Several enums, such as accidentals or diatonic steps are also defined here.
 * Compilation:
   In the context of BMC, compilation refers to the process of post-processing
   the bare result gained from parsing braille music code.
   The file compiler.hpp defines the function object class
   music::braille::compiler which is used as an entry point for all associated
   algorithms.
   disambiguate.hpp and octave_calculator.hpp do implement code required
   for disambiguating note values and calcualting exact octaves of notes and
   chords respectively.
   compiler.cpp implements a few lengthy member function of
   music::braille::compiler and comprises the top-level of the
   compiler translation unit.
 * Playback:
   As a proof of concept, some code exists to play the compiled musical score
   on Linux using the FluidSynth package (a SoundFont-based software synthesizer).
   The file midi.hpp implements a simple layer for storing MIDI data in memory.
   It offers classes for representing most basic MIDI events and a priority_queue
   based class for implicitly ordering MIDI events by their begin time.
   fluidsynth.hpp and fluidsynth.cpp implement a simple wrapper around the
   FluidSynth C API to allow playing of compiled musical scores.
 * Testing and utilities:
   The file test.cpp contains all the test cases implemented so far.
   For the convenience of developers, brltr.cpp contains a small command-line
   tool which can be used to translate text characters to Unicode braille
   given a particular braille table (see Text to braille).
 * The program:
   main.cpp contains the main routine necessary to link a final executable.

         
TODO
----

* Port current codebase to Windows:
  * Port the build system, or replace with a portable one (CMake?)
  * Figure out how to mimmick the FluidSynth functionality currently used under
    Linux under Windows.  Ideally, create a common class for realtime MIDI
    playback which is platform independent, and implement FluidSynth (Linux) and
    Windows backends on top of that.
  * Make the command-line part (bmc) and the testsuite (test) work on Windows.
    Make sure the testsuite runs without errors.
  * Investigate encoding compatibility: BMC tries to be Unicode-based internally.
    On UNIX, wchar_t is 32bit wide, which allos for full Unicode compatibility.
    On Windows, it is 16bit wide and implicitly UTF16 coded (that is my current
    understanding).  Figure out what encodings we are to expect on
    Windows and deal with them in the most flexible way.  Unicode
    is to be prefered internally, always.

