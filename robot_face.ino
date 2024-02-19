#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h> 
#include <AnimatedGIF.h>
#include "icons.h"
#include <Adafruit_GFX.h>
#include <ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Bool.h>

#define SS_PIN  5
#define A_PIN   33 
#define B_PIN   32 
#define C_PIN   22  
#define E_PIN   21
#define PANEL_RES_X 64
#define PANEL_RES_Y 64 
#define PANEL_CHAIN 1

AnimatedGIF gif;
MatrixPanel_I2S_DMA *dma_display = nullptr;
static int totalFiles = 0; 
static File FSGifFile; 
static File GifRootFolder; 
std::vector<std::string> GifFiles; 
const int maxGifDuration    = 30000; 
ros::NodeHandle nh;
#include "gif_functions.hpp"
#include "sdcard_functions.hpp"
int gifPlay( const char* gifPath )

{ // 0=infinite
  if( ! gif.open( gifPath, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw ) ) {
    log_n("Could not open gif %s", gifPath );
  }
  int frameDelay = 0; 
  int then = 0; 
  while (gif.playFrame(true, &frameDelay)) {
    then += frameDelay;
    if( then > maxGifDuration ) {
      break;
    }
  }
  gif.close();
  return then;
}

void emojiCallback(const std_msgs::String& msg) {
    std::string value_to_check = msg.data; // Получаем значение из msg
    std::string clear_gif = "clear";
    std::string default_gif = "/gifs/center.gif";

    // Проходимся по всем файлам GIF
    for (auto &elem : GifFiles) {
        // Проверяем, совпадает ли значение с элементом в массиве
        if (elem == "/gifs/" + value_to_check + ".gif") {
            // Если совпадение найдено, проигрываем GIF
            if (!gifPlay(elem.c_str())) {
                // Возможно, здесь может быть дополнительный код в случае неудачи при проигрывании GIF
            }
            return; // Завершаем функцию после проигрывания GIF
        }
        // else if (!gifPlay(clear_gif.c_str()))
        // {
        //   gif.reset();
        //   // return;
        // }
        
    }
  
    // // Если ни одно совпадение не найдено, выводим изображение по умолчанию
    if (!gifPlay(default_gif.c_str())) {
        // Возможно, здесь может быть дополнительный код в случае неудачи при проигрывании GIF
    }
}

// void sayCallback(const std_msgs::Bool& msg){

// }
// void emojiCallback(const std_msgs::String& msg) {
//     std::string value_to_check = msg.data; // Получаем значение из msg
    
//     // Если пришло сообщение 'clear', сбрасываем GIF и выходим из функции
//     if (value_to_check == "clear") {
//         gif.reset();
//         return;
//     }
    
//     // Проходимся по всем файлам GIF
//     for (auto &elem : GifFiles) {
//         // Проверяем, совпадает ли значение с элементом в массиве
//         if (elem == "/gifs/" + value_to_check + ".gif") {
//             // Если совпадение найдено, проигрываем GIF
//             if (!gifPlay(elem.c_str())) {
//                 // Возможно, здесь может быть дополнительный код в случае неудачи при проигрывании GIF
//             }
//             return; // Завершаем функцию после проигрывания GIF
//         }
//     }
    
//     // Если ни одно совпадение не найдено, выводим изображение по умолчанию
//     std::string default_gif = "/gifs/sad.gif";
//     if (!gifPlay(default_gif.c_str())) {
//         // Возможно, здесь может быть дополнительный код в случае неудачи при проигрывании GIF
//     }
// }

// void emojiCallback(const std_msgs::String& msg) {
//     std::string value_to_check = msg.data; // Получаем значение из msg
//     // Проходимся по всем файлам GIF
//     for (auto &elem : GifFiles) {
//         // Проверяем, совпадает ли значение с элементом в массиве
//         if (elem == "/gifs/" + value_to_check + ".gif") {
//             // Если совпадение найдено, проигрываем GIF
//             if (!gifPlay(elem.c_str())) {
//                 // Возможно, тут может быть дополнительный код в случае неудачи при проигрывании GIF
//             }
//             return; // Завершаем функцию после проигрывания GIF
//             // gif.reset();
//         }
//     }
//     // Если ни одно совпадение не найдено, прописываем какое изображение будет выводить
//         std::string default_gif = "/gifs/sad.gif";
//         if (!gifPlay(default_gif.c_str())) {
// }
// //         std::string clear_gif = "clear";
// //         if (!gifPlay(clear_gif.c_str())) {
// // }
// }

ros::Subscriber<std_msgs::String> sub("/emoji", &emojiCallback );


void setup()
{
    nh.initNode();
    nh.subscribe(sub);
    if(!SD.begin(SS_PIN)){
        return;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
      return;
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    HUB75_I2S_CFG mxconfig(
      PANEL_RES_X,
      PANEL_RES_Y,
      PANEL_CHAIN
    );
    mxconfig.gpio.a = A_PIN;
    mxconfig.gpio.b = B_PIN;
    mxconfig.gpio.c = C_PIN;
    mxconfig.gpio.e = E_PIN;
    mxconfig.clkphase = false;
    mxconfig.driver = HUB75_I2S_CFG::FM6124;
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    if( not dma_display->begin() )
    dma_display->setBrightness8(255); //0-255
    dma_display->clearScreen();
    File root = SD.open("/gifs");
    if(!root){
        return;
    }
    File file = root.openNextFile();
    while(file){
        if(!file.isDirectory())
        {
            std::string filename = "/gifs/" + std::string(file.name());
            GifFiles.push_back( filename );
            totalFiles++;
        }
        file = root.openNextFile();
    }
    file.close();
    gif.begin(LITTLE_ENDIAN_PIXELS);
}
void loop(){
    nh.spinOnce();

    delay(2);
}
