
# Device Trees
Copyright (C) 2023, by John Ryland
All rights reserved


On a number of embedded architectures / SoCs, besides x86, device trees are
often used as opposed to something like ACPI.

A device tree is typically a binary blob that is passed to the bootloader
on boot, which can pass it on to the OS, such as Linux. This binary blob
contains information similar to ACPI with which devices are on the board
and how they can be accessed / used.

Overview of the format:

https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html

specifics:

https://devicetree-specification.readthedocs.io/en/stable/device-bindings.html


Working with device-trees with qemu:

https://u-boot.readthedocs.io/en/latest/develop/devicetree/dt_qemu.html


Seems on macOS with brew installed, have a dtc executable for working with
device tress.


