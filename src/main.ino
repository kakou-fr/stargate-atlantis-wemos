// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include "settings.h"
#include "secret.h"
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "FS.h"

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_SHOW_CORE 0

#include <FastLED.h>
FASTLED_USING_NAMESPACE

/********************   FASTLED *****************/
//leds
CRGB leds1[NUM_LEDS1]; // gate symbol
CRGB leds2[NUM_LEDS2]; // gate chevron
CRGB leds3[NUM_LEDS3]; // light Front Right
CRGB leds4[NUM_LEDS4]; // light Middle Right
CRGB leds5[NUM_LEDS5]; // light Middle Left
CRGB leds6[NUM_LEDS6]; // light Front Left
CRGB leds7[NUM_LEDS7]; // light STAIRS (5,5,4,4,3,3,2,2)
CRGB leds8[NUM_LEDS8]; //

enum strip
{
        symbols,
        chevrons,
        front_right,
        middle_right,
        middle_left,
        front_left,
        stairs,
        vortex
};

typedef struct
{
        strip s;
        CRGB *leds;
        int nb_leds;
        int strip_id;

} strip_struct;

strip_struct ledstrips[8] = {
    {symbols, leds1, NUM_LEDS1, 0},
    {chevrons, leds2, NUM_LEDS2, 1},
    {front_right, leds3, NUM_LEDS3, 2},
    {middle_right, leds4, NUM_LEDS4, 3},
    {middle_left, leds5, NUM_LEDS5, 4},
    {front_left, leds6, NUM_LEDS6, 5},
    {stairs, leds7, NUM_LEDS7, 6},
    {vortex, leds8, NUM_LEDS8, 7}};

#define FASTLED_SHOW_CORE 0
#define BRIGHTNESS 100
#define MILLI_AMPS 2000 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define VOLTS 5

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif

/********************   END FASTLED *****************/

// choose the ntp server that serves your timezone
#define NTP_OFFSET 2 * 60 * 60          // In seconds
#define NTP_INTERVAL 60 * 1000          // In miliseconds
#define NTP_ADDRESS "0.fr.pool.ntp.org" //  NTP SERVER

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

/*********STARGATE************/

void setup()
{
        Serial.begin(115200);
        // We start by connecting to a WiFi network
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
        // Wifi with
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        pinMode(DATA_PIN1, OUTPUT);
        pinMode(DATA_PIN2, OUTPUT);
        pinMode(DATA_PIN3, OUTPUT);
        pinMode(DATA_PIN4, OUTPUT);
        pinMode(DATA_PIN5, OUTPUT);
        pinMode(DATA_PIN6, OUTPUT);
        pinMode(DATA_PIN7, OUTPUT);
        pinMode(DATA_PIN8, OUTPUT);
        while (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
                Serial.println("Connection Failed! Rebooting...");
                delay(500);
                ESP.restart();
        }
        setupOTA();
        for (int i = 0; i < 10; i++)
        {
                ArduinoOTA.handle();
                delay(100);
        }
        SPIFFS.begin();
        initSettings();

        // start server and time client
        server.begin();
        timeClient.begin();

        //led
        //Initialize the lib for the ledstrip
        FastLED.addLeds<WS2812B, DATA_PIN1, GRB>(leds1, NUM_LEDS1); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN2, GRB>(leds2, NUM_LEDS2); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN3, GRB>(leds3, NUM_LEDS3); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN4, GRB>(leds4, NUM_LEDS4); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN5, GRB>(leds5, NUM_LEDS5); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN6, GRB>(leds6, NUM_LEDS6); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN7, GRB>(leds7, NUM_LEDS7); //.setCorrection(TypicalLEDStrip);
        FastLED.addLeds<WS2812B, DATA_PIN8, GRB>(leds8, NUM_LEDS8); //.setCorrection(TypicalLEDStrip);
        FastLED.setMaxRefreshRate(100);
        FastLED.setBrightness(BRIGHTNESS);
        FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MILLI_AMPS);
        //FastLED.setDither(0);
        FastLED.show();
        /****/
        /*fillAll(255, 0, 0);
        FastLED.delay(1000);
        fillAll(0, 255, 0);
        FastLED.delay(1000);
        fillAll(0, 0, 255);
        FastLED.delay(1000);*/
        ClearAllLedData();
        /**/
        Serial.print("Setup done");
        //
}

void loop()
{
        // if OTA called we need this
        ArduinoOTA.handle();
        WiFiClient client = server.available(); // listen for incoming clients

        if (dialing)
        {
                if (Dialling == 0)
                {
                        resetsymbolsON();
                        ledRamp(HIGH);
                        ledStairs(HIGH);

                        fillAll(symbols, R, G, B, FADE_SYMBOLS);
                }
                if (Dialling < Address_Length)
                {
                        dial(Address[Dialling]);
                        if (Dialling == (Address_Length - 1))
                        {

                                Serial.println("Wormhole Established");
                                FastLED.delay(1000);
                                fillAll(symbols, 0, 0, 0, 0);
                                fillAll(vortex, 0, 0, 255, 0);
                                fillAll(chevrons, R, G, B, FADE_SYMBOLS);
                                ledStairs(LOW);
                                ledRamp(LOW);
                                vortex_ON();
                                Serial.println("Wormhole Disengaged");
                                ClearAllLedData();
                                /********/
                                dialing = 0;
                                /******/
                                resetsymbolsON();
                        }
                        Dialling++;
                        if (Dialling > 8)
                        {
                                dialing = 0;
                                resetsymbolsON();
                        }
                }
        }
        else
        {
                if (client)
                { // If a new client connects,
                        clientRequest(client);
                }
                /*
                if(nothing_to_do) {
                        fill_rainbow(ledsChevron,NUM_LEDS_CHEVRONS, thishue, deltahue);
                        fill_rainbow(ledsChevronFINAL,NUM_LEDS_CHEVRONS_FINAL, thishue, deltahue);
                        fill_rainbow(ledsRamps,NUM_LEDS_RAMPS, thishue, deltahue);

                        EVERY_N_MILLISECONDS( 20 ) {
                                thishue++;
                        }                                   // slowly cycle the "base color" through the rainbow
                        FastLED.show();
                }
                */
        }
        /*
        EVERY_N_MILLISECONDS(20)
        {
                pacifica_loop();
                FastLED.show();
        }
        */
}

void initSettings()
{
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("Place this IP address into a browser window");
        // make an ip address you can read
        IPAddress myIP = WiFi.localIP();
        ipStr = String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3]);
}

void resetESPdaily()
{
        // once a day do a restart if noInit in not false is. (sometimes the esp hangs up)
        if (formattedTime == "00:00:02" && noInit == true)
        {
                ESP.restart();
        }
}

/***********  STARGATE ATLANTIS    *********/
int symbolsON[NUM_SYMBOLS + 1];
void resetsymbolsON()
{
        for (int i = 0; i < NUM_SYMBOLS + 1; i++)
                symbolsON[i] = 0;
        old_symbols = SYMBOL4;
}

void ledRamp(int state)
{
        if (state == HIGH)
        {
                fillAll(front_right, 255, 255, 255);
                fillAll(front_left, 255, 255, 255);
                FastLED.delay(500);
                fillAll(middle_right, 255, 255, 255);
                fillAll(middle_left, 255, 255, 255);
                FastLED.delay(500);
        }
        else
        {
                fillAll(front_right, 0, 0, 0);
                fillAll(front_left, 0, 0, 0);
                FastLED.delay(500);
                fillAll(middle_right, 0, 0, 0);
                fillAll(middle_left, 0, 0, 0);
                FastLED.delay(500);
        }
}

void ledStairs(int state)
{
        if (state == HIGH)
        {
                fillAllStairs(1, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(2, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(3, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(4, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(5, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(6, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(7, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
                fillAllStairs(8, R_stairs, G_stairs, B_stairs);
                FastLED.delay(200);
        }
        else
        {
                fillAllStairs(8, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(7, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(6, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(5, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(4, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(3, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(2, 0, 0, 0);
                FastLED.delay(200);
                fillAllStairs(1, 0, 0, 0);
                FastLED.delay(200);
        }
}

void ledSymbols(int state)
{
        if (state == HIGH)
        {
                for (int i = 0; i < 36; i++)
                {
                        setSymbol(i, R_symbols, G_symbols, B_symols, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
                for (int i = 0; i < 9; i++)
                {
                        set_chevron(i, R_chevrons, G_chevrons, B_chevrons, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
        }
        else
        {
                for (int i = 35; i >= 0; i--)
                {
                        setSymbol(i, 0, 0, 0, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
                for (int i = 8; i >= 0; i--)
                {
                        set_chevron(i, 0, 0, 0, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
        }
}

int dial_clk = 1;
void dial(int symbol)
{
        compose_one(symbol, R, G, B, R, G, B, (dial_clk++) % 2);
        FastLED.delay(1000);
}

void dial()
{
        resetsymbolsON();
        ledRamp(HIGH);
        ledStairs(HIGH);

        fillAll(symbols, R_symbols, G_symbols, B_symols, FADE_SYMBOLS);

        FastLED.delay(2000);
        compose_one(SYMBOL5, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 1);
        FastLED.delay(1000);
        compose_one(SYMBOL6, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 0);
        FastLED.delay(1000);
        compose_one(SYMBOL7, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 1);
        FastLED.delay(1000);
        compose_one(SYMBOL1, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 0);
        FastLED.delay(1000);
        compose_one(SYMBOL2, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 0);
        FastLED.delay(1000);
        compose_one(SYMBOL3, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 0);
        FastLED.delay(1000);
        compose_one(SYMBOL4, R_symbols, G_symbols, B_symols, R_chevrons, G_chevrons, B_chevrons, 1);
        FastLED.delay(1000);
        fillAll(vortex, 0, 0, 255, 0);
        for (int i = 0; i < 5; i++)
        {
                fillAll(symbols, R_symbols, G_symbols, B_symols);
                FastLED.delay(50);
                fillAll(chevrons, 0, 0, 255);
                FastLED.delay(50);
                fillAll(vortex, 0, 0, 255, 0);
                FastLED.delay(50);
                fillAll(symbols, R_symbols, G_symbols, B_symols, FADE_SYMBOLS);
                FastLED.delay(50);
                fillAll(vortex, 0, 0, 255, FADE_SYMBOLS);
                FastLED.delay(50);
                fillAll(chevrons, R_chevrons, G_chevrons, B_chevrons);
                FastLED.delay(50);
        }
        fillAll(symbols, 0, 0, 0, 0);
        fillAll(vortex, 0, 0, 255, 0);
        fillAll(chevrons, R_chevrons, G_chevrons, B_chevrons, FADE_SYMBOLS);
        ledStairs(LOW);
        ledRamp(LOW);
        vortex_ON();
        ClearAllLedData();
        resetsymbolsON();
}

void vortex_ON()
{
        long time = millis();
        long current = time;
        FastLED.setBrightness(255);
        while (current - time < 10000)
        {
                pacifica_loop();
                FastLED.show();
                FastLED.delay(20);
                current = millis();
        }
        FastLED.setBrightness(BRIGHTNESS);
}

/************    FASTLED  HELPERS   *******/
strip_struct getStrip_struct(strip LEDS_STRIP_ID)
{
        switch (LEDS_STRIP_ID)
        {
        case symbols:
                return ledstrips[0];
                break;
        case chevrons:
                return ledstrips[1];
                break;
        case front_right:
                return ledstrips[2];
                break;
        case middle_right:
                return ledstrips[3];
                break;
        case middle_left:
                return ledstrips[4];
                break;
        case front_left:
                return ledstrips[5];
                break;
        case stairs:
                return ledstrips[6];
                break;
        case vortex:
                return ledstrips[7];
                break;
        }
        return ledstrips[0];
}

void fillAllStairs(int step_number, int r, int g, int b)
{
        strip_struct tmp = getStrip_struct(stairs);

        switch (step_number)
        {
        case 1:
                for (int l = 0; l < 5; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 2:
                for (int l = 5; l < 9; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 3:
                for (int l = 9; l < 13; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 4:
                for (int l = 13; l < 16; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 5:
                for (int l = 16; l < 19; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 6:
                for (int l = 19; l < 22; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 7:
                for (int l = 22; l < 24; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        case 8:
                for (int l = 24; l < 26; l++)
                {
                        tmp.leds[l].setRGB(r, g, b);
                }
                break;
        default:
                break;
        }
        FastLED.show();
}

void set_chevron(int num, int r, int g, int b, int fade)
{
        setPixel(chevrons, num * 3, r, g, b, fade);
        setPixel(chevrons, num * 3 + 1, r, g, b, fade);
        setPixel(chevrons, num * 3 + 2, r, g, b, fade);
}

void fillAll(strip ledstrip, int r, int g, int b, int fade)
{
        strip_struct tmp = getStrip_struct(ledstrip);
        for (int l = 0; l < tmp.nb_leds; l++)
        {
                tmp.leds[l].setRGB(r, g, b);
                if (fade)
                        tmp.leds[l] %= fade;
                else
                        tmp.leds[l].maximizeBrightness();
        }
        FastLED.show();
}

void fillAll(strip ledstrip, int r, int g, int b)
{
        fillAll(ledstrip, r, g, b, 0);
}

void fillAll(int r, int g, int b)
{
        fillAll(symbols, r, g, b);
        fillAll(chevrons, r, g, b);
        fillAll(front_right, r, g, b);
        fillAll(middle_right, r, g, b);
        fillAll(front_left, r, g, b);
        fillAll(middle_left, r, g, b);
        fillAll(stairs, r, g, b);
        fillAll(vortex, r, g, b);
        FastLED.show();
}

void compose_one(int num_symbol, int r, int g, int b, int rc, int gc, int bc, int CLKW)
{
        if (CLKW)
        {
                num_symbol += NUM_SYMBOLS;
                setSymbol(old_symbols, r, g, b, 0);
                FastLED.delay(SPEED_SYMBOLS);
                for (int i = old_symbols; i < num_symbol; i++)
                {
                        setSymbol(i % NUM_SYMBOLS, r, g, b, FADE_SYMBOLS);
                        setSymbol((i + 1) % NUM_SYMBOLS, r, g, b, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
                symbolsON[(num_symbol % NUM_SYMBOLS)] = 1;
                set_chevron((num_symbol % NUM_SYMBOLS) / 4, rc, gc, bc, 0);
                FastLED.delay(SPEED_SYMBOLS);
        }
        else
        {
                if (old_symbols < num_symbol)
                        old_symbols += NUM_SYMBOLS;
                setSymbol(old_symbols, r, g, b, 0);
                FastLED.delay(SPEED_SYMBOLS);
                for (int i = old_symbols; i > num_symbol; i--)
                {
                        setSymbol(i % NUM_SYMBOLS, r, g, b, FADE_SYMBOLS);
                        setSymbol((i - 1) % NUM_SYMBOLS, r, g, b, 0);
                        FastLED.delay(SPEED_SYMBOLS);
                }
                symbolsON[(num_symbol % NUM_SYMBOLS)] = 1;
                FastLED.delay(SPEED_SYMBOLS);
                set_chevron((num_symbol % NUM_SYMBOLS) / 4, rc, gc, bc, 0);
        }
        old_symbols = (num_symbol % NUM_SYMBOLS);
}

void setSymbol(int num, int r, int g, int b, int fade)
{
        if (fade && symbolsON[num])
                return;
        setPixel(symbols, num * 2, r, g, b, fade);
        setPixel(symbols, num * 2 + 1, r, g, b, fade);
}

void setPixel(strip ledstrip, int num, int r, int g, int b, int fade)
{
        strip_struct tmp = getStrip_struct(ledstrip);
        tmp.leds[num].setRGB(r, g, b);
        if (fade)
                tmp.leds[num] %= fade;
        else
                tmp.leds[num].maximizeBrightness();
        FastLED.show();
}

/*** lEDS ****/
//Clears the data for all configured ledstrip
void ClearAllLedData()
{
        fillAll(0, 0, 0);
}

/************   VORTEX   ***********/
//////////////////////////////////////////////////////////////////////////
//
// The code for this animation is more complicated than other examples, and
// while it is "ready to run", and documented in general, it is probably not
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques.
//
//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then
// another filter is applied that adds "whitecaps" of brightness where the
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
CRGBPalette16 pacifica_palette_1 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50};
CRGBPalette16 pacifica_palette_2 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F};
CRGBPalette16 pacifica_palette_3 =
    {0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
     0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF};

void pacifica_loop()
{
        // Increment the four "color index start" counters, one for each wave layer.
        // Each is incremented at a different speed, and the speeds vary over time.
        static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
        static uint32_t sLastms = 0;
        uint32_t ms = GET_MILLIS();
        uint32_t deltams = ms - sLastms;
        sLastms = ms;
        uint16_t speedfactor1 = beatsin16(3, 179, 269);
        uint16_t speedfactor2 = beatsin16(4, 179, 269);
        uint32_t deltams1 = (deltams * speedfactor1) / 256;
        uint32_t deltams2 = (deltams * speedfactor2) / 256;
        uint32_t deltams21 = (deltams1 + deltams2) / 2;
        sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
        sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
        sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
        sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

        // Clear out the LED array to a dim background blue-green
        fill_solid(leds8, NUM_LEDS8, CRGB(2, 6, 10));

        // Render each of four layers, with different scales and speeds, that vary over time
        pacifica_one_layer(pacifica_palette_1, sCIStart1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), 0 - beat16(301));
        pacifica_one_layer(pacifica_palette_2, sCIStart2, beatsin16(4, 6 * 256, 9 * 256), beatsin8(17, 40, 80), beat16(401));
        pacifica_one_layer(pacifica_palette_3, sCIStart3, 6 * 256, beatsin8(9, 10, 38), 0 - beat16(503));
        pacifica_one_layer(pacifica_palette_3, sCIStart4, 5 * 256, beatsin8(8, 10, 28), beat16(601));

        // Add brighter 'whitecaps' where the waves lines up more
        pacifica_add_whitecaps();

        // Deepen the blues and greens a bit
        pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer(CRGBPalette16 &p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
        uint16_t ci = cistart;
        uint16_t waveangle = ioff;
        uint16_t wavescale_half = (wavescale / 2) + 20;
        for (uint16_t i = 0; i < NUM_LEDS8; i++)
        {
                waveangle += 250;
                uint16_t s16 = sin16(waveangle) + 32768;
                uint16_t cs = scale16(s16, wavescale_half) + wavescale_half;
                ci += cs;
                uint16_t sindex16 = sin16(ci) + 32768;
                uint8_t sindex8 = scale16(sindex16, 240);
                CRGB c = ColorFromPalette(p, sindex8, bri, LINEARBLEND);
                leds8[i] += c;
        }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
        uint8_t basethreshold = beatsin8(9, 55, 65);
        uint8_t wave = beat8(7);

        for (uint16_t i = 0; i < NUM_LEDS8; i++)
        {
                uint8_t threshold = scale8(sin8(wave), 20) + basethreshold;
                wave += 7;
                uint8_t l = leds8[i].getAverageLight();
                if (l > threshold)
                {
                        uint8_t overage = l - threshold;
                        uint8_t overage2 = qadd8(overage, overage);
                        leds8[i] += CRGB(overage, overage2, qadd8(overage2, overage2));
                }
        }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
        for (uint16_t i = 0; i < NUM_LEDS8; i++)
        {
                leds8[i].blue = scale8(leds8[i].blue, 255);
                //leds[i].green= scale8( leds[i].green, 10);
                leds8[i] |= CRGB(2, 5, 7);
        }
}

/***********    OTA    ********/

void setupOTA()
{
        ArduinoOTA.setHostname("stargate-atlantis"); // Hostname for OTA
        //ArduinoOTA.setPassword(my_OTA_PW);           // set in credidentials.h
        ArduinoOTA
            .onStart([]() {
                    String type;
                    if (ArduinoOTA.getCommand() == U_FLASH)
                            type = "sketch";
                    else // U_SPIFFS
                            type = "filesystem";

                    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                    Serial.println("Start updating " + type);
            });
        ArduinoOTA.onEnd([]() {
                Serial.println("\nEnd");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR)
                        Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR)
                        Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR)
                        Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR)
                        Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR)
                        Serial.println("End Failed");
        });

        ArduinoOTA.begin(); // Start OTA
}

/*******   WEB   ************/

String outputRamp_LightsState = "off";
String outputChevron_LightsState = "off";
String outputSTAIRS_LightsState = "off";

void clientRequest(WiFiClient client)
{
        Serial.println("==========================");
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
                if (client.available())
                {                               // if there's bytes to read from the client,
                        char c = client.read(); // read a byte, then
                        Serial.write(c);        // print it out the serial monitor
                        header += c;
                        if (c == '\n')
                        { // if the byte is a newline character
                                // if the current line is blank, you got two newline characters in a row.
                                // that's the end of the client HTTP request, so send a response:
                                if (currentLine.length() == 0)
                                {
                                        if (header.indexOf("GET /") >= 0)
                                        {
                                                String URI = midString(header, "GET ", " ");
                                                if (loadFromSpiffs(URI, client))
                                                        break;
                                        }
                                        if (header.indexOf("POST /dialstatus") >= 0)
                                        {
                                                client.println(F("HTTP/1.1 204 No Content"));
                                                client.println(F("Content-Type: text/html"));
                                                client.println(F("Content-Length: 0"));
                                                client.println(F("Connection: close"));
                                                client.println();
                                                client.flush();
                                                client.stop();
                                                break;
                                                /*
                                                   if DialProgram.is_dialing:
                                                   self.send_response(200, '1')
                                                   else:
                                                   self.send_response(204, '0')
                                                   return
                                                 */
                                        }
                                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                        // and a content-type so the client knows what's coming, then a blank line:
                                        client.println("HTTP/1.1 200 OK");
                                        client.println("Content-type:text/html");
                                        client.println("Connection: close");
                                        client.println();
                                        /**/
                                        if (header.indexOf("POST /dialing/on/Earth") >= 0)
                                        {
                                                dial();
                                        }
                                        if (header.indexOf("POST /dialing/on/Abydos") >= 0)
                                        {
                                                memcpy(Address, Address_Abydos, 7 * sizeof(int));
                                                Address_Length = 7;
                                                R = R_Abydos;
                                                G = G_Abydos;
                                                B = B_Abydos;
                                                dialing = 1;
                                                Dialling = 0;
                                        }
                                        if (header.indexOf("POST /dialing/on/Asgard") >= 0)
                                        {
                                                memcpy(Address, Address_Asgard, 7 * sizeof(int));
                                                Address_Length = 7;
                                                R = R_Asgard;
                                                G = G_Asgard;
                                                B = B_Asgard;
                                                dialing = 1;
                                                Dialling = 0;
                                        }
                                        if (header.indexOf("POST /dialing/on/Destiny") >= 0)
                                        {
                                                memcpy(Address, Address_Destiny, 9 * sizeof(int));
                                                Address_Length = 9;
                                                R = R_Destiny;
                                                G = G_Destiny;
                                                B = B_Destiny;
                                                dialing = 1;
                                                Dialling = 0;
                                        }
                                        // turns the GPIOs on and off
                                        if (header.indexOf("POST /Ramp_Lights/on") >= 0)
                                        {
                                                Serial.println("GPIO Ramp_Lights on");
                                                outputRamp_LightsState = "on";
                                                ledRamp(HIGH);
                                        }
                                        else if (header.indexOf("POST /Ramp_Lights/off") >= 0)
                                        {
                                                Serial.println("GPIO Ramp_Lights off");
                                                outputRamp_LightsState = "off";
                                                ledRamp(LOW);
                                        }
                                        if (header.indexOf("POST /STAIRS/on") >= 0)
                                        {
                                                outputSTAIRS_LightsState = "on";
                                                ledStairs(HIGH);
                                        }
                                        if (header.indexOf("POST /STAIRS/off") >= 0)
                                        {
                                                nothing_to_do = 0;
                                                outputSTAIRS_LightsState = "off";
                                                ledStairs(LOW);
                                        }
                                        // turns the GPIOs on and off
                                        if (header.indexOf("POST /Ramp_Chevrons/on") >= 0)
                                        {
                                                Serial.println("GPIO Ramp_Lights on");
                                                outputChevron_LightsState = "on";
                                                ledSymbols(HIGH);
                                        }
                                        else if (header.indexOf("POST /Ramp_Chevrons/off") >= 0)
                                        {
                                                Serial.println("GPIO Ramp_Lights off");
                                                outputChevron_LightsState = "off";
                                                ledSymbols(LOW);
                                        }
                                        else if (header.indexOf("POST /update") >= 0)
                                        {
                                                //
                                                String line = "";
                                                while (client.available() && (c = client.read()) != -1)
                                                {
                                                        Serial.write(c); // print it out the serial monitor
                                                        line += c;
                                                }
                                                Serial.print("==>");
                                                Serial.println(line);
                                                // {"anim":2,"sequence":[2,3,4,6,18,17,30]}
                                                String tmp = midString(line, "[", "]");
                                                R = R_chevrons;
                                                G = G_chevrons;
                                                B = B_chevrons;
                                                Address_Length = 0;
                                                while (tmp.indexOf(",") != -1)
                                                {
                                                        Address[Address_Length] = tmp.substring(0, tmp.indexOf(",")).toInt();
                                                        Address_Length++;
                                                        tmp = tmp.substring(tmp.indexOf(",") + 1);
                                                }
                                                Address[Address_Length] = tmp.toInt();
                                                Address_Length++;
                                                dialing = 1;
                                                Dialling = 0;
                                        }
                                        break;
                                }
                                else
                                { // if you got a newline, then clear currentLine
                                        currentLine = "";
                                }
                        }
                        else if (c != '\r')
                        {                         // if you got anything else but a carriage return character,
                                currentLine += c; // add it to the end of the currentLine
                        }
                }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
}

String midString(String str, String start, String finish)
{
        int locStart = str.indexOf(start);
        if (locStart == -1)
                return "";
        locStart += start.length();
        int locFinish = str.indexOf(finish, locStart);
        if (locFinish == -1)
                return "";
        return str.substring(locStart, locFinish);
}

bool loadFromSpiffs(String path, WiFiClient client)
{
        String dataType = "text/plain";
        if (path.endsWith("/"))
                path += "index.htm";

        if (!SPIFFS.exists(path))
                return false;
        if (path.endsWith(".src"))
                path = path.substring(0, path.lastIndexOf("."));
        else if (path.endsWith(".svg"))
                dataType = "image/svg+xml";
        else if (path.endsWith(".html"))
                dataType = "text/html";
        else if (path.endsWith(".htm"))
                dataType = "text/html";
        else if (path.endsWith(".css"))
                dataType = "text/css";
        else if (path.endsWith(".js"))
                dataType = "application/javascript";
        else if (path.endsWith(".png"))
                dataType = "image/png";
        else if (path.endsWith(".gif"))
                dataType = "image/gif";
        else if (path.endsWith(".jpg"))
                dataType = "image/jpeg";
        else if (path.endsWith(".ico"))
                dataType = "image/x-icon";
        else if (path.endsWith(".xml"))
                dataType = "text/xml";
        else if (path.endsWith(".pdf"))
                dataType = "application/pdf";
        else if (path.endsWith(".zip"))
                dataType = "application/zip";
        File dataFile = SPIFFS.open(path.c_str(), "r");

        /*if (server.hasArg("download")) dataType = "application/octet-stream";
           if (server.streamFile(dataFile, dataType) != dataFile.size()) {
           }
         */
        //client.print(F("POST "));
        //client.print(path);
        client.println("HTTP/1.1 200 OK");
        client.print(F("Content-Type: "));
        client.println(dataType);
        client.print(F("Host: "));
        client.println(WiFi.localIP());
        if (!(path.endsWith(".css") || path.endsWith("/")))
                client.println(F("Cache-Control: max-age=864000"));
        client.println(F("Connection: close"));
        client.print(F("Content-Length: "));
        client.println(dataFile.size());
        client.println();
        while (dataFile.available())
        {
                client.write(dataFile.read());
        }
        client.flush();
        dataFile.close();
        client.stop();

        return true;
}
