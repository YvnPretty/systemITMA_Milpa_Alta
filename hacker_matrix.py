#!/usr/bin/env python3
"""
Hacker Matrix Rain Simulator
Pure Python terminal Matrix animation using ANSI colors.
Press Ctrl+C to exit.
"""

import random
import sys
import time
import shutil
import os

# Matrix characters (Katakana, numbers, symbols)
CHARS = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'X', 'Y', 'Z',
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    'ｱ', 'ｲ', 'ｳ', 'ｴ', 'ｵ', 'ｶ', 'ｷ', 'ｸ', 'ｹ', 'ｺ',
    'ｻ', 'ｼ', 'ｽ', 'ｾ', 'ｿ', 'ﾀ', 'ﾁ', 'ﾂ', 'ﾃ', 'ﾄ'
]

# ANSI Colors
GREEN_BRIGHT = "\033[92;1m"
GREEN_DIM = "\033[32m"
WHITE_BRIGHT = "\033[97;1m"
RESET = "\033[0m"
CLEAR_SCREEN = "\033[2J\033[H"
HIDE_CURSOR = "\033[?25l"
SHOW_CURSOR = "\033[?25h"

class MatrixColumn:
    def __init__(self, x, max_y):
        self.x = x
        self.max_y = max_y
        self.reset()

    def reset(self):
        self.y = random.randint(-self.max_y, 0)
        self.length = random.randint(8, max(12, self.max_y - 2))
        self.speed = random.choice([1, 1, 2])
        self.counter = 0

    def step(self):
        self.counter += 1
        if self.counter >= self.speed:
            self.counter = 0
            self.y += 1
            if self.y - self.length > self.max_y:
                self.reset()

def main():
    cols, rows = shutil.get_terminal_size((80, 24))
    columns = [MatrixColumn(x, rows) for x in range(0, cols, 2)]

    sys.stdout.write(CLEAR_SCREEN + HIDE_CURSOR)
    sys.stdout.flush()

    try:
        while True:
            # Check terminal resize
            new_cols, new_rows = shutil.get_terminal_size((80, 24))
            if new_cols != cols or new_rows != rows:
                cols, rows = new_cols, new_rows
                columns = [MatrixColumn(x, rows) for x in range(0, cols, 2)]
                sys.stdout.write(CLEAR_SCREEN)

            buffer = {}

            for col in columns:
                col.step()
                head_y = col.y
                if 0 <= head_y < rows:
                    buffer[(col.x, head_y)] = (WHITE_BRIGHT, random.choice(CHARS))
                
                body_y = head_y - 1
                if 0 <= body_y < rows:
                    buffer[(col.x, body_y)] = (GREEN_BRIGHT, random.choice(CHARS))

                tail_y = head_y - col.length
                if 0 <= tail_y < rows:
                    buffer[(col.x, tail_y)] = (RESET, ' ')

                # Fill middle body chars
                for y in range(max(0, head_y - col.length + 1), max(0, head_y - 1)):
                    if 0 <= y < rows and random.random() < 0.15:
                        buffer[(col.x, y)] = (GREEN_DIM, random.choice(CHARS))

            # Render buffer changes
            output = []
            for (x, y), (color, char) in buffer.items():
                output.append(f"\033[{y+1};{x+1}H{color}{char}")
            
            sys.stdout.write("".join(output))
            sys.stdout.flush()
            time.sleep(0.04)

    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(SHOW_CURSOR + RESET + "\n")
        sys.stdout.flush()

if __name__ == "__main__":
    main()
