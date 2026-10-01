CONFIG_NUM_CORES=1
CONFIG_USE_TAP_NETWORK=true

MICROKIT_PATH=../vendor/microkit-sdk
MICROKIT_BOARD=qemu_virt_aarch64_el1
MICROKIT_CONFIG=debug
MICROKIT_BOARD_DIR=$(MICROKIT_PATH)/board/$(MICROKIT_BOARD)/$(MICROKIT_CONFIG)
MICROKIT_TOOL=$(MICROKIT_PATH)/bin/microkit
TOOLCHAIN_CPU=cortex-a53
TOOLCHAIN_PREFIX=aarch64-linux-gnu
ARCH=aarch64
CC=$(TOOLCHAIN_PREFIX)-gcc
LD=$(TOOLCHAIN_PREFIX)-ld
AS=$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY=$(TOOLCHAIN_PREFIX)-objcopy
SDDF=../vendor/sddf
RANLIB=ranlib
SDDF_CUSTOM_LIBC=1

VIRTIO_DISCOVER=../vendor/VirtioDiscover-Aarch64-Loader-Off0x70000000.img

SDDF_INCLUDE=-I../vendor/sddf/include \
             -I../vendor/sddf/include/microkit/ \
             -I../vendor/sddf/network/ipstacks/lwip/src/include \
             -Ilwip/include

CFLAGS=-nostdlib -ffreestanding -g -Wall -Wextra -mstrict-align \
       -I$(MICROKIT_BOARD_DIR)/include -DBOARD_$(MICROKIT_BOARD) \
       $(SDDF_INCLUDE) -Ivendor/sddf/network/ipstacks/lwip/src/include\
       -Wno-unused-function -Wno-unused-parameter -Ibuild/lwip/include \
       -Wno-sign-compare -Wno-implicit-function-declaration -O2
LDFLAGS=-L$(MICROKIT_BOARD_DIR)/lib -L.

LIBS=--start-group -lmicrokit -Tmicrokit.ld -lsddf_util_debug --end-group
LIBS_NODEBUG=-start-group -lmicrokit -Tmicrokit.ld -lsddf_util --end-group

SDDF_BIN=serial_driver.elf \
         serial_virt_rx.elf \
         serial_virt_tx.elf \
         eth_driver_virtio.elf \
         network_copy.elf \
         network_virt_rx.elf \
         network_virt_tx.elf \
         timer_driver.elf

MICROKIT_FLAGS =system.system
MICROKIT_FLAGS+=--search-path ./build
MICROKIT_FLAGS+=--board $(MICROKIT_BOARD)
MICROKIT_FLAGS+=--config $(MICROKIT_CONFIG)

include ../vendor/sddf/util/util.mk

include ../vendor/sddf/drivers/serial/arm/serial_driver.mk
include ../vendor/sddf/serial/components/serial_components.mk

include ../vendor/sddf/drivers/network/virtio/mmio/eth_driver.mk
include ../vendor/sddf/network/components/network_components.mk
include ../vendor/sddf/network/lib_sddf_lwip/lib_sddf_lwip.mk

include ../vendor/sddf/drivers/timer/arm/timer_driver.mk

ECHOSERVER_OBJ=echoserver/echoserver.o \
               echoserver/raw_socket.o

echoserver/%.o: ../echoserver/%.c
	$(CC) -c $(CFLAGS) $< -o $@

echoserver.elf: $(ECHOSERVER_OBJ) libsddf_util.a lib_sddf_lwip.a
	$(LD) $(ECHOSERVER_OBJ) -o echoserver.elf $(LDFLAGS) -l_sddf_lwip $(LIBS_NODEBUG) $(LIBC_OBJ)

WEBSERVER_OBJ=webserver/webserver.o \
              webserver/fs.o \
              webserver/httpd.o

webserver/%.o: ../webserver/%.c
	$(CC) -c $(CFLAGS) $< -o $@

webserver/%.o: ../vendor/sddf/network/ipstacks/lwip/src/apps/http/%.c
	$(CC) -c $(CFLAGS) $< -o $@

webserver.elf: $(WEBSERVER_OBJ) libsddf_util.a lib_sddf_lwip.a
	$(LD) $(WEBSERVER_OBJ) -o webserver.elf $(LDFLAGS) -l_sddf_lwip $(LIBS_NODEBUG) $(LIBC_OBJ)

webserver.img: $(SDDF_BIN) webserver.elf ../meta.py qemuvirt.dtb
	python3 ../meta.py core_config/$(CONFIG_NUM_CORES).json webserver
	$(OBJCOPY) --update-section .device_resources=serial_driver_device_resources.data serial_driver.elf
	$(OBJCOPY) --update-section .serial_driver_config=serial_driver_config.data serial_driver.elf
	$(OBJCOPY) --update-section .serial_virt_tx_config=serial_virt_tx.data serial_virt_tx.elf
	$(OBJCOPY) --update-section .serial_virt_rx_config=serial_virt_rx.data serial_virt_rx.elf
	$(OBJCOPY) --update-section .net_driver_config=net_driver.data eth_driver_virtio.elf
	$(OBJCOPY) --update-section .device_resources=eth_driver_device_resources.data eth_driver_virtio.elf
	$(OBJCOPY) --update-section .net_virt_tx_config=net_virt_tx.data network_virt_tx.elf
	$(OBJCOPY) --update-section .net_virt_rx_config=net_virt_rx.data network_virt_rx.elf
	$(OBJCOPY) --update-section .net_copy_config=net_copy_network_copy.data network_copy.elf
	$(OBJCOPY) --update-section .device_resources=timer_driver_device_resources.data timer_driver.elf
	$(OBJCOPY) --update-section .serial_client_config=serial_client_netserver.data webserver.elf
	$(OBJCOPY) --update-section .timer_client_config=timer_client_netserver.data webserver.elf
	$(OBJCOPY) --update-section .net_client_config=net_client_netserver.data webserver.elf
	$(OBJCOPY) --update-section .lib_sddf_lwip_config=lib_sddf_lwip_config_netserver.data webserver.elf
	$(MICROKIT_TOOL) $(MICROKIT_FLAGS) -o webserver.img -r webserver.txt

echoserver.img: ../meta.py qemuvirt.dtb $(SDDF_BIN) echoserver.elf qemuvirt.dtb
	python3 ../meta.py core_config/$(CONFIG_NUM_CORES).json echoserver
	$(OBJCOPY) --update-section .device_resources=serial_driver_device_resources.data serial_driver.elf
	$(OBJCOPY) --update-section .serial_driver_config=serial_driver_config.data serial_driver.elf
	$(OBJCOPY) --update-section .serial_virt_tx_config=serial_virt_tx.data serial_virt_tx.elf
	$(OBJCOPY) --update-section .serial_virt_rx_config=serial_virt_rx.data serial_virt_rx.elf
	$(OBJCOPY) --update-section .net_driver_config=net_driver.data eth_driver_virtio.elf
	$(OBJCOPY) --update-section .device_resources=eth_driver_device_resources.data eth_driver_virtio.elf
	$(OBJCOPY) --update-section .net_virt_tx_config=net_virt_tx.data network_virt_tx.elf
	$(OBJCOPY) --update-section .net_virt_rx_config=net_virt_rx.data network_virt_rx.elf
	$(OBJCOPY) --update-section .net_copy_config=net_copy_network_copy.data network_copy.elf
	$(OBJCOPY) --update-section .device_resources=timer_driver_device_resources.data timer_driver.elf
	$(OBJCOPY) --update-section .serial_client_config=serial_client_netserver.data echoserver.elf
	$(OBJCOPY) --update-section .timer_client_config=timer_client_netserver.data echoserver.elf
	$(OBJCOPY) --update-section .net_client_config=net_client_netserver.data echoserver.elf
	$(OBJCOPY) --update-section .lib_sddf_lwip_config=lib_sddf_lwip_config_netserver.data echoserver.elf
	$(MICROKIT_TOOL) $(MICROKIT_FLAGS) -o echoserver.img -r echoserver.txt

QEMU_MACHINE=-machine virt
QEMU_FLAGS =-cpu host
QEMU_FLAGS+=-nographic
QEMU_FLAGS+=-serial mon:stdio
QEMU_FLAGS+=-m size=2G
QEMU_FLAGS+=-smp $(CONFIG_NUM_CORES)
QEMU_FLAGS+=-netdev user,id=mynet0,hostfwd=tcp::80-:80
QEMU_FLAGS+=-device virtio-net-device,netdev=mynet0,mac=52:55:00:d1:55:01
QEMU_FLAGS+=-global virtio-mmio.force-legacy=false
QEMU_FLAGS+=-accel kvm

QEMU_FLAGS_WEBSERVER=-device loader,file=webserver.img,addr=0x70000000,cpu-num=0
QEMU_FLAGS_ECHOSERVER=-device loader,file=echoserver.img,addr=0x70000000,cpu-num=0
QEMU_FLAGS_DISCOVER=-device loader,file=$(VIRTIO_DISCOVER),addr=0x70000000,cpu-num=0

qemuvirt.dtb:
	qemu-system-aarch64 $(QEMU_MACHINE),dumpdtb=qemuvirt.dtb $(QEMU_FLAGS)

qemu-webserver: webserver.img
	qemu-system-aarch64 $(QEMU_MACHINE) $(QEMU_FLAGS) $(QEMU_FLAGS_WEBSERVER)

qemu-echoserver: echoserver.img
	qemu-system-aarch64 $(QEMU_MACHINE) $(QEMU_FLAGS) $(QEMU_FLAGS_ECHOSERVER)

$(VIRTIO_DISCOVER):
	wget https://github.com/IkerGalardi/VirtioDiscover/releases/download/v0.1.0/VirtioDiscover-Aarch64-Loader-Off0x70000000.img -O $(VIRTIO_DISCOVER)

virtio-discover: $(VIRTIO_DISCOVER)
	@ timeout -f --preserve-status 1s qemu-system-aarch64 $(QEMU_MACHINE) $(QEMU_FLAGS) $(QEMU_FLAGS_DISCOVER) 2> /dev/null
