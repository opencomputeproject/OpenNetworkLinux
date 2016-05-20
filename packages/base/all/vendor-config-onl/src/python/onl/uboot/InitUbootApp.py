"""InitUbootApp.py

Initialize the fw_env.config file

See
https://developer.ridgerun.com/wiki/index.php/Setting_up_fw_printenv_to_modify_u-boot_environment_variables

"""

PATH = "/etc/fw_env.config"

import sys
import onl.platform.current

platform = onl.platform.current.OnlPlatform()

d = platform.platform_config
if 'flat_image_tree' not in d:
    raise ValueError("missing flat_image_tree section, probably not U-Boot")
d = d.get('loader', {})
d = d.get('environment', [])
if not d:
    raise ValueError("missing or empty loader.environment config")

with open(PATH, "w") as fd:
    fd.write("# device env_offset env_size sector_size sector_count\n")
    for m in d:

        dev = m.get('device', None)
        if not dev:
            raise ValueError("missing device key in environment settings")

        off = m.get('env_offset', None)
        if off is None:
            raise ValueError("missing env_offset key in environment settings")

        sz = m.get('env_size', None)
        if not sz:
            raise ValueError("missing env_size key in environment settings")

        ssz = m.get('sector_size', None)
        if not ssz:
            raise ValueError("missing sector_size key in environment settings")

        ns = m.get('sector_count', None)
        if ns is None:
            ns = sz / ssz
            if ns == 0: ns = 1

        fd.write("%s 0x%x 0x%x 0x%x %d\n"
                 % (dev, off, sz, ssz, ns,))

sys.exit(0)
