/*
 * robot.c
 *
 * Created: 23.03.2014 16:35:55
 *  Author: stas
 */ 

#define	BAUDRATE	9600 // Скорость обмена данными
#define	F_CPU		1000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define A_PORT		PORTC
#define A_ENABLE	PORTC1
#define A_FORWARD	PORTC0
#define A_BACKWARD	PORTC2

#define B_PORT		PORTC
#define B_ENABLE	PORTC4
#define B_FORWARD	PORTC3
#define B_BACKWARD	PORTC5

#define S_PORT		PORTD
#define S_PIN		PORTD7

// Функция передачи данных по USART
void USART_Transmit( unsigned char data )
{
	while ( !( UCSR0A & (1<<UDRE0)) );	// Wait for empty transmit buffer
	UDR0 = data;						// Put data into buffer, sends the data
}

int main(void)
{
	/*
	конфигурация портов:
	двигатель А	PC1 - enable
				PC0 - input1
				PC2 - input2
				
	двигатель B	PC4 - enable
				PC3 - input3
				PC5 - input4
	
	оптрон А	PD3
				PD4
				
	оптрон B	PD5
				PD6
				
	пищалка		PD7
	*/
	
	//инициализируем порты
	
	//двигатель
	DDRC = 0;
	PORTC = (1 << A_ENABLE) | (1 << B_ENABLE);
	
	//пищалка
	DDRD &= ~(1 << DDD7);
	PORTD &= ~(1 << PORTD7);
	
	//таймер
	TCCR0A = 0;							// нормальный режим
	TCCR0B = (1 << CS00);				// без делителя (при 1МГц - каждые 0.256 мс)
	TIMSK0 = (1 << TOIE0);				// включить прерывания
	
	//usart
	
	UBRR0 = 12;								//задаем скорость (9600, по таблице, из расчета на множитель скорости)
	UCSR0A = (1 << U2X0);					//множитель скорости передачи (удвоить)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);	//разрешаем прием, передачу и прерывание по завершению приема
	UCSR0C = (1 << UCSZ00)| (1 << UCSZ01);	// асинхронный, без бита четности, 1 стоп, 8 бит передача, 
	
	/*
	UBRR0H = 0;								//задаем скорость (9600, по таблице, из расчета на множитель скорости)
	UBRR0L = 12;
	UCSR0A = (1 << U2X0);					//множитель скорости передачи (удвоить)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0B |= (1 << RXCIE0);	//разрешаем прием, передачу и прерывание по завершению приема
	UCSR0C = (1 << UCSZ00)| (1 << UCSZ01);	// асинхронный, без бита четности, 1 стоп, 8 бит передача,
	*/
	
	USART_Transmit('O');	//Передаем при включении
	USART_Transmit('k');	//сообщение "Ok!", что свидетельствует
	USART_Transmit('!');	//о правильно работе программы
	
	//разрешить прерывания
	sei();
	
    while(1)
    {
		_delay_ms(1);	
    }
}

//глобальные переменные
volatile int16_t moveLeft = 0;
volatile int16_t moveRight = 0;
volatile uint16_t makeSound = 0;

inline void doLeftForward()
{
	A_PORT &= ~(1 << A_BACKWARD);
	A_PORT |= (1 << A_FORWARD);
}

inline void doLeftBackward()
{
	A_PORT &= ~(1 << A_FORWARD);
	A_PORT |= (1 << A_BACKWARD);
}

inline void doLeftStop()
{
	A_PORT &= ~(1 << A_FORWARD);
	A_PORT &= ~(1 << A_BACKWARD);
}

inline void doRightForward()
{
	B_PORT &= ~(1 << B_BACKWARD);
	B_PORT |= (1 << B_FORWARD);
}

inline void doRightBackward()
{
	B_PORT &= ~(1 << B_FORWARD);
	B_PORT |= (1 << B_BACKWARD);
}

inline void doRightStop()
{
	B_PORT &= ~(1 << B_FORWARD);
	B_PORT &= ~(1 << B_BACKWARD);
}

inline void doSound()
{
	uint8_t curState = S_PORT & (1 << S_PIN);
	if(curState > 0)
		S_PORT &= ~(1 << S_PIN);
	else
		S_PORT |= (1 << S_PIN);
}

inline void doSoundOff()
{
	S_PORT &= ~(1 << S_PIN);
}

//прерывание по завершению приема
ISR(USART_RX_vect)
{
	const uint16_t increment = 1000;
	const uint16_t limit = 1000;
	
	unsigned char data = UDR0;
	
	USART_Transmit(data);
		
	switch (data)
	{
		//вперед
		case 'w':
			if(moveLeft < limit)
				moveLeft += increment;
			if(moveRight < limit)
				moveRight += increment;
			break;
		
		//назад
		case 's':
			if(abs(moveLeft) < limit)
				moveLeft -= increment;
			if(abs(moveRight) < limit)
				moveRight -= increment;
			break;
		
		//влево
		case 'a':
			if(moveRight < limit)
				moveRight += increment;
			break;
		
		//вправо
		case 'd':
			if(moveLeft < limit)
				moveLeft += increment;
			break;
			
		//кручение влево
		case 'q':
			if(moveRight < limit)
				moveRight += increment;
			if(abs(moveLeft) < limit)
				moveLeft -= increment;
			break;
			
		//кручение вправо
		case 'e':	
			if(abs(moveRight) < limit)
				moveRight -= increment;	
			if(moveLeft < limit)
				moveLeft += increment;
			break;
			
		//пищалка
		case ' ':
			if(makeSound < 1000)
				makeSound += 1000;
			break;
			
		default:
			break;
	}
}

uint8_t test = 0;

//прерывание по таймеру
ISR(TIMER0_OVF_vect)
{
	//левая
	if(moveLeft > 0)
	{
		doLeftForward();
		moveLeft--;
	} 
	else if(moveLeft < 0)
	{
		doLeftBackward();
		moveLeft++;
	} 
	else
		doLeftStop();
	
	//правая
	if(moveRight > 0)
	{
		doRightForward();
		moveRight--;
	} 
	else if(moveRight < 0)
	{
		doRightBackward();
		moveRight++;
	} 
	else
		doRightStop();
		
	//пищалка
	if(makeSound > 0)
	{
		doSound();
		makeSound--;
	}
	else
		doSoundOff();
}
