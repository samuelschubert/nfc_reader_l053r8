################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/st25r500/rfal_rfst25r500.c \
../Drivers/BSP/Components/st25r500/st25r500.c \
../Drivers/BSP/Components/st25r500/st25r500_com.c \
../Drivers/BSP/Components/st25r500/st25r500_dpocr.c \
../Drivers/BSP/Components/st25r500/st25r500_irq.c 

OBJS += \
./Drivers/BSP/Components/st25r500/rfal_rfst25r500.o \
./Drivers/BSP/Components/st25r500/st25r500.o \
./Drivers/BSP/Components/st25r500/st25r500_com.o \
./Drivers/BSP/Components/st25r500/st25r500_dpocr.o \
./Drivers/BSP/Components/st25r500/st25r500_irq.o 

C_DEPS += \
./Drivers/BSP/Components/st25r500/rfal_rfst25r500.d \
./Drivers/BSP/Components/st25r500/st25r500.d \
./Drivers/BSP/Components/st25r500/st25r500_com.d \
./Drivers/BSP/Components/st25r500/st25r500_dpocr.d \
./Drivers/BSP/Components/st25r500/st25r500_irq.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/st25r500/%.o Drivers/BSP/Components/st25r500/%.su Drivers/BSP/Components/st25r500/%.cyclo: ../Drivers/BSP/Components/st25r500/%.c Drivers/BSP/Components/st25r500/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DST25R500 -DUSE_HAL_DRIVER -DSTM32L053xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/NFC12A1" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/Components/st25r500" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Inc" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-st25r500

clean-Drivers-2f-BSP-2f-Components-2f-st25r500:
	-$(RM) ./Drivers/BSP/Components/st25r500/rfal_rfst25r500.cyclo ./Drivers/BSP/Components/st25r500/rfal_rfst25r500.d ./Drivers/BSP/Components/st25r500/rfal_rfst25r500.o ./Drivers/BSP/Components/st25r500/rfal_rfst25r500.su ./Drivers/BSP/Components/st25r500/st25r500.cyclo ./Drivers/BSP/Components/st25r500/st25r500.d ./Drivers/BSP/Components/st25r500/st25r500.o ./Drivers/BSP/Components/st25r500/st25r500.su ./Drivers/BSP/Components/st25r500/st25r500_com.cyclo ./Drivers/BSP/Components/st25r500/st25r500_com.d ./Drivers/BSP/Components/st25r500/st25r500_com.o ./Drivers/BSP/Components/st25r500/st25r500_com.su ./Drivers/BSP/Components/st25r500/st25r500_dpocr.cyclo ./Drivers/BSP/Components/st25r500/st25r500_dpocr.d ./Drivers/BSP/Components/st25r500/st25r500_dpocr.o ./Drivers/BSP/Components/st25r500/st25r500_dpocr.su ./Drivers/BSP/Components/st25r500/st25r500_irq.cyclo ./Drivers/BSP/Components/st25r500/st25r500_irq.d ./Drivers/BSP/Components/st25r500/st25r500_irq.o ./Drivers/BSP/Components/st25r500/st25r500_irq.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-st25r500

