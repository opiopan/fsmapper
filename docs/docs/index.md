---
sidebar_position: 1
---

# Introduction

**fsmapper** is a Windows application designed to connect a variety of input and output devices to flight simulators.
Originally, the development of fsmapper began to connect my DIY controller device, [**SimHID G1000**](https://github.com/opiopan/simhid-g1000), to FS2020. However, it has now evolved to encompass functionalities that allow the creation of a home cockpit using a wide range of devices.<br/>
fsmapper targets not only home cockpit builders aiming for an exact replica of a specific aircraft's cockpit,
but also general flight simulator players who have limited equipment and space but want to efficiently operate a wide range of aircraft
(The truth is, I fall into the latter category as well).

## What fsmapper enables

Although fsmapper offers a multitude of functions, it can be summarized into following three main aspects when viewed from the perspective of improving device and space utilization efficiency, as mentioned earlier.

- **Interaction between devices and FS2020**<br/>
  You can reflect the operations of various devices, including custom-made ones, onto various cockpit controls. 
  fsmapper utilize [Gauge API](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/Gauge_API.htm), allows support for operations not configurable within FS2020's **Controls options**. Additionally, it's possible to mirror the status of in-flight aircraft lamps and instrument panels onto physical devices.<br/>
  The interaction rules between devices and FS2020 can be automatically adjusted to suit different aircraft.

- **Changes to Input Device Characteristics**<br/>
  You can flexibly modify the characteristics and behaviors of input devices,
  such as altering the linearity of analog axes, changing polarity for alternate switch, generating events that account for the position and hysteresis of analog axes, and so on. 
  This allows for consistent handling of the same throttle device across aircraft with and without afterburners, or enables adjustments for actions like fuel cut-off and thrust reversal.

- **Virtual instrument panel utilizing Touchscreen Monitors**<br/>
  In a multi-monitor environment, you can construct virtual instrument panels on the screen. Particularly when using a touchscreen monitor, intuitive actions like tapping or flicking can be used to manipulate switches and knobs displayed on the screen.
  By configuring windows that pop out with **`[Right Alt]` + `[Mouse Click]`** as elements of the virtual instrument panel, constructing modern glass cockpits becomes seamless.<br/>
  fsmapper excels in handling FS2020's pop-out windows, allowing natural integration of these as part of the instrument panel by hiding title bars and window frames. It also offers workarounds for touch operation glitches inherent in FS2020's implementation.

If you watch the following video, you'll get a better understanding of what can be achieved with fsmapper as described above.

<div align="center" className="movie">
    <iframe
        width="100%" height="100%"
        src="https://www.youtube.com/embed/Ee6uw2BYdgE" 
        title="YouTube video player" frameborder="0" 
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen>
    </iframe>
</div>

In this video, A320's virtual instrument panels are assembled on a 10.5-inch touchscreen by combining six popup windows 
(FCU, PFD, ND, ECAM x 2, MCDU) and user-defined instruments and switches. 
Additionally, by dynamically switching and displaying the instrument panels,
it enables diverse information representation and manipulation even on a small screen.

## Script centric architecture

All the functionalities mentioned earlier are accessed through [Lua 5.4](https://www.lua.org/manual/5.4/) scripts.
Although fsmapper is implemented as a Windows GUI application, 
it actually only features a dashboard to display operational status and a console function to show messages during script execution,　particularly error messages.
If there wasn't a necessity to allow users to select FS2020's pop-out windows,　I believe it would have been implemented as a command-line interface (CLI).

:::info

Initially, in FS2020, when multiple instruments were popped out, they would default to displaying within a single window. 
Even when manipulated to display in separate windows, each window had same identical attributes such as window titles, making it impossible for the program to mechanically discern which window corresponded to which instrument.<br/>
On the other hand, fsmapper dynamically controls the visibility, size, position, and other aspects of these windows to treat them as components of a virtual instrument panel. When multiple instruments, such as the G1000's PFD and MFD, are popped out, fsmapper needs to identify which window corresponds to each instrument. Therefore, an interface was created where users can click on each window to select and match them with the respective PFD or MFD.

In 2023, FS2020 was improved to pop out individual windows with a unique window title with each **`[Right Alt]` + `[Mouse Click]`**. In response to this update, a feature was added to fsmapper to automatically capture these windows without requiring users to explicitly select them.

:::

import ThemedImage from '@theme/ThemedImage';
import useBaseUrl from '@docusaurus/useBaseUrl';

<p>
  <ThemedImage
    alt="fsmapper architecture image"
    sources={{
      light: useBaseUrl('/img/fsmapper-arch.svg'),
      dark: useBaseUrl('/img/fsmapper-arch-dark.svg'),
    }}
  />
</p>

The diagram above illustrates how fsmapper works. fsmapper patiently waits for events (***<span className="annotation_green">green arrows</span>***) such as operations from various input devices, touchscreen interactions, and changes in the aircraft's status within FS2020. Upon detecting an event, fsmapper executes the corresponding action. These actions are Lua function objects that allow interaction (***<span className="annotation_orange">orange arrows</span>***) with various objects such as aircraft controls within FS2020, graphical representation on the screen, and data output to devices, facilitated through Lua functions and Lua objects provided by fsmapper.

The '**Configuration File**' specified by the user during fsmapper execution refers to the definition of the correspondence between these events and actions ([***Event-Action mappings***](/guide/event-action-mapping)) as a Lua script.
