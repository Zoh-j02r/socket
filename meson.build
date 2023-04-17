project(
  'sockets',
  'c',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'c_std=c99'
  ]
)

curses = dependency('ncursesw')
#pthread = dependency('pthread')

executable('client', 'src/client.c', dependencies: curses)
executable('serial', 'src/serial_client.c', dependencies: curses)
executable('server','src/server.c')
