<h2><u>STM32H7S78DK MB1736D platform instructions</u></h1>
<br>
<br>
<h4 { style="margin-bottom: 0px;" }><u>HW integration</u></h4>
<br>
<img src="images/STM32H7_top_side.jpg" width="600"><br>
<br>
<ol margin-top="0px"; margin-bottom="0px"; margin-left="0"; type="a"><b><u>Prepare the HW as follows:</u></b>
  <li> Remove the on board U23 device (see the Yellow circule)</li>
  <li> Replace with Winbond device (for example W77T64NWS)</li>
  <li>Connect a USB-C cable to the board's STLINK port on one side (see the green arrow above)</li>  
  <br>
  <div><img src="images/STM32H7-bootSwitch.png" width="300"></div><br>
  <li>Make sure theat the mechanical switch <strong>BOOT0 (SW1)</strong> is set to <strong>0</strong> position as in the above image</li>
</ol>

<br>
<u><h4 { style="margin-bottom: 0px;" }>SW integration</u></h4>
<ol>
  <li>Installation steps:
  <ol  style="list-style-type: lower-alpha;"; margin-top="0px"; margin-bottom="0px"; margin-left="0";>
    <li>Install the STM32CubeIDE - version 2.0.0 Build: 26820_20251114_1348</li>
    <Li>Install the STMCubeIDE terminal console or any other terminal tool</li>
    <li>Clone the Winbond's Compatibility tests repository</li>
    <li>Load the Project</li>
    <li>Compile</li>
    <li>Run the test</li>
  </ol></li><br>

  <Li>Installation steps details:
  <ol style="list-style-type: lower-alpha;"; margin-top="0px"; margin-bottom="0px"; margin-left="0">
    <li><u><b>Install the STM32CubeIDE - version 2.0.0 Build: 26820_20251114_1348</b></u><br>
    The test was run on the version & build as mentioned above, but it may run on other versions</li><br>
    <li><u><b>Install the STMCubeIDE terminal console or any other terminal tool</b></u><br>
    This step is optional, you can skip this step if you only want a pass/fail indication by the LEDs or when using external terminal tool.<br>
    The test was run on the STM32H7S78DK with the following serial connection settings "115200bps 8N1".<br><br>
    <u>Following are the steps for installing the IDE's terminal console</u>
     <ol  style="list-style-type: decimal;">
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
      <li> Now, create a new Console and configure to accept the STM32H7 log transmissions<br>
      <div><img src="images/install_the_console_terminal8.png" width="400"></div></li><br>
      <li> Select <strong>Command Shell Console</strong> and fill the parameters as shown in the image<br>
      <div><img src="images/install_the_console_terminal9.png" width="400"></div><br>
      <li> select <strong>New</strong> verify the serial port parameters as in the image <br>
      <div><img src="images/install_the_console_terminal10.png" width="700"></div></li><br>
      <li>Connect the STLink port of the board via a USB-C cable to the host computer and the comm port wlil be automatically filled.<BR>
      Name the connection and click Finish and then click OK.<BR>
      <div><img src="images/install_the_console_terminal11.png" width="500"></div></li>
      <li>Switch between the different console contents (build-log / run-log /  <strong>STM32H7S78DK output</strong>) by clicking on the blue arrow<br>
      <div><img src="images/install_the_console_terminal12.png" width="400"></div></li><br>
      </ol>
    </li>
    <li><u><b>Clone the Winbond's Compatibility tests repository</b></u><br>
    The project was created using the STM32CubeMX GUI (with STM32Cube FWH7RS V1.2.0, toolchain/IDE=stmCubeIDE, enable XSPI2 & UART4 & <strong>generate the code</strong> to create the project).<br>
    SW changes that were made include</strong>:<br>
    <ol  style="list-style-type: decimal"; >
      <li>stm32h7_bsp\Boot\Inc\stm32h7rsxx_hal_conf.h      // HAL configuration file</li>
      <li>stm32h7_bsp\Boot\Inc\stm32h7rsxx_it.h            // Interrupt handlers header file</li>
      <li>stm32h7_bsp\Boot\Src\stm32h7rsxx_it.c            // Interrupt handlers</li>
      <li>stm32h7_bsp\Boot\Src\stm32h7rsxx_hal_msp.c       // HAL MSP module</li>
      <li>stm32h7_bsp\Boot\Src\system_stm32h7rsxx.c        // STM32H7RSxx system source file</li>
      <li>The SystemClock_Config() function is used to configure the system clock (SYSCLK) to run at 600 MHz</li>
    </ol></li>
    <br>
    <li><u><b>Load the project</u></b>
    <ol style="list-style-type: decimal;">
      <li>In the IDE click on [File]->Import Projects from File System or Archive
      <div><img src="images/file-importProject.png" width="400"></div></li><br>
      <li>click on the <strong>Directory</strong>
      <div><img src="images/file-importProject-selectFolder.png" width="600"></div></li><br>
      <li>At the opened GUI, Browse to the root folder of the suite and click <strong>Select Folder</strong>
      <div><img src="images/select_the_root_of_the_suite.png" width="600"></div></li><br>
      <li>un-select all but  the STM32H7_setup checkboxes, then, click on the <strong>Finish</strong>
      <div><img src="images/file-importProject-finish.png" width="600"></div></li><br>
    </ol></li>
    <br>
    <li><u><b>Compile</b></u><br>
      Right-click on the project -> Build Configurations -> Set Active -> Debug as the active build,  
      then, (at the above associative menu) choose <strong>Build Project</strong> or  <strong>[CTRL]+B</strong> or <strong>project->Build All</strong><br>
      <img src="images/selectActiveBuild.png" width="300">
    </li>
    <br>
    <li><u><b>Run the test</b></u><br>
    <ol  style="list-style-type: decimal;" >
      <li>connect the board's STLink port to the host computer</li>
      <li>Opitonal for the internal console terminal - to receive the log messages switch the view <strong>command shell console</strong> to the desired connection name.
      <li>Click the Bug in Debug mode with breakpoint capability, or, click the right-pointing triangle to Run
        <div><img src="images/run_the_test.png" "width="80"></div></li><br>
      <li>The test result sets either the green (test pass) or RED (test fail) LEDs which are located at the bottom side of the board.</li><br>
      <li>Observe the green or red LEDs
        <div><img src="images/STM32H7_bottom_side_test_pass.jpg" width="500"></div><br>
        <div>Example for test pass indication</div>
        <div><img src="images/STM32H7_bottom_side_zoom_on_leds_test_pass.jpg" width="200"></div><br>
        <div>Example for test fail indication</div>
        <div><img src="images/STM32H7_bottom_side_zoom_on_leds_test_fail.jpg" width="200"></div><br>
      </li>
      <li>Compare the test result to the log files which are located under the logs/ folder</li><br>
    </ol>
    </li>
  </ol>
  </li>
</ol>

###### (*)Note: The mentioned IDE and TERATERM versions have been validated by Winbond. Different IDE and TERATERM versions may work as well.
---
[← Return to Parent](../../../README.md)