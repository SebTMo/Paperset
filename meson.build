project('paperset', 'c',
  version: '0.1.0',
  meson_version: '>= 0.62.0',
  default_options: [ 'warning_level=2' ],
)

# 1. Imports
gnome = import('gnome')

# 2. Dependencies
gtk_dep = dependency('gtk4')
adwaita_dep = dependency('libadwaita-1')

# 3. Executable
executable('paperset',
  'src/main.c',
  dependencies: [gtk_dep, adwaita_dep],
  install: true
)

# 4. Install Desktop File
install_data('data/com.sebtech.paperset.desktop',
  install_dir: get_option('datadir') / 'applications'
)

# 5. Install the Icon
# This puts your icon in /usr/local/share/icons/hicolor/scalable/apps/
install_data('data/com.sebtech.paperset.png',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
)

# 6. Optional: Post-install cache refresh (GNOME standard)
gnome.post_install(
  gtk_update_icon_cache: true,
  update_desktop_database: true,
)
