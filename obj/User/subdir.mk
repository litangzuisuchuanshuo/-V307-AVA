################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v30x_it.c \
../User/lcd.c \
../User/main.c \
../User/system_ch32v30x.c 

OBJS += \
./User/ch32v30x_it.o \
./User/lcd.o \
./User/main.o \
./User/system_ch32v30x.o 

C_DEPS += \
./User/ch32v30x_it.d \
./User/lcd.d \
./User/main.d \
./User/system_ch32v30x.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized  -g -I"C:\Users\Administrator\Desktop\Ƕ��ʽ\ch32-v307-exercise-master\WIFI_CLOCK\Debug" -I"C:\Users\Administrator\Desktop\Ƕ��ʽ\ch32-v307-exercise-master\WIFI_CLOCK\Core" -I"C:\Users\Administrator\Desktop\Ƕ��ʽ\ch32-v307-exercise-master\WIFI_CLOCK\User" -I"C:\Users\Administrator\Desktop\Ƕ��ʽ\ch32-v307-exercise-master\WIFI_CLOCK\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

