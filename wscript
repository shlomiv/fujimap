VERSION= '0.1.3'
APPNAME= 'fujimap'

srcdir= '.'
blddir= 'bin'

def set_options(ctx):
  ctx.tool_options('compiler_cxx')
    
def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.env.CXXFLAGS += ['-O2', '-Wall', '-g']

def build(bld):
  task1= bld(features='cxx cshlib',
       source       = 'fujimap.cpp fujimapBlock.cpp fujimapCommon.cpp bitVec.cpp keyEdge.cpp keyFile.cpp',
       name         = 'fujimap',
       target       = 'fujimap',
       includes     = '.')
  task2= bld(features='cxx cprogram',
       source       = 'fujimapMain.cpp smaz.cpp',
       target       ='fujimap',
       includes     = '.',
       uselib_local = 'fujimap')
  task3= bld(features='cxx cprogram',
       source       = 'fujimapTest.cpp',
       target       ='fujimapTest',
       includes     = '.',
       uselib_local = 'fujimap')
  task4= bld(features='cxx cprogram',
       source       = 'bitVecTest.cpp bitVec.cpp fujimapCommon.cpp',
       target       ='bitVecTest',
       includes     = '.'),
  task5= bld(features='cxx cprogram',
       source       = 'fujimapPerformanceTest.cpp',
       target       ='fujimapPerformanceTest',
       includes     = '.',
       uselib_local = 'fujimap'),
  task6= bld(features='cxx cprogram',
       source       = 'keyFile.cpp fujimapCommon.cpp keyFileTest.cpp',
       target       ='keyFileTest',
       includes     = '.')
  task7= bld(features='cxx cprogram',
       source       = 'encodeTest.cpp fujimapCommon.cpp',
       target       ='encodeTest',
       includes     = '.')
  bld.install_files('${PREFIX}/include/fujimap', bld.path.ant_glob('*.hpp'))
