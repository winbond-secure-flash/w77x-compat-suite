<h4 { style="margin-bottom: 0px;" }><u>NXP IMXRT685-EVKB platform instructions</u></h4>
<img src="images/IMX-RT600-EVK-TOP.png" width="700"><br>
Replace the on board flash device (marked with a yellow circle) with a Winbond W77T Octal SPI device in a 24-Ball 8x6-mm TFBGA package.<br><br>

<h4 { style="margin-bottom: 0px;" }><u>SW integration</u></h4>
<ol>
	<li>SW Installation (*):
	<ol type="a">
		<li>NXP IDE - MCUXpresso IDE v25.6 [Build 136][2025-06-27]</li>
		<li>SDK_2.x_EVKB-IMXRT685 version 25.09.00 (epluginsite815 2025-09-22) Manifest Version 3.15.0</li>
		<li>Clone  Winbond's Compatibility tests repository
	</ol></li>
	<br>
	<li>Load the project:
	<ol type="a">
		<li>Import the project:
		<div><img src="images/file-importProject.png" width="400"></div></li><br>
		<li>Select the folder which contains the .cproject
		<div><img src="images/file-importProject-selectFolder.png" width="600"></div></li><br>
		<li>click Finish
		<div><img src="images/file-importProject-finish.png" width="500"></div></li><br>
	</ol></li>
</ol>

<h4 { style="margin-bottom: 0px;" }><u>Compile</u></h4>
<ol>
   <li>Right-click on the project -> Build Configurations -> Set Active -> Debug_NXP685 as the active build,
   <div><img src="images/selectActiveBuild.png" width="400"></div></li><br>
   <li>then, (at the above associative menu) choose Build Project   or   [CTRL]+B   or project->Build All
   <div><img src="images/BuildAll.png" width="200"></div></li><br>
</ol>

<h4 { style="margin-bottom: 0px;" }><u>Run</u></h4>
<ol>
	<li>Click the link server
	<div><img src="images/runInDebugMode.png" width="500"></div></li><br>
	<li>Observe the test result on the output panel</li>
	<li>Compare the test result to the log file. the logs are located inside the logs/ folder
	<div><img src="images/logsFolder.png" width="100"></div></li><br>
</ol>
  
(*)Note: The mentioned IDE and SDK versions were validated by Winbond to work with this setup. Other IDE and SDK versions may also work.

---
[← Return to Parent](../../../README.md)