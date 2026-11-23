#!/usr/bin/env python3
"""
MSPM0G3507 BSL UART Flasher for 天猛星 board (CH340E)
Usage: python bsl_flash.py <com_port> <firmware.bin>

BSL entry method (天猛星):
  1. Hold BSL button (NOT release)
  2. Press and release RST button
  3. Release BSL button
  4. < 10 seconds window to flash >
"""
import sys
import struct
import time
import serial
import serial.tools.list_ports

# BSL Protocol Constants (MSPM0L/G device family)
BSL_HEADER = 0x80
CMD_CONNECTION = 0x12
CMD_GET_ID = 0x19
CMD_PASSWORD = 0x21
CMD_MASS_ERASE = 0x15
CMD_PROGRAM_DATA = 0x20
CMD_START_APP = 0x40

DEFAULT_PASSWORD = bytes([0xFF] * 32)  # 32 bytes of 0xFF = unlocked
FLASH_BASE = 0x00000000
MAX_DATA_PER_PACKET = 128
BSL_TIMEOUT_SEC = 10  # BSL mode timeout window


def crc32(data):
    """CRC32 (Ethernet polynomial) used by MSPM0 BSL"""
    crc = 0xFFFFFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            mask = -(crc & 1)
            crc = (crc >> 1) ^ (0xEDB88320 & mask)
    return crc


def bsl_pack(cmd_byte, data=b""):
    """Pack BSL command: [0x80][len:2LE][cmd+data][crc32:4LE]"""
    payload = bytes([cmd_byte]) + data
    length = len(payload)
    header = struct.pack("<BH", BSL_HEADER, length)
    checksum = crc32(payload)
    return header + payload + struct.pack("<I", checksum)


def bsl_send_and_ack(ser, pkt, desc="command"):
    """Send a BSL packet and check ACK (expects 0x00 first byte)"""
    ser.write(pkt)
    ack = ser.read(1)
    if ack == b'\x00':
        return True
    if ack:
        code = ack[0]
        errors = {0x51: "Header incorrect", 0x52: "Checksum incorrect",
                  0x53: "Packet size zero", 0x54: "Packet size too big",
                  0x55: "Unknown error", 0x56: "Unknown baud rate"}
        print(f"  ERROR [{desc}]: {errors.get(code, f'code 0x{code:02X}')}")
    else:
        print(f"  TIMEOUT [{desc}]: No response from device")
    return False


def bsl_check_status(ser):
    """Read 9-byte status response, returns status code or -1"""
    resp = ser.read(9)
    if len(resp) >= 9:
        return resp[8]
    return -1


STATUS_MSG = {
    0x00: "OK",
    0x01: "Flash program failed",
    0x02: "Mass erase failed",
    0x04: "BSL locked",
    0x05: "BSL password error",
    0x06: "Multiple password error",
    0x07: "Unknown command",
    0x08: "Invalid memory range",
    0x0B: "Factory reset disabled",
    0x0C: "Factory reset password error",
}


def bsl_flash(ser, firmware_data):
    """Full BSL flash sequence with retries"""
    # Step 1: Connection
    print("\n[1/6] Connecting to BSL...")
    for attempt in range(3):
        # Try with baud sync first, then without
        if attempt == 0:
            print("  Sending baud rate sync (0x55)...")
            ser.write(b'\x55')
            time.sleep(0.1)
        elif attempt == 1:
            print("  Retry with direct connection command...")

        pkt = bsl_pack(CMD_CONNECTION)
        ser.write(pkt)
        ack = ser.read(1)
        if ack == b'\x00':
            print("  Connection OK!")
            break
        elif ack:
            print(f"  ACK=0x{ack.hex()} (attempt {attempt+1})")
        else:
            print(f"  Timeout (attempt {attempt+1})")
    else:
        print("FAILED: Cannot connect to BSL after 3 attempts.")
        return False

    # Step 2: Confirm BSL mode
    print("\n[2/6] Confirming BSL mode...")
    ser.write(b'\xBB')
    resp = ser.read(1)
    if resp == b'\x51':
        print("  BSL mode confirmed (0x51)")
    else:
        print(f"  Note: BSL confirm response = 0x{resp.hex() if resp else 'TIMEOUT'} (continuing anyway)")

    # Step 3: Get Device ID
    print("\n[3/6] Get Device ID...")
    pkt = bsl_pack(CMD_GET_ID)
    ser.write(pkt)
    resp = ser.read(33)
    if len(resp) >= 33:
        print(f"  Device ID: {resp.hex()}")
    else:
        print(f"  Note: Short ID response ({len(resp)} bytes), continuing...")

    # Step 4: Unlock BSL
    print("\n[4/6] Unlocking BSL (default password)...")
    pkt = bsl_pack(CMD_PASSWORD, DEFAULT_PASSWORD)
    if not bsl_send_and_ack(ser, pkt, "Unlock"):
        status = bsl_check_status(ser)
        if status in STATUS_MSG:
            print(f"  Status: {STATUS_MSG[status]}")
            if status == 0x04:
                print("  BSL is locked! Need correct password or factory reset.")
            return False
        return False
    status = bsl_check_status(ser)
    if status == 0x00:
        print("  BSL unlocked successfully")
    elif status in STATUS_MSG:
        print(f"  WARN: Unlock status = {STATUS_MSG[status]}, continuing anyway...")

    # Step 5: Mass Erase
    print("\n[5/6] Mass Erasing...")
    pkt = bsl_pack(CMD_MASS_ERASE)
    if not bsl_send_and_ack(ser, pkt, "Mass Erase"):
        status = bsl_check_status(ser)
        if status in STATUS_MSG:
            print(f"  Status: {STATUS_MSG[status]}")
        return False
    status = bsl_check_status(ser)
    if status == 0x00:
        print("  Mass erase completed")
    elif status in STATUS_MSG:
        print(f"  WARN: Erase status = {STATUS_MSG[status]}, continuing anyway...")

    # Step 6: Program firmware
    print(f"\n[6/6] Programming {len(firmware_data)} bytes...")
    total = len(firmware_data)
    addr = FLASH_BASE
    offset = 0
    errors = 0

    while offset < total:
        chunk_size = min(MAX_DATA_PER_PACKET, total - offset)
        chunk = firmware_data[offset:offset + chunk_size]
        addr_bytes = struct.pack("<I", addr)
        pkt = bsl_pack(CMD_PROGRAM_DATA, addr_bytes + chunk)

        if bsl_send_and_ack(ser, pkt, f"Program @0x{addr:08X}"):
            status = bsl_check_status(ser)
            if status != 0x00:
                print(f"  ERROR: Program status = {STATUS_MSG.get(status, hex(status))}")
                errors += 1
                if errors > 3:
                    print("  Too many errors, aborting.")
                    return False
        else:
            errors += 1
            if errors > 3:
                print("  Too many errors, aborting.")
                return False

        offset += chunk_size
        addr += chunk_size

        if offset % (MAX_DATA_PER_PACKET * 8) == 0 or offset >= total:
            pct = offset * 100 // total
            print(f"  Progress: {pct}% ({offset}/{total} bytes)")

    print(f"  Programming complete! ({total} bytes, {offset // MAX_DATA_PER_PACKET + 1} packets)")

    # Step 7: Start Application
    print("\n=== Starting Application ===")
    pkt = bsl_pack(CMD_START_APP)
    ser.write(pkt)
    ser.read(1)  # Don't care about response
    print("  Application started!")

    return True


def detect_ch340_port():
    """Auto-detect CH340E COM port"""
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        desc = (port.description + " " + (port.hwid or "")).lower()
        if "ch340" in desc or "1a86" in desc:
            return port.device
    if ports:
        print("Available COM ports:")
        for p in ports:
            print(f"  {p.device}: {p.description}")
    return None


def main():
    if len(sys.argv) < 2:
        print("MSPM0G3507 BSL UART Flasher for 天猛星")
        print("Usage: python bsl_flash.py <com_port> [firmware.bin]")
        print("       python bsl_flash.py auto [firmware.bin]")
        sys.exit(1)

    com_port = sys.argv[1]
    firmware_path = sys.argv[2] if len(sys.argv) > 2 else "build/test_blink.bin"

    if com_port.lower() == "auto":
        com_port = detect_ch340_port()
        if not com_port:
            print("ERROR: Could not auto-detect CH340 COM port.")
            sys.exit(1)
        print(f"Auto-detected CH340: {com_port}")

    # Read firmware
    print(f"Firmware: {firmware_path}")
    with open(firmware_path, "rb") as f:
        firmware = f.read()
    print(f"Size: {len(firmware)} bytes")

    print(f"\nOpening {com_port} at 9600 baud...")
    ser = serial.Serial(com_port, baudrate=9600, timeout=2.0)

    try:
        # Instructions
        print("\n" + "=" * 55)
        print("  >>> BSL ENTRY SEQUENCE <<<")
        print("=" * 55)
        print("  1. HOLD the BSL button (keep it pressed)")
        print("  2. PRESS the RST button (about 1 sec)")
        print("  3. RELEASE the RST button")
        print("  4. RELEASE the BSL button")
        print("  5. You have ~10 seconds to flash!")
        print("=" * 55)
        print("\n" + "!" * 55)
        print("  DO THE BUTTON SEQUENCE NOW:")
        print("  1. HOLD BSL button")
        print("  2. PRESS + RELEASE RST button")
        print("  3. RELEASE BSL button")
        print("  Starting BSL in 3 seconds...")
        print("!" * 55)
        time.sleep(3)

        print("Starting BSL communication NOW...\n")

        success = bsl_flash(ser, firmware)

        if success:
            print("\n" + "=" * 55)
            print("  FLASH SUCCESS! LED1/LED2 should now blink")
            print("  Press RST or power-cycle to run the new firmware")
            print("=" * 55)
        else:
            print("\nFAILED: BSL flashing did not complete.")
            print("\nTroubleshooting:")
            print("  - Make sure BSL button is HELD before pressing RST")
            print("  - Try again quickly (10 sec window)")
            print("  - Check USB cable connection")

    finally:
        ser.close()


if __name__ == "__main__":
    main()
