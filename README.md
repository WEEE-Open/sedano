# S.E.D.A.N.O.
**Scanner Elevato Dall'Ambiente nostrano organizzato.**  
Sedano is a small utility written in C that interfaces with the barcode scanner we use in our laboratory.

## How does it work?
The scanner sends string representation of the barcode over RS232 serial communication (19200 baud, 8 bits per character, one stop bit, no parity, no handshaking) as a stream of ASCII characters delimited by the `0x02` and `0x03` markers (respectively for the start and end of the string).

This utility collects that string and types it into the currently selected input field simulating a series of keystroke that get sent to the X server.

## How to compile?
```shell script
# Compile the program
# Requisites: gcc and make
make

# Clean the environment to a pristine state
make clean

# Only build release binary
make release

# Only build debug binary
make debug
```

Object files are put into the `obj` subfolder (`obj/dbg` for the debug counterpart) and the binaries are found in `bin/release` and `bin/debug`. The default configuration adds debug information readable by GDB.

## How to use?
For now just as any other utility - directly executing it from the command line:

> NOTE: In order for the program to function, it must be able to access the specified file (for example `/dev/ttyS0`). For this to happen the program has to either be run as root (**higly discouraged**) or the user must be part of the group that owns that file, which can be seen from the command line with `ls -la /path/to/file`.

```shell script
bin/release --device /path/to/file
```

The program has a series of available command line options:

| Argument             | Meaning                                                          |
|----------------------|------------------------------------------------------------------|
| `--device [path]`    | Specifies path of the scanner file                               |
| `--terminator [id]`  | Prints a terminator after the string (see the following section) |
| `--loglevel [level]` | Specifies log level                                              |
| `--delay [seconds]`  | Delay in seconds to wait before writing after a read             |
| `--loopback`         | Enables loopback mode                                            |
| `--nosetserial`      | Skips serial parameters initialization                           |
| `--quiet`            | Suppresses **ALL** errors (including fatals)                     |
| `--help`             | Shows an usage page                                              |

### Terminator
Terminators are special keys sent to the graphical server after the scanned string. Their usage can help improve the workflow (for example automatically pressing `tab` to switch to the next field after filling in one). The following terminators are currently available:

* `NONE`
* `ENTER`
* `TABULATION`
* `SPACE`

The names are pretty self-explanatory. The default is `NONE`.

### Loopback mode
Loopback mode disregards the scanner and asks for barcodes directly on the command line. It's primarly a debug feature used to debug code interacting with the X server that bypasses the need to always have the scanner at disposal for development purposes.

### No-set-serial
This flag prevents the program from setting up the serial communication's parameters, like baudrate, parity, number of stop bits and so on. Primarily intended to debug issues with the serial communication and find the correct list of parameters.

### Log levels
* 0: Debug messages
* 1: Informational messages
* 2: Warnings
* 3: Non-fatal errors (don't cause the program to terminate)
* 4: Fatal messages (cause the program to terminate)