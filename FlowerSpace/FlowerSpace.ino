#include "ESP8266.h"
#include "dht11.h"
#include "SoftwareSerial.h"
#include "HX711.h"
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 
#define setFont_L u8g.setFont(u8g_font_7x13)
/*************************配置************************/
#define SSID "zwc"    //填写2.4GHz的WIFI名称，不要使用校园网
#define PASSWORD "1234567890"//填写自己的WIFI密码
#define HOST_NAME "api.heclouds.com"  //API主机名称，连接到OneNET平台，无需修改
#define DEVICE_ID "657094838"       //填写自己的OneNet设备ID
#define HOST_PORT (80)                //API端口，连接到OneNET平台，无需修改
#define APIKey "IOfxAQLbqM=x17OsPFEB78zV3vw="; //与设备绑定的APIKey
#define INTERVAL_SENSOR 5000 //定义发送时间间隔
#define LightLimit 512 //光照下限
#define SoilLimit 512 //土壤湿度下限
/*************************配置************************/


/*************************配置DHT11************************/
dht11 DHT11;
#define DHT11PIN 4
/*************************配置DHT11************************/

/**************定义ESP8266所连接的软串口**************/
/* Arduino上的软串口RX定义为D3,
 * 接ESP8266上的TX口,
 * Arduino上的软串口TX定义为D2,
 * 接ESP8266上的RX口.*/
SoftwareSerial mySerial(3, 2);
ESP8266 wifi(mySerial);
/**************定义ESP8266所连接的软串口**************/


void setup()
{
  /**************设置输出口**************/
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  /**************设置输出口**************/
  
  /**************初始化ESP8266所连接的软串口**************/
  mySerial.begin(115200); //初始化软串口
  Serial.begin(9600);     //初始化串口
  Serial.print("开始初始化：\r\n"); 
  /**************初始化ESP8266所连接的软串口**************/
  
  /*************************ESP8266初始化************************/
  Serial.print("FW Version: ");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStation()) {
    Serial.print("to station ok\r\n");
  } else {
    Serial.print("to station err\r\n");
  }
  /*************************ESP8266初始化************************/
  
  /*************************ESP8266接入WIFI************************/
  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  Serial.println("");
  Serial.print("DHT11 LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);

  mySerial.println("AT+UART_CUR=9600,8,1,0,0");
  mySerial.begin(9600);
  Serial.println("setup end\r\n");
  /*************************ESP8266接入WIFI************************/

  /*************************HX711配置************************/
  Init_Hx711();       //初始化HX711模块连接的IO设置
  delay(1000);
  Get_Maopi();        //获取毛皮
  /*************************HX711配置************************/
}

/*************************变量声明************************/
float sensor_hum =0;    //环境湿度
float sensor_tem =0;    //环境温度
float sensor_soil=0;    //土壤湿度
float sensor_light=0;   //原光照
float sensor_light1=0;  //开灯后光照
float sensor_water=0;   //水箱重量
float Weight=0;         //HX711读取的重量
char light_flag='0';    //太阳灯状态
char water_flag='0';    //水泵状态
unsigned long net_time1 = millis();//数据上传标记
/*************************变量声明************************/


void loop() {
  u8g.firstPage();
  do{
    setFont_L;
    
    u8g.setPrintPos(0,10);  //oled显示温度
    u8g.print("Temperature:");
    u8g.print(int(sensor_tem));
    u8g.print("oC");
    
    u8g.setPrintPos(0,30);  //oled显示土壤湿度
    u8g.print("SoilMoisture:");
    u8g.print(int(sensor_soil/1024*100));
    u8g.print("%");

    u8g.setPrintPos(0,50);  //oled显示水箱重量
    u8g.print("SurplusWater:");
    u8g.print(int(sensor_water*1000));
    u8g.print("g");
    
    loop1();//由于u8g实时显示是在循环中，故将原本的循环loop1放入该循环    
  }while(u8g.nextPage());
}


void loop1()
{
  /*************************millis溢出处理************************/  
  if (net_time1 > millis())
    net_time1 = millis();     //millis()函数可取Arduino开始运行后的毫秒数，约50天后溢出归零
  /*************************millis溢出处理************************/
 
  if (millis() - net_time1 > INTERVAL_SENSOR) //发送数据时间间隔
  {
    /*************************DHTLL读取************************/  
    int chk = DHT11.read(DHT11PIN);  
    Serial.print("Read sensor: ");
    switch (chk) {
      case DHTLIB_OK:
        Serial.println("OK");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Checksum error");
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.println("Time out error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }//调用DHTLL读取库函数，读取数据并输出读取结果
    
    sensor_hum = (float)DHT11.humidity;
    sensor_tem = (float)DHT11.temperature;//获得DHTLL数据
    
    Serial.print("Humidity (%): ");
    Serial.println(sensor_hum, 2);
    Serial.print("Temperature (oC): ");
    Serial.println(sensor_tem, 2);
    Serial.println("");//打印读取的DHTLL数据
    /*************************DHTLL读取************************/ 
    
    /*************************土壤湿度************************/
    float soil=analogRead(A0);
    sensor_soil=1024-soil;
    if(sensor_soil<SoilLimit){
      digitalWrite(8,HIGH);
      water_flag='1';
      delay(10000);
      digitalWrite(8,LOW);
    }else{
      water_flag='0';
    }
    /*************************土壤湿度************************/
      
    /*************************光照强度************************/
    digitalWrite(9,LOW);
    delay(1000);
    float light=analogRead(A1);//关灯后再读取亮度
    sensor_light=1024-light;
    if(sensor_light<LightLimit){
      digitalWrite(9,HIGH);
      light_flag='1';
    }else{
      light_flag='0';
    }
    delay(1000);
    light=analogRead(A1);//开灯后的亮度
    sensor_light1=1024-light;
    /*************************光照强度************************/
    
    /*************************水箱重量************************/
    Weight = Get_Weight();  //计算放在传感器上的重物重量
    sensor_water=Weight*1.3375/1000;
    Serial.print(float(sensor_water),3);  //串口显示重量
    Serial.print(" kg\n");  //显示单位
    Serial.print("\n");   //显示单位
     /*************************水箱重量************************/
    
    /*************************上传数据************************/ 
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接
      Serial.print("create tcp ok\r\n");
      char buf[10];//用来储存浮点数转化为的字符串
      /*************************拼接字符串************************/ 
      String jsonToSend = "{\"Temperature\":";
      dtostrf(sensor_tem, 2, 2, buf); //dtostrf函数可以讲浮点数转化为字符串
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"Humidity\":";
      dtostrf(sensor_hum, 2, 2, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"SoilMoisture\":";
      dtostrf(sensor_soil, 4, 2, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"LightIntensity\":";
      dtostrf(sensor_light, 4, 0, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"LightIntensity1\":";
      dtostrf(sensor_light1, 4, 0, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"TankWeight\":";
      dtostrf(sensor_water, 2, 3, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"WaterFlag\":";
      jsonToSend += "\"" + String(water_flag) + "\"";
      jsonToSend += ",\"LightFlag\":";
      jsonToSend += "\"" + String(light_flag) + "\"";
      jsonToSend += "}";
      /*************************拼接字符串************************/ 

      /*************************拼接POST请求字符串************************/ 
      String postString = "POST /devices/";
      postString += DEVICE_ID;
      postString += "/datapoints?type=3 HTTP/1.1";
      postString += "\r\n";
      postString += "api-key:";
      postString += APIKey;
      postString += "\r\n";
      postString += "Host:api.heclouds.com\r\n";
      postString += "Connection:close\r\n";
      postString += "Content-Length:";
      postString += jsonToSend.length();
      postString += "\r\n";
      postString += "\r\n";
      postString += jsonToSend;
      postString += "\r\n";
      postString += "\r\n";
      postString += "\r\n";
      /*************************拼接POST请求字符串************************/ 
      const char *postArray = postString.c_str(); //将str转化为char数组

      Serial.println(postArray);
      wifi.send((const uint8_t *)postArray, strlen(postArray)); //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
      Serial.println("send success");
      
      if (wifi.releaseTCP()) { //释放TCP连接
        Serial.print("release tcp ok\r\n");
      } else {
        Serial.print("release tcp err\r\n");
      }
      
      postArray = NULL; //清空数组，等待下次传输数据
    } 
    
    else {
      Serial.print("create tcp err\r\n"); //TCP建立失败
    }
    
    net_time1 = millis();//发送完成，net_timel更新
    /*************************上传数据************************/
  }
}
