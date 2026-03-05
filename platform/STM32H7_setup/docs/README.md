## STM32H7S78DK MB1736D platform instructions

<img src="images/STM32H7_top_side.jpg" width="700"><br>


<br>
<u><h4 { style="margin-bottom: 0px;" }>SW integration</u></h4>
<ol>
  <li>Install the following tool chain:
  <ol margin-top="0px"; margin-bottom="0px"; margin-left="0"; type="a">
    <li>STM32CubeIDE - version 2.0.0 Build: 26820_20251114_1348</li>
    <li>Clone  Winbond's Compatibility tests repository and combine into the project</li>
  </ol></li><br>

<Li> Installation of the terminal console (one time operation after installing the IDE)<br>
    <ol margin-top="0px"; margin-bottom="0px"; margin-left="0"; type="a">
      <li> Click on Menu->Help<br>
      <div><img src="images/install_the_console_terminal1.png" width="700"></div></li><br>
      Click "Install new Software.."<br>
      <li> Select from the combo-box the item <strong>Eclipse SimRel 2024-09 - https://download.eclipse.org/releases/2024-09</strong><br>
      <div><img src="images/install_the_console_terminal2.png" width="700"></div></li><br>
      <li> Wait for few minutes while the available addons are being collected<br>
      <div><img src="images/install_the_console_terminal3.png" width="700"></div></li><br>
      <li> In the Filter line (above the populated list) write terminal<br>
      <div><img src="images/install_the_console_terminal4.png" width="700"></div></li><br>
      <li> Click on the checkbox of <strong>TM Terminal Connector Extensions</strong> and then, Click <strong>Next</strong>
      <div><img src="images/install_the_console_terminal5.png" width="700"></div></li><br>      
      <li>Click <strong>Finish</strong><br>
      <div><img src="images/install_the_console_terminal6.png" width="700"></div></li><br>
      <li> At the bottom right corner you wlil see the installation progress<br>
      <div><img src="images/install_the_console_terminal7.png" width="400"></div></li><br>
      <li> Once the installation is done, you will be prompetd to restat the IDE - Do it</li><br>
      <li> Now, create a new Console and configure to accept UART4 transmissions<br>
      <div><img src="images/install_the_console_terminal8.png" width="400"></div></li><br>
      <li> Select <strong>Command Shell Console</strong> and fill the parameters as shown in the image<br>
      <div><img src="images/install_the_console_terminal9.png" width="400"></div><br>
      <li> select <strong>New</strong> verify the serial port parameters as in the image <br>
      <div><img src="images/install_the_console_terminal10.png" width="700"></div></li><br>
      <li>Connect the STMLNK port of the board via a USB-C cable to the host computer and the comm port wlil be automatically filled.<BR>
      Name the connection and click Finish and then click OK.<BR>
      <div><img src="images/install_the_console_terminal11.png" width="500"></div></li>
      <li>Switch between the different console contents (build-log / run-log /  <strong>UART4 output</strong>) by clicking on the blue arrow<br>
      <div><img src="images/install_the_console_terminal12.png" width="400"></div></li><br>
    </ol><br>

<li>The project was created using the STM32CubeMX GUI (with STM32Cube FWH7RS V1.2.0, toolchain/IDE=stmCubeIDE, enable XSPI2 & UART4 & <strong>generate the code</strong> to create the project).<br>
SW changes that were made include</strong>:<br>
  <ol type="a">
    <li>stm32h7_bsp\Boot\Inc\stm32h7rsxx_hal_conf.h      // HAL configuration file</li>
    <li>stm32h7_bsp\Boot\Inc\stm32h7rsxx_it.h            // Interrupt handlers header file</li>
    <li>stm32h7_bsp\Boot\Src\stm32h7rsxx_it.c            // Interrupt handlers</li>
    <li>stm32h7_bsp\Boot\Src\stm32h7rsxx_hal_msp.c       // HAL MSP module</li>
    <li>stm32h7_bsp\Boot\Src\system_stm32h7rsxx.c        // STM32H7RSxx system source file</li>
    <li>The SystemClock_Config() function is used to configure the system clock (SYSCLK) to run at 600 MHz</li>
  </ol></li>
<br>
</ol>
<br>
<u><h4 { style="margin-bottom: 0px;" }>Load the project</u></h4>
<ol>
  <li>In the IDE click on [File]->Import Projects from File System or Archive
  <div><img src="images/file-importProject.png" width="400"></div></li><br>
  
  <li>click on the <strong>Directory</strong>
  <div><img src="images/file-importProject-selectFolder.png" width="600"></div></li><br>
  
  <li>At the opened GUI, Browse to the root folder of the suite and click <strong>Select Folder</strong>
  <div><img src="images/select_the_root_of_the_suite.png" width="600"></div></li><br>

  <li>un-select all but  the STM32H7_setup checkboxes, then, click on the <strong>Finish</strong>
  <div><img src="images/file-importProject-finish.png" width="600"></div></li><br>
</ol>
<br>
<u><h4 { style="margin-bottom: 0px;" }>Compile</h4></u>
Right-click on the project -> Build Configurations -> Set Active -> Debug as the active build,  
   then, (at the above associative menu) choose <strong>Build Project</strong> or  <strong>[CTRL]+B</strong> or <strong>project->Build All</strong><br>
   <img src="images/selectActiveBuild.png" width="300"><br>
<br>
<u><h4 { style="margin-bottom: 0px;" }>Run the test</h4></u>
<ol>
  <li>connect the board's STMLNK port to the host computer</li>
  <li>To receive log messages switch the console display to uart4-output (see above in the console terminal installation)

<li>Run the test:  
<ol>
  <li>Make sure theat the mechanical switch <strong>BOOT0 (SW1)</strong> is set to <strong>0</strong> position</li>
  <li>Connect a USB-C cable to the board's STLINK port on one side</li>
  <li>Click the Bug in Debug mode with breakpoint capability, or, click the right-pointing triangle to Run
  <div><img src="images/run_the_test.png" "width="80"></div></li><br>
  <li>The test result enables either the green (test pass) or RED (test fail) LEDs which are located at the bottom side of the board.</li><br>
  <li><strong>Optionally</strong>, (in the case that the teraterm is running and configured)
  <ol>
    <li>Observe the test result on the output panel</li>
    <li>Compare the test result to the log files which are located under the logs/ folder</li><br>
  </ol></li>

  <li>Observe the green and red LEDs (located at the lower left corner of the board as shown below) which are configured to signal the user whether the test has passed (green LED on) or failed (red LED on)
  <div><img src="images/STM32H7_bottom_side_test_pass.jpg" width="500"></div><br>

  <div>Example for test pass indication</div>
  <div><img src="images/STM32H7_bottom_side_zoom_on_leds_test_pass.jpg" width="200"></div><br>
  
  <div>Example for test fail indication</div>
  <div><img src="images/STM32H7_bottom_side_zoom_on_leds_test_fail.jpg" width="200"></div><br>
</li></ol>

###### (*)Note: The mentioned IDE and TERATERM versions have been validated by Winbond. Different IDE and TERATERM versions may work as well.
---
[← Return to Parent](../../../README.md)