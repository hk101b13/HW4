#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

static BufferedSerial pc(STDIO_UART_TX, STDIO_UART_RX);
static BufferedSerial xbee(D1, D0);
Ticker servo_ticker;
PwmOut pin5(D5), pin6(D6);
Ticker encoder_ticker;
DigitalIn encoder(D11);
volatile int steps;
volatile int last;

BBCar car(pin5, pin6, servo_ticker);
void Car_control(Arguments *in, Reply *out);
RPCFunction control(&Car_control,"Cargo");

RpcDigitalOut myled1(LED1,"myled1");
RpcDigitalOut myled2(LED2,"myled2");
RpcDigitalOut myled3(LED3,"myled3");

void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}


int main(){

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

   xbee.write("ATMY <Remote MY>\r\n", 12);
   reply_messange(xbee_reply, "setting MY : <Remote MY>");
   xbee.write("ATDL <Remote DL>\r\n", 12);
   reply_messange(xbee_reply, "setting DL : <Remote DL>");

   xbee.write("ATID <PAN ID>\r\n", 10);
   reply_messange(xbee_reply, "setting PAN ID : <PAN ID>");

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
   t.start(callback(&queue, &EventQueue::dispatch_forever));

   // Setup a serial interrupt function of receiving data from xbee
   xbee.set_blocking(false);
   xbee.sigio(mbed_event_queue()->event(xbee_rx_interrupt));
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

void Car_control(Arguments *in, Reply *out) {
    int x = in->getArg<double>();
    int y = in->getArg<double>();
    encoder_ticker.attach(&encoder_control, 10ms);
    steps = 0;
    last = 0;

    //第一象限
    if(x>0 && y>0){
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < x) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;
        car.turn_L(100);
        while(steps*6.5*3.14/32 < 8) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;       
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < y) {
            ThisThread::sleep_for(100ms);
        }
        car.stop();       
    }

    //第二象限
    else if(x<0 && y>0){
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < -x) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;
        car.turn_R(100);
        while(steps*6.5*3.14/32 < 8) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;       
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < y) {
            ThisThread::sleep_for(100ms);
        }
        car.stop();       
    }
    
    //第三象限
    else if(x<0 && y<0){
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < -x) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;
        car.turn_L(100);
        while(steps*6.5*3.14/32 < 8) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;       
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < -y) {
            ThisThread::sleep_for(100ms);
        }
        car.stop();       
    }
    
    //第四象限
    else if(x>0 && y<0){
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < x) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;
        car.turn_R(100);
        while(steps*6.5*3.14/32 < 8) {
            ThisThread::sleep_for(100ms);
        }
        car.stop(); 
        steps = 0;
        last = 0;       
        car.goStraight(-100);
        while(steps*6.5*3.14/32 < -y) {
            ThisThread::sleep_for(100ms);
        }
        car.stop();       
    }
    return;
}