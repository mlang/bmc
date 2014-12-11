#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals
from docutils.nodes import raw
from docutils.parsers.rst import Directive
from docutils.parsers.rst.directives import register_directive, unchanged
from jinja2 import DictLoader, Environment
from os.path import abspath, dirname, join
from subprocess import PIPE, Popen

__docformat__ = 'reStructuredText'
__doc__ = '''\
reStructuredText directive for entering braille music code
==========================================================

:Author: Mario Lang
:Contact: mlang@delysid.org
:Date: 2014-02-17

This module enables a new reStructuredText directive when you import it.

.. braille-::bmc:: bmc-rst-example-1
   :title: A minimal example

   ⠐⠽⠣⠅

The syntax for this directive is as follows::

  .. braille-::bmc:: unique-identifier-used-as-basename-for-generated-files
     :title: An optional title (caption) for the music

     ⠐⠹⠳⠳⠳⠀⠹⠳⠳⠳⠀⠱⠪⠪⠪⠀⠽⠣⠅

'''

html5 = '''\
<section id="{{ arguments[0] }}">
{% if options['title'] %}
  <header>
    <h2>{{ options['title'] | escape }}</h2>
  </header>
{% endif %}
  <pre>{{ content | join('\n') }}</pre>
  <p>View as: <a href="{{ arguments[0] }}.pdf">A4 PDF</a>.</p>
  <audio controls="controls">
    <p>Listen to: <a href="{{ arguments[0] }}.midi">MIDI</a>.</p>
  </audio>
  <!-- LilyPond input:
{{ lilypond | escape }}
  -->
</section>
'''

bmc_path = '../../bmc-gcc/bmc2ly'
lilypond_path = '/usr/bin/lilypond'
output_path = None

class BrailleMusic(Directive):
    required_arguments = 1
    option_spec = {'locale': unchanged, 'title': unchanged}
    has_content = True

    default_braille_locale = 'brf'

    def run(self):
        """Generate multiple representations for given braille music."""
        self.assert_has_content()
        source_dir = dirname(abspath(self.state_machine.input_lines.source(self.lineno - self.state_machine.input_offset - 1))) if output_path is None else output_path
        bmc = Popen([bmc_path, '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
        self.lilypond, self.errors = bmc.communicate(input='\n'.join(self.content).encode('utf-8'))
        if bmc.returncode != 0:
            raise self.error('BMC failed: ' + self.errors)
        lilypond = Popen([lilypond_path,
                          '-l', 'ERROR',
                          '-o', join(source_dir, self.arguments[0]),
                          '-'],
                         stdin=PIPE, stdout=PIPE, stderr=PIPE)
        _, self.lilypond_errors = lilypond.communicate(input=self.lilypond)
        if lilypond.returncode != 0:
            raise self.error('LilyPond failed: ' + self.lilypond_errors.decode('utf-8')) 
        env = Environment(loader=DictLoader(globals()))
        return [raw('', env.get_template('html5').render(vars(self)), format='html')]

    def locale(self):
        return self.options.get('locale', self.default_braille_locale)

register_directive('braille-music', BrailleMusic)

if __name__ == "__main__":
    # Publish our own docstring to stdout.
    from docutils.core import publish_string
    print(publish_string(__doc__, writer_name='html').decode('utf-8'))

