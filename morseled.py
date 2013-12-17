import tty
import sys
from termios import tcgetattr, tcsetattr, TCSAFLUSH
import time

import RPi.GPIO as gpio

from morsedict import morsedict

PIN = 7
DIT = 50  # ms

gpio.setmode(gpio.BOARD)
gpio.setup(PIN, gpio.OUT)
gpio.output(PIN, False)


old = None
try:
    if sys.stdin.isatty():
        old = tcgetattr(sys.stdin)
        tty.setraw(sys.stdin)
    print('proceed\r')

    while True:
        input = sys.stdin.read(1)
        if input == '\x1b':
            input = '\x7f'
        elif input == '\0':
            break
        morse = morsedict[input]
        print(repr(input), morse, '\r')
        for c in morse:
            gpio.output(PIN, True)
            if c == '.':
                time.sleep(DIT / 1000)
            elif c == '-':
                time.sleep(DIT * 3 / 1000)
            else:
                time.sleep(DIT / 5 / 1000)
            gpio.output(PIN, False)
            time.sleep(DIT / 1000)
        time.sleep(DIT * 2 / 1000)
            

finally:
    if old is not None:
        gpio.output(PIN, False)
        tcsetattr(sys.stdin, TCSAFLUSH, old)
        print()
        print('restored')
