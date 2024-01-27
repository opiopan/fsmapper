---
id: sample_index
---

# Sample Scripts
The fsmapper package contains several configuration script samples for practical use of [SimHID G1000](https://github.com/opiopan/simhid-g1000). These scripts can be found at ```samples/practical``` folder under the instration folder of fsmapper.<br/>
To use these scripts, [vJoy driver](https://sourceforge.net/projects/vjoystick) and [MobiFlight WASM module](https://github.com/MobiFlight/MobiFlight-WASM-Module) must be installed. In addition, it's assumed that the virtual serial Port for SimHID G1000 is recognaized as **COM3** and the display for SimHID G1000 is secondary monitor (**moniter No. is 2**).<br/>
If your environment is not same, change ```config``` table defined at the top of each script as below according to your environment.

``` Lua
local config = {
    simhid_g1000_identifier = {path = "COM3"},
    simhid_g1000_display = 2,
}
```

Please refer to the following for details on each script.

- [**g1000.lua**](/samples/g1000)
- [**g1000_x56.lua**](/samples/g1000_x56)
- [**a32nx.lua**](/samples/a32nx)
- [**c172.lua**](/samples/c172)

:::info
More complex scripts can be found in [this GitHub repository](https://github.com/opiopan/scripts_for_fsmapper). <br/>
I actually use the scripts stored in that repository(```config.lua```), and the way how to switch the configuration correspond to current aircraft can be found in those scripts.
:::
