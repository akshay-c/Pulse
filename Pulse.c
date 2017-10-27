#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define btnUp 		PINB0
#define btnDown 	PINB2
#define btnLeft 	PINB3
#define btnRight 	PINB4
#define btnOn 		PINB5
#define btnEnter 	PINB6
#define btnBack 	PINB7
#define rs 			PD0
#define en 			PD1


#define LCD 	PORTD
#define BUTTONS PINB
#define FREQ 	1
#define ONT  	2
#define OFFT 	3
#define TIME  	4


int prescalar = 1024;
int selpos = 0;
int curpos = 0;
int intcurpos = 0;
int dispno = 0;
int on = 0;
int dur = 15;
int ondur = 1;
int offdur = 1;
int tsec = 0;
int time = 0;
int totdur = 0;
int adj1;
int adj2;
char loc;

void initLCD();
void ClearLCD();
void LCDdata(char*);
void LCDcmd(char);
void LCDint(int);
void disppos();
void dispintpos(int);
void setintpos(int);
void showData(int,int,char);
void disp3();
void disp4();
void disp5();
void disp6();
void disp7();

void Faradiac();  // 60Hz, 6% Full on
void SFaradiac(); // 60Hz, 6% User on 1-9
void TensNormal(); //1-150 Hz gap 10, 0.1% full on
void TensBurst(); //1-150 Hz gap 10, 1.5% User on 1-9
void Galvanic(); //1,3,5,10,50,100 Hz, Full on
void IGalvanic(); //1,3,5,10,50,100 Hz, User On 1-9

void GenFreq(int Freq, int DutyCycle,int on, int off);
void GenPulse(int Freq, int ton);

ISR(TIMER2_COMP_vect )
{
	time --;
	if(totdur && (time%4 == 0))
	{
		if(((time/4)%totdur) == adj1)
		{
			DDRB |= (1<<DDB1);
			LCDcmd(0xCF);
			LCDdata("On");
		}
		if(((time/4)%totdur) == adj2)
		{
			DDRB &= ~(1<<DDB1);
			LCDcmd(0xCF);
			LCDdata("Off");
		}
	}
	if(time%4 == 0)
	{
		showData((time/4)%60,2,loc);
		showData(time/(4*60),2,(loc-0x03));
	}
	if(time == 0)
	{
		DDRB &= ~(1<<DDB1);
		TCCR2=0x00;
		TCCR1A=0x00;
		TCCR1B=0x00;
		TIMSK &= ~(1<<OCIE2);
	}
}


void disp3()
{
	LCDcmd(0x94);
	LCDdata("  Time:  :00min");
}

void disp4()
{
	LCDcmd(0xC0);
	LCDdata("  OnTime:");
	LCDcmd(0x94);
	LCDdata("  OffTime:");
	LCDcmd(0xD4);
	LCDdata("  Time:  :00min");
	
}

void disp5()
{
	LCDcmd(0xC0);
	LCDdata("  Frequency:   Hz");
	LCDcmd(0x94);
	LCDdata("  Pulse:   ms");
	LCDcmd(0xD4);
	LCDdata("  Time:  :00min");
	
}

void disp6()
{
	LCDcmd(0xC0);
	LCDdata("  Frequency:   Hz");
	LCDcmd(0x94);
	LCDdata("  Time:  :00min");
	
}
void disp7()
{
	LCDcmd(0xC0);
	LCDdata("  Frequency:   Hz");
	LCDcmd(0x94);
	LCDdata("  OnTime:");
	LCDcmd(0xD4);
	LCDdata("  OffTime:");
	
}

void stopTimers()
{
		DDRB &= ~(1<<DDB1);
		TCCR2=0x00;
		TCCR1A=0x00;
		TCCR1B=0x00;
		TIMSK &= ~(1<<OCIE2);
}

void setintpos(int index)
{
	switch(intcurpos)
	{
		case 0:
		case 3:
			LCDcmd(0xC0);
			break;
		case 1:
		case 4:
			LCDcmd(0x94);
			break;
		case 2:
		case 5:
			LCDcmd(0xD4);
			break;
			break;
	}
	LCDdata(" ");
	intcurpos = index;
	dispintpos(index);
}

void dispintpos(int index)
{
switch(intcurpos)
	{
		case 0:
			LCDcmd(0xC0);
			break;
		case 1:
		case 4:
			LCDcmd(0x94);
			break;
		case 2:
		case 5:
			LCDcmd(0xD4);
			break;
		case 3:
			LCDcmd(0xC0);
			LCDdata("                 ");
			LCDcmd(0xD4);
			LCDdata("                 ");
			LCDcmd(0x94);
			break;
	}
	LCDdata(">");
}
void LCDint(int num)
{
	char val;
	LCDcmd(0x04);
	while(num>0)
	{
		val = (char)(num % 10);
		val |= 0x30;
		num = num/10;
		LCD |= (1<<rs);
		LCD = (LCD & 0x0F) | (val & 0xF0);
		LCD &= ~(1<<en);
		_delay_ms(20);
		LCD |= (1<<en);
		LCD = (LCD & 0x0F) | (((val)<<4) & 0xF0);
		LCD &= ~(1<<en);
		_delay_ms(20);
		LCD |= (1<<en);
	}
	LCDcmd(0x06);
}

void showData(int value, int len, char start)
{
	LCDcmd(start);
	for(int i=0;i<len;i++)
	{
		LCDdata(" ");
	}
	LCDcmd(0x10);
	LCDint(value);
}

void Faradiac()
{
	LCDcmd(0x01);
	LCDdata("  Faradiac");
	disp3();
	LCDcmd(0x9D);
	showData(dur, 2,0x9B);
	loc = 0x9B;
	setintpos(1);
	//GenFreq(60, 6, 0xFF);
	while((BUTTONS&(1<<btnBack)))
	{
		if(!(BUTTONS&(1<<btnUp)))
		{
			if(dur== 60)
			{
				dur = 5;
			}
			else
				dur += 5;
			showData(dur, 2,0x9B);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			if(dur== 5)
			{
				dur = 60;
			}
			else
				dur -= 5;
			showData(dur, 2,0x9B);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenFreq(60,6,0,0);
			while(!(BUTTONS&(1<<btnEnter)));
			
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}
void SFaradiac()
{
	ondur =1;
	offdur =1;
	LCDcmd(0x01);
	LCDdata("  SFaradiac");
	disp4();
	intcurpos = 0;
	setintpos(intcurpos);
	showData(ondur,1,0xC5);
	showData(offdur,1,0x9E);
	showData(dur,2,0xDB);
	loc = 0xDE;
	while((BUTTONS&(1<<btnBack)))
	{
		
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenFreq(60,6,ondur,offdur);
			while(!(BUTTONS&(1<<btnEnter)));
		}
		if(!(BUTTONS&(1<<btnUp)))
		{
			int temp = intcurpos;
			temp++;
			if(temp>2)
			{
				temp=0;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			int temp = intcurpos;
			temp--;
			if(temp<0)
			{
				temp=2;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnLeft)))
		{
			switch(intcurpos)
			{
				case 0:
					ondur--;
					if(ondur<1)
						ondur = 9;
					showData(ondur,1,0xC9);
				break;
				case 1:
					offdur--;
					if(offdur<1)
						offdur =9;
					showData(offdur,1,0x9E);
				break;
				case 2:
					dur -= 5;
					if(dur<5)
						dur=60;
					showData(dur,2,0xDB);
				break;
			}
			while(!(BUTTONS&(1<<btnLeft)));
		}
		if(!(BUTTONS&(1<<btnRight)))
		{
			switch(intcurpos)
			{
				case 0:
					ondur++;
					if(ondur>9)
						ondur = 1;
						showData(ondur,1,0xC9);
				break;
				case 1:
					offdur++;
					if(offdur>9)
						offdur =1;
					showData(offdur,1,0x9E);
				break;
				case 2:
					dur += 5;
					if(dur>60)
						dur=5;
					showData(dur,2,0xDB);
				break;
			}
			while(!(BUTTONS&(1<<btnRight)));
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}
void TensNormal()
{
	int freq =1;
	int dur =15;
	LCDcmd(0x01);
	LCDdata("  Tens Normal");
	disp6();
	intcurpos = 0;
	setintpos(intcurpos);
	showData(freq,3,0xCC);
	showData(dur,2,0x9B);
	loc = 0xDE;
	setintpos(0);
	while((BUTTONS&(1<<btnBack)))
	{
		if(!(BUTTONS&(1<<btnUp)))
		{
			int temp = intcurpos;
			temp++;
			if(temp>1)
			{
				temp=0;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			int temp = intcurpos;
			temp--;
			if(temp<0)
			{
				temp=1;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnLeft)))
		{
			switch(intcurpos)
			{
				case 0:
					freq-=10;
					if(freq==0)
					{
						freq =1;
					}
					if(freq<0)
					{
						freq =150;
					}
					showData(freq,3,0xCC);
				break;
				case 1:
					dur -= 5;
					if(dur<5)
						dur=60;
					showData(dur,2,0x9B);
				break;
			}
			while(!(BUTTONS&(1<<btnLeft)));
		}
		if(!(BUTTONS&(1<<btnRight)))
		{
			switch(intcurpos)
			{
				case 0:
					freq+=10;
					if(freq==11)
					{
						freq =10;
					}
					if(freq>150)
					{
						freq =1;
					}
					showData(freq,3,0xCC);
				break;
				case 1:
					dur += 5;
					if(dur>60)
						dur=5;
					showData(dur,2,0x9B);
				break;
			}
			while(!(BUTTONS&(1<<btnRight)));
		}
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenFreq(freq,50,0,0);
			while(!(BUTTONS&(1<<btnEnter)));
			
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}
void TensBurst()
{
	int freq=1;
	ondur =1;
	offdur =1;
	LCDcmd(0x01);
	LCDdata("  Tens Burst");
	disp7();
	intcurpos = 0;
	setintpos(intcurpos);
	showData(freq,3,0xCC);
	showData(ondur,1,0x9D);
	showData(offdur,1,0xDE);
	loc = 0xDE;
	while((BUTTONS&(1<<btnBack)))
	{
		
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenFreq(freq,50,ondur,offdur);
			while(!(BUTTONS&(1<<btnEnter)));
		}
		if(!(BUTTONS&(1<<btnUp)))
		{
			int temp = intcurpos;
			temp++;
			if(temp>3)
			{
				temp=0;
				disp7();
			}
			if(temp==3)
			{
				disp3();
				showData(dur,2,0x9B);
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			int temp = intcurpos;
			temp--;
			if(temp<0)
			{
				temp=3;
				disp3();
				showData(dur,2,0x9B);
			}
			if(temp==2)
			{
				disp7();
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnLeft)))
		{
			switch(intcurpos)
			{
				case 0:
					freq-=10;
					if(freq==0)
					{
						freq =1;
					}
					if(freq<0)
					{
						freq =150;
					}
					showData(freq,3,0xCC);
				break;
				case 1:
					ondur --;
					if(ondur<1)
						ondur=9;
					showData(ondur,1,0x9D);
				break;
				case 2:
					offdur --;
					if(offdur<1)
						offdur=9;
					showData(offdur,1,0xDE);
				break;
				case 3:
					dur -=5;
					if(dur<5)
						dur=60;
					showData(dur,2,0x9B);
				break;
			}
			while(!(BUTTONS&(1<<btnLeft)));
		}
		if(!(BUTTONS&(1<<btnRight)))
		{
			switch(intcurpos)
			{
				case 0:
					freq-=10;
					if(freq==0)
					{
						freq =1;
					}
					if(freq<0)
					{
						freq =150;
					}
					showData(freq,3,0xCC);
				break;
				case 1:
					ondur ++;
					if(ondur>9)
						ondur=1;
					showData(ondur,1,0x9D);
				break;
				case 2:
					offdur ++;
					if(offdur>9)
						offdur=1;
					showData(offdur,1,0xDE);
				break;
				case 3:
					dur +=5;
					if(dur>60)
						dur=5;
					showData(dur,2,0x9B);
				break;
			}
			while(!(BUTTONS&(1<<btnRight)));
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}

void Galvanic()
{
	LCDcmd(0x01);
	LCDdata("  Galvanic");
	disp3();
	LCDcmd(0x9D);
	showData(dur, 2,0x9B);
	loc = 0x9E;
	setintpos(1);
	while((BUTTONS&(1<<btnBack)))
	{
		if(!(BUTTONS&(1<<btnUp)))
		{
			if(dur== 60)
			{
				dur = 5;
			}
			else
				dur += 5;
			showData(dur, 2,0x9B);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			if(dur== 5)
			{
				dur = 60;
			}
			else
				dur -= 5;
			showData(dur, 2,0x9B);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenFreq(120,50,0,0);
			while(!(BUTTONS&(1<<btnEnter)));
			
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}
void IGalvanic()
{
	int pulse = 1, freq = 1;
	ondur =1;
	offdur =1;
	LCDcmd(0x01);
	LCDdata("  IGalvanic");
	disp5();
	intcurpos = 0;
	setintpos(intcurpos);
	showData(freq,3,0xCC);
	showData(pulse,3,0x9C);
	showData(dur,2,0xDB);
	loc = 0xDE;
	while((BUTTONS&(1<<btnBack)))
	{
		
		if(!(BUTTONS&(1<<btnEnter)))
		{
			GenPulse(freq,pulse);
			while(!(BUTTONS&(1<<btnEnter)));
		}
		if(!(BUTTONS&(1<<btnUp)))
		{
			int temp = intcurpos;
			temp++;
			if(temp>2)
			{
				temp=0;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnUp)));
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			int temp = intcurpos;
			temp--;
			if(temp<0)
			{
				temp=2;
			}
			setintpos(temp);
			while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnLeft)))
		{
			switch(intcurpos)
			{
				case 0:
					switch(freq)
					{
						case 1:
							freq=100;
						break;
						case 3:
							freq = 1;
						break;
						case 5:
							freq = 3;
						break;
						case 10:
							freq = 5;
						break;
						case 50:
							freq =10;
						break;
						case 100:
							freq = 50;
						break;
					}
					if(ondur<1)
						ondur = 9;
					showData(freq,3,0xCC);
				break;
				case 1:
					switch(pulse)
					{
						case 1:
							pulse=100;
						break;
						case 3:
							pulse = 1;
						break;
						case 10:
							pulse = 3;
						break;
						case 30:
							pulse =10;
						break;
						case 100:
							pulse = 30;
						break;
					}
					showData(pulse,3,0x9C);
				break;
				case 2:
					dur -= 5;
					if(dur<5)
						dur=60;
					showData(dur,2,0xDB);
				break;
			}
			while(!(BUTTONS&(1<<btnLeft)));
		}
		if(!(BUTTONS&(1<<btnRight)))
		{
			switch(intcurpos)
			{
				case 0:
					switch(freq)
					{
						case 1:
							freq=3;
						break;
						case 3:
							freq = 5;
						break;
						case 5:
							freq = 10;
						break;
						case 10:
							freq = 50;
						break;
						case 50:
							freq =100;
						break;
						case 100:
							freq = 1;
						break;
					}
						showData(freq,3,0xCC);
				break;
				case 1:
					switch(pulse)
					{
						case 1:
							pulse=100;
						break;
						case 3:
							pulse = 1;
						break;
						case 10:
							pulse = 3;
						break;
						case 30:
							pulse =10;
						break;
						case 100:
							pulse = 30;
						break;
					}
					showData(pulse,3,0x9C);
				break;
				case 2:
					dur += 5;
					if(dur>60)
						dur=5;
					showData(dur,2,0xDB);
				break;
			}
			while(!(BUTTONS&(1<<btnRight)));
		}
	}
	while(!(BUTTONS&(1<<btnBack)));
	stopTimers();
}

void GenPulse(int Freq, int ton)
{
	DDRB |= (1<<DDB1);
	time = dur * 4 * 60;
	OCR2 = 244;
	totdur = 0;
	ICR1 = (uint16_t)(F_CPU / (Freq * 64));
	switch(ton)
	{
		case 1:
			OCR1A =ICR1 - 1;
		break;
		case 3:
			OCR1A =ICR1 - 2;
		break;
		case 10:
			OCR1A =ICR1 - 3;
		break;
		case 30:
			OCR1A =ICR1 - 4;
		break;
		case 100:
			OCR1A =ICR1 - 5;
		break;
	}
	OCR1A = ICR1 - (ton);
	if(OCR1A ==0)
	{
		OCR1A =1;
	}
	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
	TCCR1B |= (1 << CS10) | (1<<CS11);   //1024 prescalar
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	
	TCCR2 = (1<<WGM21) | (1<<CS22) | (1<<CS21) | (1<<CS20);
	TIMSK = (1 << OCIE2);
	sei();
}

void GenFreq(int Freq, int DutyCycle, int on, int off)
{
	DDRB |= (1<<DDB1);
	time = dur * 4 * 60;
	OCR2 = 244;
	totdur = on + off;
	adj1 = (time/4)%totdur;
	if(adj1 == 0)
		adj2 = totdur -1;
	else
		adj2=adj1 - 1;
	ICR1 = (uint16_t)(F_CPU / (Freq * 64));
	OCR1A = ICR1 - 1 -(uint16_t)(ICR1*DutyCycle)/100;
	if(OCR1A ==0)
	{
		OCR1A =1;
	}
	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
	TCCR1B |= (1 << CS10) | (1<<CS11);   //1024 prescalar
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	
	TCCR2 = (1<<WGM21) | (1<<CS22) | (1<<CS21) | (1<<CS20);
	TIMSK = (1 << OCIE2);
	sei();
}
void LCDcmd(char msg)
{
	LCD &= ~(1<<rs);
	LCD = (LCD & 0x0F)| (msg & 0xF0);
	LCD &= ~(1<<en);
	_delay_ms(20);
	LCD |= (1<<en);
	LCD = (LCD & 0x0F) | ((msg<<4) & 0xF0);
	LCD &= ~(1<<en);
	_delay_ms(20);
	LCD |= (1<<en);
}
void LCDdata(char* msg)
{
	while(*msg != '\0')
	{
	LCD |= (1<<rs);
	LCD = (LCD & 0x0F) | (*msg & 0xF0);
	LCD &= ~(1<<en);
	_delay_ms(20);
	LCD |= (1<<en);
	LCD = (LCD & 0x0F) | (((*msg)<<4) & 0xF0);
	LCD &= ~(1<<en);
	_delay_ms(20);
	LCD |= (1<<en);
	msg++;
	}
}

void display1()
{
	LCDcmd(0x01);
	LCDcmd(0x80);
	LCDdata("   -:Select Mode:-");
	LCDcmd(0xc0);
	LCDdata("  Faradiac");
	LCDcmd(0x94);
	LCDdata("  Sfaradiac");
	LCDcmd(0xD4);
	LCDdata("  Galvanic");
}

void display2()
{
	LCDcmd(0x01);
	LCDcmd(0x80);
	LCDdata("   -:Select Mode:-");
	LCDcmd(0xc0);
	LCDdata("  IGalvanic");
	LCDcmd(0x94);
	LCDdata("  Tens Normal");
	LCDcmd(0xD4);
	LCDdata("  Tens Burst");
}

void initLCD()
{
	LCDcmd(0x02);
	LCDcmd(0x28);
	LCDcmd(0x0C);
	LCDcmd(0x01);
}

void setpos(int index)
{
	switch(curpos)
	{
		case 0:
		case 3:
			LCDcmd(0xC0);
			break;
		case 1:
		case 4:
			LCDcmd(0x94);
			break;
		case 2:
		case 5:
			LCDcmd(0xD4);
			break;
	}
	LCDdata(" ");
	curpos = index;
	disppos();
}

void disppos()
{
switch(curpos)
	{
		case 0:
		case 3:
			LCDcmd(0xC0);
			break;
		case 1:
		case 4:
			LCDcmd(0x94);
			break;
		case 2:
		case 5:
			LCDcmd(0xD4);
			break;
	}
	LCDdata("*");
}
int main()
{
	PORTD |= 0xF3;
	PORTB |= 0xFD;
	DDRB &= 0x02;
	DDRD |= (uint8_t)(0xF3);
	_delay_ms(50);
	initLCD();
	LCDcmd(0x80);
	display1();
	disppos();
	while(1)
	{
		if((selpos<3) && (dispno))
		{
			display1();
			dispno = 0;
			disppos();
		}
		if((selpos>=3) && (!dispno))
		{
			display2();
			dispno = 1;
			disppos();
		}
		if(!(BUTTONS&(1<<btnDown)))
		{
			if(selpos>0)
				selpos--;
			else
				selpos = 5;
			setpos(selpos);
		while(!(BUTTONS&(1<<btnDown)));
		}
		if(!(BUTTONS&(1<<btnUp)))
		{
			if(selpos<5)
				selpos++;
			else
				selpos = 0;
			setpos(selpos);
		while(!(BUTTONS&(1<<btnUp)));
		}
		if((!(BUTTONS&(1<<btnEnter))) && !on)
		{
			on = 1;
			while(!(BUTTONS&(1<<btnEnter)));
			LCDcmd(0x01);
			switch(selpos)
			{
				case 0:
					Faradiac();
				break;
				case 1:
					SFaradiac();
				break;
				case 2:
					Galvanic();
				break;
				case 3:
					IGalvanic();
				break;
				case 4:
					TensNormal();
				break;
				case 5:
					TensBurst();
				break;
				default:
					return 0;
				break;
			}
			intcurpos=0;
			setpos(selpos);
			dispno = !dispno;
			on = 0;
		}
	}
	return 0;
}
