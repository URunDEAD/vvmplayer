vvmplayer_sources = [
  'main.c',
  'vvmplayer-application.c',
  'vvmplayer-contact.c',
  'vvmplayer-contact-provider.c',
  'vvmplayer-debug-info.c',
  'vvmplayer-notification.c',
  'vvmplayer-preferences-window.c',
  'vvmplayer-settings.c',
  'vvmplayer-voicemail-window.c',
  'vvmplayer-vvmd.c',
  'vvmplayer-window.c',
]

vvmplayer_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version : '>=1.2.0'),
  dependency('libebook-contacts-1.2'),
  dependency('libebook-1.2'),
  dependency('gstreamer-1.0', version : '>1.16'),
  dependency('gstreamer-pbutils-1.0', version : '>1.0'),
  dependency('libcallaudio-0.1'),
]

revision_tag = vcs_tag(
  input: 'version.h.in',
  output: 'version.h',
)
vvmplayer_deps += declare_dependency(sources: revision_tag)

vvmplayer_sources += gnome.compile_resources('vvmplayer-resources',
  'vvmplayer.gresource.xml',
  c_name: 'vvmplayer'
)

executable('vvmplayer', vvmplayer_sources,
  dependencies: vvmplayer_deps,
       install: true,
)
