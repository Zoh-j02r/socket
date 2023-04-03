### Whatsapp Premium 

This is a simple client-server communcation application using C sockets, it was made for communcation with ONLY 2 devices in mind (pointless trackers and privacy disturbing functionality not yet implemented).
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
