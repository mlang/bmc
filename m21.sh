#!/bin/sh

set -e

python $@ | lilypond -o music21_snippets -
