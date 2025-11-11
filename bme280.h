/**************************************************************************************************************************************************
*                                                       -LECTURA SENSOR BME280                                                                    *
*                                                                                                                                                 *
*    @author Héctor Monroy Fuertes                                                                                                                *
*    @version 1.2                                                                                                                                 *
*    @brief Metodo reutilizable para leer valores de los sensores "meteorologicos" BME280.                                                        *
*           Incluye todas las variables para leer valores y generar graficas                                                                      *
*                                                                                                                                                 *
*                                                                                                                                                 *
**************************************************************************************************************************************************/
/***************INCLUIR LO SIGUIENTE EN EL SETUP() O DONDE SE REQUIERA
bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
*/
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
#define PresionNivelMar_HPA (1013.25)

struct sensores {
  float temperatura, presion;
  int8_t humedad;
  int luz; //Se incluye el LDR aunque no sea parte del BME para unificar sensores
  //int altura;
};

struct sensores sensor;

void lecturaSensores() {
  sensor.temperatura = bme.readTemperature();
  sensor.humedad = bme.readHumidity();
  sensor.presion = (bme.readPressure() / 100.0F);
  sensor.luz = analogRead(ldr);
  //sensor.altura = bme.readAltitude(PresionNivelMar_HPA);
}
