#!/bin/sh

set -e

python $@ | lilypond -s -o music21_snippets -
