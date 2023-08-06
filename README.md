# ng-audio-backend

A backend compatible with [NG Audio Bridge](https://www.newgrounds.com/portal/view/553053) indexing newgrounds audio files.

## Build

To build this project, you need the following dependencies:

* [libpistache](https://github.com/pistacheio/pistache/)
* jsoncpp
* sqlite3

Build use `cmake`:

```sh
$ mkdir build
$ cd build
$ cmake <path/to/ng-audio-backend> [VARIABLES]
$ make
# make install
```

* possible `cmake` variables:

| Variable                            | Default | Description                                                     |
| ----------------------------------- | ------- | --------------------------------------------------------------- |
| NG\_AUDIO\_COMPATIBILITY\_ENDPOINTS | OFF     | Compile compatibility endpoints for old flash app compatibility |

## Network Modification

* You need to be an administrator to do this modification.

To use it with the old flash application on your machine, you have to add the following lines to your `/etc/hosts` :
```
127.0.0.1	gumonshoe.net
127.0.0.1	www.gumonshoe.net
```

Host file path are:

For GNU/Linux: `/etc/hosts`

For MacOS: `/private/etc/hosts`

For Windows: `%WINDIR%\\System32\\Drivers\\etc\\hosts`

## Functionality

* Fully Compatible with [NG Audio Bridge](https://www.newgrounds.com/portal/view/553053)
* Custom API more RESTful compliant

## Incomming

* Crawler to update Tracks periodicly


