#include "mbed.h"
#include "bbcar.h"

BufferedSerial pc(USBTX,USBRX); //tx,rx
BufferedSerial uart(A4,A5); //tx,rx
Ticker servo_ticker;
PwmOut pin5(D5), pin6(D6);
Ticker encoder_ticker;
DigitalIn encoder(D11);
volatile int steps;
volatile int last;
BBCar car(pin5, pin6, servo_ticker);

//ping
DigitalInOut ping(D10);
Timer t;
float val;

char recv[1];

void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

// void ping_1(){
//    float val;
//    while(ping.read() == 1);
//    val = t.read();
//    printf("Ping = %lf\r\n", val*17700.4f);
// }

int main(){
   uart.set_baud(9600);
   encoder_ticker.attach(&encoder_control, 10ms);
   while(1){
      if(uart.readable()){
            uart.read(recv, sizeof(recv));
            pc.write(recv, sizeof(recv));
            printf("%c",recv);
            if(recv[0] =='s'){
            //    sprintf("now is front!");
            car.stop();
            ping.output();
            ping = 0; wait_us(200);
            ping = 1; wait_us(5);
            ping = 0; wait_us(5);

            ping.input();
            while(ping.read() == 0);
            t.start();
            while(ping.read() == 1);
            val = t.read();
            printf("Ping = %lf\r\n", val*17700.4f);
            t.stop();
            t.reset();
      ThisThread::sleep_for(1s);
            }
            else{
               printf("now is offset!");
               car.stop();
               steps = 0;
               last = 0;
               car.turn_R(20);
            }      
      }
   //    //ThisThread::sleep_for(500ms);
   }
}