---
sidebar_position: 3
---

# a32nx.lua

## Location
[```samples/practical/a32nx.lua```](https://github.com/opiopan/fsmapper/blob/main/samples/practical/a32nx.lua)

## Description
<img alt="a32nx.lua" src={require('./images/a32nx.jpg').default} width="400" align="right"/>

This script was written for [FlyByWire A32NX](https://flybywiresim.com).<br/>
This is more complex than the above two scripts. This script provides the integrated virtual cockpit which contains physical operable units such as buttons and knobs, touch controllable units on the monitor, and poped out window contents such as PFD.<br/>
This script includes several [sub-module scripts](https://github.com/opiopan/fsmapper/tree/main/samples/practical/a32nx), and it also uses [several bitmap images](https://github.com/opiopan/fsmapper/tree/main/samples/practical/assets) to render each operable units.


In this scrit, you can see the following example usage of fsmapper in addition to [```g1000_x56.lua```](g1000_x56) case.
- Observing the aircraft status represented by [RPN script](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm)
- Drawing graphics on the screen
- Handling the touch events and mouse events occured on the screen

This script defines following 5 views. Every view consist FS2020 poped out window and self rendered operable gauges.

|| Poped out windows | Self rendered operable units
|-|-----------------|----------------------
|1| PFD<br/>FCU       | buttons on the FCU<br/>BARO indicator
|2| ND               | buttons and toggle switches on the EFIS controll unit<br/>auto brake buttons<br/>etc
|3| Upper ECAM | engine mode selectors, master engine switch and engine status indicators<br/>APU controll switches<br/>battery controll switches and voltage indicators
|4|Lower ECAM| ECAM page selector buttons
|5|MCDU| MCDU buttons

The width of every view is a half width of the SimHID monitor width. So two views can be displayed simultaneously side by side.<br/>
By pressing soft-keys placed at bottom of SimHID G1000, two views to show can be specified.<br/>
And the switches placed on the left end and the right end of SimHID G1000 housing can also control views to show.

![a32nx scripting](images/a32nx_script_desc.svg)

The pushbuttons rendered on each view can be operated by tapping them. The toggle switches include engine master switches and the selector knobs can be operated by flicking them.
The correspondence between SimHID G1000 operations and A32NX operations are shown below.

| SimHID G1000      | A32NX 
|-------------------|---------
|NAV volume knob    |SPD/Mac knob on FCU except pull operation
|NAV swap button    |Pulling Speed Knob on FUC
|HDG knob           |HDG/TRK knob on FCU except pull operation
|HDG button         |Pulling HDG/TRK knob on FCU
|AP button          |AP1 button on FCU
|FD button          |FD button on EFIS
|APR button         |APPR button on FCU
|ALT outer knob     |Increase / Decrease altitute indicator on FCU by 1000 feet
|ALT inner knob     |Increase / Decrease altitute indicator on FCU by 100 feet
|ALT knob push button|Pushing ALT knob on FCU
|FLC button         |Pulling ALT knob on FCU
|COM volume knob    |V/S or FPA knob on FCU except pull operation
|COM swap button    |Pulling V/S or FPA knob on FCU
|COM knob           |Freequency selector knob on RMP panel
|BARO knob rotation |Manipuration of the value displayed in the barometer reference window
|BARO knob push button|Switching barometer reference mode between QNH and standard barometer reference
|Range knob rotation|Range selector switch on EFIS
|Range knob joystick|Changing the mode displayed on the respective ND
|NAV inner knob     |Adjusting the brightness of PFD, ND, upper ECAM, and lower ECAM in sync
|NAV outer knob     | Adjusting the brightness of both the weather rader image and the EGPWS terrain image on ND
|FMS inner knob     | Adjusting the brightness of FCU display
|FMS outer knob     | Adjusting the brightness of all glareshields in sync
