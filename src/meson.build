gnome = import('gnome')
cc = meson.get_compiler('c')

dronesimulation_sources = [
  'main.c',
  'dronesimulation-window.c',
]

dronesimulation_deps = [
  dependency('gtk+-3.0', version: '>=3.24'),
  dependency('champlain-gtk-0.12', version: '>= 0.12'),
  dependency('champlain-0.12', version: '>= 0.12'),
  dependency('clutter-1.0',version: '>=1.2'),
  dependency('clutter-gtk-1.0',version: '>=1.8'),

]
dronesimulation_deps += cc.find_library('m')

dronesimulation_sources += gnome.compile_resources('dronesimulation-resources',
  '../data/com.github.yiwenxue.dronesimulation.gresource.xml',
  source_dir: '../data',
  c_name: 'dronesimulation'
)

drone_plate_sources = ['plate.c']

drone_plate_deps = [
  dependency('cairo',version:'>=1.17'),
  ]

drone_plate_deps += cc.find_library('m')

executable('dronesimulation', dronesimulation_sources,
  dependencies: dronesimulation_deps,
  install: true,
)

executable('drone_plate',
  drone_plate_sources,
  dependencies: drone_plate_deps,
  install: true,
  )
