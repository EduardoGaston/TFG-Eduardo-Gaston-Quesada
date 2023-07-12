//

#include "Controladora.h"
#include "Deposito.h"

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
               
#include "DetectaFlanco.h" 
DetectaFlanco dfEmergencia(PIN_EMERGENCIA);


void setup () {
  Serial.begin(57600);
  Serial.println("Comenzamos...");
  
// Asociación de pines de entradas y salidas //
  pinMode (alarma_rebosamiento, OUTPUT);
  pinMode (PIN_START, INPUT);
  pinMode (PIN_STOP, INPUT);
  pinMode (PIN_EMERGENCIA, INPUT);    
  pinMode (PIN_DIAL, INPUT);             
  
  dfEmergencia.inicio(INPUT_PULLUP);

  pinMode (PIN_bomba_llenado_recarga, OUTPUT);
  pinMode (PIN_Rele1_Vaciado_recarga, OUTPUT);  
  pinMode (PIN_Rele2_Vaciado_recarga, OUTPUT);                        
  pinMode (PIN_bomba_vaciado_recarga, OUTPUT);
  pinMode (PIN_Rele1_Llenado_dispensador_1, OUTPUT);  
  pinMode (PIN_Rele2_Llenado_dispensador_1, OUTPUT);  
  pinMode (PIN_bomba_llenado_dispensador_1, OUTPUT);  
  pinMode (PIN_Rele1_Vaciado_dispensador_1, OUTPUT);   
  pinMode (PIN_Rele2_Vaciado_dispensador_1, OUTPUT);   
  pinMode (PIN_bomba_vaciado_dispensador_1, OUTPUT);                                                                                                              
  pinMode (PIN_Rele1_Vaciado_fuga, OUTPUT); 
  pinMode (PIN_Rele2_Vaciado_fuga, OUTPUT); 
  pinMode (PIN_bomba_vaciado_fugas, OUTPUT);
  pinMode (PIN_ENA_fuga, OUTPUT);
  pinMode (PIN_IN1_fuga, OUTPUT);
  pinMode (PIN_IN2_fuga, OUTPUT);

  pinMode (sensor_0_recarga, INPUT);
  pinMode (sensor_1_recarga, INPUT);  
  pinMode (sensor_2_recarga, INPUT);
  pinMode (sensor_3_recarga, INPUT);   

//  Hacemos que no conmute los reles debido a su logica inversa enviando un HIGH desde el Arduino //
//  ACTIVAMOS RELE (LOW) PARA QUE LA VALVULA ESTÉ CONECTADA A TIERRA Y NO SE CALIENTE LA PLACA REGULADORA DE 4,5 V
  digitalWrite (PIN_bomba_llenado_recarga, HIGH);
  digitalWrite (PIN_Rele1_Vaciado_recarga, HIGH);  
  digitalWrite (PIN_Rele2_Vaciado_recarga, LOW);            //  ACTIVAMOS RELE PARA QUE LA VALVULA ESTÉ CONECTADA A TIERRA Y NO SE CALIENTE LA PLACA REGULADORA DE 4,5 V
  digitalWrite (PIN_bomba_vaciado_recarga, HIGH);
  digitalWrite (PIN_Rele1_Llenado_dispensador_1, HIGH);  
  digitalWrite (PIN_Rele2_Llenado_dispensador_1, LOW);  
  digitalWrite (PIN_bomba_llenado_dispensador_1, HIGH);  
  digitalWrite (PIN_Rele1_Vaciado_dispensador_1, HIGH);   
  digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
  digitalWrite (PIN_bomba_vaciado_dispensador_1, HIGH);                                                                                                              
  digitalWrite (PIN_Rele1_Vaciado_fuga, HIGH); 
  digitalWrite (PIN_Rele2_Vaciado_fuga, LOW); 
  digitalWrite (PIN_bomba_vaciado_fugas, HIGH);      

}

//  Tanques declarados a partir de la clase deposito  //
Deposito tanque_retorno (PIN_bomba_vaciado_dispensador_1, PIN_bomba_llenado_recarga, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
volumen_retorno_limite, volumen_retorno_seguridad, 0, 0, 0, 0, 0, &tanque_retorno ,&tanque_retorno, 0, 1, 0, 0, 0);

Deposito tanque_almacenamiento (222, 222, 0, 0, 0, 0, 0, 0, 0, dout_almacenamiento, sck_almacenamiento, calibracion_almacenamiento, 0, 0, 0, 0,
volumen_almacenamiento_limite, volumen_almacenamiento_seguridad, 0, 0, 0, 0, 1, &tanque_almacenamiento, &tanque_almacenamiento, 0, 0, 0, 0, 0);

Deposito tanque_recarga (PIN_bomba_llenado_recarga, PIN_bomba_vaciado_recarga, 0, 0, PIN_Rele1_Vaciado_recarga, 
PIN_Rele2_Vaciado_recarga, 0, 0, 0, 0, 0, 0, sensor_0_recarga, sensor_1_recarga, sensor_2_recarga, sensor_3_recarga, volumen_recarga_limite, volumen_recarga_seguridad,
volumen_recarga_max, volumen_recarga_min, t_prox_recarga_max, t_prox_recarga_min, 1, &tanque_retorno, &tanque_almacenamiento, 0, 1, 0, 0, 0);

Deposito dispensador_1 (PIN_bomba_llenado_dispensador_1, PIN_bomba_vaciado_dispensador_1, PIN_Rele1_Llenado_dispensador_1, PIN_Rele2_Llenado_dispensador_1, PIN_Rele1_Vaciado_dispensador_1, 
PIN_Rele2_Vaciado_dispensador_1, 0, 0, 0, dout_dispensador_1, sck_dispensador_1, calibracion_dispensador_1, 0, 0, 0, 0, volumen_ventas_limite,
volumen_ventas_seguridad, volumen_ventas_max, volumen_ventas_min, t_prox_venta_max, t_prox_venta_min, 0, &tanque_almacenamiento,&tanque_retorno, 0, 0, volumen_almacenamiento_seguridad, volumen_almacenamiento_limite, vol_min_alm_comienzo_bomba);

Deposito tanque_fugas (0, PIN_bomba_vaciado_fugas, 0, 0, PIN_Rele1_Vaciado_fuga, PIN_Rele2_Vaciado_fuga, PIN_IN1_fuga, PIN_IN2_fuga, 
PIN_ENA_fuga, dout_fugas, sck_fugas, calibracion_fugas, 0, 0, 0, 0, volumen_fugas_limite, volumen_fugas_seguridad, 0, 0, 0, 0, 1, &tanque_almacenamiento, &tanque_retorno, 1, 0, 0, 0, 0);


//  Bucle principal donde se irán recorriendo los valores de los sensores y se ejecutaran las llamadas a las funciones pertinentes  //
void loop () {
  start = digitalRead (PIN_START);
  stop_ = digitalRead (PIN_STOP);
  seta_emergencia = digitalRead (PIN_EMERGENCIA);
  dial = digitalRead (PIN_DIAL);
             
  int flancoEmergencia = dfEmergencia.comprueba();  

//  Si la seta_emergencia está presionada al iniciar arduino, se queda a la espera de iniciar el CICLO que deja los limites inferiores de los depositos
  if ((seta_emergencia == HIGH) && (estado_actual == COMPROBAR_CICLO_INICIAL)) {
    estado_actual = CICLO_FONDO_TANQUES;
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: CICLO_FONDO_TANQUES" );
    }
  } 

//  Al dejar de prersionar la seta_emergencia y producirse un flanco de bajada, se inicia el CICLO. Abrimos todas las valvulas  //
//  Comenzamos llenando el tanque de recarga y a su vez el tanque de almacenamiento, LLENANDO_RECARGA_ALMACEN //
  if ((flancoEmergencia == -1) && (estado_actual == CICLO_FONDO_TANQUES)) {            
    digitalWrite (PIN_bomba_llenado_recarga, LOW);      //  Los Relés tienen logica inversa con Arduino, al mandar un LOW = Relé conmuta/activado
    digitalWrite (PIN_bomba_vaciado_recarga, LOW);      
    t_inicio_valvulas_abiertas = millis();              //  Tomamos el tiempo de inicio para establecer el tiempo de CONMUTADO de las valvulas, en este caso abiertas    
    digitalWrite (PIN_Rele1_Vaciado_recarga, HIGH);
    digitalWrite (PIN_Rele2_Vaciado_recarga, HIGH);     //  Los Relés tienen logica inversa con Arduino, al mandar un HIGH = Relé no conmuta/desactivado
    digitalWrite (PIN_Rele1_Llenado_dispensador_1, HIGH);   
    digitalWrite (PIN_Rele2_Llenado_dispensador_1, HIGH);  
    digitalWrite (PIN_Rele1_Vaciado_dispensador_1, HIGH);   
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, HIGH);   
    digitalWrite (PIN_Rele1_Vaciado_fuga, HIGH); 
    digitalWrite (PIN_Rele2_Vaciado_fuga, HIGH);    
    t_inicio_tanque_recarga_almacen = millis();         //  Tomamos el tiempo de inicio del estado: LLENANDO_RECARGA_ALMACEN 
    estado_actual = LLENANDO_RECARGA_ALMACEN; 
  }

//  Se está llenando el tanque de recarga y a su vez el tanque de almacenamiento //
  if (estado_actual == LLENANDO_RECARGA_ALMACEN) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: LLENANDO_RECARGA_ALMACEN" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    }
  //  Una vez pasado el tiempo definido en T_PARTE_RECARGA_ALMACEN dejamos de llenar el tanque de recarga y comenzamos a llenar el tanque de fugas  // 
  //  Comenzamos el siguiente estado: LLENANDO_FUGA  //
    if (millis() > (T_PARTE_RECARGA_ALMACEN + t_inicio_tanque_recarga_almacen)) {    
      digitalWrite (PIN_bomba_llenado_recarga, HIGH);  
      digitalWrite (PIN_IN1_fuga, HIGH);                                         
      digitalWrite (PIN_IN2_fuga, LOW);
      analogWrite (PIN_ENA_fuga, 255);
      t_inicio_tanque_fuga = millis();                  //  Tomamos el tiempo de inicio del estado: LLENANDO_FUGA 
      estado_actual = LLENANDO_FUGA;      
    }
  }
  
//  Se está llenando el tanque de almacenamiento y a su vez el tanque de fugas //
  if (estado_actual == LLENANDO_FUGA) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: LLENANDO_FUGA" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    }
  //  Una vez pasado el tiempo definido en T_PARTE_FUGAS dejamos de llenar el tanque de almacenamiento y el tanque de fugas  //
  //  Comenzamos el siguiente estado: LLENANDO_DEPOSITO_1, llenamos el deposito de ventas 1 (vaciamos el tanque de almacenamiento) a la vez que vaciamos el mismo //    
    if (millis() > (T_PARTE_FUGAS + t_inicio_tanque_fuga)) {
      digitalWrite (PIN_IN1_fuga, LOW);                                         
      digitalWrite (PIN_IN2_fuga, LOW);
      analogWrite (PIN_ENA_fuga, 0);  
      digitalWrite (PIN_bomba_vaciado_recarga, HIGH); 
      digitalWrite (PIN_bomba_vaciado_dispensador_1, LOW);
      digitalWrite (PIN_bomba_llenado_dispensador_1, LOW);    
      t_inicio_tanque_deposito_1 = millis();            //  Tomamos el tiempo de inicio del estado: LLENANDO_DEPOSITO_1 
      estado_actual = LLENANDO_DEPOSITO_1;      
    }
  }

//  Se está llenando (vaciando tanque de almacenamiento)  y vaciando el deposito de ventas 1  //
  if (estado_actual == LLENANDO_DEPOSITO_1) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: LLENANDO_DEPOSITO_1" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    }
  //  Una vez pasado el tiempo definido en T_PARTE_DEPOSITO_1 dejamos de llenar el deposito de ventas 1 y seguimos vaciando dicho deposito  //
  //  Comenzamos el siguiente estado: COLOCANDO_FONDOS_FINALES, vaciamos tambien el tanque de fugas //
    if (millis() > (T_PARTE_DEPOSITO_1 + t_inicio_tanque_deposito_1)) {
      digitalWrite (PIN_bomba_llenado_dispensador_1, HIGH);
      digitalWrite (PIN_bomba_vaciado_fugas, LOW);      
      t_inicio_fondos_finales = millis();               //  Tomamos el tiempo de inicio del estado: COLOCANDO_FONDOS_FINALES 
      estado_actual = COLOCANDO_FONDOS_FINALES;      
    }
  }
  
//  Se están vaciando el deposito de ventas 1 y el deposito de fugas //
  if (estado_actual == COLOCANDO_FONDOS_FINALES) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: COLOCANDO_FONDOS_FINALES" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    }
  //  Una vez pasado el tiempo definido en T_PARTE_FONDO_FINAL dejamos de vaciar el deposito de ventas 1 y tanque de fugas  //
  //  Cerramos todas las valvulas. Comenzamos el siguiente estado: HACER_TARA  //   
    if (millis() > (T_PARTE_FONDO_FINAL + t_inicio_fondos_finales)) {
      digitalWrite (PIN_bomba_vaciado_dispensador_1, HIGH); 
      digitalWrite (PIN_bomba_vaciado_fugas, HIGH);
      t_inicio_valvulas_cerradas = millis();            //  Tomamos el tiempo de inicio para establecer el tiempo de CONMUTADO de las valvulas, en este caso cerradas      
      digitalWrite (PIN_Rele1_Vaciado_recarga, LOW);  
      digitalWrite (PIN_Rele2_Vaciado_recarga, LOW); 
      digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
      digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);  
      digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
      digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
      digitalWrite (PIN_Rele1_Vaciado_fuga, LOW); 
      digitalWrite (PIN_Rele2_Vaciado_fuga, LOW); 
      estado_actual = HACER_TARA; 
    }
  }
  
//  Si la seta_emergencia no está presionada al iniciar arduino, se supone que ya tenemos realizados los fondos de los depositos de la planta //
//  Cerramos todas las valvulas. Comenzamos el siguiente estado: HACER_TARA  //     
  if ((seta_emergencia == LOW) && (estado_actual == COMPROBAR_CICLO_INICIAL)) {
    t_inicio_valvulas_abiertas = millis();              //  Tomamos el tiempo de inicio para establecer el tiempo de CONMUTADO de las valvulas, en este caso cerradas      
    digitalWrite (PIN_Rele1_Vaciado_recarga, LOW);  
    digitalWrite (PIN_Rele2_Vaciado_recarga, LOW); 
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);  
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
    digitalWrite (PIN_Rele1_Vaciado_fuga, LOW); 
    digitalWrite (PIN_Rele2_Vaciado_fuga, LOW); 
    estado_actual = HACER_TARA;
  }


//  Control del tiempo de conmutado de las VALVULAS, tanto abiertas como cerradas para evitar el calentamiento de la placa reguladora de 4,5V //
//  Una vez pasado el tiempo definido en T_VALVULAS_CONMUTADAS, la entrada alimentada pasa a conectarse a tierra. 
  //  Condicion para valvulas que se abren  // 
  if ((t_inicio_valvulas_abiertas > 0) && (millis() > (T_VALVULAS_CONMUTADAS + t_inicio_valvulas_abiertas))) {     
    digitalWrite (PIN_Rele1_Vaciado_recarga, HIGH);  
    digitalWrite (PIN_Rele2_Vaciado_recarga, LOW);            //  ACTIVAMOS RELE PARA QUE LA VALVULA ESTÉ CONECTADA A TIERRA Y NO SE CALIENTE LA PLACA REGULADORA DE 4,5 V
    digitalWrite (PIN_Rele1_Llenado_dispensador_1, HIGH);  
    digitalWrite (PIN_Rele2_Llenado_dispensador_1, LOW);  
    digitalWrite (PIN_Rele1_Vaciado_dispensador_1, HIGH);   
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
    digitalWrite (PIN_Rele1_Vaciado_fuga, HIGH); 
    digitalWrite (PIN_Rele2_Vaciado_fuga, LOW);      
    t_inicio_valvulas_abiertas = 0;
  }
  //  Condicion para valvulas que se cierran  // 
  if ((t_inicio_valvulas_cerradas > 0) && (millis() > (T_VALVULAS_CONMUTADAS + t_inicio_valvulas_cerradas))) {   
    digitalWrite (PIN_Rele1_Vaciado_recarga, HIGH);  
    digitalWrite (PIN_Rele2_Vaciado_recarga, LOW);            //  ACTIVAMOS RELE PARA QUE LA VALVULA ESTÉ CONECTADA A TIERRA Y NO SE CALIENTE LA PLACA REGULADORA DE 4,5 V
    digitalWrite (PIN_Rele1_Llenado_dispensador_1, HIGH);  
    digitalWrite (PIN_Rele2_Llenado_dispensador_1, LOW);  
    digitalWrite (PIN_Rele1_Vaciado_dispensador_1, HIGH);   
    digitalWrite (PIN_Rele2_Vaciado_dispensador_1, LOW);   
    digitalWrite (PIN_Rele1_Vaciado_fuga, HIGH); 
    digitalWrite (PIN_Rele2_Vaciado_fuga, LOW); 
    t_inicio_valvulas_cerradas = 0;
  }

//  Realizamos la tara de los depositos con el fondo ya establecido //
  if (estado_actual == HACER_TARA) {
    if (TIPO_DEPURACION == 5){        
      Serial.println( "Estado: HACER_TARAS" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    bool tara_almacenam = tanque_almacenamiento.realizar_tara();
    bool tara_disp_1 = dispensador_1.realizar_tara();
    bool tara_fugas = tanque_fugas.realizar_tara();    
  // Una vez realizadas las taras de los depositos, entramos en funcionamiento normal // 
    if ((tara_disp_1) && (tara_almacenam) && (tara_fugas) == true) {
      estado_actual = LLENAR_VOLUMEN_RECARGA_1;
      estado_taras = HECHO;           
    }
  }


//  Si se produce un flanco de subida en la seta_emergencia entramos en el estado de EMERGENCIA //
  if (flancoEmergencia == 1) {    
    estado_actual = EMERGENCIA;
  }

//  Tomamos el volumen de los depositos //
  if (estado_taras == HECHO) {                  
    tanque_almacenamiento.get_volumen(); 
    dispensador_1.get_volumen();
    tanque_recarga.get_volumen();
    tanque_fugas.get_volumen();
  }
  

//**************************************  FUNCIONAMIENTO NORMAL  *****************************************************************************************
  if (estado_actual == FUNC_NORMAL) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: FUNCIONAMIENTO_NORMAL" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if (stop_ == HIGH) {
      estado_actual = PAUSA;
    }
    estado_recarga = tanque_recarga.actualizar_estado(tSimulacion, seta_emergencia, 0);
    estado_disp_1 = dispensador_1.actualizar_estado(tSimulacion, seta_emergencia, 0);
    estado_fugas = tanque_fugas.actualizar_estado(tSimulacion, seta_emergencia, 0);
    if ((estado_recarga == true) || (estado_disp_1 == true) || (estado_fugas == true)) {
      estado_actual = EMERGENCIA;
    }
  }

//*******************************************  PAUSA  *****************************************************************************************
  if (estado_actual == PAUSA) {
    if (TIPO_DEPURACION == 5){            
      Serial.println( "Estado: PAUSA" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    tanque_recarga.cerrar_valvula_llenado();
    tanque_recarga.cerrar_valvula_vaciado();
    dispensador_1.cerrar_valvula_llenado();
    dispensador_1.cerrar_valvula_vaciado();             
    if ((stop_ == LOW) && (seta_emergencia == LOW) && (start == HIGH)) {
      estado_actual = FUNC_NORMAL;
    }
  }

//*******************************************  EMERGENCIA  *****************************************************************************************
  if (estado_actual == EMERGENCIA) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: EMERGENCIA" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    digitalWrite (alarma_rebosamiento, HIGH);
    tanque_recarga.actualizar_estado(tSimulacion, HIGH, 0);
    dispensador_1.actualizar_estado(tSimulacion, HIGH, 0);
    tanque_fugas.actualizar_estado(tSimulacion, HIGH, 0); 
    if ((seta_emergencia == LOW) && (start == HIGH)) {
      estado_actual = LLEVAR_COND_INIC;
    }
  }
 
//*************************************  LLEVAR COND INICIALES  *****************************************************************************************
  if (estado_actual == LLEVAR_COND_INIC) {
    if (TIPO_DEPURACION == 5){ 
      Serial.println( "Estado: LLEVAR_A_COND_INICIALES" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    digitalWrite (alarma_rebosamiento, LOW);
    tanque_recarga.actualizar_estado(tSimulacion, seta_emergencia, 0);
    tanque_recarga.cerrar_valvula_llenado();
    dispensador_1.actualizar_estado(tSimulacion, seta_emergencia, 0);
    tanque_fugas.actualizar_estado(tSimulacion, seta_emergencia, 0);    
    if (((tanque_recarga.get_volumen() <= UMBRAL_VACIO) && (tanque_almacenamiento.get_volumen() <= UMBRAL_VACIO) && (dispensador_1.get_volumen() <= UMBRAL_VACIO) &&
      (dial == 0)) || ((dial == 1) && (start == HIGH))) {
      tanque_recarga.actualizar_estado(tSimulacion, seta_emergencia, 1);
      dispensador_1.actualizar_estado(tSimulacion, seta_emergencia, 1);
      tanque_fugas.actualizar_estado(tSimulacion, seta_emergencia, 1);
      estado_actual = FUNC_NORMAL;         
    }
  }




                  //  PASOS DESPUES DE HABER REALIZADO LA TARA Y ANTES DE ENTRAR EN FUNC. NORMAL  //

/********************************************************************************************************************************/
/**************************************     LLLENAR Y MEDIR VOLUMEN SENSOR 1    *************************************************/
/********************************************************************************************************************************/
  
  if (estado_actual == LLENAR_VOLUMEN_RECARGA_1) {
    tanque_recarga.llenar_recarga_1();
    if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: LLENAR_VOLUMEN_RECARGA_1" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if ( tanque_recarga.llenar_recarga_1() == 1 ) {
      tanque_recarga.cerrar_valvula_llenado();
      estado_actual = VACIAR_VOLUMEN_RECARGA_1;
    }
  }

  if (estado_actual == VACIAR_VOLUMEN_RECARGA_1) {
    tanque_recarga.vaciar_recarga_1();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_VOLUMEN_RECARGA_1" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if (  tanque_recarga.vaciar_recarga_1() == 1 ) {
      tanque_recarga.cerrar_valvula_vaciado();
      estado_actual = MEDIR_VOLUMEN_RECARGA_1;
    }
  }
  
  if (estado_actual == MEDIR_VOLUMEN_RECARGA_1) {
    tanque_recarga.medir_recarga_1();
    estado_actual = VACIAR_ALMACENAMINETO_1;
  }

  if (estado_actual == VACIAR_ALMACENAMINETO_1) {
    dispensador_1.vaciar_almacemiento();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_ALMACENAMINETO_1" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
      if ( dispensador_1.vaciar_almacemiento() == 1 ) {
        dispensador_1.cerrar_valvula_llenado();     
        dispensador_1.cerrar_valvula_vaciado();
        estado_actual = LLENAR_VOLUMEN_RECARGA_2;
    }
  }

  
/********************************************************************************************************************************/
/**************************************     LLLENAR Y MEDIR VOLUMEN SENSOR 2    *************************************************/
/********************************************************************************************************************************/

  if (estado_actual == LLENAR_VOLUMEN_RECARGA_2) {
    dispensador_1.resetar_vaciar_almacemiento();
    dispensador_1.ciclos_cero();   
    tanque_recarga.llenar_recarga_2();
    if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: LLENAR_VOLUMEN_RECARGA_2" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if ( tanque_recarga.llenar_recarga_2() == 1 ) {
      tanque_recarga.cerrar_valvula_llenado();
      estado_actual = VACIAR_VOLUMEN_RECARGA_2;
    }
  }

  if (estado_actual == VACIAR_VOLUMEN_RECARGA_2) {
    tanque_recarga.vaciar_recarga_2();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_VOLUMEN_RECARGA_2" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if (  tanque_recarga.vaciar_recarga_2() == 1 ) {
      tanque_recarga.cerrar_valvula_vaciado();
      estado_actual = MEDIR_VOLUMEN_RECARGA_2;
    }
  }
  
  if (estado_actual == MEDIR_VOLUMEN_RECARGA_2) {
    tanque_recarga.medir_recarga_2();
    estado_actual = VACIAR_ALMACENAMINETO_2;
  }

  if (estado_actual == VACIAR_ALMACENAMINETO_2) {
    dispensador_1.vaciar_almacemiento();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_ALMACENAMINETO_2" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
      if ( dispensador_1.vaciar_almacemiento() == 1 ) {
        dispensador_1.cerrar_valvula_llenado();     
        dispensador_1.cerrar_valvula_vaciado();
        estado_actual = LLENAR_VOLUMEN_RECARGA_3;
    }
  }
  

/********************************************************************************************************************************/
/**************************************     LLLENAR Y MEDIR VOLUMEN SENSOR 3    *************************************************/
/********************************************************************************************************************************/

  if (estado_actual == LLENAR_VOLUMEN_RECARGA_3) {
    dispensador_1.resetar_vaciar_almacemiento();
    dispensador_1.ciclos_cero();   
    tanque_recarga.llenar_recarga_3();
    if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: LLENAR_VOLUMEN_RECARGA_3" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if ( tanque_recarga.llenar_recarga_3() == 1 ) {
      tanque_recarga.cerrar_valvula_llenado();
      estado_actual = VACIAR_VOLUMEN_RECARGA_3;
    }
  }

  if (estado_actual == VACIAR_VOLUMEN_RECARGA_3) {
    tanque_recarga.vaciar_recarga_3();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_VOLUMEN_RECARGA_3" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
    if (  tanque_recarga.vaciar_recarga_3() == 1 ) {
      tanque_recarga.cerrar_valvula_vaciado();
      estado_actual = MEDIR_VOLUMEN_RECARGA_3;
    }
  }
  
  if (estado_actual == MEDIR_VOLUMEN_RECARGA_3) {
    tanque_recarga.medir_recarga_3();
    estado_actual = VACIAR_ALMACENAMINETO_3;
  }

  if (estado_actual == VACIAR_ALMACENAMINETO_3) {
    dispensador_1.vaciar_almacemiento();
        if (TIPO_DEPURACION == 1){ 
      Serial.println( "Estado: VACIAR_ALMACENAMINETO_3" );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
    } 
      if ( dispensador_1.vaciar_almacemiento() == 1 ) {
        dispensador_1.cerrar_valvula_llenado();     
        dispensador_1.cerrar_valvula_vaciado();
        estado_actual = CONDICIONES_FUNC_NORMAL;
    }
  }


/********************************************************************************************************************************/
/**************************************     CONDICIONES PARA ENTRAR EN FUNC NORMAL    *************************************************/
/********************************************************************************************************************************/
  
  if (estado_actual == CONDICIONES_FUNC_NORMAL) {
    dispensador_1.resetar_vaciar_almacemiento();
    dispensador_1.ciclos_cero();
    tanque_recarga.nuevo_vol_descarga ();                   /*********************************************************************************/   
    estado_actual = FUNC_NORMAL;
  }




/********************************************************************************************************************************/
/**************************************     SERIAL    *************************************************/
/********************************************************************************************************************************/
    
    if (TIPO_DEPURACION == 2){ 
      Serial.print( "Volumen: " );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
      Serial.println( tanque_recarga.serial_volumen() );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
      Serial.print( "Volumen Proximo Mov: " );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
      Serial.println( tanque_recarga.serial_prox_vol()  );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial    
    } 



    if (TIPO_DEPURACION == 4){ 
      Serial.print( "Volumen: " );      //  Mensaje utilizado para depurar errores mediante el Monitor Serial
      Serial.println( dispensador_1.get_volumen() );
} 

/****************************************************************************************************************************
/**************************************    DATALOGGER   *************************************************/
/**************** Toma de valores de volumen de los depositos de manera periódica ************************************************/
/****************************************************************************************************************************/
  
  if ( (millis() > (T_INTERVALO_DATALOGGER + t_datalogger)) && (estado_actual == FUNC_NORMAL) ) {   
    tanque_recarga.solicitud_pausa_recarga();
    dispensador_1.solicitud_pausa_venta();
    tanque_fugas.solicitud_pausa_fuga();
    
    if ( (tanque_recarga.solicitud_pausa_recarga() && dispensador_1.solicitud_pausa_venta() && tanque_fugas.solicitud_pausa_fuga() ) == 1) {
      Serial.print("# ");
      Serial.print( tanque_almacenamiento.get_volumen() );
      Serial.print(" , ");
      Serial.print( tanque_recarga.recargas_realizadas() );
      Serial.print(" , ");
      Serial.print( tanque_recarga.volumen_acumulado_recarga() );
      Serial.print(" , ");
      Serial.print( dispensador_1.ventas_realizadas() );
      Serial.print(" , ");
      Serial.print( dispensador_1.volumen_acumulado_venta() );
      Serial.print(" , ");
      Serial.println( tanque_fugas.get_volumen() ); 
      t_datalogger = millis();
      tanque_recarga.reset_recargas_realizadas();
      dispensador_1.reset_ventas_realizadas();
      tanque_recarga.quitar_pausa();
      dispensador_1.quitar_pausa();
      tanque_fugas.quitar_pausa();
    }
  }




//  Incrementamos tiempos de simulación //
  tSimulacion = millis();
  tSimulacion = tSimulacion/1000;
}
