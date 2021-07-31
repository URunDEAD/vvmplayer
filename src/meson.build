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
  'vvmplayer-log.c',
  'vvmplayer-utils.c',
  'vvmplayer-notification.c',
  'vvmplayer-preferences-window.c',
  'vvmplayer-settings.c',
  'vvmplayer-window.c',
  'vvmplayer-voicemail-window.c',
  'vvmplayer-vvmd.c',
  'vvmplayer-contact.c',
  'vvmplayer-contact-provider.c',
]

libvvmplayer = both_libraries(
  'vvmplayer', libsrc,
  install: false,
  include_directories: top_inc,
  dependencies: pkg_dep,
)

ui_files = files(
  'resources/gtk/help-overlay.ui',
  'resources/ui/vvmplayer-window.ui',
  'resources/ui/vvmplayer-preferences-window.ui',
  'resources/ui/vvmplayer-voicemail-window.ui',
)

gtk_builder_tool = find_program('gtk-builder-tool', required: false)
if gtk_builder_tool.found()
  # Separate multiple libraries by ‘:’ eg.: 'LD_PRELOAD=@0@:libhandy-1.so'...
  preload_env = 'LD_PRELOAD=@0@'.format(libvvmplayer.get_shared_lib().full_path())
  foreach file: ui_files
    test('Validate @0@'.format(file), gtk_builder_tool,
         env: [preload_env],
         args: ['validate', file])
  endforeach
endif

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
