from distutils.core import setup, Extension
from sysconfig import get_paths
from pprint import pprint

info = get_paths()  # a dictionary of key-paths
pythonHeaders = info['include']
pythonLib = 'python'

module1 = Extension('_ais_decoder', ['ais_decoder.i'],
                    libraries = [pythonLib, 'ais_decoder'],
                    swig_opts=['-c++', '-I '+pythonHeaders])

setup(name = 'AisDecoder',
      version = '1.0',
      description = 'AIS Decoder',
      author = 'Arno Duvenhage',
      author_email = '',
      url = 'https://github.com/aduvenhage/ais-decoder',
      ext_modules=[module1],
      py_modules=['ais_decoder'],
      )

