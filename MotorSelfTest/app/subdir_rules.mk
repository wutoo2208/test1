################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/AAADiansai/CCS/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered" -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered/MotorSelfTest" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source" -DMOTOR_SELFTEST_BUILD=1 -g -Wall -MMD -MP -MF"app/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


