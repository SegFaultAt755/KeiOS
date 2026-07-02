#!/bin/sh
set -e

# Check if disk exist, otherwise create it
if [ ! -f "./disk.qcow2" ]; then
    qemu-img create -f qcow2 disk.qcow2 2G
fi

qemu-system-x86_64 -enable-kvm \
    -cpu host -m 1G -smp 1 \
    -drive if=pflash,format=raw,readonly=on,file=./qemu-uefi/OVMF_CODE.4m.fd \
    -drive if=pflash,format=raw,file=./qemu-uefi/OVMF_VARS.4m.fd \
    -drive file=disk.qcow2,if=virtio \
    -cdrom ./keios.iso
