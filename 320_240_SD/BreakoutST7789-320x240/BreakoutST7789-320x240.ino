// Adafruit_ImageReader test for Adafruit ST7789 320x240 TFT Breakout for Arduino.
// Demonstrates loading images to the screen, to RAM, and how to query
// image file dimensions.
// Requires three BMP files in root directory of SD card:
// parrot.bmp, miniwoof.bmp and wales.bmp.
// As written, this uses the microcontroller's SPI interface for the screen
// (not 'bitbang') and must be wired to specific pins (e.g. for Arduino Uno,
// MOSI = pin 11, MISO = 12, SCK = 13). Other pins are configurable below.

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h>      // Hardware-specific library for ST7789
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS 26   // SD card select pin
#define TFT_CS 32  // TFT select pin
#define TFT_DC 25  // TFT display/command pin
#define TFT_RST 33 // Or set to -1 and connect to Arduino RESET pin

#if defined(USE_SD_CARD)
SdFat SD;                        // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
// SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
#if defined(__SAMD51__) || defined(NRF52840_XXAA)
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
                                            PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
#else
#if (SPI_INTERFACES_COUNT == 1)
Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
#else
Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
#endif
#endif
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem filesys;
Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image img; // An image loaded into RAM
int32_t width = 0,  // BMP image dimensions
    height = 0;

float p = 3.1415926;
int32_t smallScreen = 0;
int32_t foundImages = 0;

void setup(void)
{

  delay(2000);
  ImageReturnCode stat;  // Status from image-reading functions
  ImageReturnCode stat2; // Status from image-reading functions
  ImageReturnCode stat3; // Status from image-reading functions
  ImageReturnCode stat4; // Status from image-reading functions

  Serial.begin(9600);
  Serial.println(F("Hello, world!"));
#if !defined(ESP32)
  while (!Serial)
    ; // Wait for Serial Monitor before continuing
#endif

  tft.init(240, 320); // Init ST7789 320x240

// The Adafruit_ImageReader constructor call (above, before setup())
// accepts an uninitialized SdFat or FatFileSystem object. This MUST
// BE INITIALIZED before using any of the image reader functions!
//  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
                      // SD card is pretty straightforward, a single call...
  if (!SD.begin(SD_CS, SD_SCK_MHZ(10)))
  { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    //      for(;;); // Fatal error, do not continue
  }
#else
                                      // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if (!flash.begin())
  {
    Serial.println(F("flash begin() failed"));
    for (;;)
      ;
  }
  if (!filesys.begin(&flash))
  {
    Serial.println(F("filesys begin() failed"));
    for (;;)
      ;
  }
#endif
  Serial.println(F("OK!"));

  // Fill screen blue. Not a required step, this just shows that we're
  // successfully communicating with the screen.
  tft.fillScreen(ST77XX_BLUE);

  // Load full-screen BMP file 'purple.bmp' at position (0,0) (top left).
  // Notice the 'reader' object performs this, with 'tft' as an argument.
  //  Serial.print(F("Loading purple.bmp to screen..."));
  //  stat = reader.drawBMP("/circle icon test 240x320.bmp", tft, 0, 0);
  //  reader.printStatus(stat);   // How'd we do?

  //   Query the dimensions of image 'miniwoof.bmp':
  stat = reader.drawBMP("/circle icon test 240x320.bmp", tft, 0, 0);
  stat2 = reader.drawBMP("/circle-icon-test-grayscale-240x320.bmp", tft, 0, 0);
  stat3 = reader.drawBMP("/circle icon test 135x240.bmp", tft, 0, 0);
  stat4 = reader.drawBMP("/circle-icon-test-grayscale-135x240.bmp", tft, 0, 0);
  // If it didn't work, load tests
  // If it DID work, smallScreen stays at 0.
  Serial.println(F("Testing Larger."));
  if (stat != IMAGE_SUCCESS || stat2 != IMAGE_SUCCESS)
  {
    // If the 240x320 didn't work, try the smaller ones THEN run tests.
    smallScreen = 1;
    Serial.println(F("Testing Smaller."));
    // Now, if this works, smallScreen is 1.
    if (stat3 != IMAGE_SUCCESS || stat4 != IMAGE_SUCCESS)
    {
      Serial.println(F("Running tests."));
      runTests();
    }
    Serial.println(F("Found smaller images."));
    foundImages = 1;
  }
  else
  {
    Serial.println(F("Found larger images."));
    foundImages = 1;
  }
}

// Load small BMP 'wales.bmp' into a GFX canvas in RAM. This should fail
// gracefully on Arduino Uno and other small devices, meaning the image
// will not load, but this won't make the program stop or crash, it just
// continues on without it. Should work on Arduino Mega, Zero, etc.
//  Serial.print(F("Loading wales.bmp to canvas..."));
//  stat = reader.loadBMP("/wales.bmp", img);
//  reader.printStatus(stat); // How'd we do?
//
//  delay(2000); // Pause 2 seconds before moving on to loop()

void loop()
{
  if (foundImages == 1)
  {
    if (smallScreen == 0)
    {
      reader.drawBMP("/circle icon test 240x320.bmp", tft, 0, 0);
      delay(5000);
      reader.drawBMP("/circle-icon-test-grayscale-240x320.bmp", tft, 0, 0);
      delay(5000);
    }
    else
    {
      reader.drawBMP("/circle icon test 135x240.bmp", tft, 53, 40);
      delay(5000);
      reader.drawBMP("/circle-icon-test-grayscale-135x240.bmp", tft, 53, 40);
      delay(5000);
    }
  }
  //  for(int r=0; r<4; r++) { // For each of 4 rotations...
  //    tft.setRotation(r);    // Set rotation
  //    tft.fillScreen(0);     // and clear screen
  //    reader.drawBMP("/circle icon test 240x320.bmp", tft, 0, 0);

  // Load 4 copies of the 'miniwoof.bmp' image to the screen, some
  // partially off screen edges to demonstrate clipping. Globals
  // 'width' and 'height' were set by bmpDimensions() call in setup().
  //    for(int i=0; i<4; i++) {
  //      reader.drawBMP("/miniwoof.bmp", tft,
  //        (tft.width()  * i / 3) - (width  / 2),
  //        (tft.height() * i / 3) - (height / 2));
  //    }

  //    delay(3000); // Pause 1 sec.

  //    // Draw 50 Welsh dragon flags in random positions. This has no effect
  //    // on memory-constrained boards like the Arduino Uno, where the image
  //    // failed to load due to insufficient RAM, but it's NOT fatal.
  //    for(int i=0; i<50; i++) {
  //      // Rather than reader.drawBMP() (which works from SD card),
  //      // a different function is used for RAM-resident images:
  //      img.draw(tft,                                    // Pass in tft object
  //        (int16_t)random(-img.width() , tft.width()) ,  // Horiz pos.
  //        (int16_t)random(-img.height(), tft.height())); // Vert pos
  //      // Reiterating a prior point: img.draw() does nothing and returns
  //      // if the image failed to load. It's unfortunate but not disastrous.
  //    }
  //
  //    delay(2000); // Pause 2 sec.
  //  }
}

void runTests()
{
  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(3000);

  // large block of text
  tft.fillScreen(ST77XX_BLACK);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ST77XX_WHITE);
  delay(5000);

  // tft print function!
  tftPrintTest();
  delay(5000);

  // a single pixel
  tft.drawPixel(tft.width() / 2, tft.height() / 2, ST77XX_GREEN);
  delay(1500);

  // line draw test
  testlines(ST77XX_YELLOW);
  delay(1500);

  // optimized lines
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  delay(1500);

  testdrawrects(ST77XX_GREEN);
  delay(1500);

  testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  delay(1500);

  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(1500);

  testroundrects();
  delay(1500);

  testtriangles();
  delay(1500);

  mediabuttons();
  delay(1500);

  Serial.println("done");
  delay(5000);
  runTests();
}

void testlines(uint16_t color)
{
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6)
  {
    tft.drawLine(0, 0, x, tft.height() - 1, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6)
  {
    tft.drawLine(0, 0, tft.width() - 1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6)
  {
    tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6)
  {
    tft.drawLine(tft.width() - 1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6)
  {
    tft.drawLine(0, tft.height() - 1, x, 0, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6)
  {
    tft.drawLine(0, tft.height() - 1, tft.width() - 1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6)
  {
    tft.drawLine(tft.width() - 1, tft.height() - 1, x, 0, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6)
  {
    tft.drawLine(tft.width() - 1, tft.height() - 1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(char *text, uint16_t color)
{
  tft.setCursor(50, 50);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2)
{
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y = 0; y < tft.height(); y += 5)
  {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x = 0; x < tft.width(); x += 5)
  {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color)
{
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6)
  {
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2)
{
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6)
  {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2, x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color)
{
  for (int16_t x = radius; x < tft.width(); x += radius * 2)
  {
    for (int16_t y = radius; y < tft.height(); y += radius * 2)
    {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color)
{
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2)
  {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2)
    {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles()
{
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 0xF800;
  int t;
  int w = tft.width() / 2;
  int x = tft.height() - 1;
  int y = 0;
  int z = tft.width();
  for (t = 0; t <= 15; t++)
  {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    color += 100;
  }
}

void testroundrects()
{
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 100;
  int i;
  int t;
  for (t = 0; t <= 4; t += 1)
  {
    int x = 0;
    int y = 0;
    int w = tft.width() - 2;
    int h = tft.height() - 2;
    for (i = 0; i <= 16; i += 1)
    {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
}

void tftPrintTest()
{
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(50, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(3000);
  tft.setCursor(50, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void mediabuttons()
{
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}
