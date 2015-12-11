#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//100kHz
//External pull-up resistor required 
//Standard 7 bit I2C address for iAQ-core is decimal 90 or hexadecimal 0x5A
//The communication with the iAQ-core starts with 0xB5 for reading data. 

#define VOC_ADDRESS 0x5A  // I2C address of iAQ-engine: 10110101
#define RECV_DATA_SIZE 9

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() 
{
        Serial.begin(9600);
        Serial2.begin(9600);
        Serial3.begin(19200);
        Wire.begin();

        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        // Clear the buffer.
        display.clearDisplay();
}

int pad = 0;

unsigned int co2_value = 0;
unsigned int voc_value = 0;
unsigned int resistance = 0;
int count = 0;
byte buffer[RECV_DATA_SIZE];

char ZG01CV_pb;
const byte ZG01CV_command[4]={0x23, 0x31, 0x30, 0x0D};
char ZG01CV_co2_data[4]={0x00,};
char ZG01CV_temp_data[4]={0x00,};

const byte CM1104_command[4]={0x11, 0x01, 0x01, 0xED};
byte CM1104_co2_data[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void loop() 
{
        display.clearDisplay();
        //-------------------------------------------
        Wire.requestFrom(VOC_ADDRESS, RECV_DATA_SIZE);
        co2_value = 0;
        voc_value = 0;
        count = 0;
        resistance = 0;
        for (int i=0;i<RECV_DATA_SIZE;i++){
                buffer[i] = 0x00;
        }
        delay(20);

        while(Wire.available()){
                buffer[count] = Wire.read();
                count++;
                if(count == RECV_DATA_SIZE){
                        break;
                }
        }
        co2_value = (unsigned int) (buffer[pad+0]<<8 | buffer[pad+1]);
        voc_value = (unsigned int) (buffer[pad+7]<<8 | buffer[pad+8]);

        Serial.print("CO2:");Serial.println(co2_value);
        Serial.print("VOC:");Serial.println(voc_value);
        display.setCursor(0,0);
        display.print("iAQ:");
        display.println(co2_value);
        display.display();
        // resistance = ((buffer[pad+3]& 0x00) | (buffer[pad+4]<<16) | (buffer[pad+5]<<8) | buffer[pad+6]);
        // Serial.print("Resistance: ");Serial.println(resistance);
        checkStatus(buffer[pad+2]);
        //-------------------------------------------
        Serial3.write(ZG01CV_command,4);
        while (Serial3.available()){
                ZG01CV_pb = Serial3.read();
                if (ZG01CV_pb == 'P'){
                        memset(ZG01CV_co2_data,0x00,4);
                        while(!Serial3.available()){}
                        ZG01CV_co2_data[0]  = (char)Serial3.read();

                        while(!Serial3.available()){}
                        ZG01CV_co2_data[1]  = (char)Serial3.read();

                        while(!Serial3.available()){}
                        ZG01CV_co2_data[2]  = (char)Serial3.read();

                        while(!Serial3.available()){}
                        ZG01CV_co2_data[4]  = (char)Serial3.read();

                        Serial.print("ZG01CV_CO2:");
                        Serial.println(hex2int(ZG01CV_co2_data, 4));
                        display.setCursor(0,25);
                        display.print("ZG:");
                        display.println(hex2int(ZG01CV_co2_data, 4));

                        //-------------------------------------------
                        Serial2.write(CM1104_command,8);
                        while(!Serial2.available()){}
                        memset(CM1104_co2_data,0x00,4);
                                for(int ii=0; ii<8; ii++){
                                        while(!Serial2.available()){}
                                        CM1104_co2_data[ii] = Serial2.read();
                                }
                                Serial.print("CM1104_CO2:");
                                Serial.println(CM1104_co2_data[3]<<8 | CM1104_co2_data[4]);
                                display.setCursor(0,50);
                                display.print("CM:");
                                display.println(CM1104_co2_data[3]<<8 | CM1104_co2_data[4]);
                                display.display();
                                break;
                       //-------------------------------------------
                        break;
                }
        }
        Serial.println("");
        display.display();
        //-------------------------------------------
        delay(5000);
}

void checkStatus(byte statu)
{
        if(statu == 0x10)
        {
                Serial.println("Warming up...");
        }
        else if(statu == 0x00)
        {
                Serial.println("Ready");  
        }
        else if(statu == 0x01)
        {
                Serial.println("Busy");  
        }
        else if(statu == 0x80)
        {
                Serial.println("Error");  
        }
        else
                Serial.println("No Status, check module");
}

unsigned long hex2int(char *a, unsigned int len)
{
        int i;
        unsigned long val = 0;

        for(i=0;i<len;i++)
                if(a[i] <= 57)
                        val += (a[i]-48)*(1<<(4*(len-1-i)));
                else
                        val += (a[i]-55)*(1<<(4*(len-1-i)));
        return val;
}
