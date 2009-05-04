#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name='Smache',
      version='1.0',
      description='Smache Utilities',
      author='Adin Scannell',
      author_email='adin@scannell.ca',
      url='http://www.adin.scannell.ca/',
      packages=['smache'],
      scripts=['smachezip', 'smachefs', 'smachestats'],
      ext_modules=[Extension('smache._native', ['smache/swig.i'],
                             swig_opts=['-modern', '-I../include'],
                             library_dirs=['../lib'],
                             libraries=['smache'])],
     )
