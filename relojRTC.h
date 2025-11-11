/**************************************************************************************************************************************************
*                                                                                                                                                 *
*    @author Héctor Monroy Fuertes                                                                                                                *
*    @version 1.0                                                                                                                                 *
*    @brief Metodo reutilizable para proyectos que requieran el uso de un rtc. Se incluyen todas las variables para poder trabajar con el.        *
*                                                                                                                                                 *
*                                                                                                                                                 *
**************************************************************************************************************************************************/
/*******Incluir en el setup()
if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    //Descomentar la siguiente linea para poner en hora el rtc si es nuevo o si se ha cambiado la pila 
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
*/
#include "RTClib.h"
#include <Wire.h>
RTC_DS3231 rtc;
//DateTime now = rtc.now();

//Strings para almacenar la fecha
String dayStr;
String monthStr;
String yearStr;
String fecha;
//Strings para almacenar la hora
String hora;
String hourStr;
String minuteStr;
String secondStr;
//String para mostrar el dia concreto de la semana
String diaDeLaSemana;
//String para mostrar el mes concreto del año
String mesDelAno;

void horaYfecha(){ //Metodo para capturar la fecha y la hora en distintas variables

DateTime now = rtc.now();
  
  char diasDeLaSemana[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
  char mesesAno[12][20] = {"Diciembre", "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre"};

  
  
  //Guardamos cada parametro del tiempo en strings
  //Se añade un "0" flotante cuando es necesario
  dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  yearStr = String(now.year(), DEC);

  hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC); 
  minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  diaDeLaSemana = diasDeLaSemana[now.dayOfTheWeek()];
  mesDelAno = mesesAno[now.month()];
  //Serial.print("Número de mes: ");
  //Serial.println(now.month());

  //Concatenamos los strings realtivos a la fecha para mostrarla
  fecha = dayStr + "-" + monthStr + "-" + yearStr;
  hora = hourStr + ":" + minuteStr;
  Serial.println(hora);
}


