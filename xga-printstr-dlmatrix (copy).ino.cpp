
//##################################
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "time.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"
#include <Arduino.h>
#include <exception>
#include<string.h>
//#include<himem.h>


#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)


//##################################
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"

//Replace with your network credentials
//const char* ssid = "REPLACE_WITH_YOUR_SSID";
//const char* password = "REPLACE_WITH_YOUR_PASSWORD";



const char* ssid = "xxxxxx";
const char* password = "xxxxxx";
static mtmn_config_t mtmn_config = {0};

 time_t now;
struct tm timeinfo;


#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM

// Not tested with this model
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

//File file;


static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){



  
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_RGB888;
    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, fb.height-25, color, str);
}




static  int  jpg_encode_len=0;
static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    File *file = (File *)arg;

file->write((const uint8_t *)data, len); // payload (image), payload length
   // Serial.printf("Saved to  file  %d  byte\n", len );
jpg_encode_len+=len;

    
    //Serial.printf("\njpg_encode_stream(void * arg, index : %d,  data  , size :  %d)",index,len);
/*
try{
uint8_t * buff;
//memcpy(buff,data,len);
  
  char strftime_buf[64];
    //setenv("TZ", TZ_INFO, 1);  
   // tzset();
    delay(1000);
    time(&now);
    localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "/jpg_encode_stream-%F-%H-%M-%S.jpg", &timeinfo);


   
  fs::FS &fs = SD_MMC;
  file = fs.open(strftime_buf, FILE_WRITE);

}
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write((const uint8_t *)data, len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", strftime_buf);
   
  }
  file.close();
}

catch(...){
Serial.println("Exception wan handeled");
  
}
*/
//jpg_encode_len+=len;
    
    return len;
}


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/x-rgb\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t *_jpg_buf_len = 0;
  uint8_t ** _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

 // while(true){

// for(int j=0;j<20;j++){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      res = ESP_FAIL;
    } else {


char strftime_buf[64];
    //setenv("TZ", TZ_INFO, 1);  
   // tzset();
    delay(1000);
    time(&now);
    localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "/%Y%m%d-%H%M%S.jpg", &timeinfo);


 
   
  Serial.printf("\nPicture file name: %s\n", strftime_buf);
  fs::FS &fs = SD_MMC;
  File file = fs.open(strftime_buf, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    Serial.printf("file length  is %d\n",fb->len);
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("frame buffer Saved  to path: %s\n", strftime_buf);
   // EEPROM.write(0, pictureNumber);
   // EEPROM.commit();
  }
  file.close();




     /* 
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }*/





dl_matrix3du_t *image_matrix;          
    size_t fbwidth=fb->width;              
    size_t fbheight=fb->height; 
    size_t len=fbwidth*fbheight*3;   
image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

                if (!image_matrix) {
                   esp_camera_fb_return(fb);
                    Serial.println("dl_matrix3du_alloc failed");
                    res = ESP_FAIL;
                } else {
                    if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)){
                       esp_camera_fb_return(fb);
                        Serial.println("fmt2rgb888 failed");
                        res = ESP_FAIL;
                    }
//bool fmt2bmp(uint8_t *src, size_t src_len, uint16_t width, uint16_t height, pixformat_t format, uint8_t ** out, size_t * out_len);
 //bool s=fmt2bmp(image_matrix->item,len,fbwidth,fbheight,PIXFORMAT_RGB888,_jpg_buf ,_jpg_buf_len);
// _jpg_buf_len = len;
 //_jpg_buf = image_matrix->item;


bool s = fmt2jpg_cb(image_matrix->item,len,fbwidth,fbheight,PIXFORMAT_RGB888, 90, jpg_encode_stream, (void *)90);

dl_matrix3du_free(image_matrix);
if(!s){
  Serial.println("fmt2jpg failed");
}
                }
}
    /*
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }*/
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));


// String path = "/picture" + String(pictureNumber) +".jpg";


//const char* TZ_INFO = "IRDT-3:30IRST,M3.2.0/2:00:00,M11.1.0/2:00:00";//"PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00";

delay(1001);
    
//  }
  return res;
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
 /* if(psramFound()){
    config.frame_size = FRAMESIZE_CIF;//FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {*/
    config.frame_size = FRAMESIZE_XGA;//FRAMESIZE_UXGA;//FRAMESIZE_SXGA;//FRAMESIZE_HD;//FRAMESIZE_SVGA;//FRAMESIZE_CIF;//FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
 // }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("\nCamera init failed with error 0x%x\n", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());
Serial.println("");
sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_setservername(1, "time.windows.com");
  sntp_setservername(2, "time.nist.gov");

  sntp_init();


  
  const char* TZ_INFO = "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00";

  //char strftime_buf[64];
    setenv("TZ", TZ_INFO, 1);  
    tzset();

     time(&now);
    localtime_r(&now, &timeinfo);
    
  int succ = SD_MMC.begin("/sdcard", true,true);


mtmn_config.type = FAST;
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.707;
    mtmn_config.pyramid_times = 4;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.p_threshold.candidate_number = 20;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 10;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.7;
    mtmn_config.o_threshold.candidate_number = 1;

  
  // Start streaming web server
  //startCameraServer();
}





static void *dlcalloc(int cnt, int size, int align)
{
    int total_size = cnt * size + align + sizeof(void *);
    void *res = malloc(total_size);
    if (NULL == res)
    {
#if DL_SPIRAM_SUPPORT
        //res = heap_caps_malloc(total_size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        res = heap_caps_malloc(total_size,  MALLOC_CAP_SPIRAM);
    }
    if (NULL == res)
    {
        printf("Item psram alloc failed. Size: %d x %d\n", cnt, size);
#else
        printf("Item alloc failed. Size: %d x %d, SPIRAM_FLAG: %d\n", cnt, size, DL_SPIRAM_SUPPORT);
#endif
        return NULL;
    }
    bzero(res, total_size);
    void **data = (void **)res + 1;
    void **aligned;
    if (align)
        aligned = (void **)(((size_t)data + (align - 1)) & -align);
    else
        aligned = data;

    aligned[-1] = res;
    return (void *)aligned;
}



static inline dl_matrix3du_t *matrix3du_alloc(int n, int w, int h, int c)
{
    dl_matrix3du_t *r = (dl_matrix3du_t *)dlcalloc(1, sizeof(dl_matrix3du_t), 0);
    if (NULL == r)
    {
        printf("internal r failed.\n");
        return NULL;
    }
    uc_t *items = (uc_t *)dlcalloc(n * w * h * c, sizeof(uc_t), 0);
    if (NULL == items)
    {
        printf("matrix3du item alloc failed.\n");
        dl_lib_free(r);
        return NULL;
    }

    r->w = w;
    r->h = h;
    r->c = c;
    r->n = n;
    r->stride = w * c;
    r->item = items;

    return r;
}









void loop() {
  
  delay(100);
  /*
//esp_himem_get_free_size
  //heap_caps_get_largest_free_block() esp_get_free_heap_size()esp_get_minimum_free_heap_size()
  Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM) : %d  \n",heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM));
 Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_8BIT ) : %d  \n",heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
 Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM) : %d  \n",heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
 //Serial.printf("heap_caps_get_largest_free_block() : %d  \n",heap_caps_get_largest_free_block());
  Serial.printf("esp_get_free_heap_size() : %d  \n",esp_get_free_heap_size());

Serial.printf("esp_get_minimum_free_heap_size() : %d  \n",esp_get_minimum_free_heap_size());
// Serial.printf("esp_himem_get_free_size() : %d  \n",esp_himem_get_free_size());
 
*/
 
  
camera_fb_t * fb = NULL;
 fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      
    } else {
Serial.println("Camera capture ok");

//char strftime_buf[64];
  // time(&now);
  //  localtime_r(&now, &timeinfo);
 // strftime(strftime_buf, sizeof(strftime_buf), "/%Y%m%d-%H%M%S.jpg", &timeinfo);


 
   
  //Serial.printf("\nPicture file name: %s\n", strftime_buf);
  
    
dl_matrix3du_t *image_matrix;          
    size_t fbwidth=fb->width;              
    size_t fbheight=fb->height; 
    size_t len=fbwidth*fbheight*3;   
image_matrix = matrix3du_alloc(1, fb->width, fb->height, 3);




                if (!image_matrix) {
                   esp_camera_fb_return(fb);
                    Serial.println("dl_matrix3du_alloc failed");
                    dl_matrix3du_free(image_matrix);
                } else {
                    if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)){
                       esp_camera_fb_return(fb);
                        Serial.println("fmt2rgb888 failed");
                       
                    }
   


box_array_t *net_boxes=face_detect(image_matrix, &mtmn_config);
if (net_boxes){
  Serial.println("face detected");
   char str_time[64];
    time(&now);
    localtime_r(&now, &timeinfo);
  strftime(str_time, sizeof(str_time), "/%Y%m%d-%H%M%S.jpg", &timeinfo);
Serial.printf("FILE NAME IS  %S  byte\n", str_time );
  fs::FS &fs = SD_MMC;
  File file = fs.open(str_time, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
                    rgb_print(image_matrix,FACE_COLOR_GREEN,str_time);


bool s = fmt2jpg_cb(image_matrix->item,len,fbwidth,fbheight,PIXFORMAT_RGB888, 90, jpg_encode_stream, &file);


if(!s){
  Serial.println("fmt2jpg failed");
  dl_matrix3du_free(image_matrix);
}
Serial.printf("Saved   %d  byte\n", jpg_encode_len );
jpg_encode_len=0;
file.close();


                }
                
                }
                dl_matrix3du_free(image_matrix);
}
esp_camera_fb_return(fb);
    }
    
delay(1000);



  
}
