### dde-grand-search



### Dependencies


### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_
 
* pkg-config


## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if dde-grand-search is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got dde-grand-search delivered:

``` shell
$ apt build-dep dde-grand-search
```

2. Build:
```
$ cd dde-grand-search
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-grand-search`

## Usage

Execute `dde-grand-search`

## Documentations

 
## Getting help



## Getting involved

We encourage you to report issues and contribute changes


## License

dde-grand-search is licensed under [GPLv3](LICENSE)
