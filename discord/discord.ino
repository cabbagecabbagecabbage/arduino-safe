//credit to https://gist.github.com/Matheus-Garbelini/2cd780aed2eddbe17eb4adb5eca42bd6 for connecting to school wifi
//credit to https://github.com/JChristensen/Timezone for fetching the current time
//credit to https://github.com/usini/usini_discord_webHook for sending discord messages


#include <ESP8266WiFi.h>
#include <Discord_WebHook.h>
#include <Timezone.h>

TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240};
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};
Timezone myTZ(myDST, mySTD);
TimeChangeRule *tcr;

Discord_Webhook discord;
String DISCORD_WEBHOOK = "<insert webhook link>";

extern "C" {
#include "user_interface.h"
#include "wpa2_enterprise.h"
#include "c_types.h"
}

char ssid[] = "<insert wifi name>";
char username[] = ;
char identity[] = "<insert username>"; //note: since this was connecting to the school wifi, the login was the school account. another (easier) method should be used for connecting to home wifi.
char password[] = "<insert password>";
uint8_t target_esp_mac[6] = {0x24, 0x0a, 0xc4, 0x9a, 0x58, 0x28};

bool opened = false;

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(500);
  Serial.setDebugOutput(true);
  Serial.printf("SDK version: %s\n", system_get_sdk_version());
  Serial.printf("Free Heap: %4d\n",ESP.getFreeHeap());
  wifi_set_opmode(STATION_MODE);
  struct station_config wifi_config;
  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);
  strcpy((char*)wifi_config.password, password);
  wifi_station_set_config(&wifi_config);
  wifi_set_macaddr(STATION_IF,target_esp_mac);
  wifi_station_set_wpa2_enterprise_auth(1);
  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();
  wifi_station_clear_enterprise_identity();
  wifi_station_clear_enterprise_username();
  wifi_station_clear_enterprise_password();
  wifi_station_clear_enterprise_new_password();
  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));
  wifi_station_connect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  discord.begin(DISCORD_WEBHOOK);
  discord.send("Safe connected to discord!");
  
  pinMode(5,INPUT);
  pinMode(4,INPUT);
  setTime(myTZ.toUTC(compileTime()));
}

void loop() {
  int buttonState = digitalRead(5);
  if (buttonState == LOW) {
    discord.send("Door Closed!");
  }
  int greenState = digitalRead(4);
  if (greenState == HIGH and opened == false){
    time_t utc = now();
    time_t local = myTZ.toLocal(utc, &tcr);
    discord.send("Door opened!");
    printDateTime(local, tcr -> abbrev);
    opened = true;
  }
  else if (greenState == LOW){
    opened = false;
  }
}


time_t compileTime(){
    const time_t FUDGE(10);
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[4], *m;
    tmElements_t tm;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    time_t t = makeTime(tm);
    return t + FUDGE;
}

void printDateTime(time_t t, const char *tz){
    char buf[32];
    char m[4];
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
        hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
    discord.send(buf);
    Serial.println(buf);
}
