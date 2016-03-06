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
 [3] http://www.boost.org/doc/libs/1_50_0/?view=filtered_header-only


Parsing
-------

So BMC was started to begin completely a new, this time, with the more complex
task of parsing braille music code.  Braille music code was invented originally
around 1880, and was later refined by several comittees.  Its aim was naturally
to be unambiguous and allow verbatim transcriptions from visual music notation,
but it was never particularily designed to be read by a computer.  This aspect
of the history shows up in the more complex tricks that we will need to provide
a parser that is as intelligent as possible.

Instead of the more conservative yacc/lex approach, we are using a more
modern parsing framework, namely Boost.Spirit[4].  Spirit is a C++ framework
for creating parsers based on templates and meta-programming.
A DSEL for EBNF-alike grammars is provided, and transformed at compile time
into the necessary code to implement the actual parser.
Using Boost.Fusion, Spirit can support a wide variety of desireable C++
data structures as synthesized attributes of its grammars.  We can make use
of STL containers and even more specific types like Boost.Variant or
Boost.Optional to construct our abstract syntax tree.

 [4] http://www.boost.org/doc/libs/1_50_0/libs/spirit/doc/html/index.html

We are using a helper-class called error_handler to save iterators into the
original input data for parsed entities such that we can report the exact
location (line and column) of error conditions during the parsing (and
during later processing).


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
music manually.  Upon a special keypress or invocation of a menu item the
program will try to parse the current document and either report errors sensibly
or provide some sort of confirmation.  Once the document is successfully parsed,
several more options will be available, like starting playback or jumping to a
particular position in the document (like a particular measure of the music).
The document could now also be converted to visual notation via some external
program like Lilypond, and eventually displayed alongside the braille music code.

Since the user base is definitely relying on accessibility being available on
their respective operating systems, some extra care has to be taken when
implementing the graphical user interface.  Some seemingly obvious choices are
not possible.  For instance, the program is currently being developed under
GNU/Linux.  A natural choice on Linux for an accessible GUI is GTK, which
features well tested accessibility since a few years now.  GTK is actually also
ported to Microsoft Windows, but it is not accessible on that platform.  We are
aiming to support as many users as possible, so we do have to plan/provide a
port to MS Windows.  Some may come up with wxWindows as a possible candidate
since it features a GUI toolkit for Linux and Windows with the same API on the
different systems.  Unfortuantely, the accessibility of wxWindows is not equally
accessible on both platform (tested around 2009).

It looks like we will have to solve this "the hard way": Implement natively
accessible interfaces for both platforms separately: One for GTK on Linux and one
for MFC (or whatever is best for C++) on Windows.

To avoid substantial different behaviour on the two platforms, it might be
desireable to develop some kind of common GUI layer which provides most of our
application logic, and only a rather thin layer for the particular toolkits on
the particular platforms below.  We want to minimize the bugs introduced by
duplicated code.


Getting the source
------------------


 $ git clone --recursive http://github.com/mlang/bmc


Building on Mac OS X
--------------------

We assume you have Xcode installed.  A nice package manager is Homebrew:

 $ ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you have the Homebrew package manager, run the following commands to get
all dependencies required to build and run BMC:

 $ xcode-select --install
 $ brew install cmake pkg-config python3 qt5 xerces-c xsd
 $ pip3 install sphinx
 $ brew install caskroom/cask/brew-cask
 $ brew cask install lilypond
 $ brew install boost --with-python
 $ brew install boost-python

You can now run CMake to generate a build system:

 $ cd bmc
 $ cmake .

When running the test suite, make sure you have a UTF-8 based locale:

 $ export LANG=de_AT.UTF-8
 $ make check

Build the command-line tool and the user interface:

 $ make bmc bmc-ui

Building (on UNIX)
------------------

Compilation has only been tested with GCC 4.8 recently.

On Debian GNU/Linux systems, you will need to install the following dependencies:

 $ sudo apt-get install cmake doxygen lib{boost-{program-options,test},fluidsynth,icu}-dev

To build BMC, run the following commands:

 $ cd bmc
 $ cmake .
 $ make

To execute the test-suite, run:

 $ make check


Building (on Windows)
---------------------

After configuring via cmake, either open the Solution bmc in VS2015 or
open a "MSBuild Command Prompt for VS2015" and run the following:

 $ msbuild bmc.sln /t:bmc-ui /p:Configuration=Release


Description of source code components
-------------------------------------

To ease review, here is a rough overview of the various source code files which
make up this prototype:

 * Text to braille:
   The subdirectory ttb/ contains source code for the mapping of character values
   to braille dots.  The code has been borrowed from the BRLTTY[14] project,
   and stripped down a little to avoid excessive code bloat.  It is therefore
   compatible to the format of braille tables employed by BRLTTY (on purpose).
   This part of the code is pretty well-tested and should not need to be changed.

  [14] http://mielke.cc/brltty/

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
   ast.hpp collects all the data types necessary to represent the result
   of parsing the given input (in other words, the abstract syntax tree).
   And finally, music.hpp contains basic utility types which seem common to
   musical notation in general, not tied to a particular type of notation.
   For instance, a rational data type is created using Boost.Rational.
   Several enums, such as accidentals or diatonic steps are also defined here.
 * Compilation:
   In the context of BMC, compilation refers to the process of post-processing
   the bare result gained from parsing braille music code.
   The file compiler.hpp defines the function object class
   bmc::braille::compiler which is used as an entry point for all associated
   algorithms.
   disambiguate.hpp, octave_calculator.hpp and alteration_calculator.hpp
   implement code required
   for disambiguating note values, calcualting exact octaves of notes and
   chords and calculating the alteration of pitches respectively.
   compiler.cpp implements a few lengthy member function of
   bmc::braille::compiler and comprises the top-level of the
   compiler translation unit.
 * Conversion to musical notation formats:
   lilypond.hpp implements code to convert a braille music score to LilyPond.
 * Playback:
   As a proof of concept, some code exists to play the compiled musical score
   on Linux using the FluidSynth package (a SoundFont-based software synthesizer).
   The file midi.hpp implements a simple layer for storing MIDI data in memory.
   It offers classes for representing most basic MIDI events and a priority_queue
   based class for implicitly ordering MIDI events by their begin time.
   fluidsynth.hpp and fluidsynth.cpp implement a simple wrapper around the
   FluidSynth C API to allow playing of scores.
 * Testing and utilities:
   The file test.cpp contains all the test cases implemented so far.
 * The program:
   main.cpp contains the main routine necessary to link a final executable.

         
TODO
----

* Port current codebase to Windows:
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
    is to be prefered internally, always.  Is Unicode Braille supported
    on Windows in the command prompt?  Currently unit test input data is
    all encoded with UTF-8.  Figure out if this is a problem on Windows.
* Improve error reporting during parsing: Some useful diagnostics
  are already printed, but in many other cases the parser does not produce
  helpful error messages.  It can be quite hard to start a braille music piece
  from scratch if you have no idea why it is not accepted by BMC.
* Implement Standard MIDI File (SMF) writing: In addition to real-time playback,
  musical scores should also be exportable to MIDI files on disk such that
  they can be played or imported with other programs.  Note that the current
  playback code is only a proof of concept, and needs more work.  Its probably
  best to write something that converts a bmc::midi::evenet_queue to
  a suitable on-disk representation so that common code between real-time
  playback and file export can be shared.
* Handle tied-note playback correctly: As usual with prototypes, the playback
  code does take shortcuts currently.  One typical problem when converting
  note material to performance data is the interpretation of (usually visual)
  cues on how to play the music.  Articulations are one (more advanced) instance
  of this.  A more fundamental one is the interpretation of ties.
  If a note is tied to another one, it is supposed to be played with both
  note durations added.  Currently, the playback code ignores this and
  plays tied notes as if they are two separate notes.  This needs to be fixed.
  Note that ties can cross measure boundaries: A note at the end of one measure
  can be tied to one of the first notes of the next measure.
* Devise a method to specify subsets of the parsed note material for playback
  or export.  For instance, the user might want to play starting from a certain
  measure, or only listen to a certain staff (hand) in multistaff music.
* Design the necessary components to handle unrolling: Braille music code
  allows for specification of repeated note material in a much more fine-grained
  way as visual music notation allows for.  Simile signs can be used to repeat
  complete measures, particular voices, or even parts of a voice.  Braille
  repeats can be used to indicate repetition of an arbitrary range of measures
  of the current staff.  This implies that we will have to deal with data in
  both representations somehow: There is a stage of processing where all these
  repetition instructions are present (once the parse stage succeds), and we
  will want to unroll the given abstract syntax tree such that we get a view of
  all the notes actually implied by these contractions.  We obviously need an
  unrolled "view" for export to anything other then braille music code, since
  most other formats seem to lack these compression fascilities.  For instance,
  when generating MIDI messages, we need to have all contractions expanded such
  that we know the notes we need to generate.  However, LilyPond input data
  allows for a special kind of repeat which basically serves a similar purpose
  as in braille music, namely to reduce duplicated note material.  If we ever
  get to the stage of LilyPond export, we might want to use some of the braille
  repeats as cues to generate more human readable LilyPond files.
* Port to Cococa and Cococa Touch:
  iOS handles Unicode Braille just as expected.  It is displayed on screen
  with an appropriate font and works together with external braille displays as
  well.  Given that, ports to Cocoa and Cocoa Touch seem quite feasable.




