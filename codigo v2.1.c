1.//
//Librerias necesarias: 
//display i2c               descarga------->  https:www.prometec.net/wp-content/uploads/2014/10/LiquidCrystal_I2C.zip
//TimerOne:                 descarga------->  https:www.prometec.net/wp-content/uploads/2014/11/TimerOne.zip
//
//Ajustar mediante el preset del display el contraste.
//
//Detalles: puede que la direcciòn i2c no sea la correcta
//
//Actualmente esta configurado con la direccion 0x3f.
//En caso de no funcionar podes probar con la direccion 0x27.
//Sino descargar desde este link y cargar el sketch en el uno, 
//lo que hace es escanear el bus i2c en busca de dispositivos conectados,
//una vez encontrado lo manda por terminal https://www.prometec.net/wp-content/uploads/2014/10/I2C_scan.rar
//
//
//Conexion del lcd:
//
//arduino       display
//   scl   -->    scl 
//   sda   -->    sda 
//   5v    -->    vcc
//   gnd   -->    gnd
//
//   
//conexion del arduino a los driver PAPs:
//
//
//Alimentacion de los motores: 
//
//      Tension de entrada de 8v a 35v alimentacion logica (vdd en polulu's) 5V Se deben unir ambos gnd, tanto Vmot como Vlogic.
//      
//      Pines 'RESET' y SLEEP' deben ir juntos a VCC(5v). 
//      Pin 'DIR' puede ir tanto a vcc(5v) o gnd, dependiendo el sentido de giro que se necesite.
//      MS1,MS2 y MS3 configura los micropasos, no creo que sean necesarios, pueden ir libres o a gnd.
//      
//Motor de avance de papel:
//
//arduino       polulu 
//  pin 7        step 
//  pin 5       enable
//  
//Motor continuo:
//arduino       polulu 
//  pin 6        step
//  pin 5       enable
//  
//Botonera:
//arduino                          botones                  --------> (Con resistencia Pulldown)
//  pin 2                       Switch sensor
//  pin 3                         On/Off ---------------------------> (pulsacion simple resetea el contador de botellas, pulsacion larga apaga la placa) 
//  pin 8                     botton incremento 
//  pin 9                     botton decremento 
//  pin 12                          enter -------------------------->(si esta en el menu centrar se habilita/deshabilita el motor continuo, si esta en otros menu entras para modificar parametros)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




#include <TimerOne.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);          //<--------------------- modificar en esta linea la direccion

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////    VARIABLES Y DEFINICIONES    ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool flag=0,enable1=0,enable2=0,enter_menu=0,trabajando=0;
long int vel1=0,cont=0,cont2=0,velocidad=0;
unsigned long int T1_ant=0;
int var5=13,var6=0,index=2,copias=0,tiempo_ON=0,var9;



#define sensor        2                         //IRQ 1  <--------------- NO SE PUEDE MODIFICAR EL NUMERO DE PIN
#define onoff         3                         //IRQ 2  <--------------- NO SE PUEDE MODIFICAR EL NUMERO DE PIN
#define enable        5 
#define step2         6 
#define step1         7 
#define mas           8
#define menos         9

#define enter         12


#define longbutton    400                       // Tiempo de pulsacion para considerar una pulsacion larga


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////    INTERRUPCIONES    ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sensor_ISR(){

  if(enable1 == 0 & enable2 == 1){

    enable1=1; 
    T1_ant=millis();
    copias++;
    var5=14;

    //enter_menu=0;
    
  }
 
}


void ISR_pulsos(){

  if(flag){
    if(enable1){
        digitalWrite(step1,HIGH);
    }
    if(enable2){
        digitalWrite(step2,HIGH);
    }
  flag=0;
  }
  else{
    if(enable1){
        digitalWrite(step1,LOW);
    }
    if(enable2){
        digitalWrite(step2,LOW);
    }
  flag=1;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////    FUNCIONES    ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void apagar(){

  presentacion();
  digitalWrite(enable, HIGH);                      //desabilito los drivers
  digitalWrite(step2, LOW);                        //pongo en cero la salida de step2                
  delay(1000);
  lcd.clear();

  lcd.noBacklight();
  while(!digitalRead(onoff));

  asm volatile ("  jmp 0");                        //intruccion assembler para reinicio

}

void barra(int valor){
  int i;
  
    //lcd.clear();
  
    lcd.setCursor (1,0);
    lcd.print(" Velocidad: ");

    if(valor<10){
      lcd.print(" ");
    }
  lcd.print(valor);
  //lcd.print("  ");
  
  lcd.setCursor ( 0, 1 );
  lcd.println(" <[");

  for(i=3;i<13;i++){
      lcd.setCursor( i, 1 );

    if(i<valor+3){
        lcd.print((char)255);             //imprimo cuadrado negro
    }
    else{
        lcd.print((char)254);             //imprimo cuadrado vacio
    }
  }
  lcd.print("]> ");
  
}

void presentacion(){
int i,j,k,var7=0,var8=0,temp=50;

  lcd.clear();
  
  for(j=0;j<2;j++){
    for(i=0;i<16;i++){

      lcd.setCursor(i,j);
      lcd.print(">");
      var7= 15-i;
      lcd.setCursor(var7,j);
      lcd.print("<");
  
      if(j==1 & i>8){
    
        lcd.setCursor(7+(i-8),0);
        lcd.print(" ");
        lcd.setCursor(8-(i-8),0);
        lcd.print(" ");
      }
      delay(temp);
    }
  }


  for(j=0;j<2;j++){
    for(i=0;i<16;i++){

      lcd.setCursor(i,j);
      lcd.print(">");
      var7= 15-i;
      lcd.setCursor(var7,j);
      lcd.print("<");
      delay(temp);
    }

    if(j == 0){
      for(k=0;k<3;k++){
      
        lcd.setCursor(8-k,0);
        lcd.print(" ");
        lcd.setCursor(8+k,0);
        lcd.print(" ");
        delay(temp);
      }
      lcd.setCursor(5,0);
      lcd.print("Wayra");
    } 
  }

    for(k=0;k<5;k++){

      lcd.setCursor(4-k,0);
      lcd.print(" ");
      lcd.setCursor(11+k,0);
      lcd.print(" ");

      if(k<5){
      
        lcd.setCursor(8-k,1);
        lcd.print(" ");
        lcd.setCursor(8+k,1);
        lcd.print(" ");
      }
      delay(temp);
    }
    
  lcd.setCursor ( 0, 1 );
  lcd.println("<<<<Corsa II>>>>");

  lcd.setCursor(10,0);
  lcd.print(" ");
  
  for(i=3;i>=0;i--){

    lcd.setCursor(i,1);
    lcd.print(" ");
    var7= 15-i;
    lcd.setCursor(var7,1);
    lcd.print(" ");
    delay(50);
  }

  delay(1500);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////     SETUP     ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
int checksum=0;
  //Timer1.initialize(150000);  //desborde cada100us
  Timer1.attachInterrupt(ISR_pulsos);// interrupcion de timer;


  attachInterrupt(digitalPinToInterrupt(2), sensor_ISR, HIGH);   //configuracion IRQ sensor
  interrupts();
  
  pinMode(step1, OUTPUT);
  pinMode(step2, OUTPUT);
  pinMode(mas   , INPUT);
  pinMode(menos , INPUT); 
  pinMode(onoff , INPUT);
  pinMode(sensor, INPUT);
  pinMode(enter , INPUT);
   
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.home ();

  presentacion();
  delay(1500);

  digitalWrite(enable,HIGH);

  checksum = EEPROM.read(1) + EEPROM.read(2);

  if( checksum > 2 & checksum < 55){                  //indica que tengo grabados valores validos en la eeprom,entre 2 y 55

      velocidad = EEPROM.read(1);           
      tiempo_ON = EEPROM.read(2);
  }
  else{

      lcd.clear();
      lcd.print("FALLA EN EEPROM");
      delay(5000);
      
      velocidad = 5;                                  //valores por defecto si no hay datos guardados en la EEPROM, 5 de velocidad y 5 segundos de tiempo
      tiempo_ON = 5; 
  }
  vel1 = 1600 - (velocidad * 100);
  Timer1.initialize(vel1);

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////    MAIN    //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  unsigned long int var3=0,var4=0;

  var3= millis() - T1_ant;
  var4= tiempo_ON*1000;

    
  if(var3 > var4){                            //si se cumple el tiempo ON, pongo a 0 el enable, pongo en 0 la salida de step1 y pongo en cero t anterior 
    enable1=0;
    digitalWrite(step1,LOW);
    T1_ant=0;
  }

  if(enable1){

    if(var6>3){
        var6=0;
        var5++;
    }
    else{
      var6++;
    }
    
    lcd.setCursor(var5,1);
    lcd.print(".");
    if(var5>13){

      //lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" Introduciendo  ");
      lcd.setCursor(0,1);
      lcd.print("   Etiqueta.    ");
      var5=11;    
    }
   
  }
  else{

      if(digitalRead(onoff)){
        delay(longbutton);

        if(digitalRead(onoff)){
            apagar();
        }
        else{
          copias=0;
        }        
      }


      if(digitalRead(enter)){

          if(index == 2){
            if(trabajando){                         //si la maquina esta habilitada apago todo y voy a la pantalla principal
              index=2;

              trabajando=0;
              enable2=0;
              digitalWrite(enable,HIGH);            //deshabilito ambos drivers
              digitalWrite(step2, LOW);             //pongo en 0 la salida de step2    
              enter_menu=0;
              
            }
            else{                                    //si no esta habilitada, habilito motores y pongo a funcionar el motor permanente
              while(digitalRead(enter));
              trabajando=1;
              enable2=1;
              digitalWrite(enable, LOW);
            }         
        }                                           //si no se mantuvo presionado entro/salgo de la configuracion de parametros
        else{
          
          if(!enter_menu ){
              enter_menu=1;
          }
          else{
              enter_menu=0;
          }
        }
      }

      if(!enter_menu){                             //si estoy fuera de la configuracion de parametros, con up/down me muevo de pantalla
        
               if(digitalRead(mas) == 1 & index < 3){
                  index++;
               }
               else if(digitalRead(menos) == 1 & index > 1 ){
                  index--;
               }
      }

      switch(index){
        case 1:                                       //pantalla 1: configuracion de avance de cinta

            if(enter_menu){                           //si entro a la modificacion de parametros uso up/down para modificar los segundos
               if(digitalRead(mas) == 1 & tiempo_ON < 45){
                  tiempo_ON++;
               }
               else if(digitalRead(menos) == 1 & tiempo_ON > 1 ){
                  tiempo_ON--;
               }
            }
              
              //lcd.clear();
              lcd.setCursor(1,0); 
              lcd.print("    Avance    ");
              lcd.setCursor(1,1);
              lcd.print((char)127);        
              //lcd.print(" ");

              if(tiempo_ON < 10){                     //agrego un 0 delante si el tiempo es menor a 10
                lcd.print("0");               
              }
              
              lcd.print(tiempo_ON);
              lcd.print(" Segundos ");
              lcd.setCursor(14,1);
              lcd.print((char)126);        

              EEPROM.update(2,tiempo_ON);                        //actualizo el valor tiempo_ON en la EEPROM, la direccion de tiempo_ON es 0x02

              
        break;

        case 2:                                      //pantalla 2: la principal,muestro si esta habilitada/ddeshabilitada la maquina y la cantidad de botellas grabadas

              //lcd.clear();
              lcd.setCursor(0,0);
              if(enable2){
                  lcd.print("    Preparada   ");
              }
              else{
                  lcd.print("     Pausa      ");
              }
              lcd.setCursor(0,1);
              lcd.print("   Botellas: ");
                         
              lcd.print(copias);
              lcd.print("   ");

        break;

        case 3:                                     //pantalla 3: menu de configuracion de velocidad

          if(enter_menu){                           //si entro a la modificacion de parametros con up/down modifico la velocidad de ambos motores(T de desborde del timer)
             if(digitalRead(mas) == 1 & velocidad< 10){
                 velocidad++;
              }
             else if(digitalRead(menos) == 1 & velocidad > 1 ){
                 velocidad--;
              }
          }
          vel1= 1600 -  (velocidad*100);            //actualizo la velocidad seteada al desborde del timer
          Timer1.initialize(vel1);
          barra(velocidad);
          EEPROM.update(1,velocidad);               //actualizo el valor de velocidad en la EEPROM , la direccion de velocidad es 0x01
       break;
        
     }

     if(var9<8){
      var9++;
     }
     else{
      var9=0;
     }
        
        if(enter_menu & index != 2 & var9<4){
              lcd.setCursor(0,0);
              lcd.print((char)126);
              lcd.setCursor(15,0);  
              lcd.print((char)127); 
        }
        else{
              lcd.setCursor(0,0);
              lcd.print(" ");
              lcd.setCursor(15,0);
              lcd.print(" ");
        }
     
   }

   delay(100);
     
}



