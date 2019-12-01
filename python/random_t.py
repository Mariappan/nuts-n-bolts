#!/usr/bin/env python3
import random
import time

rand = random.randint(0, 100) % 19
print(f"Sleeping for {rand} seconds")
time.sleep(rand)
