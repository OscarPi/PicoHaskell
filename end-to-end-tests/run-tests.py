#!/usr/bin/env python3

import os
import glob
import sys
import subprocess
import serial

def run_command(command):
    print()
    print("************************* " + " ".join(command))
    print()
    completed_process = subprocess.run(command)
    return completed_process.returncode

test_programs = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test-programs")
src_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
image_maker = os.path.join(src_dir, "image-maker", "image-maker.py")
build_dir = os.path.join(src_dir, "build")
picohaskell = os.path.join(build_dir, "src", "picohaskell")
assembly = os.path.join(os.path.dirname(os.path.abspath(__file__)), "run.S")
elf = os.path.join(os.path.dirname(os.path.abspath(__file__)), "image.elf")

if not os.path.exists(build_dir):
    os.mkdir(build_dir)

return_code = run_command(["cmake", "-S", src_dir, "-B", build_dir])
if return_code != 0:
    print("Failed to run cmake.")
    sys.exit(1)

return_code = run_command(["make", "-C", build_dir, "picohaskell"])
if return_code != 0:
    print("Failed to run make.")
    sys.exit(1)

test_files = glob.glob(os.path.join(test_programs, "*.hs"))
results = {}

for f in test_files:
    results[os.path.basename(f)[:-3]] = "not run"

ser = serial.Serial("/dev/serial0", 115200, timeout=10)
for f in test_files:
    test_name = os.path.basename(f)[:-3]

    return_code = run_command([picohaskell, "-i", f, "-o", assembly])
    if return_code != 0:
        results[test_name] = "failed to compile"
        continue

    return_code = run_command([image_maker, "-e", elf, assembly])
    if return_code != 0:
        results[test_name] = "failed to build ELF"
        continue

    return_code = run_command(["openocd", "-f", "interface/raspberrypi-swd.cfg", "-f", "target/rp2040.cfg", "-c", "init", "-c", "reset halt", "-c", "exit"])    
    if return_code != 0:
        print("Failed to reset Pico.")
        break

    ser.reset_input_buffer()

    return_code = run_command(["openocd", "-f", "interface/raspberrypi-swd.cfg", "-f", "target/rp2040.cfg", "-c", "program " + elf + " verify reset exit"])
    if return_code != 0:
        results[test_name] = "failed to flash ELF"
        continue

    result = ser.readline().strip().decode().replace("\x00", "")

    result_file = f[:-3] + ".txt"

    try:
        with open(result_file, "r") as r:
            expected_result = r.readline().strip()
            if result == expected_result:
                results[test_name] = "success"
            else:
                results[test_name] = "unexpected result: expected \"" + expected_result + "\", but got \"" + result + "\""
    except:
        results[test_name] = "failed to read expeted result"
        continue

print()
print("Results:")
for name, result in results.items():
    print(name + "\t\t" + result)
