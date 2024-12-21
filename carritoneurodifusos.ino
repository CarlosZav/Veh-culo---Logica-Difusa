//EQUIPO

/*
BALDERAS VILCHEZ HUGO ALEXIS
Navarro Angeles Ramon Amaury
ZAVALETA CARRILLO CARLOS ANTONIO
*/


#include <Arduino.h>

float numerodistancia = 0.0; // Variable para guardar 
float numerorpm = 0.0; // Variable que guarda la velcoidad otorgada por el algpritmo de logica difusa (A la que irá el carrito)

// Pines del motor 1 Izquierda
const int IN1 = 27;  // Dirección 1 motor 1
const int IN2 = 26;  // Dirección 2 motor 1
const int ENA = 25;  // PWM velocidad motor 1

// Pines del motor 2 Derecha
const int IN3 = 18;  // Dirección 1 motor 2
const int IN4 = 19;  // Dirección 2 motor 2
const int ENB = 21;  // PWM velocidad motor 2

const int pin_encoder1A = 35; // Pin en el cual se conecta el encoder para el calculo de rpms y distancia recorrida

//Variables para calcular las rpms
unsigned long ultima_medicion = 0; // 
float velocidad_rpm_actual = 0; // Variable que almacena la Velocidad en rpms a la que se encuentra el carrito en cierto momento

int PPR = 447; // Pulsos por revolucion del encoder
bool estado = true; // True = recto && false = curva Estado en el que se encuentra el carrito (avanzando recto o haciendo curva)

float radio_llanta = 0.068; // radio de la llanta en metros
float distancia_curva = 1.0; // En metros, ditancia que recorre el carrito mientras hace la curva
volatile int conteo_pulsos = 0; // Pulsos emitidos por el encoder
volatile int conteo_pulsosRPM = 0; // Conteo de pulsos para cálculo de rpms


float distancia_recorrida = 0; // Variable que guarda la distancia recorrida por el carrito cada ciclo

// Este arreglo de variables de tipo float almacena el valor otorgado por el algortimo fuzzy
float fuzzy_distancia[3];
float fuzzy_velocidad[3];

// Variables que alamcenan el porcentaje de distancia y velocidad otorgadas por el algoritmo fuzzy
float valor_cerca;
float valor_medio;
float valor_lejos;

float valor_lento;
float valor_moderado;
float valor_rapido;

// Variables que obtienen los valores menores al tratarse de reglas de tipo and y además el cálculo del promedio para el defuzzy
float valor_promedio_giro;
float valor_menor_giro1;
float valor_menor_giro2;
float valor_menor_giro3;

float valor_promedio_rectoLento;
float valor_menor_rectoLento1;
float valor_menor_rectoLento2;
float valor_menor_rectoLento3;

float valor_promedio_recto;
float valor_menor_recto1;
float valor_menor_recto2;
float valor_menor_recto3;

float valorY = 0; // Valor de velocidad otorgado por el algoritmo de logica difusa (Este es el valor que debe coincidir con matlab)

float valorPWM = 0; // Valor mapeado otorgado por el algoritmo de logica difusa (valorY mapeado)

void setup() {

  //Impresión en puerto Serial para testeo del funcionamiento
  Serial.begin(9600);

  // Pines de control del motor 1
  pinMode(IN1, OUTPUT); // Pin de control de giro
  pinMode(IN2, OUTPUT); // Pin de control de giro
  pinMode(ENA, OUTPUT); // Pin de control de pwm
  
  // Pines de control del motor 2
  pinMode(IN3, OUTPUT);// Pin de control de giro
  pinMode(IN4, OUTPUT);// Pin de control de giro
  pinMode(ENB, OUTPUT);// Pin de control de pwm

  pinMode(pin_encoder1A, INPUT); // Pin del encoder para cálculo de velocidad y distancia

  attachInterrupt(digitalPinToInterrupt(pin_encoder1A), contar_pulsos, RISING); // Función para detectar cambios del encoder en todo momento
}

void loop() {

  //pedir_numeros(); // Esta función se activa sólo para testo

  // Cálculo de la deistancia recorrida por el carrito 
  float distancia_recorrida = (((conteo_pulsos/360) * 3.1415925536) / 180) * radio_llanta;

  // Manda a llamar a la función que se encarga de calcular la distancia a la que se encuentra el carrtito
  calcular_velocidad();

  funcion_fuzzyDistancia(distancia_recorrida, fuzzy_distancia); // distancia_recorrida
  funcion_fuzzyVelocidad(velocidad_rpm_actual, fuzzy_velocidad); // velocidad_rpm_actual

  // Impresion de Valores otorgados por el algoritmo fuzzy para testeo (Se encuentra deshabilitado al ser sólo para comprobación de resultados)
  /*Serial.print("Distancia: ");
  for (int i = 0; i < 3; i++) {-
    Serial.print(fuzzy_distancia)[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Velocidad: ");
  for (int i = 0; i < 3; i++) {
    Serial.print(fuzzy_velocidad[i]);
    Serial.print(" ");
  }

  Serial.println("\n");*/
// Obtención de los valores menores según nuestras reglas de logica difusa (Son 9 reglas)

//if poco recorrido (cerca) and lento then recto
  if (valor_cerca <= valor_lento){
    valor_menor_recto1 = valor_cerca;
  } else{
    valor_menor_recto1 = valor_lento;
  }

//if poco recorrido (cerca) and moderado then recto
  if (valor_cerca <= valor_moderado){
    valor_menor_recto2 = valor_cerca;
  } else{
    valor_menor_recto2 = valor_moderado;
  }

//if poco recorrido (cerca) and rapido then recto
  if (valor_cerca <= valor_rapido){
    valor_menor_recto3 = valor_cerca;
  } else{
    valor_menor_recto3 = valor_rapido;
  }
// if medio recorrido and lento then recto_lento
  if (valor_medio <= valor_lento){
    valor_menor_rectoLento1 = valor_medio;
  } else{
    valor_menor_rectoLento1 = valor_lento;
  }

// if medio recorrido and moderado then recto_lento
  if (valor_medio <= valor_moderado){
    valor_menor_rectoLento2 = valor_medio;
  } else{
    valor_menor_rectoLento2 = valor_moderado;
  }

// if medio recorrido and rapido then recto_lento
  if (valor_medio <= valor_rapido){
    valor_menor_rectoLento3 = valor_medio;
  } else{
    valor_menor_rectoLento3 = valor_rapido;
  }

  // if muchos recorrido (lejos) and lento  then giro
  if (valor_lejos <= valor_lento){
    valor_menor_giro1 = valor_lejos;
  } else{
    valor_menor_giro1 = valor_lento;
  }

 // if muchos recorrido (lejos) and moderado  then giro
  if (valor_lejos <= valor_moderado){
    valor_menor_giro2 = valor_lejos;
  } else{
    valor_menor_giro2 = valor_moderado;
  }

 // if muchos recorrido (lejos) and rapido  then giro
  if (valor_lejos <= valor_rapido){
    valor_menor_giro3 = valor_lejos;
  } else{
    valor_menor_giro3 = valor_rapido;
  }

  // Defuzzy, en esta parte obtenemos el valor de la velocidad a la que debe ir el carrito otorgada por la logica difusa
  valor_promedio_giro = (valor_menor_giro1 + valor_menor_giro2 + valor_menor_giro3) / 3;
  valor_promedio_rectoLento = (valor_menor_rectoLento1 + valor_menor_rectoLento2 + valor_menor_rectoLento3) / 3;
  valor_promedio_recto = (valor_menor_recto1 + valor_menor_recto2 + valor_menor_recto3) / 3;

  valorY = ((valor_promedio_giro *137.5) + (valor_promedio_rectoLento * 200) + (valor_promedio_recto * 312.5)) / (valor_promedio_giro + valor_promedio_rectoLento + valor_promedio_recto ); // Cálculo de velocidad otorgado por el defuzzy

  /*Serial.println(valor_cerca);
  Serial.println(valor_medio);
  Serial.println(valor_lejos);
  Serial.println(valor_lento);
  Serial.println(valor_moderado);
  Serial.println(valor_rapido);

  Serial.println(valor_promedio_giro);
  Serial.println(valor_promedio_rectoLento);
  Serial.println(valor_promedio_recto);*/

  // Impresión del valor en rpms otorgado por el algoritmo de logica difusa para revisar que sea coincidente con el obtenido desde matlab
  Serial.print("Valor de velocidad: ");
  Serial.println(valorY);

  // Mapeo de Rpms a pwm para el control de los motores
  valorPWM = map(valorY, 100, 400, 0, 255);

  //Impresión para revisión de un correcto resultado
  Serial.print("PWM:   ");
  Serial.println(valorPWM);

  //Control de velocidad de motor 1 y motor 2
  analogWrite(ENA, valorPWM);
  analogWrite(ENB, valorPWM);

  if(estado == false){ // Si se encuentra avanzando recto y debe girar, realiza el giro
    // Realizar giro bloquenado un motor para que se pare (1, 1) y haciendo que el otro motor avance recto (1, 0)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);

    // Impresión del estado del carrito para testo
    Serial.println("Giro");

    // Si la distancia recorrida es igual o mayor al correspondiente, pasar al siguiente estado (avanzar recto)
    if(distancia_recorrida >= distancia_curva){
      estado = true; // Indica que debe avanzar recto
      conteo_pulsos = 0; // Reinicio del conteo de pulsos para volver a calcular la distancia recorrida
    }    

  } else if (estado == true){
    //Avanzar recto 
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    // Impresión del estado del carrito para testo
    Serial.println("Rectoooooooooo");

    // Si la distancia recorrida es igual o mayor al correspondiente, pasar al siguiente estado (girar)
    if(valor_lejos > valor_medio && valor_lejos > valor_cerca){
    estado = false; // Indica que debe girar
    conteo_pulsos = 0; // Reinicio del conteo de pulsos para volver a calcular la distancia recorrida
    }

  }

}

// En esta funcion se realiza el calculo de velocidad del carrito con el encoder
void calcular_velocidad(){
  unsigned long tiempo_actual = millis(); // Se registra el tiempo actual

  if(tiempo_actual - ultima_medicion >= 100){ // Se hace una resta entre el tiempoa ctual y el tiempo anterior para saber el tiempo transcurrido, se hace el cálculo cada 100 ms

    ultima_medicion = tiempo_actual; // Se actualiza el valor de la última medicion del tiempo

    velocidad_rpm_actual = (conteo_pulsosRPM/ PPR) * 600; // Se hace el cálculo de rpms a la que se mueve el carrito (Para hacer esto tomamos en cuenta los pulsos por revolucion del encoder)
    conteo_pulsosRPM = 0; // Se reinicia del conteo de pulsos del encoder para hacer un nuevo cálculo de velocidad cada 100 ms
  }
}


// Aquí empieza el algoritmo de "fusificación" fuzzy

// En la siguiente funcion se guardan las variables de los valores de pertenencia de distancia
void funcion_fuzzyDistancia(float distanciaC, float dis_output[]) {
  // Manda a llamar a las funciones para obtencion de los valores de pertenencia
  dis_output[0] = dis_cerca(distanciaC);
  dis_output[1] = dis_medio(distanciaC);
  dis_output[2] = dis_lejos(distanciaC);
  // En la siguientes variables se guardan los valores de pertenencia
  valor_cerca = dis_output[0];
  valor_medio = dis_output[1];
  valor_lejos = dis_output[2];
}

// En la siguiente funcion se guardan las variables de los valores de pertenencia de velocidad
void funcion_fuzzyVelocidad(float velocidadC, float vel_output[]) {
  // Manda a llamar a las funciones para obtencion de los valores de pertenencia
  vel_output[0] = vel_lento(velocidadC);
  vel_output[1] = vel_medio(velocidadC);
  vel_output[2] = vel_rapido(velocidadC);
  // En la siguientes variables se guardan los valores de pertenencia
  valor_lento = vel_output[0];
  valor_moderado = vel_output[1];
  valor_rapido = vel_output[2];
}

// Cálculo del valor de pertencia de cerca según la distancia recorrida por el carrito
float dis_cerca(float distanciaC) {
  // Coordenadas de la gráfica correspondiente a cerca (entrada)
  float a = 0.0, b = 0.0, c = 0.0, d = 3.0;
  
  // Se obtiene el valor de pertenencia de cerca y se retorna
  if (distanciaC <= a || distanciaC >= d) {
    return 0;
  } else if (distanciaC > a && distanciaC <= b) {
    return (distanciaC - a) / (b - a);
  } else if (distanciaC > b && distanciaC <= c) {
    return 1;
  } else if (distanciaC > c && distanciaC < d) {
    return (d - distanciaC) / (d - c);
  }
  return 0;
}

// Cálculo del valor de pertencia de distancia media según la distancia recorrida por el carrito
float dis_medio(float distanciaC) {

  // Coordenadas de la gráfica correspondiente a media distancia (entrada)
  float a = 2.0, b = 3.5, c = 5.0;
  
  // Se obtiene el valor de pertenencia de media distancia y se retorna
  if (distanciaC <= a || distanciaC >= c) {
    return 0;
  } else if (distanciaC > a && distanciaC <= b) {
    return (distanciaC - a) / (b - a);
  } else if (distanciaC > b && distanciaC < c) {
    return (c - distanciaC) / (c - b);
  }
  return 0;
}

// Cálculo del valor de pertencia de lejos según la distancia recorrida por el carrito
float dis_lejos(float distanciaC) {
  // Coordenadas de la gráfica correspondiente a lejos (entrada)
  float a = 4, b = 5, c = 6, d = 6;
  
  // Se obtiene el valor de pertenencia de lejos y se retorna
  if (distanciaC <= a || distanciaC >= d) {
    return 0;
  } else if (distanciaC > a && distanciaC <= b) {
    return (distanciaC - a) / (b - a);
  } else if (distanciaC > b && distanciaC <= c) {
    return 1;
  } else if (distanciaC > c && distanciaC < d) {
    return (d - distanciaC) / (d - c);
  }
  return 0;
}

// Cálculo del valor de pertencia de lento según la velocidad del carrito
float vel_lento(float velocidadC) {
  // Coordenadas de la gráfica correspondiente a lento (entrada)
  float a = 100.0, b = 100.0, c = 100.0, d = 175.0;
  // Se obtiene el valor de pertenencia de lento y se retorna
  if (velocidadC <= a || velocidadC >= d) {
    return 0;
  } else if (velocidadC > a && velocidadC <= b) {
    return (velocidadC - a) / (b - a);
  } else if (velocidadC > b && velocidadC <= c) {
    return 1;
  } else if (velocidadC > c && velocidadC < d) {
    return (d - velocidadC) / (d - c);
  }
  return 0;
}

// Cálculo del valor de pertencia de velocidad media según la velocidad del carrito
float vel_medio(float velocidadC) {
  // Coordenadas de la gráfica correspondiente a velocidad media (entrada)
  float a = 150.0, b = 200.0, c = 250.0;
  
  // Se obtiene el valor de pertenencia de velocidad media y se retorna
  if (velocidadC <= a || velocidadC >= c) {
    return 0;
  } else if (velocidadC > a && velocidadC <= b) {
    return (velocidadC - a) / (b - a);
  } else if (velocidadC > b && velocidadC < c) {
    return (c - velocidadC) / (c - b);
  }
  return 0;
}

// Cálculo del valor de pertencia de rapido según la velocidad del carrito
float vel_rapido(float velocidadC) {
  // Coordenadas de la gráfica correspondiente a rapido (entrada)
  float a = 225.0, b = 275.0, c = 400.0, d = 400.0;
  
  // Se obtiene el valor de pertenencia de rapido y se retorna
  if (velocidadC <= a || velocidadC >= d) {
    return 0;
  } else if (velocidadC > a && velocidadC <= b) {
    return (velocidadC - a) / (b - a);
  } else if (velocidadC > b && velocidadC <= c) {
    return 1;
  } else if (velocidadC > c && velocidadC < d) {
    return (d - velocidadC) / (d - c);
  }
  return 0;
}

// En esta funcion sólo se piden números al usuario para comprobar el funcionamiento del codigo y que coincida con lo obtenido en matlab, sin embargo, no sé está mandando a llamar en el codigo principal, es sólo para testeo
void pedir_numeros(){
   // Verifica si hay datos disponibles en el puerto serial

   Serial.println("Por favor, introduce dos números separados por un espacio.");
  if (Serial.available() > 0) {
    // Lee una línea completa
    String input = Serial.readStringUntil('\n');
    
    // Intenta dividir la línea en dos números
    int espacio = input.indexOf(' '); // Encuentra el espacio que separa los números
    if (espacio != -1) {
      String strNum1 = input.substring(0, espacio);
      String strNum2 = input.substring(espacio + 1);

      // Convierte las cadenas a flotantes
      numerodistancia = strNum1.toFloat();
      numerorpm = strNum2.toFloat();

      // Muestra los números recibidos
      Serial.print("Numero 1: ");
      Serial.println(numerodistancia);
      Serial.print("Numero 2: ");
      Serial.println(numerorpm);
    } else {
      Serial.println("Por favor, introduce dos números separados por un espacio.");
    }
  }
}


// Esta es la función que se activa cada vez que existen cambios en el encoder 
IRAM_ATTR void contar_pulsos(){
  conteo_pulsos ++; // Registro de pulsos del encoder para cálculo de distancia
  conteo_pulsosRPM ++; // Registro de pulsos del encoder para cálculo de velocidad
}

