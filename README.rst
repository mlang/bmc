=============================
BMC -- Braille Music Compiler
=============================

.. image:: https://secure.travis-ci.org/mlang/bmc.png?branch=master
   :target: http://travis-ci.org/mlang/bmc

BMC aims to become a system for parsing and generating braille music code.


History
-------

A predecessor to this project is FreeDots_, a Java-based program for
converting MusicXML Documents to Braille Music code, as well as offering
playback and some interactive editing capabilities.

.. _FreeDots: http://code.google.com/p/freedots/

FreeDots has been very helpful in providing a general understanding of the
various aspects of braille music code.  However, it was only designed to cover
one way of conversion, namely from some music notation source (like MusicXML) to
braille music code.  A full system for working with braille music code should
eventually cover both directions.


Dependencies
------------

Being a C++ program, BMC naturally uses the Standard Template Library (STL).

In addition to that some Boost_ C++ Libraries are employed.

.. _Boost: http://www.boost.org/


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


Getting the source
------------------

You need Git_ to retrieve the source repository.

.. code-block:: bash

   git clone --quiet https://github.com/mlang/bmc.git
   git submodule --quiet update --init --recursive

.. _Git: https://git-scm.com/

Building on Mac OS X
--------------------

We assume you have Xcode installed.  A nice package manager is Homebrew:

.. code-block:: bash

   ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

If you have the Homebrew package manager, run the following commands to get
all dependencies required to build and run BMC:

.. code-block:: bash

   xcode-select --install
   brew install cmake lame pkg-config python3 qt5 timidity xerces-c xsd
   pip3 install sphinx
   brew install caskroom/cask/brew-cask
   brew cask install lilypond
   brew install boost --with-python
   brew install boost-python

You can now run CMake to generate a build system:

.. code-block:: bash

   cd bmc
   cmake .

When running the test suite, make sure you have a UTF-8 based locale:

.. code-block:: bash

   export LANG=de_AT.UTF-8
   make check

Build the command-line tool and the user interface:

.. code-block:: bash

   make bmc bmc-ui

Building (on UNIX)
------------------

Compilation has only been tested with GCC 4.8 recently.

On Debian GNU/Linux systems, you will need to install the following dependencies:

.. code-block:: bash

   sudo apt-get install cmake doxygen lib{boost-{program-options,test},fluidsynth,icu}-dev

To build BMC, run the following commands:

.. code-block:: bash

   cd bmc
   cmake .
   make

To execute the test-suite, run:

.. code-block:: bash

   make check


Building (on Windows)
---------------------

After configuring via cmake, either open the Solution bmc in VS2015 or
open a "MSBuild Command Prompt for VS2015" and run the following:

.. code-block:: console

   msbuild bmc.sln /t:bmc-ui /p:Configuration=Release


TODO
----

* Port current codebase to Windows:

  * Figure out how to mimmick the FluidSynth functionality currently used under
    Linux under Windows.  Ideally, create a common class for realtime MIDI
    playback which is platform independent, and implement FluidSynth (Linux) and
    Windows backends on top of that.
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
* Port Cococa Touch:
  iOS handles Unicode Braille just as expected.  It is displayed on screen
  with an appropriate font and works together with external braille displays as
  well.  Given that, a port to Cocoa Touch seem quite feasable.




