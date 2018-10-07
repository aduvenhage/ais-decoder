from distutils.core import setup, Extension
from sysconfig import get_paths
from pprint import pprint

info = get_paths()  # a dictionary of key-paths
pythonHeaders = info['include']
pythonLib = 'python'

# NOTE: ais_decoder should be installed on the system first
module1 = Extension('aisdecoder',
                    define_macros = [],
                    include_dirs = [pythonHeaders],
                    libraries = [pythonLib, 'ais_decoder'],
                    library_dirs = [],
                    sources = ['ais_decoder_wrap.cxx'])

setup (name = 'AisDecoder',
       version = '1.0',
       description = 'AIS Decoder',
       author = 'Arno Duvenhage',
       author_email = '',
       url = 'https://github.com/aduvenhage/ais-decoder',
       long_description = '''
''',
       ext_modules = [module1])
