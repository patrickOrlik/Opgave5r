/**
 * I2C.h
 * driver for I2C from AVR freaks adjusted with an init function
 * Created: 22-12-2017 19:00:53
 *  Author: osch
 */ 


#ifndef I2C_H_
#define I2C_H_
#define SCL_CLK 100000
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>

#define BITRATE(TWSR)	((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1)))))

void    I2C_Init(void);
uint8_t I2C_Start(char write_address);          /* I2C start function */
uint8_t I2C_Repeated_Start(char read_address);  /* I2C repeated start function */
uint8_t I2C_Write(char data);                   /* I2C write function */
char    I2C_Read_Ack(void);                     /* I2C read ack function */
char    I2C_Read_Nack(void);                    /* I2C read nack function */
void    I2C_Stop(void);                         /* I2C stop function */
#endif /* I2C_H_ */