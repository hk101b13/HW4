#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

BufferedSerial pc(USBTX,USBRX); //tx,rx
BufferedSerial uart(A4,A5); //tx,rx
static BufferedSerial xbee(D1, D0);
Ticker servo_ticker;
PwmOut pin5(D5), pin6(D6);
Ticker encoder_ticker;
DigitalIn encoder(D11);
volatile int steps;
volatile int last;
BBCar car(pin5, pin6, servo_ticker);
char recv[1];
int s=1;
int n =0;
float dis[1000];


//ping
DigitalInOut ping(D10);
Timer t;
float val=0.0f;

//xbee
void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
Thread t1;
Thread t2;

void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

void loop()
{    
  for(int i = 0; i < n; i++){
    char str[7];
    sprintf(str, "%.3f ,", dis[i]);
    xbee.write(str,7);
  }
}



int main(){
   uart.set_baud(9600);
   pc.set_baud(9600);

   char xbee_reply[4];

   xbee.set_baud(9600);
   xbee.write("+++", 3);
   xbee.read(&xbee_reply[0], sizeof(xbee_reply[0]));
   xbee.read(&xbee_reply[1], sizeof(xbee_reply[1]));
   if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
      printf("enter AT mode.\r\n");
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
   }

   xbee.write("ATMY 0x264\r\n", 12);
   reply_messange(xbee_reply, "setting MY : 0x264");
   xbee.write("ATDL 0x164\r\n", 12);
   reply_messange(xbee_reply, "setting DL : 0x164");

   xbee.write("ATID 0x1\r\n", 10);
   reply_messange(xbee_reply, "setting PAN ID : 0x1");

   xbee.write("ATWR\r\n", 6);
   reply_messange(xbee_reply, "write config");

   xbee.write("ATMY\r\n", 6);
   check_addr(xbee_reply, "MY");

   xbee.write("ATDL\r\n", 6);
   check_addr(xbee_reply, "DL");

   xbee.write("ATCN\r\n", 6);
   reply_messange(xbee_reply, "exit AT mode");

   while(xbee.readable()){
      char *k = new char[1];
      xbee.read(k,1);
      printf("clear\r\n");
   }

   // start
   printf("start\r\n");
   t1.start(callback(&queue, &EventQueue::dispatch_forever));

   // Setup a serial interrupt function of receiving data from xbee
   xbee.set_blocking(false);
   xbee.sigio(mbed_event_queue()->event(xbee_rx_interrupt));

   t2.start(callback(&queue2, &EventQueue::dispatch_forever));

   encoder_ticker.attach(&encoder_control, 10ms);
   while(1){
      if(uart.readable()){
            printf("yes");
            uart.read(recv, sizeof(recv));
            pc.write(recv, sizeof(recv));
            printf("%c",recv);
            if(recv[0] == 'f'){
               //  car.stop();
               car.goStraight(20);
                }
            else if(recv[0] == 'r'){
               // car.stop();
               steps = 0;
               last = 0;
               //正要往右
               car.turn(20,-0.3);
               // while(steps*6.5*3.14/32 < 2) {
               //        if(recv[0]=='s'|| recv[0]=='k'){
               //            car.stop();
               //            break;
               //        }
               //       ThisThread::sleep_for(100ms);
               // }
               // car.stop();
            }
            else if(recv[0] == 'l'){
               // car.stop();
               steps = 0;
               last = 0;
               car.turn(20,0.4);
               // while(steps*6.5*3.14/32 < 2) {
               //        if(recv[0]=='s'|| recv[0]=='k' ){
               //            car.stop();
               //        }                   
               //       ThisThread::sleep_for(100ms);
               // }
               // car.stop();
            }  
            else if(recv[0] =='s'){
                if(s ==1 ){
                    car.stop();
                    // ThisThread::sleep_for(100ms);
                    car.turn_R(20);
                }
                else{
                    car.stop();
                    loop();
                    ThisThread::sleep_for(1s);
                    break;
                }
            }
            else if(recv[0] =='k'){
                car.stop();
                car.turn_R(20);
                ping.output();
                ping = 0; wait_us(200);
                ping = 1; wait_us(5);
                ping = 0; wait_us(5);
                ping.input();
                while(ping.read() == 0);
                t.start();
                while(ping.read() == 1);
                val = t.read();
                dis[n]= val*17700.4f;
                printf("Ping = %lf\r\n", val*17700.4f);
                t.stop();
                t.reset();
                s++;
                n++;
            }
            else{
                break;
            }
      }
   }
   loop();
   int length = sizeof(dis) / sizeof(dis[0]);
   for(int n = 0; n < length; n++){
        printf("%f",dis[n]);
   }
}

void xbee_rx_interrupt(void)
{
   queue.call(&xbee_rx);
}

void xbee_rx(void)
{
   char buf[500] = {0};
   char outbuf[500] = {0};
   while(xbee.readable()){
      for (int i=0; ; i++) {
         char *recv = new char[1];
         xbee.read(recv, 1);
         buf[i] = *recv;
         if (*recv == '\r') {
         break;
         }
      }

      RPC::call(buf, outbuf);

      printf("%s\r\n", outbuf);
      ThisThread::sleep_for(1s);
   }

}

void reply_messange(char *xbee_reply, char *messange){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
      printf("%s\r\n", messange);
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
      xbee_reply[2] = '\0';
   }
}

void check_addr(char *xbee_reply, char *messenger){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   xbee.read(&xbee_reply[3], 1);
   printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
   xbee_reply[0] = '\0';
   xbee_reply[1] = '\0';
   xbee_reply[2] = '\0';
   xbee_reply[3] = '\0';
}