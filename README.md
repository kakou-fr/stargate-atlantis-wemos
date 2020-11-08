# stargate-atlantis-wemos

Based on the code of : https://github.com/danclarke/WorkingStargateMk2Raspi
STL : https://www.thingiverse.com/thing:3153542

Create a secret.h file into src with  the following code :
```
// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

#define my_OTA_PW "OTA_PW"     // password for OTA updates
```
