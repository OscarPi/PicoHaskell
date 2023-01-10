#!/usr/bin/env python3

import argparse
import os
import shutil
import sys
import subprocess

parser = argparse.ArgumentParser(
    prog = "image-maker",
    description = "Build an ELF and/or UF2 image from the assembly code output by the picohaskell compiler.")

parser.add_argument("assembly_file", metavar="assembly-file", help="the path to an assembly file output by picohaskell")
parser.add_argument("-u", "--output-uf2", help="where to place the output UF2 image", required=False)
parser.add_argument("-e", "--output-elf", help="where to place the output ELF image", required=False)

args = parser.parse_args()

pico_sdk_project = os.path.join(os.path.dirname(os.path.abspath(__file__)), "pico-sdk-project")
assembly_location =  os.path.join(pico_sdk_project, "run.S")
build_dir = os.path.join(pico_sdk_project, "build")
elf_file = os.path.join(build_dir, "PicoHaskell.elf")
uf2_file = os.path.join(build_dir, "PicoHaskell.uf2")

try:
    shutil.copyfile(args.assembly_file, assembly_location)
except:
    print("Failed to access assembly file.")
    sys.exit(1)

if not os.path.exists(build_dir):
    os.mkdir(build_dir)

completed_process = subprocess.run(["cmake", "-S", pico_sdk_project, "-B", build_dir])

if completed_process.returncode != 0:
    print("Failed to run cmake.")
    sys.exit(1)

completed_process = subprocess.run(["make", "-C", build_dir])

if completed_process.returncode != 0:
    print("Failed to run make.")
    sys.exit(1)

if args.output_uf2 is not None:
    try:
        shutil.copyfile(uf2_file, args.output_uf2)
    except:
        print("Failed to copy UF2 file to specified destination.")
        sys.exit(1)

if args.output_elf is not None:
    try:
        shutil.copyfile(elf_file, args.output_elf)
    except:
        print("Failed to copy ELF file to specified destination.")
        sys.exit(1)

print("Success.")
