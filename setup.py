from distutils.core import setup, Extension
import os

def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

module = Extension('set_combinations',
                   sources = ['set_combinations.c'])

setup (name = 'set_combinations',
       version = '1.0',
       description = 'Combinations for sets',
       author = 'Keyhan Vakil',
       author_email = 'kvakil@berkeley.edu',
       url = 'https://kvakil.github.io/',
       license = 'MIT',
       long_description = read('README.md'),
       ext_modules = [module])
