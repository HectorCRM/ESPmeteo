/********************************************************************************************************************************************************
**                                                           -IAmeteo-                                                                                 **
**                                                                                                                                                     **
**   @author: Héctor Monroy Fuertes. Mail: monroygti2.0@gmail.com                                                                                      **
**   @brief:                                                                                                                                           **
**   Programa para recoger datos ambientales y almacenarlos en formato CSV y posteriormente usarlos para entrenar un pequeño modelo de IA propio       **     
**   que sea capaz de hacer predicciones(lluvia, calor, frio, niebla...) y enviar alertas via WhatsApp/email.                                          **
**   @version: -V1.0(DESDE 04/11/2025): Version inicial, por el momento recoge valores correctamente y los almacena en la tarjeta SD.                   ** 
**                                                                                                                                                     **
**                                                                                                                                                     **
**                                                                                                                                                     ** 
********************************************************************************************************************************************************/
#include "esp_sleep.h"
#define debug //Descomentar para depurar por puerto serie
#define led 2 //GPIO2 para usar el led interno del esp32

#define ldr 36 //GPIO36(VP) para leer valores analogicos de fotorresistencia LDR
float puntoRocio;
int8_t counter;
String meteorologia;

//SPI para comunicacion con modulo microSD
#include <SPI.h>

//Gestion de la tarjeta microSD para guardar logs de datos
#include "SD.h"
#include "FS.h"
File file;
String mensajeSD; //variable para concatenar datos como un string y guardarlos en la tarjeta sd
String fechaArchivo; //Variable utilizada para generar archivos autoincrementales para cada fecha en la tarjeta sd
String nombreArchivo; //Variable para crear el nombre del archivo en la sd


#include "bme280.h"
#include "relojRTC.h"

//Escribir en la SD
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
//Añadir datos a la SD
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
void calculoPuntoRocio(float temp, int hum){
  puntoRocio = temp - ((100-hum) /5);
  if (temp == puntoRocio && temp > 3.00){
    meteorologia = "niebla";
  }
  if (temp == puntoRocio && temp < 3.00){
    meteorologia = "helada";
  }
  if (puntoRocio >= 20.00){
    meteorologia = "bochorno";
  }
  else{
    meteorologia = "normal";
  }
}
void setup() {
  #ifdef debug
  Serial.begin(115200);
    #endif
  pinMode(led, OUTPUT);
  pinMode(ldr, INPUT);

  SD.begin(); //Inicializa la tarjeta sd
  delay(500);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
//Descomentar la siguiente linea para poner en hora el rtc si la hora ha cambiado(horario invierno/verano)
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (rtc.lostPower()) {
    //El rtc se pone en hora automaticamente si pierde alimentacion(SÓLO si la pierde)
    Serial.println("RTC lost power, let's set the time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
  }
  esp_err_t result = esp_sleep_enable_timer_wakeup(900000000); //timer wake-up para light sleep durante (15min)
  horaYfecha();
  nombreArchivo = "/" + String(fecha) + ".csv"; //concatena "/" +fecha del log+ ".csv"
  Serial.println(nombreArchivo);
  Serial.println(fecha);
  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }

}

void loop() {
  DateTime now = rtc.now();

  if(fechaArchivo != fecha && SD.exists(nombreArchivo.c_str())){ //Si se detecta que la fecha para dar nombre al archivo es distinto de la fecha actual, se crea un nuevo archivo(nuevo día)
    fechaArchivo = fecha;
    nombreArchivo = "/" + String(fechaArchivo) + ".csv";
    Serial.println("Nuevo dia, generando nuevo archivo...");
    writeFile(SD, nombreArchivo.c_str(), "Fecha,Hora,Punto de rocío(ºC),Temperatura(ºC),Humedad(%),Presión Atmosférica,Luminosidad,Climatología, \r\n");
    
  }
  File file = SD.open(nombreArchivo.c_str());

  if(!file) {
    Serial.println("No existe el archivo");
    Serial.println("Creando archivo...");
    writeFile(SD, nombreArchivo.c_str(), "Fecha,Hora,Punto de rocío(ºC),Temperatura(ºC),Humedad(%),Presión Atmosferica,Luminosidad,Climatología \r\n");
    
  }
  
  Serial.println("Entrando al Modo light_sleep");
  esp_light_sleep_start();
  //delay(10000);
  
  digitalWrite(led, HIGH);
  horaYfecha();
  lecturaSensores();
  calculoPuntoRocio(sensor.temperatura, sensor.humedad);
  //Depuracion por puerto serie
  Serial.println("Grabando nuevo log...");
  Serial.println(fecha);
  Serial.println(hora);
  Serial.print("Punto de rocio= ");
  Serial.print(puntoRocio);
  Serial.println("ºC");
  if (counter < 13){
    counter ++;

  switch (counter) {
    case 3:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Temperatura mínima: ," +"=MIN(D$2:D$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 4:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Temperatura máxima: ," +"=MAX(D$2:D$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 5:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Temperatura media: ," +"=PROMEDIO(D$2:D$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 7:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Humedad mínima: ," +"=MIN(E$2:E$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 8:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Humedad mínima: ," +"=MIN(E$2:E$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 9:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Humedad media: ," +"=PROMEDIO(E$2:E$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    case 11:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Presión atmosferica media: ," +"=PROMEDIO(F$2:F$96) \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str()); 
    break;
    case 12:
    mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia +",Horas de luz: ," +"=CONTAR.SI(G$2:G$97;">10")*15/60 \r\n";
    appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
    break;
    }
  }
  mensajeSD = String(fecha) +","+ String(hora) +","+ String(puntoRocio) +","+ String(sensor.temperatura) +","+ String(sensor.humedad) +","+ String(sensor.presion) +","+ String(sensor.luz) +","+  meteorologia + "\r\n";
  appendFile(SD, nombreArchivo.c_str(), mensajeSD.c_str());
  delay(250);
  digitalWrite(led, LOW);
  delay(250);
  digitalWrite(led, HIGH);
  delay(250);
  digitalWrite(led, LOW);

}
