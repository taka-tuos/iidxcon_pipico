# IIDX(INFINITAS Compatible)/BMS controller using Raspberry Pi Pico and TinyUSB

This based on modifications to the dev_hid_composite example from the
pico-examples repo.

## How to build

```bash
git clone https://github.com/taka-tuos/iidxcon_pipico
cd iidxcon_pipico
mkdir build
cd build
export PICO_SDK_PATH=../../pico-sdk
cmake ..
make -j4
```
