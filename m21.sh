#!/bin/sh

set -e

python $@ | lilypond -lWARNING -o music21_snippets -
