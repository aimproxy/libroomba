project('roomba', 'c',
  version : '1.0.0',
  license : 'GPLv3')

cc = meson.get_compiler('c')
lib_wolfssl = cc.find_library('libwolfssl',
               dirs : ['/usr/local/lib/'])

lib_json = dependency('json-c', method: 'pkg-config')

inc = include_directories('include')

subdir('include')

sources = [
  'src/libroomba.c'
]

libroomba = shared_library(
  meson.project_name(),
  sources,
  include_directories : inc,
  dependencies: [
    lib_wolfssl,
    lib_json
  ],
  install: true
)

subdir('examples')
