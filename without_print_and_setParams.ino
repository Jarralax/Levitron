int pinPWM = 9;
int pinRead = A0;
uint32_t freq, realFreq;
int32_t pwm = 0;
uint32_t timerD = 0;
uint16_t delayD;
int32_t err[] = {0, 0};

uint8_t scale;
uint32_t P, D, I;
int16_t target, readableData, averD, averP;
uint8_t alphaD, alphaP;
int32_t maxPwm, accum, error;
 
void setup()
{
    analogReference(INTERNAL);
    cli(); //запрет прерываний
    //режим вывода не инверсный
    TCCR1A|=(1<<COM1A1);  //1
    TCCR1A&=~(1<<COM1A0);  //0
    
    //режим шим c точной фазой и частотой
    TCCR1A&=~(1<<WGM10);  //0
    TCCR1A&=~(1<<WGM11);  //0
    TCCR1B&=~(1<<WGM12);  //0
    TCCR1B|=(1<<WGM13);  //1
    
    //предделитель 1
    TCCR1B|=(1<<CS10);  //1
    TCCR1B&=~((1<<CS11)|(1<<CS12));  //0 0
    
    sei(); //разрешение прерываний
    
    pwm = 0;
    freq = 30000;
    setFreq();
    pinMode(pinPWM, OUTPUT);
    pinMode(pinRead, INPUT);
    err[0] = analogRead(A0);

    target = 300;
    scale = 22;//bits (max 22 for cliping in func setFreq(), for freq 30 kHz)
    averD = 0, averP = 0;
    alphaD = 5, alphaP = 3;
    delayD = 5000;
    
    maxPwm = static_cast<int32_t>(0.8 * ((uint32_t)1 << scale));
    accum = static_cast<int32_t>(0.3 * ((uint32_t)1 << scale));
    P = 15000;
    D = 60000;
    I = 1;
}
 
void loop()
{
  innerFunc();
}

void innerFunc()
{
for(;;)
  {
    readableData = analogRead(A0);
    averWindP();
    error = target - averP;
    averWindD();
    if(micros() - timerD > delayD)
    {
      timerD = micros();
      err[0] = err[1];
      err[1] = target - averD;
    }
    if(readableData > 50)
    {
      if(readableData < 900)
      {
        accum += error;
        pwm = error * P + ((err[1] - err[0]) * D) + accum * I;
        if(pwm > maxPwm)
        {
          pwm = maxPwm;
        }
        else if(pwm < 0)
        {
          pwm = 0;
        }
      }
    }
    else
    {
      pwm = 0;
    }
    setFreq();
  }
}

void setFreq(){
  realFreq = 8000000UL / freq;
  ICR1 = realFreq;
  OCR1A = (uint16_t)((realFreq * (uint32_t)pwm) >> scale);
}
void averWindD()
{
  averD = ((alphaD-1)*averD + readableData)/alphaD;
}
void averWindP()
{
  averP = ((alphaP-1)*averP + readableData)/alphaP;
}
