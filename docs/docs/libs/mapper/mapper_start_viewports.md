---
sidebar_position: 20
---

# mapper.start_viewports()
```lua
mapper.start_viewports()
```
This function activates all viewports and transitions them to a visible state.<br/>
Even if you call this function, the viewport will not be immediately visible as long as there is at least one view where the [`CapturedWindow`](/libs/mapper/CapturedWindow) element is placed. 
It will only become visible once all windows targeted by [`CapturedWindow`](/libs/mapper/CapturedWindow) are captured, or you press the `[Activate Viewports]` button on the dashboard.


## Return Values
This function doesn't return any value.