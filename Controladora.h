//  DECLARACION DE VARIABLES  //

//  Espacio para detectar y depurar errores del codigo mediante el Monitor Serial //
#define TIPO_DEPURACION 0
// 1 = Ciclo Reacarga


//  Tiempos utilizados  //
long t_inicio_tanque_recarga_almacen = 0;                     
#define T_PARTE_RECARGA_ALMACEN 10000                         //  Tiempo utilizado para llenar tanque de almacenamiento

long t_inicio_tanque_fuga = 0;                                
#define T_PARTE_FUGAS 10000                                   //  Tiempo utilizado para llenar deposito de fugas y vaciar t. recargas

long t_inicio_tanque_deposito_1 = 0;                          
#define T_PARTE_DEPOSITO_1 40000                              //  Tiempo utilizado para llenar deposito de ventas 1 y vaciar t. almacenamiento

long t_inicio_fondos_finales = 0;                             
#define T_PARTE_FONDO_FINAL 30000                             //  Tiempo utilizado para terminar de vaciar los depositos

long t_inicio_valvulas_abiertas = 0;
long t_inicio_valvulas_cerradas = 0;
#define T_VALVULAS_CONMUTADAS 2000                            //  Tiempo para conmutar las valvulas, tanto para abrir como cerrar

long t_datalogger = 0;                                        //  Tiempo en el que se realizó el Datalogger
#define T_INTERVALO_DATALOGGER 20000                          //  Tiempo periodico para realizar el Datalogger


//  Variables asociadas a las entradas y salidas analógicas del arduino, encargadas de las lectura de los sensores  //

#define sensor_0_recarga 8                                    //  Sensor optico de nivel situado mas abajo en el tanque de recarga
#define sensor_1_recarga 9                                    //  Sensor optico de nivel situado por encima del sensor anterior (500)g          
#define sensor_2_recarga 10                                   //  Sensor optico de nivel situado por encima del sensor anterior (1000)g      
#define sensor_3_recarga 11                                   //  Sensor optico de nivel situado mas arriba en el tanque de recarga (1500)g          

#define dout_almacenamiento 22                                //  Sensor de volumen (báscula) asociado al tanque de almacenamiento 
#define sck_almacenamiento 23                                    
float calibracion_almacenamiento = 686;                       //  Valor de calibracion asosiado a la bascula del tanque de almacenamiento       

#define dout_dispensador_1 24                                 //  Sensor de volumen (báscula) asociado al dispensador 1
#define sck_dispensador_1 25 
float calibracion_dispensador_1 = 675;                      //  677 Valor de calibracion asosiado a la bascula del dispensador 1    

#define dout_fugas 26                                         //  Sensor de volumen (báscula) asociado al tanque de fugas
#define sck_fugas 27
float calibracion_fugas = 700;                                //  662 Valor de calibracion asosiado a la bascula del tanque de fugas  

//  Variables asociadas a las salidas digitales del arduino para controlar los reles utilizados en el cambio de sentido del voltaje en la valvulas  //

#define PIN_Rele1_Vaciado_recarga 28
#define PIN_Rele2_Vaciado_recarga 29                                                                                                                                                                      
#define PIN_Rele1_Llenado_dispensador_1 30
#define PIN_Rele2_Llenado_dispensador_1 31
#define PIN_Rele1_Vaciado_fuga 34
#define PIN_Rele2_Vaciado_fuga 35
#define PIN_Rele1_Vaciado_dispensador_1 39
#define PIN_Rele2_Vaciado_dispensador_1 40

//  Variables asociadas a las salidas del arduino y las PINes de apertura y cierre de bombas, así como las señales de emergencia  //

#define PIN_bomba_llenado_recarga 42       //36      antes                    //  Señal que activa el relé de la bomba de llenado del tanque de recarga 
#define PIN_bomba_vaciado_recarga 33                          //  Señal que activa el relé de la bomba de vaciado del tanque de recarga hacia el tanque principal                           
#define PIN_bomba_llenado_dispensador_1 32                    //  Señal que activa el relé de la bomba de llenado del dispensador 1  
#define PIN_bomba_vaciado_dispensador_1 36   //42      antes                                                                     //  Señal que activa el relé de la bomba de vaciado en el dispensador 1
#define PIN_bomba_vaciado_fugas 37                                                                                        //  Señal que activa la bomba de vaciado del tanque de fugas                                                                                 
#define PIN_ENA_fuga 7                                        //  Señal analógica que controla la velocidad del caudal que se está fugando 
#define PIN_IN1_fuga 44                                       //  Señal que controla el sentido de la bomba peristaltica del tanque de fugas                              
#define PIN_IN2_fuga 45                                       //  Señal que controla el sentido de la bomba peristaltica del tanque de fugas                        

//  Variables que definen los límites de los depositos  //

float volumen_recarga_limite = 2000;                          //  Valor máximo de volumen del tanque de recarga
float volumen_recarga_seguridad = 1900;                       //  Valor de seguridad de volumen del tanque de recarga

float volumen_almacenamiento_seguridad = 1600;                //  Valor de seguridad de volumen del tanque de almacenamiento
float volumen_almacenamiento_limite = 1800;                   //  Valor máximo de volumen del tanque de almacenamiento
float vol_min_alm_comienzo_bomba = 700;                   //  Valor de volumen minimo en t. almacenamiento para que comience a rotar la bomba 

float volumen_ventas_limite = 500;                            //  Valor máximo de volumen de los dispensadores
float volumen_ventas_seguridad = 450;                         //  Valor de seguridad de volumen de los dispensadores

float volumen_fugas_limite = 500;                             //  Valor máximo de volumen del tanque de fugas
float volumen_fugas_seguridad = 450;                          //  Valor de seguridad de volumen del tanque de fugas

float volumen_retorno_limite = 10000;
float volumen_retorno_seguridad = 10000;


//  Variables que definen los límites de cada una de las distribuciones aleatorias que se usarán a lo largo del código  //

#define volumen_recarga_min 500                               //  Valor mínimo del volumen de descarga para su distribución aleatoria
#define volumen_recarga_max 1500                              //  Valor máximo del volumen de descarga para su distribución aleatoria
#define volumen_ventas_min 100                                //  Valor mínimo del volumen de ventas para su distribución aleatoria
#define volumen_ventas_max 400                                //  Valor máximo del volumen de ventas para su distribución aleatoria
#define t_prox_recarga_min 25000                              //  Valor mínimo del tiempo de la próxima descarga para su distribución aleatoria
#define t_prox_recarga_max 50000                              //  Valor máximo del tiempo de la próxima descarga para su distribución aleatoria
#define t_prox_venta_min 13000                                //  Valor mínimo del tiempo de la próxima ventas para su distribución aleatoria
#define t_prox_venta_max 20000                                //  Valor máximo del tiempo de la próxima ventas para su distribución aleatoria

//  Indica si se ha realizado o no la TARA del deposito
bool tara_recarga;                                            //  Indica si se ha realizado la tara de la bascula ('1') o no ('0') del tanque de recarga
bool tara_disp_1;                                             //  Indica si se ha realizado la tara de la bascula ('1') o no ('0') del deposito 1
bool tara_fugas;                                              //  Indica si se ha realizado la tara de la bascula ('1') o no ('0') del tanque de fungas

//  Variables temporales que influyen en la simulación de los eventos aleatorios  //
bool estado_recarga;
bool estado_disp_1;
bool estado_fugas;
long tSimulacion = 0;                                         //  Tiempo de la simulación

//  MAQUINAS DE ESTADOS  //

//  Variables de la máquina de estados de la controladora y del panel de control  //
#define FUNC_NORMAL 1
#define PAUSA 2
#define EMERGENCIA 3
#define LLEVAR_COND_INIC 4

//  Variables de la maquina de estados al realizar el llenado de los fondos de los depositos  //
#define COMPROBAR_CICLO_INICIAL 5
#define CICLO_FONDO_TANQUES 6
#define LLENANDO_RECARGA_ALMACEN 7
#define LLENANDO_FUGA 8
#define LLENANDO_DEPOSITO_1 9
#define COLOCANDO_FONDOS_FINALES 10
int estado_actual = COMPROBAR_CICLO_INICIAL;

//  Variables de la maquina de estados al realizar la tara  //
#define HACER_TARA 11
#define SIN_HACER 12                                          //  Estados del tarado de los depositos
#define HECHO 13
int estado_taras = SIN_HACER;  

//  Variables de la maquina de estados al calcular el volumen de las recargas //

#define LLENAR_VOLUMEN_RECARGA_1 15
#define VACIAR_VOLUMEN_RECARGA_1 16
#define MEDIR_VOLUMEN_RECARGA_1 17
#define VACIAR_ALMACENAMINETO_1 18

#define LLENAR_VOLUMEN_RECARGA_2 19
#define VACIAR_VOLUMEN_RECARGA_2 20
#define MEDIR_VOLUMEN_RECARGA_2 21
#define VACIAR_ALMACENAMINETO_2 22

#define LLENAR_VOLUMEN_RECARGA_3 23
#define VACIAR_VOLUMEN_RECARGA_3 24
#define MEDIR_VOLUMEN_RECARGA_3 25
#define VACIAR_ALMACENAMINETO_3 26

#define CONDICIONES_FUNC_NORMAL 27

//  Botonera de la planta  //
#define PIN_START 2                                           //  START
#define PIN_STOP 3                                            //  Botón STOP 
#define PIN_DIAL 4                                            //  Posición del dial (0->automático, 1->manual)  
#define PIN_EMERGENCIA 5                                      //  Reset de la planta                           
#define alarma_rebosamiento 6                                 //  Señal de alarma en caso de rebosamiento
int start;                                                    //  Señal de START
int stop_;                                                    //  Señal de STOP 
int seta_emergencia;                                          //  Señal de reset                            
int dial;                                                     //  Señal que recoge la posición del dial (0->automático, 1->manual)

 
