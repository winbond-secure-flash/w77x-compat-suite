```
Initializing flash device...
deviceID = 0x18
JEDEC manufacturerID = 0xEF, memoryType = 0x8E, capacity = 0x19 (32768 KB)
Read HW_VER using OP1 and OP2 commands
HW_VER = 0x0F083510
Detected Target is W77T25NW
Octal bus mode is supported
Read SSR register
SSR = 0xF2000060
SSR Read passed
Device was initialized successfully
Transport test started...
Test Single bus mode: 1s-1s-1s
Frequency is set to 133 MHz
Execute erase sector command
Using opcode 0x20
Execute page program command
Program Pattern 0x5A
Using opcode 0x02
Execute fast read page command
Using opcode 0x0B
Verify Data Pattern 0x5A
Verify Successfully
Test Quad bus mode: 1s-4s-4s
Frequency is set to 166 MHz
Execute fast read page command
Using opcode 0xEB
Verify Data Pattern 0x5A
Verify Successfully
Test Octal bus mode: 1s-8s-8s
Frequency is set to 166 MHz
Execute fast read page command
Using opcode 0xCB
Verify Data Pattern 0x5A
Verify Successfully
Test Octal SOPI bus mode: 8s-8s-8s
Execute fast read page command
Using opcode 0xCB
Verify Data Pattern 0x5A
Verify Successfully
Test Octal DOPI bus mode: 8d-8d-8d
Frequency is set to 200 MHz
Execute fast read page command
Using opcode 0xCD
Verify Data Pattern 0x5A
Verify Successfully
Frequency is set to 133 MHz
Transport test was completed successfully
```