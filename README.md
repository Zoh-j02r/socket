### Socket 

This is a simple client-server communcation application using C sockets, it was made for communcation with ONLY 2 devices in mind.
### How to build
```
$ meson setup build
$ meson compile -C build
```

### How to use

```
./server <PORT>
./client <ADDRESS> <PORT> <CLIENT-NAME>
```
