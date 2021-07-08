vvmplayer_sources = [
  'main.c',
  'vvmplayer-window.c',
]

vvmplayer_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
]

gnome = import('gnome')

vvmplayer_sources += gnome.compile_resources('vvmplayer-resources',
  'vvmplayer.gresource.xml',
  c_name: 'vvmplayer'
)

executable('vvmplayer', vvmplayer_sources,
  dependencies: vvmplayer_deps,
  install: true,
)
