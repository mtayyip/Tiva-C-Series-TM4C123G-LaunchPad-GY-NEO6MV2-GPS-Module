/*
 * main.c
 *  Created on: 29 Haziran 2018
 *      Author: Muhammet Tayyip Çankaya
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

/* GPS
 * TX-->PD6
 * RX-->PD7
 * Vcc--> 3.3V
 */

char* saatAyarla(char str[]);
void readGPSModule();

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    //GPS PINLERI
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);
    GPIOPinTypeUART(GPIO_PORTD_BASE,GPIO_PIN_6|GPIO_PIN_7);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    UARTConfigSetExpClk(UART2_BASE,SysCtlClockGet(),9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));
    UARTEnable(UART2_BASE);



    //$GPRMC,200751.00,A,4047.32510,N,02929.63031,E,9.879,105.80,301117,,,A*6C
    while(1){
        readGPSModule();
    }
    //UARTDisable(UART1_BASE);

}

//Kaza olduðu zaman konum alýnacak.Ve SMS yollanacak.
//latitude ==>enlem
//longitude==>boylam
//Ornek:
//$GPRMC,200751.00,A,4047.32510,N,02929.63031,E,9.879,105.80,301117,,,A*6C
void readGPSModule(void){
    char c0,GPSValues[100],latitudeResult[10],longitudeResult[10],parseValue[12][20],*token,tarih[9],*saat,guncelSaat[9];
    double latitude=0.0,longitude=0.0,seconds=0.0,result=0.0,minutes=0.0;
    const char comma[2] = ",";
    int index=0,degrees,i=0,j=0;


    while(!UARTCharsAvail(UART2_BASE));
    c0=UARTCharGetNonBlocking(UART2_BASE);

    //gelen data $GPRMC mi?
    if(c0=='$'){
        while(!UARTCharsAvail(UART2_BASE));
        char c1=UARTCharGetNonBlocking(UART2_BASE);
        if(c1=='G'){
            while(!UARTCharsAvail(UART2_BASE));
            char c2=UARTCharGetNonBlocking(UART2_BASE);
            if(c2=='P'){
                while(!UARTCharsAvail(UART2_BASE));
                char c3=UARTCharGetNonBlocking(UART2_BASE);
                if(c3=='R'){
                    while(!UARTCharsAvail(UART2_BASE));
                    char c4=UARTCharGetNonBlocking(UART2_BASE);
                    if(c4=='M'){
                        while(!UARTCharsAvail(UART2_BASE));
                        char c5=UARTCharGetNonBlocking(UART2_BASE);
                        if(c5=='C'){
                            while(!UARTCharsAvail(UART2_BASE));
                            char c6=UARTCharGetNonBlocking(UART2_BASE);
                            if(c6==','){
                                while(!UARTCharsAvail(UART2_BASE));
                                char c7=UARTCharGetNonBlocking(UART2_BASE);

                                //verileri GPSValues arrayine atama.son veri olan checksum a kadar oku(checksum:A*60 gibi)
                                while(c7!='*'){
                                    GPSValues[index]=c7;
                                    while(!UARTCharsAvail(UART2_BASE));
                                    c7=UARTCharGetNonBlocking(UART2_BASE);
                                    index++;}


                                //GPSValues arrayindeki verileri virgul e gore ayirma
                                index=0;
                                token = strtok(GPSValues, comma);
                                while( token != NULL ) {
                                    strcpy(parseValue[index], token);
                                    token = strtok(NULL, comma);
                                    index++;}


                                //parseValue[1] = A ise veri gecerli - V ise gecerli degil
                                if(strcmp(parseValue[1],"A")==0){
                                    latitude=atof(parseValue[2]);
                                    longitude=atof(parseValue[4]);


                                    //latitude hesaplama
                                    degrees=latitude/100;
                                    minutes=latitude-(double)(degrees*100);
                                    seconds=minutes/60.00;
                                    result=degrees+seconds;
                                    sprintf(latitudeResult,"%f", result);


                                    //longitude hesaplama
                                    degrees=longitude/100;
                                    minutes=longitude-(double)(degrees*100);
                                    seconds=minutes/60.00;
                                    result=degrees+seconds;
                                    sprintf(longitudeResult, "%f", result);


                                    //printf("https://www.google.com/maps/place/%s+%s \n",latitudeResult,longitudeResult);
                                    //tarih duzeltme
                                    for(i=0;i<6;i++){
                                        tarih[j]=parseValue[index-2][i];
                                        if(i==1 || i==3){
                                            j++;
                                            tarih[j]='/';}
                                        j++;}
                                    tarih[8]='\0';


                                    //saat düzeltme +3 UTC ayarlama
                                    saat=saatAyarla(parseValue[0]);
                                    j=0;
                                    for(i=0;i<6;i++){
                                        guncelSaat[j]=saat[i];
                                        if(i==1 || i==3){
                                            j++;
                                            guncelSaat[j]=':';}
                                        j++;}
                                    guncelSaat[8]='\0';




                                    printf("Saat  = %s\n",guncelSaat);
                                    printf("Tarih = %s\n",tarih);
                                    printf("Enlem = %s\n",latitudeResult);
                                    printf("Boylam= %s\n\n\n",longitudeResult);
                                    SysCtlDelay(SysCtlClockGet()/6);}
                                else{
                                    printf("  GPS Verileri Okunuyor\n\n\n");}

                                printf("");
                        }}}}}}}
}


char* saatAyarla(char str[]){
    int num10 = str[0]-'0';
    int num1 = str[1]-'0';
    int saatVerisi=num10*10+num1;

    saatVerisi=saatVerisi+3;
    if(saatVerisi>24){
        saatVerisi=saatVerisi%24;
        if(saatVerisi<10){
            str[0]='0';
            str[1]=saatVerisi+'0';}
        else{
            str[0]=(saatVerisi/10)+'0';
            str[1]=(saatVerisi%10)+'0';}}
    else{
        str[0]=(saatVerisi/10)+'0';
        str[1]=(saatVerisi%10)+'0';}
    return str;
}
