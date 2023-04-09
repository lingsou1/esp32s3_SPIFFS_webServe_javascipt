/*
接线说明:无

程序说明:此程序用于演示如何通过网页图形界面控制ESP8266的PWM引脚,使用JavaScript编写的图形界面(我自己完全不知道JavaScript是怎么写的)

注意事项:(1)在下面的网站中有SPIFFS的下载方法:
         https://docs.platformio.org/en/latest/platforms/espressif32.html#uploading-files-to-file-system
         在下载程序之前要记得先把 data文件夹中的内容下载到 esp32s3的闪存中去,即先 Build Filesystem Image (编译),
         再 upload Filesystem Image(下载)即可
         相关的内容放在esp32s3中的闪存文件中(即 data 文件夹);网站显示的内容通过data文件夹中的文件决定
         (2)esp32s3的PWM引脚的数字是:0~255,而esp8266的是:0~1023,两个的PWM值是不同的,写程序的时候要注意一下
         


函数示例:无

作者:灵首

时间:2023_3_15

*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <SPIFFS.h>

WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi
WebServer esp32s3_webServe(80);//实例化一个网页服务的对象,端口是80


#define LED_A 10
#define LED_B 11

//相关函数的声明,要先声明了VSCODE才不会报错,否则会报错的
void handleUserRequet();
String getContentType(String filename);
bool handleFileRead(String path);
//上面的三个函数是在使用SPIFF,SwebServe中通用的(都要写)

//下面的这个函数只是该程序中的函数
void handlePWM();




/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}



/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU123","12345678");
  wifi_multi.addAP("LINGSOU12","12345678");
  wifi_multi.addAP("LINGSOU1","12345678");
  wifi_multi.addAP("LINGSOU234","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/*
# brief 启动SPIFFS
# param 无
# retval 无
*/
void SPIFFS_start_init(){
  if(SPIFFS.begin()){
    Serial.print("\nSPIFFS Start!!!");
  }
  else{
    Serial.print("\nSPIFFS Failed to start!!!");
  }
}



/*
# brief esp32s3建立网页服务,设置一个处理用户请求的函数来调整LED灯的PWM值,以一个错误处理函数来解决所有的请求
# param 无
# retval  无
*/
void esp32s3_webServe_init(void){
  esp32s3_webServe.on("/setLED", handlePWM);
  esp32s3_webServe.onNotFound(handleUserRequet);    //用该函数解决除了请求"LED_Control"的所有的请求,包括正常的请求以及错误处理
  esp32s3_webServe.begin();
  Serial.print("\nHTTp esp32s3_webServe started");
}



/*
# brief   获取文件类型
# param   String filename:所请求的文件的文件名
# retval    String:返回字符串,返回该文件的文件类型(就是后缀名)
*/
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}



/*
# brief   处理浏览器HTTP访问,在闪存文件中查询请求的文件

# param   String path:这是用户请求的文件路径,不需要手动输入,例如:
    (
    String webAddress = esp32s3_webServe.uri();   // 获取用户请求网址信息
    bool fileReadOK = handleFileRead(webAddress);   // 通过handleFileRead函数处处理用户访问
    )
    通过形如此类的调用获取(应该是的)

# retval bool型返回,当在闪存文件中查询到存在请求的文件时返回: true,否则返回:false
*/
bool handleFileRead(String path) {            //

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  //在SPIFFS中查找访问的文件,若有则打开文件并返回给服务器
  if (SPIFFS.exists(path)) {                     
    File file = SPIFFS.open(path, "r");          
    esp32s3_webServe.streamFile(file, contentType);
    file.close();                               
    return true;                                
  }
  return false;                                  
}




/*
# brief 获取用户请求的网址信息,并处理用户浏览器的HTTP访问
# param   无
# retval    无
*/
void handleUserRequet() {    
  // 获取用户请求网址信息     
  String reqResource = esp32s3_webServe.uri();   
  Serial.print("reqResource: ");
  Serial.println(reqResource);

  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(reqResource);   

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp32s3_webServe.send(404, "text/plain", "404 Not Found"); 
  }
}


/*
# brief 处理/LED-Control请求,设置PWM值
# param   无
# retval   无
*/ 
// 处理PWM设置请求并对引脚进行PWM设置
void handlePWM(){
  String pwmStr = esp32s3_webServe.arg("pwm"); // 获取用户请求中的PWM数值
  Serial.print("pwmStr = ");Serial.println(pwmStr);//串口输出PWM数值
  
  int pwmVal = pwmStr.toInt();              // 将用户请求中的PWM数值转换为整数
  pwmVal = map(pwmVal,0,100,0,255); // 用户请求数值为0-100，转为0-1023,在esp32s3中是不对的,应该是0~255
  Serial.print("pwmVal = ");Serial.println(pwmVal);

  //实现两个灯亮度相反
  analogWrite(LED_A, pwmVal);         // 实现PWM引脚设置
  analogWrite(LED_B, 255-pwmVal);         // 实现PWM引脚设置
  
  esp32s3_webServe.send(200, "text/plain");//向客户端发送200响应信息
}






void setup() {
  //启动串口通信
  Serial.begin(9600);        
  Serial.println("");

  //设置LED模式
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  //digitalWrite(LED_A,1);
  //digitalWrite(LED_B,1);
  
  //WiFi设置
  wifi_multi_init();//储存多个WiFi
  wifi_multi_con();//自动连接WiFi

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //开启闪存文件系统
  SPIFFS_start_init();
  
  //开启网页服务器功能
  esp32s3_webServe_init();
}

void loop() {
 esp32s3_webServe.handleClient();   // 处理客户端请求
}

