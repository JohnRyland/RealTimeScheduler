#!/bin/bash

QEMU_TEST_DEVICES="\
    pci-bridge \
    pcie-pci-bridge \
    pcie-root-port \
    usb-host \
    usb-hub \
    ich9-usb-ehci1 \
    ich9-usb-ehci2 \
    ich9-usb-uhci1 \
    ich9-usb-uhci6 \
    nec-usb-xhci \
    pci-ohci \
    piix3-usb-uhci \
    piix4-usb-uhci \
    qemu-xhci \
    usb-ehci \
    floppy \
    ich9-ahci \
    ide-cd \
    ide-hd \
    nvme \
    piix4-ide \
    sd-card \
    usb-storage \
    i8042 \
    ipoctal232 \
    isa-parallel \
    isa-serial \
    pci-serial \
    pci-serial-2x \
    pci-serial-4x \
    tpci200 \
    usb-mouse \
    usb-serial \
    usb-tablet \
    usb-wacom-tablet \
    virtconsole \
    virtio-keyboard-pci \
    virtio-mouse-pci \
    virtio-tablet-pci \
    vmmouse \
    ati-vga \
    bochs-display \
    cirrus-vga \
    isa-cirrus-vga \
    isa-vga \
    ramfb \
    secondary-vga \
    VGA \
    virtio-gpu-device \
    virtio-gpu-pci \
    virtio-vga \
    vmware-svga \
    AC97 \
    adlib \
    gus \
    hda-duplex \
    hda-micro \
    hda-output \
    ich9-intel-hda \
    intel-hda \
    sb16 \
    usb-audio \
    amd-iommu \
    AMDVI-PCI \
    i2c-ddc \
    i2c-echo \
    intel-iommu \
    smbus-ipmi \
    virtio-balloon-device \
    virtio-balloon-pci \
    virtio-balloon-pci-non-transitional \
    virtio-balloon-pci-transitional \
    virtio-crypto-device \
    virtio-crypto-pci \
    virtio-iommu-device \
    virtio-iommu-pci \
    virtio-pmem-pci \
    virtio-rng-device \
    virtio-rng-pci \
    virtio-rng-pci-non-transitional \
    virtio-rng-pci-transitional \
    vmcoreinfo \
    vmgenid \
    486-x86_64-cpu \
    athlon-x86_64-cpu \
    base-x86_64-cpu \
    Broadwell-x86_64-cpu \
    Cascadelake-Server-x86_64-cpu \
    Conroe-x86_64-cpu \
    Cooperlake-x86_64-cpu \
    core2duo-x86_64-cpu \
    coreduo-x86_64-cpu \
    Denverton-x86_64-cpu \
    Dhyana-x86_64-cpu \
    EPYC-x86_64-cpu \
    GraniteRapids-x86_64-cpu \
    Haswell-x86_64-cpu \
    Icelake-Server-x86_64-cpu \
    IvyBridge-x86_64-cpu \
    KnightsMill-x86_64-cpu \
    kvm64-x86_64-cpu \
    max-x86_64-cpu \
    n270-x86_64-cpu \
    Nehalem-x86_64-cpu \
    Opteron_G5-x86_64-cpu \
    Penryn-x86_64-cpu \
    pentium-x86_64-cpu \
    pentium2-x86_64-cpu \
    pentium3-x86_64-cpu \
    phenom-v1-x86_64-cpu \
    phenom-x86_64-cpu \
    qemu32-x86_64-cpu \
    qemu64-x86_64-cpu \
    SandyBridge-x86_64-cpu \
    SapphireRapids-x86_64-cpu \
    Skylake-Client-x86_64-cpu \
    Skylake-Server-x86_64-cpu \
    Snowridge-x86_64-cpu \
    Westmere-x86_64-cpu \
    pc-dimm \
    pci-ipmi-bt \
    pci-ipmi-kcs \
    "

for dev in $(echo ${QEMU_TEST_DEVICES})
do
  echo "Device: -$dev-"
  qemu-system-x86_64 -device $dev -serial stdio bin/image.qcow2 > tmp.txt
  diff -du orig.txt tmp.txt > devs/$dev.txt
done


