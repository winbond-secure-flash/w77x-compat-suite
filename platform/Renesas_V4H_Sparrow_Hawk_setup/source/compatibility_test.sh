#!/bin/sh

# --------------------------------------------------------------------------------------------------------- #
#  Winbond Electronics Corporation                                                                          #
#                                                                                                           #
#                                                                                                           #
#  Copyright (c) 2026 by Winbond Electronics Corporation                                                    #
#  All rights reserved                                                                                      #
#                                                                                                           #
# --------------------------------------------------------------------------------------------------------- #


# Winbond compatibility test for Renesas V4H Sparrow Hawk board
overall=0
echo "Starting compatibility test for Winbond flash on Renesas V4H Sparrow Hawk board"

# Verify run on Renesas V4H Sparrow Hawk board
echo "Checking board model..."
if grep -q "Retronix Sparrow Hawk board" /proc/device-tree/model; then echo OK; else echo FAIL; overall=1; fi

# Verify mtd1 is “user” partition
echo "Checking mtd1 \"user\" partition..."
if [ "$(cat /sys/class/mtd/mtd1/name)" = "user" ]; then echo OK; else echo FAIL; overall=1; fi

# Verify first 4KB on mtd1 are erased (0xFF), if not then STOP and ask the user if to continue.
echo "Checking first 4KB of /dev/mtd1 are erased (0xFF)..."

# Read 4KB and check for any non-FF bytes
if dd if=/dev/mtd1 bs=4096 count=1 2>/dev/null | hexdump -v -e '1/1 "%02X"' | grep -qv '^FF*$'; then
    echo "WARNING: First 4KB NOT fully erased (not all 0xFF)."

    read -p "Do you want to continue anyway? (yes/no): " ans
    case "$ans" in
        yes|y|Y)
            echo "Erase 4KB..."
            ./flash_erase /dev/mtd1 0 1
            ;;
        *)
            echo "STOPPED by user."
            exit 1
            ;;
    esac
else
    echo "OK: First 4KB fully erased (all 0xFF)."
fi

# Create pattern.bin file with 256B data (0x00, 0x01, 0x02, .. 0xFF)
echo "Creating pattern.bin with 256B data (0x00, 0x01, 0x02, .. 0xFF)"
for i in $(seq 0 255); do printf "\\x$(printf %02x $i)"; done > pattern.bin

# Verify pattern.bin was created
if [ ! -f "pattern.bin" ]; then
    echo "pattern.bin does not exist"
    overall=1
fi

# Write pattern.bin into 256B page to mtd1 address 0x00
echo "Writing 256B pattern.bin to /dev/mtd1..."
if dd if=pattern.bin of=/dev/mtd1 bs=256 count=1 conv=notrunc 2>/dev/null; then
    echo "SUCCESS"
else
    echo "FAILED"
    overall=1
fi

# Read 256B from mtd1 address 0x00
echo "Reading 256B from /dev/mtd1 address 0x00..."
if dd if=/dev/mtd1 of=readpattern.bin bs=256 count=1 2>/dev/null; then
    echo "SUCCESS"
else
    echo "FAILED"
    overall=1
fi

# Compare read and write (pattern.bin & readpattern.bin)
echo "Comparing read and write files..."
if cmp -s readpattern.bin pattern.bin; then
    echo "OK: read&write files are identical"
else
    echo "ERROR: read&write files differ"
    overall=1
fi

# Revert flash mtd1 to its previous state - Erase sector 0
echo "Reverting flash mtd1 to its previous state - Erasing sector 0..."
./flash_erase /dev/mtd1 0 1

# print PASS/FAIL
if [ "$overall" -eq 0 ]; then
    echo ""
    echo "   PPPPPPP        A          SSSSSSS      SSSSSSS  "
    echo "   P      P      A A        SS     SS    SS     SS "
    echo "   P      P     A   A         SS           SS      "
    echo "   PPPPPP      AAAAAAA          SS           SS    "
    echo "   P          A       A           SS           SS  "
    echo "   P         A         A    SS     SS    SS     SS "
    echo "   P        A           A    SSSSSSS      SSSSSSS  "
    echo ""
else
    echo ""
    echo "   FFFFFFFF       A         IIIII    L        "
    echo "   F             A A          I      L        "
    echo "   F            A   A         I      L        "
    echo "   FFFFFFF     AAAAAAA        I      L        "
    echo "   F          A       A       I      L        "
    echo "   F         A         A      I      L        "
    echo "   F        A           A   IIIII    LLLLLLLL "
    echo ""
fi
echo "Test done"