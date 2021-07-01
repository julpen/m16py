import serial
import time
import readline
import atexit
import os
histfile = ".history"

if os.path.exists(histfile):
    readline.read_history_file(histfile)

readline.set_history_length(400)
atexit.register(readline.write_history_file, histfile)

s = serial.Serial("/dev/ttyS4", 19200)
s.stopbits = 1
s.parity = serial.PARITY_EVEN
s.timeout = 0.2

while True:
    a = s.read(1)
    quiet = False
    while a:
        try:
            if ord(a) > 128: # Discard all the garbage we get while flashing
                quiet = True

            if not quiet:
                print(a.decode('utf-8'), end='')
        except:
            pass
        a = s.read(1)
    quiet = False

    try:
        cmd = bytes(input(">").encode('utf-8'))
    except (EOFError, KeyboardInterrupt):
        print("")
        break

    if cmd.split(b" ")[0] != b"load":
        s.write(cmd + b"\n")
    else:
        for fname in cmd.split(b" ")[1:]:
            try:
                with open(fname) as f:
                    a = f.readline()
                    while a:
                        print(a.rstrip())
                        while a:
                            s.write(bytes(a[0:10].encode('utf-8')))
                            time.sleep(0.01)
                            a = a[10:]

                        a = f.readline()
                    s.write(b'\n')
            except FileNotFoundError:
                print(f"File '{fname.decode('utf-8')}' not found")
                break

