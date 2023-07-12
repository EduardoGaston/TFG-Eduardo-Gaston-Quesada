#include "Arduino.h"
#include "Deposito.h"

#define TANQUE_VACIO 0
#define LLENADO 1
#define TANQUE_LLENO 2
#define DESCARGANDO 3
#define EMERGENCIA 4
#define REINICIO 5
#define bit_PIN 1023
#define bit_PIN_OUT 255
#define t_ciclo 0.003
#define r_max 51
#define v_r_max 512

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
const int calVal_eepromAdress = 0;
unsigned long t = 0;

Deposito::Deposito (int _PIN_bomba_llenado, int _PIN_bomba_vaciado, int _PIN_Rele1_Llenado, int _PIN_Rele2_Llenado, int _PIN_Rele1_Vaciado, int _PIN_Rele2_Vaciado, int _PIN_IN1, int _PIN_IN2, int _PIN_ENA, int _HX711_dout, 
int _HX711_sck, float _valor_calibracion_basculas, int _sensor_0, int _sensor_1,  int _sensor_2, int _sensor_3, float _capacidad_maxima, float _vol_max_seguridad, float _volumen_descarga_max, float _volumen_descarga_min, 
float _t_prox_descarga_max, float _t_prox_descarga_min, bool _recarga_o_descarga, Deposito *deposito_ant, Deposito *deposito_sig, bool _es_fugas, bool _es_recarga, float _vol_seguridad_anterior, float _vol_limite_anterior, float _vol_min_comienzo_bomba) : LoadCell(_HX711_dout, _HX711_sck) {   

  estado_actual = TANQUE_VACIO;
  abrir_valv_llenado_activada_antes = 0;
  cerrar_valv_llenado_activada_antes = 0;
  abrir_valv_vaciado_activada_antes = 0;
  cerrar_valv_vaciado_activada_antes = 0;     

  //Añadido nuevo
  PIN_bomba_llenado = _PIN_bomba_llenado;
  PIN_bomba_vaciado = _PIN_bomba_vaciado;
  PIN_Rele1_Llenado = _PIN_Rele1_Llenado;
  PIN_Rele2_Llenado = _PIN_Rele2_Llenado;
  PIN_Rele1_Vaciado = _PIN_Rele1_Vaciado;
  PIN_Rele2_Vaciado = _PIN_Rele2_Vaciado;
  PIN_IN1 = _PIN_IN1;
  PIN_IN2 = _PIN_IN2;
  PIN_ENA = _PIN_ENA;
  
  sensor_0 = _sensor_0;
  sensor_1 = _sensor_1;
  sensor_2 = _sensor_2;
  sensor_3 = _sensor_3;
  
  HX711_dout = _HX711_dout;
  HX711_sck = _HX711_sck;
  valor_calibracion_basculas = _valor_calibracion_basculas;

  capacidad_maxima = _capacidad_maxima;
  vol_max_seguridad = _vol_max_seguridad;
  volumen_descarga_max = _volumen_descarga_max;
  volumen_descarga_min = _volumen_descarga_min;
  t_prox_descarga_max = _t_prox_descarga_max;
  t_prox_descarga_min = _t_prox_descarga_min;  
  vol_prox_mov = 0;
  t_prox_mov = 0;
  nuevo_vol_descarga();
  nuevo_t_prox_mov();    
  
  recarga_o_descarga = _recarga_o_descarga; 
  es_fugas = _es_fugas;
  es_recarga = _es_recarga; 
  
  this->deposito_sig = deposito_sig;
  this->deposito_ant = deposito_ant;

  vol_seguridad_anterior = _vol_seguridad_anterior; 
  vol_limite_anterior = _vol_limite_anterior;
  vol_min_comienzo_bomba = _vol_min_comienzo_bomba;
  
//  Asignación de los pines a las válvulas correspondientes //

  pinMode(sensor_0, INPUT);
  pinMode(sensor_1, INPUT);  
  pinMode(sensor_2, INPUT); 
  pinMode(sensor_3, INPUT);   
  pinMode(PIN_bomba_llenado, OUTPUT);
  pinMode(PIN_bomba_vaciado, OUTPUT); 
  pinMode(PIN_Rele1_Llenado, OUTPUT); 
  pinMode(PIN_Rele2_Llenado, OUTPUT);
  pinMode(PIN_Rele1_Vaciado, OUTPUT); 
  pinMode(PIN_Rele2_Vaciado, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);  

  pinMode(HX711_dout, INPUT); 
  pinMode(HX711_sck, INPUT);
   
}
  
//  Método que comprueba el estado del deposito //
  
bool Deposito::actualizar_estado (int tSimulacion, int alarma, int volver_a_empezar) {                      //  El parámetro volver_a_empezar indíca que el conjunto de todos los depositos se han vaciado y podemos salir del estado de reinicio 

  if ((es_fugas == 1) && (tSimulacion > 72)) {              //7200
    velocidad = genera_caudal();
  }
    
  if (alarma == HIGH) {                                                                                     //  Condición de emergencia en función del reset
    estado_actual = EMERGENCIA;
    abrir_valv_llenado_activada_antes = 0;
    cerrar_valv_llenado_activada_antes = 0;
    abrir_valv_vaciado_activada_antes = 0;
    cerrar_valv_vaciado_activada_antes = 0;
  }
  
  if ((volumen >= vol_max_seguridad) && (estado_actual != REINICIO)) {                                      //  Condición de emergencia
    estado_actual = EMERGENCIA;
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
  }

  if (estado_actual == TANQUE_VACIO) {                                                                      //  Estado que define las condiciones de cuando el tanque espera una descarga   
    cerrar_valvula_llenado();                                                                               //  Cerramos la válvula de llenado del tanque 
    cerrar_valvula_vaciado();                                                                               //  Cerramos la válvula de vaciado del tanque       
    if ( ( ( (tSimulacion >= t_prox_mov) && (deposito_ant->ready_to_send(vol_prox_mov) == 1) ) || (recarga_o_descarga == true) ) && (peticion_pausa_venta == 0) ) {                                      //  Condición de cambio de estado, tanque vacio a llenado               
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;    
      estado_actual = LLENADO; 
    }
  }

  if ( (estado_actual == LLENADO) && (peticion_pausa_fuga == 0) ) {          //  Estado que define el llenado del tanque
    abrir_valvula_llenado();                                                                                //  Abrimos la válvula de llenado del tanque    
    cerrar_valvula_vaciado();                                                                               //  Cerramos la válvula de vaciado del tanque                
    if ((volumen >= vol_prox_mov) && (es_fugas == 0)) {
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
      volumen_total_venta = volumen_total_venta + volumen;
      ventas_totales++;
      estado_actual = TANQUE_LLENO;
    }
  }   
  
  if (estado_actual == TANQUE_LLENO) {                                                                      //  Estado que define las condiciones de cuando el tanque ha recibido una descarga   
    cerrar_valvula_llenado();                                                                               //  Cerramos la válvula de llenado del tanque
    cerrar_valvula_vaciado();                                                                               //  Cerramos la válvula de vaciado del tanque               
    if ((deposito_sig->ready_to_receive(vol_prox_mov) == 1) && ((tSimulacion >= t_prox_mov) ||              //  Condición de cambio de estado, tanque lleno a descargando
    (recarga_o_descarga == false)) && (peticion_pausa_recarga == 0) ) {
      // toDo generalizar la condicion de que existe suficiente comb. en el principal cuando haya mas de 1 dep. de ventas
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
      volumen_total_recarga = volumen_total_recarga + volumen;
      recargas_totales++; 
      estado_actual = DESCARGANDO;
    }    
  }
  
  if (estado_actual == DESCARGANDO) {                                                                       //  Estado que define cuando el tanque está descargando  
    cerrar_valvula_llenado();                                                                               //  Cerramos la válvula de llenado del tanque    
    abrir_valvula_vaciado();                                                                                //  Abrimos la válvula de vaciado del tanque                 
    if (volumen < UMBRAL_VACIO) {                                                                          //  Condición de cambio de estado, descargando a tanque vacio
      estado_actual = TANQUE_VACIO;
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
      nuevo_vol_descarga();                                                                                 //  Asignamos nuevamente valores aleatorios a las variables de simulación
      nuevo_t_prox_mov();
    }    
  } 
   
  if (estado_actual == EMERGENCIA) {                                                                        //  Estado que define la entrada a los límites de seguridad del tanque
    cerrar_valvula_llenado();                                                                               //  Cerramos la válvula de llenado del tanque    
    cerrar_valvula_vaciado();                                                                               //  Cerramos la válvula de vaciado del tanque      
    if (alarma == LOW) {                                                                                    //  Condición de cambio de estado, emergencia a reinicio
      estado_actual = REINICIO;
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
    }
    return true;
  }
  
  if (estado_actual == REINICIO) {                                                                          //  Estado que define el reinicio de la planta   
    abrir_valvula_llenado();                                                                                //  Abrimos la válvula de llenado del tanque    
    abrir_valvula_vaciado();                                                                                //  Abrimos la válvula de vaciado del tanque     
    if (volver_a_empezar == 1) {                                                                            //  Condición de cambio de estado, de reinicio a tanque vacio
      estado_actual = TANQUE_VACIO;
      abrir_valv_llenado_activada_antes = 0;
      cerrar_valv_llenado_activada_antes = 0;
      abrir_valv_vaciado_activada_antes = 0;
      cerrar_valv_vaciado_activada_antes = 0;
      nuevo_vol_descarga();                                                                                 //  Asignamos nuevamente valores aleatorios a las variables de simulación
      nuevo_t_prox_mov();       
    }
  }

  if ((t_inicio_valv_llenado_abierta > 0) && (millis() > (T_VAL_CONMUT + t_inicio_valv_llenado_abierta))) {                        
    digitalWrite (PIN_Rele2_Llenado, LOW);    //R1 HIGH -- R2 LOW 
    t_inicio_valv_llenado_abierta = 0;
  }
  if ((t_inicio_valv_llenado_cerrada > 0) && (millis() > (T_VAL_CONMUT + t_inicio_valv_llenado_cerrada))) {                        
    digitalWrite (PIN_Rele1_Llenado, HIGH);     
    t_inicio_valv_llenado_cerrada = 0;
  }
  if ((t_inicio_valv_vaciado_abierta > 0) && (millis() > (T_VAL_CONMUT + t_inicio_valv_vaciado_abierta))) {                        
    digitalWrite (PIN_Rele2_Vaciado, LOW);    
    t_inicio_valv_vaciado_abierta = 0;
  } 
  if ((t_inicio_valv_vaciado_cerrada > 0) && (millis() > (T_VAL_CONMUT + t_inicio_valv_vaciado_cerrada))) {                        
    digitalWrite (PIN_Rele1_Vaciado, HIGH);    
    t_inicio_valv_vaciado_cerrada = 0;
  }

  return false;
}


//  Método que abre la válvula de llenado //

void Deposito::abrir_valvula_llenado () {
  if (es_fugas == 0) {
    if (abrir_valv_llenado_activada_antes == 0) {
      digitalWrite(PIN_Rele1_Llenado, HIGH);
      digitalWrite(PIN_Rele2_Llenado, HIGH);
      t_inicio_valv_llenado_abierta = millis();
      abrir_valv_llenado_activada_antes = 1;
    }
    digitalWrite(PIN_bomba_llenado, LOW);  //Motor activado   
  } else {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    analogWrite(PIN_ENA, velocidad);                                                                           
  }
}

//  Método que abre la válvula de vaciado //

void Deposito::abrir_valvula_vaciado () {
  if (abrir_valv_vaciado_activada_antes == 0) {
    digitalWrite(PIN_Rele1_Vaciado, HIGH);
    digitalWrite(PIN_Rele2_Vaciado, HIGH);
    t_inicio_valv_vaciado_abierta = millis();
    abrir_valv_vaciado_activada_antes = 1;              // INDICA QUE YA SE HA ACTIVADO LA VALVULA ANTERIORMENTE  //
  }
  digitalWrite(PIN_bomba_vaciado, LOW);      
}

//  Método que cierra la válvula de llenado //

void Deposito::cerrar_valvula_llenado () {
  if (es_fugas == 0) {
    if (cerrar_valv_llenado_activada_antes == 0) {
      digitalWrite(PIN_Rele1_Llenado, LOW);
      digitalWrite(PIN_Rele2_Llenado, LOW);
      t_inicio_valv_llenado_cerrada = millis(); 
      cerrar_valv_llenado_activada_antes = 1;              // INDICA QUE YA SE HA ACTIVADO LA VALVULA ANTERIORMENTE  // 
    }
    digitalWrite(PIN_bomba_llenado, HIGH);      

  } else {
    analogWrite(PIN_ENA, 0);                   
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
  }
}

//  Método que cierra la válvula de vaciado //

void Deposito::cerrar_valvula_vaciado () {
  if (cerrar_valv_vaciado_activada_antes == 0) {
    digitalWrite(PIN_Rele1_Vaciado, LOW);
    digitalWrite(PIN_Rele2_Vaciado, LOW);
    t_inicio_valv_vaciado_cerrada = millis(); 
    cerrar_valv_vaciado_activada_antes = 1;              // INDICA QUE YA SE HA ACTIVADO LA VALVULA ANTERIORMENTE  //
  }
  digitalWrite(PIN_bomba_vaciado, HIGH);
}


//  Método que generará valores aleatorios de tiempo a las variables de la simulación //

void Deposito::nuevo_t_prox_mov () {
  t_prox_mov = t_prox_mov + random (t_prox_descarga_min, t_prox_descarga_max)/1000;
}

//  Método que nos permite ver si el tanque está listo para que se realice una descarga en él //

int Deposito::ready_to_receive (float vol_prox_mov) {
  if(vol_max_seguridad > (volumen + vol_prox_mov)) {     
    return 1;
  } else {
    return 0;
  }
}


//  Método que nos permite ver si el tanque anterior está listo para poder realizar una descarga  //

int Deposito::ready_to_send (float vol_prox_mov) {
  if ( (deposito_ant->get_volumen() > vol_prox_mov) ) {     
    return 1;
  } else {
    return 0;
  }
}


//  Método de conversión de volumen medido por las basculas //

float Deposito::conversion_volumen () {
  float resultado;
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0;                              //  Aumentar el valor para ralentizar la actividad de impresión en serie
  if (LoadCell.update()) newDataReady = true;                     //  Comprobar si hay nuevos datos / iniciar la siguiente conversión
  if (newDataReady) {                                             //  Obtener un valor suavizado del conjunto de datos:
      resultado = LoadCell.getData();   
  }
  return resultado;
}

// ********** REVISAR GENERAR CAUDAL FUGAS ********** //
//  Método genera un caudal para las fugas  //
int Deposito::genera_caudal() {
  int caudal;
  
  if (deposito_sig->get_volumen() > vol_seguridad_anterior) {
    caudal = bit_PIN_OUT;
  }
  if ((deposito_sig->get_volumen() < vol_seguridad_anterior) && (deposito_sig->get_volumen() > vol_min_comienzo_bomba))  {
    caudal = (deposito_sig->get_volumen() / vol_limite_anterior)*bit_PIN_OUT;      // Apartir de 1110 g es cuando comienza la bomba peristaltica (PIN_ENA > 150)
  }
  if (deposito_sig->get_volumen() < vol_min_comienzo_bomba) {
    caudal = 0;
  }

  return caudal;
}

//  Método que realiza la tara de las basculas  //

bool Deposito::realizar_tara() {
  LoadCell.begin();
  float calibrationValue;
  bool tara_realizada;                                          
  calibrationValue = valor_calibracion_basculas;                    //  Valor de calibración
  #if defined(ESP8266)|| defined(ESP32)
    //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
  #endif
    //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom
  
  unsigned long stabilizingtime = 3000;                             //  La precisión justo después del encendido se puede mejorar agregando unos segundos de tiempo de estabilización
  boolean _tare = true;                                             //  Establezca esto en falso si no desea que se realice la tara en el siguiente paso
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    while (1);
    tara_realizada = false;
  
  } else {
    LoadCell.setCalFactor(calibrationValue);                        //  set calibration value (float)
    Serial.println("Tara Realizada correctamente");
    tara_realizada = true;
  }
  return tara_realizada;   
}



float Deposito::volumen_acumulado_recarga() {
  return volumen_total_recarga;
}

float Deposito::volumen_acumulado_venta() {
  return volumen_total_venta;
}

int Deposito::recargas_realizadas() {
  return recargas_totales;
}

int Deposito::ventas_realizadas() {
  return ventas_totales;
}

void Deposito::reset_recargas_realizadas(){
  recargas_totales = 0;
}
  
void Deposito::reset_ventas_realizadas(){
  ventas_totales = 0;
}


/*****************************************************************************************************************/
/**************************************     VOLUMEN SENSORES RECARGA     *************************************************/
/*****************************************************************************************************************/
float Deposito::recarga_sensor_1_volumen(){
  return descarga_sensor_1;
}

float Deposito::recarga_sensor_2_volumen(){
  return descarga_sensor_2;
}

float Deposito::recarga_sensor_3_volumen(){
  return descarga_sensor_3;
}


/*****************************************************************************************************************/
/**************************************     VOLUMEN Y VOL PROXIMO     *************************************************/
/*****************************************************************************************************************/
float Deposito::serial_prox_vol(){
  return vol_prox_mov;
}

float Deposito::serial_volumen(){
  return volumen;
}








// CODIGO AÑADIDO PARA MEDIR VOLUMEN DE LAS DESCARGAS //

/*****************************************************************************************************************/
/**************************************     RECARGA SENSOR 1     *************************************************/
/*****************************************************************************************************************/
bool Deposito::llenar_recarga_1() {
  abrir_valvula_llenado(); 
  if (digitalRead(sensor_1) == 1) {
    llenar_sensor_1 = 1;  
  } 
  return llenar_sensor_1;
}

bool Deposito::vaciar_recarga_1() {
  abrir_valv_llenado_activada_antes = 0;    
  cerrar_valv_llenado_activada_antes = 0;
  abrir_valvula_vaciado();
  if (digitalRead(sensor_0) == 0) {
    vaciar_sensor_1 = 1;                      
  } 
  return vaciar_sensor_1;
}

void Deposito::medir_recarga_1()  {
  abrir_valv_vaciado_activada_antes = 0;                         
  cerrar_valv_vaciado_activada_antes = 0;
  descarga_sensor_1 = deposito_sig->get_volumen();  //float descarga_sensor_1; en el .h
}

/*****************************************************************************************************************/
/**************************************     RECARGA SENSOR 2    *************************************************/
/*****************************************************************************************************************/
bool Deposito::llenar_recarga_2() {
  abrir_valvula_llenado(); 
  if (digitalRead(sensor_2) == 1) {
    llenar_sensor_2 = 1;  
  } 
  return llenar_sensor_2;
}

bool Deposito::vaciar_recarga_2() {
  abrir_valv_llenado_activada_antes = 0;    
  cerrar_valv_llenado_activada_antes = 0;
  abrir_valvula_vaciado();
  if (digitalRead(sensor_0) == 0) {
    vaciar_sensor_2 = 1;                      
  } 
  return vaciar_sensor_2;
}

void Deposito::medir_recarga_2()  {
  abrir_valv_vaciado_activada_antes = 0;                         
  cerrar_valv_vaciado_activada_antes = 0;
  descarga_sensor_2 = deposito_sig->get_volumen();  //float descarga_sensor_1; en el .h
}

/*****************************************************************************************************************/
/**************************************     RECARGA SENSOR 3    *************************************************/
/*****************************************************************************************************************/
bool Deposito::llenar_recarga_3() {
  abrir_valvula_llenado(); 
  if (digitalRead(sensor_3) == 1) {
    llenar_sensor_3 = 1;  
  } 
  return llenar_sensor_3;
}

bool Deposito::vaciar_recarga_3() {
  abrir_valv_llenado_activada_antes = 0;    
  cerrar_valv_llenado_activada_antes = 0;
  abrir_valvula_vaciado();
  if (digitalRead(sensor_0) == 0) {
    vaciar_sensor_3 = 1;                      
  } 
  return vaciar_sensor_3;
}

void Deposito::medir_recarga_3()  {
  abrir_valv_vaciado_activada_antes = 0;                         
  cerrar_valv_vaciado_activada_antes = 0;
  descarga_sensor_3 = deposito_sig->get_volumen();  //float descarga_sensor_1; en el .h
}

/*****************************************************************************************************************/
/*********************************     VACIAR TANQUE ALMACENAMIENTO     ******************************************/
/*****************************************************************************************************************/
bool Deposito::vaciar_almacemiento() {
  abrir_valvula_llenado();     
  abrir_valvula_vaciado();
  if ( (deposito_ant->get_volumen() < UMBRAL_VACIO) && (volumen < UMBRAL_VACIO) ) {     
      vacio_almacenamiento = 1;
  }
  return vacio_almacenamiento;  
}

void Deposito::resetar_vaciar_almacemiento() {
  vacio_almacenamiento = 0;
}

void Deposito::ciclos_cero() {
  abrir_valv_llenado_activada_antes = 0;
  cerrar_valv_llenado_activada_antes = 0;
  abrir_valv_vaciado_activada_antes = 0;
  cerrar_valv_vaciado_activada_antes = 0;
}



/*****************************************************************************************************************/
/*********************************     SENSORES TANQUE RECARGA      ******************************************/
/*****************************************************************************************************************/

//  Método que nos permite saber el volumen del tanque en todo momento  //

float Deposito::get_volumen () {  
  
  if (es_recarga == 0) {
    volumen = conversion_volumen ();
 
  } else if (digitalRead(sensor_3) == 1) {
    volumen = recarga_sensor_3_volumen();             // o descarga_sensor_3

  } else if (digitalRead(sensor_2) == 1) {              
    volumen = recarga_sensor_2_volumen();            // o descarga_sensor_2
     
  } else if (digitalRead(sensor_1) == 1) {               
    volumen = recarga_sensor_1_volumen();            // o descarga_sensor_1
  
  } else if (digitalRead(sensor_0) == 1) {
    volumen = UMBRAL_VACIO;
 
  } else { 
    volumen = 0;
  }

  return volumen;
}


//  Método que generará valores aleatorios de volumen a las variables de la simulación  //

void Deposito::nuevo_vol_descarga () {
  int numero_sensor;
  if (es_recarga == 0) {
    vol_prox_mov = random (volumen_descarga_min, volumen_descarga_max);
    
  } else {
    numero_sensor = random (1,N_SENSORES_REC);      //N_SENSORES_REC
   
    if (numero_sensor == 3){
      vol_prox_mov = recarga_sensor_3_volumen();        
    }
    
    if (numero_sensor == 2){
      vol_prox_mov = recarga_sensor_2_volumen();        
    }
     
    if (numero_sensor == 1) {
      vol_prox_mov = recarga_sensor_1_volumen();        
    }
  }
}



//  Funciones para dejar en estado de pausa los depositos para el Datalogger  //

bool Deposito::solicitud_pausa_recarga() {
  peticion_pausa_recarga = 1;
  if (estado_actual != DESCARGANDO) {
    recarga_pausada = 1;
  }
  return recarga_pausada;
}


bool Deposito::solicitud_pausa_venta() {
  peticion_pausa_venta = 1;
  if (estado_actual != LLENADO) {
    venta_pausada = 1;
  }
  return venta_pausada;
}

//Comprobar si bomba peristaltica se para
bool Deposito::solicitud_pausa_fuga() {
  bool fuga_pausada;
  peticion_pausa_fuga = 1;
  if(  (peticion_pausa_fuga == 1)&&(estado_actual == LLENADO) ){
    fuga_pausada = 1;
    cerrar_valvula_llenado();                                                                               
  }
  return fuga_pausada;
} 

void Deposito::quitar_pausa() {
  peticion_pausa_recarga = 0;
  peticion_pausa_venta = 0;
  peticion_pausa_fuga = 0;
  recarga_pausada = 0;
  venta_pausada = 0;
  fuga_pausada = 0;
  
}
