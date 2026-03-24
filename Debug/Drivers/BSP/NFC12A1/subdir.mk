################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/NFC12A1/nfc12a1.c 

OBJS += \
./Drivers/BSP/NFC12A1/nfc12a1.o 

C_DEPS += \
./Drivers/BSP/NFC12A1/nfc12a1.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/NFC12A1/%.o Drivers/BSP/NFC12A1/%.su Drivers/BSP/NFC12A1/%.cyclo: ../Drivers/BSP/NFC12A1/%.c Drivers/BSP/NFC12A1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DST25R500 -DUSE_HAL_DRIVER -DSTM32L053xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/NFC12A1" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/Components/st25r500" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Inc" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-NFC12A1

clean-Drivers-2f-BSP-2f-NFC12A1:
	-$(RM) ./Drivers/BSP/NFC12A1/nfc12a1.cyclo ./Drivers/BSP/NFC12A1/nfc12a1.d ./Drivers/BSP/NFC12A1/nfc12a1.o ./Drivers/BSP/NFC12A1/nfc12a1.su

.PHONY: clean-Drivers-2f-BSP-2f-NFC12A1

