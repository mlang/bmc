# -*- coding: utf-8 -*-
"""
    bmc.sphinx
    ~~~~~~~~~~

    Allow BMC-parsed braille music to be included in Sphinx-generated
    documents inline.

    :copyright: Copyright 2014 by Mario Lang.
    :license: GPL2
"""

import re
import codecs
import posixpath
from os import path
from subprocess import Popen, PIPE
try:
    from hashlib import sha1 as sha
except ImportError:
    from sha import sha

from docutils import nodes
from docutils.parsers.rst import directives

from sphinx.errors import SphinxError
from sphinx.locale import _
from sphinx.util.osutil import ensuredir, ENOENT, EPIPE, EINVAL
from sphinx.util.compat import Directive


class BMCError(SphinxError):
    category = 'BMC error'


class bmc(nodes.General, nodes.Element):
    pass


class BrailleMusic(Directive):
    """
    Directive to insert braille music code.
    """
    has_content = True
    required_arguments = 0
    optional_arguments = 1
    final_argument_whitespace = False
    option_spec = {
        'locale': directives.unchanged,
        'title': directives.unchanged,
    }

    def run(self):
        if self.arguments:
            document = self.state.document
            if self.content:
                return [document.reporter.warning(
                    'braille-music directive cannot have both content and '
                    'a filename argument', line=self.lineno)]
            env = self.state.document.settings.env
            rel_filename, filename = env.relfn2path(self.arguments[0])
            env.note_dependency(rel_filename)
            try:
                fp = codecs.open(filename, 'r', 'utf-8')
                try:
                    dotcode = fp.read()
                finally:
                    fp.close()
            except (IOError, OSError):
                return [document.reporter.warning(
                    'External braille-music file %r not found or reading '
                    'it failed' % filename, line=self.lineno)]
        else:
            dotcode = '\n'.join(self.content)
            if not dotcode.strip():
                return [self.state_machine.reporter.warning(
                    'Ignoring braille-music directive without content.',
                    line=self.lineno)]
        node = bmc()
        node['code'] = dotcode
        if 'locale' in self.options:
            node['locale'] = self.options['locale']
        if 'title' in self.options:
            node['title'] = self.options['caption']
        return [node]


def midi2mp3(midi_file, mp3_file, timidity_executable='timidity', timidity_args=[], lame_executable='lame', lame_args=[]):
    timidity = [timidity_executable]
    timidity.extend(timidity_args)
    timidity.extend(['-idqq', '-Ow', '-o', '-', midi_file])
    lame = [lame_executable]
    lame.extend(lame_args)
    lame.extend(['-b64', '--quiet', '-', mp3_file])
    p1 = Popen(timidity, stdout=PIPE)
    p2 = Popen(lame, stdin=p1.stdout, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p2.communicate()
    return (p2.returncode, stdout, stderr)

def render_bmc(self, code, prefix='bmc'):
    """Render braille music code into a MIDI and PDF output files."""
    hashkey = (code + \
              str(self.builder.config.bmc_executable) + \
              str(self.builder.config.bmc_args) + \
              str(self.builder.config.bmc_lilypond_executable) + \
              str(self.builder.config.bmc_lilypond_args)
              ).encode('utf-8')

    relfn = '%s-%s' % (prefix, sha(hashkey).hexdigest())
    outfn = path.join(self.builder.outdir, relfn)

    if path.isfile(outfn + '.preview.svg'):
        return relfn, outfn

    if hasattr(self.builder, '_bmc_warned_bmc') or \
       hasattr(self.builder, '_bmc_warned_lilypond'):
        return None, None

    ensuredir(path.dirname(outfn))

    # BMC expects UTF-8 by default
    if isinstance(code, str):
        code = code.encode('utf-8')

    bmc_args = [self.builder.config.bmc_executable]
    bmc_args.extend(self.builder.config.bmc_args)
    bmc_args.extend(['--lilypond', '-'])
    try:
        p = Popen(bmc_args, stdout=PIPE, stdin=PIPE, stderr=PIPE)
    except OSError as err:
        if err.errno != ENOENT:   # No such file or directory
            raise
        self.builder.warn('bmc command %r cannot be run (needed for bmc '
                          'output), check the bmc_executable setting' %
                          self.builder.config.bmc_executable)
        self.builder._bmc_warned_bmc = True
        return None, None
    try:
        stdout, stderr = p.communicate(code)
    except (OSError, IOError) as err:
        if err.errno not in (EPIPE, EINVAL):
            raise
        # in this case, read the standard output and standard error streams
        # directly, to get the error message(s)
        stdout, stderr = p.stdout.read(), p.stderr.read()
        p.wait()
    if p.returncode != 0:
        raise BMCError('bmc exited with error:\n%s' % stderr.decode('utf-8'))

    lilypond = stdout

    lilypond_args = [self.builder.config.bmc_lilypond_executable]
    lilypond_args.extend(self.builder.config.bmc_lilypond_args)
    lilypond_args.extend(['-l', 'WARNING', '-o', outfn, '-dbackend=svg', '-dpreview', '-dno-print-pages', '-'])
    try:
        p = Popen(lilypond_args, stdout=PIPE, stdin=PIPE, stderr=PIPE)
    except OSError as err:
        if err.errno != ENOENT:   # No such file or directory
            raise
        self.builder.warn('bmc command %r cannot be run (needed for bmc '
                          'output), check the bmc_lilypond_executable setting' %
                          self.builder.config.bmc_lilypond_executable)
        self.builder._bmc_warned_lilypond = True
        return None, None
    try:
        stdout, stderr = p.communicate(lilypond)
    except (OSError, IOError) as err:
        if err.errno not in (EPIPE, EINVAL):
            raise
        # in this case, read the standard output and standard error streams
        # directly, to get the error message(s)
        stdout, stderr = p.stdout.read(), p.stderr.read()
        p.wait()
    if p.returncode != 0:
        raise BMCError('lilypond exited with error:\n[stderr]\n%s\n'
                            '[stdout]\n%s' % (stderr.decode('utf-8'), stdout.decode('utf-8')))

    if not path.isfile(outfn + '.preview.svg') or not path.isfile(outfn + '.midi'):
        raise BMCError('lilypond did not produce an output file:\n[stderr]\n%s\n'
                            '[stdout]\n%s' % (stderr.decode('utf-8'), stdout.decode('utf-8')))

    returncode, stdin, stdout = midi2mp3(outfn + '.midi', outfn + '.mp3')
    if returncode != 0:
        raise BMCError('midi2mp3 exited with non-zero exit-code.')

    return relfn, outfn


def html_visit_bmc(self, node):
    try:
        fname, outfn = render_bmc(self, node['code'])
    except BMCError as exc:
        self.builder.warn('bmc code %r: ' % node['code'] + str(exc))
        raise nodes.SkipNode

    self.body.append(self.starttag(node, 'section', CLASS='bmc'))
    if node.get('title'):
        self.body.append('<header><h2>')
        self.body.append(self.encode(node['title']))
        self.body.append('</header></h2>')

    self.body.append('<pre>' + self.encode(node['code']) + '</pre>')

    self.body.append('<img src="%s.preview.svg" alt="Engraved music"/>' % fname)
    self.body.append('<a href="%s.mp3">MP3</a>' % fname)
    self.body.append('</section>\n')
    raise nodes.SkipNode


def man_visit_bmc(self, node):
    self.ensure_eol()
    self.body.append('.sp\n'+'\n.br\n'.join(node['code'].split('\n'))+'\n')
    raise nodes.SkipNode


def text_visit_bmc(self, node):
    self.add_text(node['code'])
    raise nodes.SkipNode


def setup(app):
    app.add_node(bmc,
                 html=(html_visit_bmc, None),
                 man=(man_visit_bmc, None),
                 text=(text_visit_bmc, None))
    app.add_directive('braille-music', BrailleMusic)
    app.add_config_value('bmc_executable', 'bmc', 'html')
    app.add_config_value('bmc_args', [], 'html')
    app.add_config_value('bmc_lilypond_executable', 'lilypond', 'html')
    app.add_config_value('bmc_lilypond_args', [], 'html')

