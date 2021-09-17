CC = arm-poky-linux-gnueabi-gcc -fmessage-length=0 -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi -rdynamic -funwind-tables -ffunction-sections -I/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include -I/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi
EXEC = ncp_host
OBJ = ncp_host.o
SRC = ncp_host.c myfun.c


all:$(EXEC)
$(EXEC):$(SRC)
	$(CC) $(SRC) -o $(EXEC)

clean:
	rm -rf $(EXEC)
