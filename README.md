# STM32F411 Bare-Metal I2C Driver — MPU-6050 IMU

Bare-metal I2C communication driver written in C for the STM32F411
ARM Cortex-M4 microcontroller. Interfaces MPU-6050 6-axis IMU sensor
through direct I2C1 peripheral register configuration without HAL libraries.

## What This Demonstrates

- RCC APB1ENR clock enable for I2C1 peripheral
- GPIOB alternate function configuration (PB6=SCL, PB7=SDA) with open-drain output
- I2C1 register-level configuration (CR1, CR2, CCR, TRISE) at 100kHz
- I2C START, STOP, address, and data transfer functions from scratch
- MPU-6050 wake-up via power management register (0x6B)
- WHO_AM_I register verification to confirm sensor connection
- Real-time accelerometer data acquisition and UART output
- Repeated START condition for I2C read operations

## Why No HAL

STM32 HAL hides the I2C hardware completely behind function calls,
limiting direct control over timing and behavior. Writing directly
to I2C1_CR1, I2C1_DR and I2C1_SR1 gives full control over
communication timing and protocol. This is how production embedded
firmware handles sensor communication.

## Build and Flash

make flash

Requires arm-none-eabi-gcc, OpenOCD, and ST-Link V2 programmer.
Open PuTTY at 9600 baud on COM3 to see accelerometer output.

## Hardware

- WeAct STM32F411CEU6 Black Pill (ARM Cortex-M4 at 100MHz)
- MPU-6050 6-axis IMU module (I2C address 0x68)
- ST-Link V2 programmer via SWD interface
- Wiring: PB6 to SCL, PB7 to SDA, 3.3V, GND

## Tools

- arm-none-eabi-gcc, arm-none-eabi-objcopy
- OpenOCD 0.12.0 with ST-Link V2
- STM32F411 Reference Manual RM0383
- MPU-6050 Register Map datasheet
- GNU Make
