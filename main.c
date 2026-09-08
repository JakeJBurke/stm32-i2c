#include <stdint.h>

// RCC registers
#define RCC_AHB1ENR   (*((volatile uint32_t *)0x40023830))
#define RCC_APB1ENR   (*((volatile uint32_t *)0x40023840))

// GPIOB registers (PB6=SCL, PB7=SDA)
#define GPIOB_MODER   (*((volatile uint32_t *)0x40020400))
#define GPIOB_OTYPER  (*((volatile uint32_t *)0x40020404))
#define GPIOB_OSPEEDR (*((volatile uint32_t *)0x40020408))
#define GPIOB_PUPDR   (*((volatile uint32_t *)0x4002040C))
#define GPIOB_AFRL    (*((volatile uint32_t *)0x40020420))

// I2C1 registers
#define I2C1_CR1      (*((volatile uint32_t *)0x40005400))
#define I2C1_CR2      (*((volatile uint32_t *)0x40005404))
#define I2C1_SR1      (*((volatile uint32_t *)0x40005414))
#define I2C1_SR2      (*((volatile uint32_t *)0x40005418))
#define I2C1_CCR      (*((volatile uint32_t *)0x4000541C))
#define I2C1_TRISE    (*((volatile uint32_t *)0x40005420))
#define I2C1_DR       (*((volatile uint32_t *)0x40005410))

// USART2 registers (for printing to PC)
#define USART2_SR     (*((volatile uint32_t *)0x40004400))
#define USART2_DR     (*((volatile uint32_t *)0x40004404))
#define USART2_BRR    (*((volatile uint32_t *)0x40004408))
#define USART2_CR1    (*((volatile uint32_t *)0x4000440C))

// MPU-6050 address and registers
#define MPU6050_ADDR  0x68
#define MPU_PWR_MGMT  0x6B
#define MPU_WHO_AM_I  0x75
#define MPU_ACCEL_X_H 0x3B

void delay(volatile uint32_t count) {
    while (count--);
}

// UART functions for printing
void uart_init(void) {
    // Enable GPIOA and USART2 clocks
    RCC_AHB1ENR |= (1 << 0);   // GPIOA
    RCC_APB1ENR |= (1 << 17);  // USART2

    // PA2=TX, PA3=RX as alternate function AF7
    GPIOB_MODER &= ~(0xF << 4);  // using GPIOA but defined as GPIOB — fix below
}

void uart_send_byte(char c) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

void uart_send_string(const char* s) {
    while (*s) uart_send_byte(*s++);
}

void uart_send_number(int32_t num) {
    char buf[12];
    int i = 0;
    if (num < 0) { uart_send_byte('-'); num = -num; }
    if (num == 0) { uart_send_byte('0'); return; }
    while (num > 0) { buf[i++] = '0' + (num % 10); num /= 10; }
    while (i > 0) uart_send_byte(buf[--i]);
}

// I2C functions
void i2c_init(void) {
    // Enable GPIOB and I2C1 clocks
    RCC_AHB1ENR |= (1 << 1);   // GPIOB clock
    RCC_APB1ENR |= (1 << 21);  // I2C1 clock

    // PB6 and PB7 as alternate function (AF4 = I2C1)
    GPIOB_MODER  &= ~(0xF << 12);
    GPIOB_MODER  |=  (0xA << 12);  // AF mode for PB6 and PB7
    GPIOB_OTYPER |=  (0x3 << 6);   // open-drain
    GPIOB_OSPEEDR|=  (0xF << 12);  // high speed
    GPIOB_PUPDR  &= ~(0xF << 12);  // no pull-up/down
    GPIOB_AFRL   &= ~(0xFF << 24);
    GPIOB_AFRL   |=  (0x44 << 24); // AF4 for PB6 and PB7

    // Configure I2C1
    I2C1_CR1  &= ~(1 << 0);    // disable I2C before configuring
    I2C1_CR2   =  16;           // 16MHz peripheral clock
    I2C1_CCR   =  80;           // 100kHz I2C speed
    I2C1_TRISE =  17;           // max rise time
    I2C1_CR1  |=  (1 << 0);    // enable I2C
}

void i2c_start(void) {
    I2C1_CR1 |= (1 << 8);                    // generate START
    while (!(I2C1_SR1 & (1 << 0)));          // wait for SB flag
}

void i2c_stop(void) {
    I2C1_CR1 |= (1 << 9);                    // generate STOP
}

void i2c_send_addr(uint8_t addr, uint8_t rw) {
    I2C1_DR = (addr << 1) | rw;              // send address + R/W bit
    while (!(I2C1_SR1 & (1 << 1)));          // wait for ADDR flag
    (void)I2C1_SR1;                           // clear ADDR by reading SR1
    (void)I2C1_SR2;                           // then SR2
}

void i2c_send_byte(uint8_t data) {
    while (!(I2C1_SR1 & (1 << 7)));          // wait for TXE flag
    I2C1_DR = data;
    while (!(I2C1_SR1 & (1 << 2)));          // wait for BTF flag
}

uint8_t i2c_read_byte(uint8_t ack) {
    if (ack) I2C1_CR1 |=  (1 << 10);        // send ACK
    else     I2C1_CR1 &= ~(1 << 10);        // send NACK (last byte)
    while (!(I2C1_SR1 & (1 << 6)));          // wait for RXNE flag
    return I2C1_DR;
}

void mpu6050_write(uint8_t reg, uint8_t data) {
    i2c_start();
    i2c_send_addr(MPU6050_ADDR, 0);          // write mode
    i2c_send_byte(reg);
    i2c_send_byte(data);
    i2c_stop();
}

uint8_t mpu6050_read(uint8_t reg) {
    uint8_t data;
    i2c_start();
    i2c_send_addr(MPU6050_ADDR, 0);          // write mode to set register
    i2c_send_byte(reg);
    i2c_start();                              // repeated START
    i2c_send_addr(MPU6050_ADDR, 1);          // read mode
    data = i2c_read_byte(0);                 // read with NACK
    i2c_stop();
    return data;
}

int main(void) {
    // Initialize UART for printing
    // Enable GPIOA and USART2
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB1ENR |= (1 << 17);

    // PA2 as TX alternate function AF7
    (*((volatile uint32_t *)0x40020000)) &= ~(3 << 4);
    (*((volatile uint32_t *)0x40020000)) |=  (2 << 4);
    (*((volatile uint32_t *)0x40020020)) &= ~(0xF << 8);
    (*((volatile uint32_t *)0x40020020)) |=  (7 << 8);

    // USART2: 9600 baud at 16MHz
    USART2_BRR = 0x683;
    USART2_CR1 = (1 << 3) | (1 << 13);     // TE + UE

    // Initialize I2C
    i2c_init();
    delay(10000);

    // Wake up MPU-6050
    mpu6050_write(MPU_PWR_MGMT, 0x00);
    delay(10000);

    // Check WHO_AM_I
    uint8_t who = mpu6050_read(MPU_WHO_AM_I);
    if (who == 0x68) {
        uart_send_string("MPU-6050 found!\r\n");
    } else {
        uart_send_string("MPU-6050 not found!\r\n");
    }

    while (1) {
        // Read accelerometer X axis (2 bytes)
        int16_t accel_x = ((int16_t)mpu6050_read(MPU_ACCEL_X_H) << 8)
                         | mpu6050_read(MPU_ACCEL_X_H + 1);

        uart_send_string("Accel X: ");
        uart_send_number(accel_x);
        uart_send_string("\r\n");

        delay(1600000);  // ~100ms
    }

    return 0;
}