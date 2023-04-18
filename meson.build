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
threads = dependency('threads')

executable('client', 'src/client.c', dependencies: [curses,threads])
executable('server','src/server.c',dependencies:threads)
