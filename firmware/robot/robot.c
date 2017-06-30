/*
 * robot.c
 *
 * Created: 23.03.2014 16:35:55
 *  Author: stas
 */ 

#define	BAUDRATE	9600 // �������� ������ �������
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

// ������� �������� ������ �� USART
void USART_Transmit( unsigned char data )
{
	while ( !( UCSR0A & (1<<UDRE0)) );	// Wait for empty transmit buffer
	UDR0 = data;						// Put data into buffer, sends the data
}

int main(void)
{
	/*
	������������ ������:
	��������� �	PC1 - enable
				PC0 - input1
				PC2 - input2
				
	��������� B	PC4 - enable
				PC3 - input3
				PC5 - input4
	
	������ �	PD3
				PD4
				
	������ B	PD5
				PD6
				
	�������		PD7
	*/
	
	//�������������� �����
	
	//���������
	DDRC = 0;
	PORTC = (1 << A_ENABLE) | (1 << B_ENABLE);
	
	//�������
	DDRD &= ~(1 << DDD7);
	PORTD &= ~(1 << PORTD7);
	
	//������
	TCCR0A = 0;							// ���������� �����
	TCCR0B = (1 << CS00);				// ��� �������� (��� 1��� - ������ 0.256 ��)
	TIMSK0 = (1 << TOIE0);				// �������� ����������
	
	//usart
	
	UBRR0 = 12;								//������ �������� (9600, �� �������, �� ������� �� ��������� ��������)
	UCSR0A = (1 << U2X0);					//��������� �������� �������� (�������)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);	//��������� �����, �������� � ���������� �� ���������� ������
	UCSR0C = (1 << UCSZ00)| (1 << UCSZ01);	// �����������, ��� ���� ��������, 1 ����, 8 ��� ��������, 
	
	/*
	UBRR0H = 0;								//������ �������� (9600, �� �������, �� ������� �� ��������� ��������)
	UBRR0L = 12;
	UCSR0A = (1 << U2X0);					//��������� �������� �������� (�������)
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0B |= (1 << RXCIE0);	//��������� �����, �������� � ���������� �� ���������� ������
	UCSR0C = (1 << UCSZ00)| (1 << UCSZ01);	// �����������, ��� ���� ��������, 1 ����, 8 ��� ��������,
	*/
	
	USART_Transmit('O');	//�������� ��� ���������
	USART_Transmit('k');	//��������� "Ok!", ��� ���������������
	USART_Transmit('!');	//� ��������� ������ ���������
	
	//��������� ����������
	sei();
	
    while(1)
    {
		_delay_ms(1);	
    }
}

//���������� ����������
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

//���������� �� ���������� ������
ISR(USART_RX_vect)
{
	const uint16_t increment = 1000;
	const uint16_t limit = 1000;
	
	unsigned char data = UDR0;
	
	USART_Transmit(data);
		
	switch (data)
	{
		//������
		case 'w':
			if(moveLeft < limit)
				moveLeft += increment;
			if(moveRight < limit)
				moveRight += increment;
			break;
		
		//�����
		case 's':
			if(abs(moveLeft) < limit)
				moveLeft -= increment;
			if(abs(moveRight) < limit)
				moveRight -= increment;
			break;
		
		//�����
		case 'a':
			if(moveRight < limit)
				moveRight += increment;
			break;
		
		//������
		case 'd':
			if(moveLeft < limit)
				moveLeft += increment;
			break;
			
		//�������� �����
		case 'q':
			if(moveRight < limit)
				moveRight += increment;
			if(abs(moveLeft) < limit)
				moveLeft -= increment;
			break;
			
		//�������� ������
		case 'e':	
			if(abs(moveRight) < limit)
				moveRight -= increment;	
			if(moveLeft < limit)
				moveLeft += increment;
			break;
			
		//�������
		case ' ':
			if(makeSound < 1000)
				makeSound += 1000;
			break;
			
		default:
			break;
	}
}

uint8_t test = 0;

//���������� �� �������
ISR(TIMER0_OVF_vect)
{
	//�����
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
	
	//������
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
		
	//�������
	if(makeSound > 0)
	{
		doSound();
		makeSound--;
	}
	else
		doSoundOff();
}
