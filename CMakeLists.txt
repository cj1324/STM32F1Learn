PROJECT(stm32learn)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(STM32CubeF1_FW ${CMAKE_SOURCE_DIR}/../STM32Cube_FW_F1_V1.4.0)
SET(STLINK_BIN ${CMAKE_SOURCE_DIR}/../stlink/st-flash)

IF(NOT EXISTS "${STM32CubeF1_FW}")
	MESSAGE( FATAL_ERROR "Not found STM32CubeF1 FW 1.4.0, please download from st.com." )
ENDIF()

IF(NOT EXISTS "${STLINK_BIN}")
	MESSAGE( WARNING "Not found st-flash, disable flash command." )
ENDIF()

SET(CMSIS_DRIVER_DIR ${STM32CubeF1_FW}/Drivers/CMSIS)
SET(HAL_DRIVERS_DIR ${STM32CubeF1_FW}/Drivers/STM32F1xx_HAL_Driver)
SET(CMSIS_RTOS_DIR ${STM32CubeF1_FW}/Middlewares/Third_Party/FreeRTOS/Source)

INCLUDE_DIRECTORIES(
    ${CMSIS_DRIVER_DIR}/Include/
    ${CMSIS_DRIVER_DIR}/Device/ST/STM32F1xx/Include/

    ${HAL_DRIVERS_DIR}/Inc/

    ${CMSIS_RTOS_DIR}/include/
    ${CMSIS_RTOS_DIR}/CMSIS_RTOS/
    ${CMSIS_RTOS_DIR}/portable/GCC/ARM_CM3/

    ${CMAKE_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Lib
)

SET(HAL_LIB_SRC ${HAL_DRIVERS_DIR}/Src/)

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_C_FLAGS_DEBUG "-O0 -gdwarf-3 -g")
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_C_COMPILER "arm-none-eabi-gcc")
SET(CMAKE_LINKER "arm-none-eabi-ld")
SET(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
SET(CMAKE_SIZE "arm-none-eabi-size")

SET(LINK_SCRIPT ${CMAKE_SOURCE_DIR}/Boot/STM32F103XB_FLASH.ld)
SET(FAMILY "STM32F103xB")

SET(STARTUP ${CMAKE_SOURCE_DIR}/Boot/startup_stm32f103xb.s
            ${CMAKE_SOURCE_DIR}/Boot/system_stm32f1xx.c
            ${CMAKE_SOURCE_DIR}/syscall.c)  # for printf

SET(STARTUP ${STARTUP}
            ${HAL_LIB_SRC}/stm32f1xx_hal.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_cortex.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_rcc.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_rcc_ex.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_gpio.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_dma.c
            ${HAL_LIB_SRC}/stm32f1xx_hal_uart.c
)


# for RTOS
SET(CMSIS_RTOS ${CMSIS_RTOS_DIR}/CMSIS_RTOS/cmsis_os.c
    ${CMSIS_RTOS_DIR}/portable/MemMang/heap_1.c
    ${CMSIS_RTOS_DIR}/portable/GCC/ARM_CM3/port.c
    ${CMSIS_RTOS_DIR}/list.c
    ${CMSIS_RTOS_DIR}/queue.c
    ${CMSIS_RTOS_DIR}/tasks.c
    ${CMSIS_RTOS_DIR}/timers.c
    )


SET_PROPERTY(SOURCE ${CMAKE_SOURCE_DIR}/Boot/startup_stm32f103xb.s PROPERTY LANGUAGE C)

SET(SRC main.c stm32f1xx_it.c stm32f1xx_hal_msp.c)


SET(MCU_FLAGS "-march=armv7-m -mcpu=cortex-m3 -mthumb")
SET(CMAKE_C_FLAGS "${MCU_FLAGS} -fno-common -Wall  -ffunction-sections -fdata-sections --std=gnu99 ")
ADD_DEFINITIONS(-D${FAMILY} -DUSE_HAL_DRIVER)
SET(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs -lc -lnosys -static -Wl,-Map=stm32f1.map -nostartfiles -Wl,--gc-section -T${LINK_SCRIPT}")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

CMAKE_POLICY(SET CMP0015 OLD)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

ADD_EXECUTABLE(${PROJECT_NAME}.elf ${SRC} ${STARTUP} ${CMSIS_RTOS})

ADD_CUSTOM_COMMAND(DEPENDS ${PROJECT_NAME}.elf OUTPUT ${PROJECT_NAME}.hex COMMAND ${CMAKE_OBJCOPY} -Oihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex)
ADD_CUSTOM_COMMAND(DEPENDS ${PROJECT_NAME}.elf OUTPUT ${PROJECT_NAME}.bin COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}.elf POST_BUILD COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf)

ADD_CUSTOM_TARGET(generate_bin ALL DEPENDS ${PROJECT_NAME}.bin)
ADD_CUSTOM_TARGET(generate_hex ALL DEPENDS ${PROJECT_NAME}.hex)

ADD_CUSTOM_TARGET(flash ${STLINK_BIN} --reset write ${PROJECT_NAME}.bin 0x8000000 depends ${PROJECT_NAME}.bin)
