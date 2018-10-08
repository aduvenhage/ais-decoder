from distutils.core import setup, Extension
from sysconfig import get_paths
from subprocess import call

# build and install AIS decoder lib
print("===============================================")
print("Building and installing C++ AIS Decoder Library")
print("===============================================")

call(["cmake", "../", "-DCMAKE_BUILD_TYPE=RELEASE"])
call(["make"])
call(["make", "install"])

# find python header and lib folders
info = get_paths()
pathHeaders = info['include']

# build module
print("=================================================")
print("Building and installing Python AIS Decoder Module")
print("=================================================")

module1 = Extension('_ais_decoder',
                    ['ais_decoder.i'],
                    include_dirs = [pathHeaders],
                    libraries = ['ais_decoder'],
                    library_dirs = [],
                    swig_opts=['-c++', '-I '+pathHeaders])

setup(name = 'AisDecoder',
      version = '1.0',
      description = 'AIS Decoder',
      author = 'Arno Duvenhage',
      author_email = '',
      url = 'https://github.com/aduvenhage/ais-decoder',
      ext_modules=[module1],
      py_modules=['ais_decoder'],
      )

