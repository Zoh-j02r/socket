### Socket 
This is a simple client-server communcation application using C sockets.
### Dependencies
Make sure to have ncurses develoment files (ncurses-dev)
### How to build
```
$ meson setup build
$ meson compile -C build
```
### How to use

```
./server <PORT> <NUM-CLIENTS>
./client <ADDRESS> <PORT> <CLIENT-NAME>
```
