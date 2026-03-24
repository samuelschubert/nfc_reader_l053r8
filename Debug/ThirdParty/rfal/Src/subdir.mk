################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/rfal/Src/rfal_analogConfig.c \
../ThirdParty/rfal/Src/rfal_cd.c \
../ThirdParty/rfal/Src/rfal_crc.c \
../ThirdParty/rfal/Src/rfal_dlma.c \
../ThirdParty/rfal/Src/rfal_dpo.c \
../ThirdParty/rfal/Src/rfal_iso15693_2.c \
../ThirdParty/rfal/Src/rfal_isoDep.c \
../ThirdParty/rfal/Src/rfal_nfc.c \
../ThirdParty/rfal/Src/rfal_nfcDep.c \
../ThirdParty/rfal/Src/rfal_nfca.c \
../ThirdParty/rfal/Src/rfal_nfcb.c \
../ThirdParty/rfal/Src/rfal_nfcf.c \
../ThirdParty/rfal/Src/rfal_nfcv.c \
../ThirdParty/rfal/Src/rfal_st25tb.c \
../ThirdParty/rfal/Src/rfal_st25xv.c \
../ThirdParty/rfal/Src/rfal_t1t.c \
../ThirdParty/rfal/Src/rfal_t2t.c \
../ThirdParty/rfal/Src/rfal_t4t.c 

OBJS += \
./ThirdParty/rfal/Src/rfal_analogConfig.o \
./ThirdParty/rfal/Src/rfal_cd.o \
./ThirdParty/rfal/Src/rfal_crc.o \
./ThirdParty/rfal/Src/rfal_dlma.o \
./ThirdParty/rfal/Src/rfal_dpo.o \
./ThirdParty/rfal/Src/rfal_iso15693_2.o \
./ThirdParty/rfal/Src/rfal_isoDep.o \
./ThirdParty/rfal/Src/rfal_nfc.o \
./ThirdParty/rfal/Src/rfal_nfcDep.o \
./ThirdParty/rfal/Src/rfal_nfca.o \
./ThirdParty/rfal/Src/rfal_nfcb.o \
./ThirdParty/rfal/Src/rfal_nfcf.o \
./ThirdParty/rfal/Src/rfal_nfcv.o \
./ThirdParty/rfal/Src/rfal_st25tb.o \
./ThirdParty/rfal/Src/rfal_st25xv.o \
./ThirdParty/rfal/Src/rfal_t1t.o \
./ThirdParty/rfal/Src/rfal_t2t.o \
./ThirdParty/rfal/Src/rfal_t4t.o 

C_DEPS += \
./ThirdParty/rfal/Src/rfal_analogConfig.d \
./ThirdParty/rfal/Src/rfal_cd.d \
./ThirdParty/rfal/Src/rfal_crc.d \
./ThirdParty/rfal/Src/rfal_dlma.d \
./ThirdParty/rfal/Src/rfal_dpo.d \
./ThirdParty/rfal/Src/rfal_iso15693_2.d \
./ThirdParty/rfal/Src/rfal_isoDep.d \
./ThirdParty/rfal/Src/rfal_nfc.d \
./ThirdParty/rfal/Src/rfal_nfcDep.d \
./ThirdParty/rfal/Src/rfal_nfca.d \
./ThirdParty/rfal/Src/rfal_nfcb.d \
./ThirdParty/rfal/Src/rfal_nfcf.d \
./ThirdParty/rfal/Src/rfal_nfcv.d \
./ThirdParty/rfal/Src/rfal_st25tb.d \
./ThirdParty/rfal/Src/rfal_st25xv.d \
./ThirdParty/rfal/Src/rfal_t1t.d \
./ThirdParty/rfal/Src/rfal_t2t.d \
./ThirdParty/rfal/Src/rfal_t4t.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/rfal/Src/%.o ThirdParty/rfal/Src/%.su ThirdParty/rfal/Src/%.cyclo: ../ThirdParty/rfal/Src/%.c ThirdParty/rfal/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DST25R500 -DUSE_HAL_DRIVER -DSTM32L053xx -c -I../Core/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc -I../Drivers/STM32L0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../Drivers/CMSIS/Include -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/NFC12A1" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP/Components/st25r500" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/Drivers/BSP" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Inc" -I"C:/repos/nfc_reader_l053r8/nfc_reader_l053r8/ThirdParty/rfal/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-ThirdParty-2f-rfal-2f-Src

clean-ThirdParty-2f-rfal-2f-Src:
	-$(RM) ./ThirdParty/rfal/Src/rfal_analogConfig.cyclo ./ThirdParty/rfal/Src/rfal_analogConfig.d ./ThirdParty/rfal/Src/rfal_analogConfig.o ./ThirdParty/rfal/Src/rfal_analogConfig.su ./ThirdParty/rfal/Src/rfal_cd.cyclo ./ThirdParty/rfal/Src/rfal_cd.d ./ThirdParty/rfal/Src/rfal_cd.o ./ThirdParty/rfal/Src/rfal_cd.su ./ThirdParty/rfal/Src/rfal_crc.cyclo ./ThirdParty/rfal/Src/rfal_crc.d ./ThirdParty/rfal/Src/rfal_crc.o ./ThirdParty/rfal/Src/rfal_crc.su ./ThirdParty/rfal/Src/rfal_dlma.cyclo ./ThirdParty/rfal/Src/rfal_dlma.d ./ThirdParty/rfal/Src/rfal_dlma.o ./ThirdParty/rfal/Src/rfal_dlma.su ./ThirdParty/rfal/Src/rfal_dpo.cyclo ./ThirdParty/rfal/Src/rfal_dpo.d ./ThirdParty/rfal/Src/rfal_dpo.o ./ThirdParty/rfal/Src/rfal_dpo.su ./ThirdParty/rfal/Src/rfal_iso15693_2.cyclo ./ThirdParty/rfal/Src/rfal_iso15693_2.d ./ThirdParty/rfal/Src/rfal_iso15693_2.o ./ThirdParty/rfal/Src/rfal_iso15693_2.su ./ThirdParty/rfal/Src/rfal_isoDep.cyclo ./ThirdParty/rfal/Src/rfal_isoDep.d ./ThirdParty/rfal/Src/rfal_isoDep.o ./ThirdParty/rfal/Src/rfal_isoDep.su ./ThirdParty/rfal/Src/rfal_nfc.cyclo ./ThirdParty/rfal/Src/rfal_nfc.d ./ThirdParty/rfal/Src/rfal_nfc.o ./ThirdParty/rfal/Src/rfal_nfc.su ./ThirdParty/rfal/Src/rfal_nfcDep.cyclo ./ThirdParty/rfal/Src/rfal_nfcDep.d ./ThirdParty/rfal/Src/rfal_nfcDep.o ./ThirdParty/rfal/Src/rfal_nfcDep.su ./ThirdParty/rfal/Src/rfal_nfca.cyclo ./ThirdParty/rfal/Src/rfal_nfca.d ./ThirdParty/rfal/Src/rfal_nfca.o ./ThirdParty/rfal/Src/rfal_nfca.su ./ThirdParty/rfal/Src/rfal_nfcb.cyclo ./ThirdParty/rfal/Src/rfal_nfcb.d ./ThirdParty/rfal/Src/rfal_nfcb.o ./ThirdParty/rfal/Src/rfal_nfcb.su ./ThirdParty/rfal/Src/rfal_nfcf.cyclo ./ThirdParty/rfal/Src/rfal_nfcf.d ./ThirdParty/rfal/Src/rfal_nfcf.o ./ThirdParty/rfal/Src/rfal_nfcf.su ./ThirdParty/rfal/Src/rfal_nfcv.cyclo ./ThirdParty/rfal/Src/rfal_nfcv.d ./ThirdParty/rfal/Src/rfal_nfcv.o ./ThirdParty/rfal/Src/rfal_nfcv.su ./ThirdParty/rfal/Src/rfal_st25tb.cyclo ./ThirdParty/rfal/Src/rfal_st25tb.d ./ThirdParty/rfal/Src/rfal_st25tb.o ./ThirdParty/rfal/Src/rfal_st25tb.su ./ThirdParty/rfal/Src/rfal_st25xv.cyclo ./ThirdParty/rfal/Src/rfal_st25xv.d ./ThirdParty/rfal/Src/rfal_st25xv.o ./ThirdParty/rfal/Src/rfal_st25xv.su ./ThirdParty/rfal/Src/rfal_t1t.cyclo ./ThirdParty/rfal/Src/rfal_t1t.d ./ThirdParty/rfal/Src/rfal_t1t.o ./ThirdParty/rfal/Src/rfal_t1t.su ./ThirdParty/rfal/Src/rfal_t2t.cyclo ./ThirdParty/rfal/Src/rfal_t2t.d ./ThirdParty/rfal/Src/rfal_t2t.o ./ThirdParty/rfal/Src/rfal_t2t.su ./ThirdParty/rfal/Src/rfal_t4t.cyclo ./ThirdParty/rfal/Src/rfal_t4t.d ./ThirdParty/rfal/Src/rfal_t4t.o ./ThirdParty/rfal/Src/rfal_t4t.su

.PHONY: clean-ThirdParty-2f-rfal-2f-Src

