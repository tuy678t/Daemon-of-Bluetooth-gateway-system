CC = arm-poky-linux-gnueabi-gcc -fmessage-length=0 -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi -rdynamic -funwind-tables -ffunction-sections -lxml2  -L. -Wl,-rpath,./
EXEC = mydaemon
SRC = mydaemon.c myfun.c
FLAGS= -Wall -L/home/mylib/libxml/libxmlinstl/lib/ -lxml2 -ldl -lm
INC = -I/home/mylib/libxml/libxmlinstl/include/libxml2 -I/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include -I/opt/myir-imx-fb-image-base/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi

all:mydaemon imake
mydaemon:mydaemon.c
	 $(CC) $(SRC) -o $(EXEC) $(FLAGS) $(INC)
imake:service
	make -C service
.PHONY:clean
clean:
	rm -rf $(EXEC)
	make clean -C service
