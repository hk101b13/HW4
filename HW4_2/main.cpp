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
char recv[1];
void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

int main(){
   uart.set_baud(9600);
   encoder_ticker.attach(&encoder_control, 10ms);
   while(1){
      if(uart.readable()){
            uart.read(recv, sizeof(recv));
            pc.write(recv, sizeof(recv));
            if(recv[0] == 'r'){
               printf("now is right!");
               car.stop();
               steps = 0;
               last = 0;
               //正要往右
               car.turn(100,-0.5);
               while(steps*6.5*3.14/32 < 5) {
                     ThisThread::sleep_for(100ms);
               }
               car.stop();
            }
            else if(recv[0] == 'l'){
               printf("now is left!");
               car.stop();
               steps = 0;
               last = 0;
               car.turn(100,0.5);
               while(steps*6.5*3.14/32 < 5) {
                     ThisThread::sleep_for(100ms);
               }
               car.stop();
            }      
      }
   //    //ThisThread::sleep_for(500ms);
   }
}