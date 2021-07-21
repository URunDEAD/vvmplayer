vvmplayer_sources = [
  'main.c',
  'vvmplayer-contact.c',
  'vvmplayer-contact-provider.c',
  'vvmplayer-preferences-window.c',
  'vvmplayer-settings.c',
  'vvmplayer-voicemail-window.c',
  'vvmplayer-vvmd.c',
  'vvmplayer-window.c'
]

vvmplayer_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libebook-contacts-1.2'),
  dependency('libebook-1.2'),
  dependency('libhandy-1', version: '>= 1.0.0'),
  dependency('gstreamer-1.0', version : '>1.16'),
  dependency('gstreamer-pbutils-1.0', version : '>1.0')
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
