### W77Q/T Compatibility Tests  
W77Q/T Secure Flash devices are fully compatible with the standard Flash instruction set used in the W25Q family of devices.<br>
This test suite demonstrates this compatibility across multiple platforms.

The platforms covered by this test suite are:
* NXP i\.MX RT1050
* NXP i\.MX RT685
* STM STM32H7S78-DK
* STM STM32N6570-DK

HW and FW integration steps for each platform are in the links below:

[For further information on the NXP1050  setup, click here](platform/NXP1050_setup/docs/README.md)  
[For further information on the NXP685   setup, click here](platform/NXP685_setup/docs/README.md)  
[For further information on the STM32H7  setup, click here](platform/STM32H7_setup/docs/README.md)  
[For further information on the STM32N6  setup, click here](platform/STM32N6_setup/docs/README.md)  
  
### Test Suite Directory Structure
```
📁compatibility_tests_suite
 │
 ├── 📁licenses
 │    └── STM Software license agreement.txt
 │
 ├── 📁common_sources                        ; API abstruction layer
 │    ├── 📁common_board_api                 ; API abstruction layer
 │    ├── 📁CompatibilityTests               ; This folder contains the functions for the compatibility test flow
 │    └── 📁defs                             ; Contains SW general definitions
 │
 ├── 📁platform
 │    ├── 📁NXP685_setup
 │    │    ├── 📁_settings                   ; MCUXpresso IDE configuration files
 │    │    ├── 📁docs                        ; This folder contains the NXP685 specific documentation, including README.md & images
 │    │    ├── 📁logs                        ; This folder contains the NXP685 test logs
 │    │    ├── 📁nxp_rt685_bsp               ;
 │    │    ├── 📁nxp_rt685_spi_bridge        ;
 │    │    └── 📁source                      ; This folder contains the NXP685 main() & supporting functions
 │    │
 │    ├── 📁NXP1050_setup
 │    │    ├── 📁_settings                   ; MCUXpresso IDE configuration files
 │    │    ├── 📁docs                        ; This folder contains the NXP1050 specific documentation, including README.md & images
 │    │    ├── 📁logs                        ; This folder contains the NXP1050 test logs
 │    │    ├── 📁nxp_10xx_hal                ;
 │    │    └── 📁source                      ; This folder contains the NXP1050 main() & supporting functions
 │    │
 │    ├── 📁STM32H7_setup
 │    │    ├── 📁_settings                   ; IDE configuration files
 │    │    ├── 📁docs                        ; This folder contains the STM32H7 specific documentation, including README.md & images
 │    │    ├── 📁logs                        ; This folder contains the STM32H7 test logs
 │    │    ├── 📁stm32h7_board_extension
 │    │    ├── 📁stm32h7_bsp
 │    │    └── 📁source                      ; This folder contains the STM32H7 main() & supporting functions
 │    │
 │    └── 📁STM32N6_setup
 │         ├── 📁_settings                   ; IDE configuration files
 │         ├── 📁docs                        ; This folder contains the STM32N6 specific documentation, including README.md & images
 │         ├── 📁logs                        ; This folder contains the STM32N6 test logs
 │         ├── 📁stm32n6_board_extension
 │         ├── 📁stm32n6_bsp
 │         └── 📁source                      ; This folder contains the STM32N6 main() & supporting functions
 │
 ├── LICENSE.txt
 │
 ├── README.md                               ; This readme file
 │
 └── THIRD_PARTY_LICENSES.txt                ; contains the 3rd party license references
```