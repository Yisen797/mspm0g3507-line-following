#!/usr/bin/env python3
"""
pyOCD-based MSPM0G3507 flasher via ST-Link (WinUSB driver required)
Usage: python pyocd_flash.py build/test_blink.hex
"""
import sys, os, struct, ctypes

# === STEP 0: Force libusb backend BEFORE any pyOCD imports ===
dll_path = r"D:/openocd_mspm0/xpack-openocd-0.12.0-3/bin/libusb-1.0.dll"
if not os.path.exists(dll_path):
    print(f"ERROR: libusb DLL not found at {dll_path}")
    sys.exit(1)

# Load the DLL explicitly
ctypes.cdll.LoadLibrary(dll_path)

# Now set up the patch EARLY
import usb.backend.libusb1 as libusb1_mod
_orig = libusb1_mod.get_backend

def _get_backend(find_library=None):
    return _orig(find_library=lambda x: dll_path)

libusb1_mod.get_backend = _get_backend

# Force early USB initialization
import usb.core
usb.core.find(idVendor=0x0483)

# === Now import pyOCD ===
from pyocd.core.helpers import ConnectHelper
from pyocd.flash.loader import FlashLoader
from pyocd.core.session import Session
from pyocd.probe.stlink_probe import StlinkProbe
from pyocd.flash.file_programmer import FileProgrammer

FIRMWARE = sys.argv[1] if len(sys.argv) > 1 else "build/test_blink.hex"

print(f"=== pyOCD MSPM0G3507 Flasher ===")
print(f"Firmware: {FIRMWARE}")

# List probes
probes = StlinkProbe.get_all_connected_probes()
print(f"Probes found: {len(probes)}")
for p in probes:
    print(f"  {p.unique_id}: {p.vendor_name} {p.product_name}")

if len(probes) == 0:
    print("\nERROR: No ST-Link probe detected!")
    print("Make sure:")
    print("  1. ST-Link USB is plugged in")
    print("  2. Zadig replaced driver with WinUSB")
    print("  3. ST-Link SWDIO, SWCLK, GND, 3V3 connected to MSPM0")
    sys.exit(1)

# Connect
print("\nConnecting to MSPM0G3507...")
try:
    session = ConnectHelper.session_with_chosen_probe(
        target_override="mspm0g3507",
        unique_id=probes[0].unique_id,
        connect_mode="attach",
    )
    session.open()
except Exception as e:
    print(f"Connect error: {e}")
    sys.exit(1)

board = session.board
target = board.target
print(f"Target: {target.part_number}")
print(f"Core state: {target.get_state()}")

# Halt and flash
print("\nHalting CPU...")
target.halt()
print(f"PC = 0x{target.read_core_register(15):08X}")

print("\nProgramming flash...")
programmer = FileProgrammer(session)
programmer.program(FIRMWARE, auto_erase=True)

# Verify
print("\nVerifying...")
programmer.verify(FIRMWARE)

print("\nResetting...")
target.reset_and_halt()
print("Flash complete! Press RST on MSPM0 board to run.")

session.close()
