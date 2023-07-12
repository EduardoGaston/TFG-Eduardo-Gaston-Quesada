#include "Arduino.h"

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// VARIABLES TANQUE RECARGA //
#define UMBRAL_VACIO 20                 //  Volumen cuando sensor_0 está activado, g = ml
#define VOL_REC_1 500                   //  Volumen cuando sensor_1 está activado, g = ml
#define VOL_REC_2 1000                  //  Volumen cuando sensor_2 está activado, g = ml
#define VOL_REC_3 1500                  //  Volumen cuando sensor_3 está activado, g = ml

#define N_SENSORES_REC 3                //  Numero de sensores utilizados en el Tanque de Recarga
#define RESOLUCION_REC 500              //  Resolucion entre cada sensor  

#define T_VAL_CONMUT 2000               //  Tiempo para conmutar las valvulas, tanto para abrir como cerrar


class Deposito { 
  
  private:
  
  //Atributos estáticos//
  bool recarga_o_descarga;                  //  Variable que nos indica si, a la hora de simular, nos interesa si el deposito se recarga o descarga agua en el siguiente//  
  bool es_fugas ;                           //  Parámetro que nos indicará si el tanque es el de fugas (1)//
  bool es_recarga ;                         //  Parámetro que nos indicará si el tanque es el de recarga (1)//

  bool peticion_pausa_recarga;
  bool peticion_pausa_venta;
  bool peticion_pausa_fuga;
  
  bool recarga_pausada;
  bool venta_pausada;
  bool fuga_pausada;


  Deposito *deposito_sig;                   //  Puntero que apunta al deposito siguiente con objetivo de obtener el permiso para realizar una descarga//
  Deposito *deposito_ant;                   //  Puntero que apunta al deposito siguiente con objetivo de obtener el permiso para realizar una descarga//

  int PIN_bomba_llenado;
  int PIN_bomba_vaciado ;
  int PIN_Rele1_Llenado;
  int PIN_Rele2_Llenado;
  int PIN_Rele1_Vaciado;
  int PIN_Rele2_Vaciado;
  int PIN_IN1;
  int PIN_IN2;
  int PIN_ENA;
  
  int sensor_0;
  int sensor_1;
  int sensor_2;
  int sensor_3;

  HX711_ADC LoadCell;                       //Declaracion de la bascula  
  int HX711_dout; 
  int HX711_sck;
  float valor_calibracion_basculas;

//  Atributos referidos al tanque anterior  //
  float vol_seguridad_anterior; 
  float vol_limite_anterior;
  float vol_min_comienzo_bomba;
  
//  Tomamos el tiempo de inicio para establecer el tiempo de CONMUTADO de las valvulas //
  long t_inicio_valv_llenado_abierta = 0;
  long t_inicio_valv_llenado_cerrada = 0;
  long t_inicio_valv_vaciado_abierta = 0;
  long t_inicio_valv_vaciado_cerrada = 0;

//  Condicion para no repetir conmutado de las valvulas //
  bool abrir_valv_llenado_activada_antes = 0;
  bool cerrar_valv_llenado_activada_antes = 0;
  bool abrir_valv_vaciado_activada_antes = 0;
  bool cerrar_valv_vaciado_activada_antes = 0;

//  Atributos de simulación //
  float capacidad_maxima;                   //  Límite máximo que puede alcanzar el deposito//
  float vol_max_seguridad;                  //  Límite hasta el cual se podrá llenar el deposito sin llegar al umbral máximo//
  float volumen_descarga_max;               //  Límites de la simulación//
  float volumen_descarga_min;               //  Límites de la simulación//
  float t_prox_descarga_max;                //  Límites de la simulación//
  float t_prox_descarga_min;                //  Límites de la simulación//
  float vol_prox_mov;                       //  Cantidad de agua que descargará el deposito cuando la simulación lo diga//
  float t_prox_mov;                         //  Tiempo en el que se realizará el próximo movimiento del tanque//
   
  //Atributos de estado//
  int estado_actual;                        //  Recoge el estado en el que se encuentra el deposito//
  float volumen;                            //  Señal que almacena el valor de volumen del deposito// 
  float velocidad;                          //  En caso de que el tanque sea el de fugas, esta variable guardará la velocidad de cuanto fugará//   
  
  float volumen_total_recarga = 0;
  float volumen_total_venta = 0;
  int ventas_totales = 0;
  int recargas_totales = 0;
  

  int NIVEL_DEPURACION = 0;

//  Volumenes calculados antes de entrar en FUNC. NORMAL
  float descarga_sensor_3 = 0;
  float descarga_sensor_2 = 0;
  float descarga_sensor_1 = 0;

  bool vacio_almacenamiento; 

  bool llenar_sensor_1;
  bool vaciar_sensor_1;
  
  bool llenar_sensor_2;
  bool vaciar_sensor_2;

  bool llenar_sensor_3;
  bool vaciar_sensor_3;





  public:

  //Constructor de la clase deposito//

  Deposito (int _PIN_bomba_llenado, int _PIN_bomba_vaciado, int _PIN_Rele1_Llenado, int _PIN_Rele2_Llenado, int _PIN_Rele1_Vaciado, 
int _PIN_Rele2_Vaciado, int _PIN_IN1, int _PIN_IN2, int _PIN_ENA, int _HX711_dout, int _HX711_sck, float _valor_calibracion_basculas, int _sensor_0, int _sensor_1,  int _sensor_2, int _sensor_3, 
float _capacidad_maxima, float _vol_max_seguridad, float _volumen_descarga_max, float _volumen_descarga_min, float _t_prox_descarga_max, float _t_prox_descarga_min, 
bool _recarga_o_descarga, Deposito *deposito_ant, Deposito *deposito_sig, bool _es_fugas, bool _es_recarga, float _vol_seguridad_anterior, float _vol_limite_anterior, float _vol_min_comienzo_bomba);

  //  Método que comprueba el estado del deposito//
  bool actualizar_estado (int tSimulacion, int alarma, int volver_a_empezar);


  //  Método que abre la válvula de llenado //
  void abrir_valvula_llenado ();


  //  Método que abre la válvula de vaciado //
  void abrir_valvula_vaciado ();


  //Método que cierra la válvula de llenado //
  void cerrar_valvula_llenado ();


  //  Método que cierra la válvula de vaciado //
  void cerrar_valvula_vaciado ();


  //  Método que generará valores aleatorios de volumen a las variables de la simulación  //
  void nuevo_vol_descarga ();

  
  //  Método que generará valores aleatorios de tiempo a las variables de la simulación //
  void nuevo_t_prox_mov ();


  //  Método que nos permite ver si el tanque está listo para que se realice una descarga en él //
  int ready_to_receive (float vol_prox_mov);


  //  Método que nos permite ver si el tanque está listo para que se realice una descarga en él//
  int ready_to_send (float vol_prox_mov);


  //  Método que nos permite saber el volumen del tanque en todo momento  //
  float get_volumen ();


  //  Método que convierte la señal recibida por los sensores a volumen //
  float conversion_volumen ();


  //  Método genera un caudal para las fugas  //
  int genera_caudal ();


  //  Método que realiza el tarado inicial  //
  bool realizar_tara ();


  //  Método que realiza la solicitud neceraria para el Datalogger  dependiendo del Deposito  //
  bool solicitud_pausa_recarga ();
  bool solicitud_pausa_venta ();
  bool solicitud_pausa_fuga ();

  void quitar_pausa();


  //  Método que nos permite saber el volumen acumulado del tanque de recarga  //
  float volumen_acumulado_recarga();

  
  //  Método que nos permite saber el volumen acumulado del deposito de ventas  //
  float volumen_acumulado_venta();
  
  //  Método que nos permite saber las recargas realizadas  //
  int recargas_realizadas();
  
  //  Método que nos permite saber las ventas realizadas  //
  int ventas_realizadas();

  //  Método que resetea las recargas realizadas  //
  void reset_recargas_realizadas();
  
  //  Método que resetea las ventas realizadas  //
  void reset_ventas_realizadas();


  //  Método que realiza el llenado de las recargas y su posterior medicion  //
  bool llenar_recarga_1 ();
  bool vaciar_recarga_1 ();
  void medir_recarga_1 ();
  
  bool llenar_recarga_2 ();
  bool vaciar_recarga_2 ();
  void medir_recarga_2 ();
  
  bool llenar_recarga_3 ();
  bool vaciar_recarga_3 ();
  void medir_recarga_3 ();

  bool vaciar_almacemiento ();
  void resetar_vaciar_almacemiento();

  void ciclos_cero ();

  
  float recarga_sensor_1_volumen();    
  float recarga_sensor_2_volumen();
  float recarga_sensor_3_volumen();

  float serial_prox_vol();
  float serial_volumen();


};
