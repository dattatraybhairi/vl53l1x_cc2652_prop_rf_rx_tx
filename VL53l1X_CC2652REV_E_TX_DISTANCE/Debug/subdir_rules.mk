################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE" --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE/Debug" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source/ti/posix/ccs" --include_path="C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.3.LTS/include" --define=DeviceFamily_CC26X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE/Debug/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-14441137: ../rfPacketTx.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.6.0/sysconfig_cli.bat" -s "C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/.metadata/product.json" -o "syscfg" --compiler ccs "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_devices_config.c: build-14441137 ../rfPacketTx.syscfg
syscfg/ti_radio_config.c: build-14441137
syscfg/ti_radio_config.h: build-14441137
syscfg/ti_drivers_config.c: build-14441137
syscfg/ti_drivers_config.h: build-14441137
syscfg/ti_utils_build_linker.cmd.genlibs: build-14441137
syscfg/syscfg_c.rov.xs: build-14441137
syscfg/ti_utils_runtime_model.gv: build-14441137
syscfg/ti_utils_runtime_Makefile: build-14441137
syscfg/: build-14441137

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE" --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE/Debug" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source/ti/posix/ccs" --include_path="C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.3.LTS/include" --define=DeviceFamily_CC26X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/Users/dbhai/workspace_v11/VL53l1X_CC2652REV_E_TX_DISTANCE/Debug/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


