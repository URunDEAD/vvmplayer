vvmplayer_sources = [
  'main.c',
  'vvmplayer-application.c',
  'vvmplayer-preferences-window.c',
  'vvmplayer-window.c',
]

vvmplayer_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1'),
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
