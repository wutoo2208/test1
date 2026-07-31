################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/AAADiansai/CCS/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered" -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered/Debug" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-693580196: ../empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"C:/TI/sysconfig_1.26.2/sysconfig_cli.bat" -s "D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/.metadata/product.json" --script "C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-693580196 ../empty.syscfg
device.opt: build-693580196
device.cmd.genlibs: build-693580196
ti_msp_dl_config.c: build-693580196
ti_msp_dl_config.h: build-693580196
Event.dot: build-693580196

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/AAADiansai/CCS/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered" -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered/Debug" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"D:/AAADiansai/CCS/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered" -I"C:/Users/Administrator/workspace_ccstheia/test1_2.2_recovered/Debug" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"D:/AAADiansai/CCS/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


