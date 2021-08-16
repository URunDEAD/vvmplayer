src = []

revision_tag = vcs_tag(
  input: 'version.h.in',
  output: 'version.h',
)
pkg_dep += declare_dependency(sources: revision_tag)

resources = gnome.compile_resources(
  'resources',
  'resources/vvmplayer.gresource.xml',
  source_dir: 'resources',
  c_name: '_vvmplayer',
)

libsrc = [
  'vvmplayer-contact.c',
  'vvmplayer-contact-provider.c',
  'vvmplayer-log.c',
  'vvmplayer-notification.c',
  'vvmplayer-preferences-window.c',
  'vvmplayer-settings.c',
  'vvmplayer-utils.c',
  'vvmplayer-voicemail-window.c',
  'vvmplayer-vvmd.c',
  'vvmplayer-window.c',
]

libvvmplayer = both_libraries(
  'vvmplayer', libsrc,
  install: false,
  include_directories: top_inc,
  dependencies: pkg_dep,
)

ui_files = files(
  'resources/ui/vvmplayer-window.ui',
  'resources/ui/vvmplayer-preferences-window.ui',
  'resources/ui/vvmplayer-voicemail-window.ui',
)

src += [
  'main.c',
  'vvmplayer-application.c',
  revision_tag,
  resources,
]

executable(
  'vvmplayer',
  src,
  install: true,
  gui_app: true,
  link_args: c_link_args,
  link_with: libvvmplayer.get_static_lib(),
  include_directories: top_inc,
  dependencies: pkg_dep,
)
