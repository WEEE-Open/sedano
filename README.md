# S.E.D.A.N.O.
**Scanner Elevato Dall'Ambiente nostrano organizzato.**  
Sedano is a small utility written in C that interfaces with the barcode scanner we use in our laboratory.

## How does it work?
The scanner sends string representation of the barcode over RS232 serial communication (19200 baud, 8 bits per character, one stop bit, no parity, no handshaking) as a stream of ASCII characters delimited by the `0x02` and `0x03` markers (respectively for the start and end of the string).

This utility collects that string and types it into the currently selected input field simulating a series of keystroke that get sent to the X server.

## How to compile?
```sh
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
For now just as any other utility - directly executing it from the command line with root privileges (needed to access the serial device):
```sh
sudo bin/release
```