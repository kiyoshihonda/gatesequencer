#include "MsTimer2.h"
#include "SoftwareSerial.h"
#include <EEPROM.h>
//const int led=3;


//SoftwareSerial mySerial(7, 8); // RX, TX
//デジタル
//const int D_PIN_PWM = 6;
const int rclkPin = 11;   // (11) ST_CP [RCLK] on 74HC595 
const int srclkPin = 10;   // (9)  SH_CP [SRCLK] on 74HC595
const int dsPin = 12;     // (12) DS [SER] on 74HC595


const int A_rclkPin = 8;   // 8x8　アノード　シフトレジスタ
const int A_srclkPin = 7;   // 
const int A_dsPin = 9;     // 

const int CR_rclkPin = 5;   //  8x8　カソードR　シフトレジスタ
const int CR_srclkPin = 4;   //
const int CR_dsPin = 6;     // 



const int A_PIN_OUT1=3; // Gate Out1
const int A_PIN_OUT2=2; // Gate Out2
const int A_PIN_OUT3=13; // Gate Out3

//アナログ
const int A_TAKTSWITCH1=3;//ボタン1-4 
const int A_TAKTSWITCH2=4; //ボタン5-8
const int A_CLOCK_INPUT=1; //Clock In 
const int A_BUTTON1=5; //設定ボタン１ 2
const int A_VOLUME=0; //つまみs


int longpressA=0;
//------
const float D_V_OUT = 5;      //出力電圧

//入力信号をn分平均とってサンプル化のために利用
#define n 1                          
int f[n]={0};
float ave = 0;   

//MsTimerの割り込み処理関連
void time_count(void);
void LEDVIEW(void);
int selectMode=0;
//シフトレジスタで利用
byte leds = 0; //ledsをbyte型としてb00000000で定義

//自分の処理で利用する変数
int lastAnalog1;
int nowPoint;

int channel;
int page;
int maxstep;

bool nowON[3]={false,false,false};
bool nowPlay;
int zeroZcount;
int lastClick[5];
int lastClickA[5];

int tiggerStep[4];

bool mode=false;//false 内部クロック  true 外部クロック
int nowInfoDisplay=0;
bool nowOpening=true;
//------------------------------ setup
void setup(){
      selectMode=0;
      nowOpening=true;
      Serial.begin(115200);
      longpressA=0;
      nowInfoDisplay=0;
      pinMode(rclkPin, OUTPUT);   //11番ピンをOUTPUTとして定義
      pinMode(dsPin, OUTPUT);     //12番ピンをOUTPUTとして定義
      pinMode(srclkPin, OUTPUT);  //10番ピンをOUTPUTとして定義
      
      pinMode(A_rclkPin, OUTPUT);
      pinMode(A_srclkPin, OUTPUT);
      pinMode(A_dsPin, OUTPUT);
      
      pinMode(CR_rclkPin, OUTPUT);
      pinMode(CR_srclkPin, OUTPUT);
      pinMode(CR_dsPin, OUTPUT);


      pinMode(A_PIN_OUT1, OUTPUT); 
      pinMode(A_PIN_OUT2, OUTPUT); 
      pinMode(A_PIN_OUT3, OUTPUT); 

    

      for(int i=0;i<5;i++){
            lastClick[i]=-2*i;
            lastClickA[i]=-2*i;
      }
      channel=0;
      page=0;
     // setIcon();
      
      nowPoint=0;
      lastAnalog1=0;

      for(int x=0;x<4;x++){
            tiggerStep[x]=0;
      }
      for(int i=0;i<5;i++){lastClick[i]=-1;}
      channel=0;

      shiftOut(dsPin, srclkPin, LSBFIRST, 0x000); //シフト演算を使って点灯するLEDを選択
      digitalWrite(rclkPin, HIGH);               //送信終了後RCLKをHighにする
     
       analogReference(DEFAULT);
  //analogReference(INTERNAL); //アナログ入力安定のおまじない

      MsTimer2::set(10, time_count);
      MsTimer2::start();      

      eraseAll();
       setIcon();
      mode=false;
      maxstep=8;

      //Serial.println( EEPROM.length());
         
}
int innerCLock=0;
int taktSW1to4,taktSW5to8;
int button1,button2;
int clickTackNo;
byte VRAM[8];
byte VRAM_BACKUP[8];
byte LOGO[8] = {
      B11111111,
      B01000001,
      B01111101,
      B01111011,
      B01110111,
      B01101111,
      B01000001,
      B11111111
};
byte NUMBERICON[20][8] = {{
      B11111111,
      B11111001,
      B11111101,
      B11111101,
      B11111101,
      B11111101,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11111101,
      B11110001,
      B11110111,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11111101,
      B11110001,
      B11111101,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11110101,
      B11110101,
      B11110101,
      B11110001,
      B11111101,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11110111,
      B11110001,
      B11111101,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11110111,
      B11110001,
      B11110101,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11110101,
      B11111101,
      B11111101,
      B11111101,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11110101,
      B11110001,
      B11110101,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11110001,
      B11110101,
      B11110001,
      B11111101,
      B11110001,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11010101,
      B11010101,
      B11010101,
      B11010001,
      B11111111,
      B11111111
},{
      B11111111,
      B11011101,
      B11011101,
      B11011101,
      B11011101,
      B11011101,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11011101,
      B11010001,
      B11010111,
      B11010001,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11011101,
      B11010001,
      B11011101,
      B11010001,
      B11111111,
      B11111111
},{
      B11111111,
      B11010101,
      B11010101,
      B11010101,
      B11010001,
      B11011101,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11010111,
      B11010001,
      B11011101,
      B11010001,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11010111,
      B11010001,
      B11010101,
      B11010001,
      B11111111,
      B11111111
},{
      B11111111,
      B11010001,
      B11010101,
      B11011101,
      B11011101,
      B11011101,
      B11111111,
      B11111111
},
{           //SELECT 
      B01111111,
      B01001001,
      B01001001,
      B01111111,
      B01001001,
      B01001001,
      B01111111,
      B01111111
},
{           //SAVE 
      B11111111,
      B11110111,
      B11110111,
      B11010101,
      B11100011,
      B11110111,
      B11011101,
      B11000001
},
{           //LOAD 
      B11111111,
      B11110111,
      B11100011,
      B11010101,
      B11110111,
      B11110111,
      B11011101,
      B11000001
}
};

int line=0;
int icontime=0;
void drawMatrixLED(){
      digitalWrite(A_rclkPin,LOW);
      shiftOut(A_dsPin, A_srclkPin, LSBFIRST, (1<<(7-line)));
        
      digitalWrite(CR_rclkPin,LOW);
      shiftOut(CR_dsPin, CR_srclkPin, LSBFIRST, VRAM[line]);
      digitalWrite(CR_rclkPin,HIGH); 
      
      digitalWrite(A_rclkPin,HIGH);
      line++;
      if(line>7)line=0;
}
void onPixel(int x,int y){
      VRAM[y]=VRAM[y] & (0xFF ^ ( B00000001 << (7-x))  );
}
void offPixel(int x,int y){
      VRAM[y]=VRAM[y] | ( ( B00000001 << (7-x))  );
}
void reversePixel(int x,int y){
      VRAM[y]=VRAM[y] ^ ( ( B00000001 << (7-x))  );
}
void eraseAll(){
      for(int y=0;y<8;y++){
            VRAM[y]=0XFF;
      }
}
void setIcon(){
      for(int y=0;y<8;y++){
            VRAM[y]=LOGO[y];
      }
}
int test=0;
int vol;
int clickTackNoAA;
int countClock=0;
int button1Count=0;
int analogcount=0;
int longpressB=0;
void setInfo(int num){
      for(int y=0;y<8;y++){
            VRAM_BACKUP[y]=VRAM[y];
      }
      if(num<100){
            for(int y=0;y<8;y++){
                  VRAM[y]=NUMBERICON[num-1][y];
            }
            nowInfoDisplay=1;
      }else if(num<1000){
            num= num-100;
            for(int y=0;y<8;y++){
                  VRAM[y]=VRAM[y] & (0xffff ^ (1<<num));
            }
            nowInfoDisplay=1;
      }else{
            selectMode=num-1000;
            for(int y=0;y<8;y++){
                  VRAM[y]=NUMBERICON[17][y];
            }
            selectModeLED();
      }
      drawMatrixLED();
}
void backVram(){
      for(int y=0;y<8;y++){
            VRAM[y]=VRAM_BACKUP[y];
      }
}
//------------------------------ loop
void loop(){
     //  Serial.println( taktSW1to4 );
     analogcount++;


     if(nowOpening){
            icontime++;
            if(icontime>15000){
                  nowOpening=false;
                  eraseAll();
            }
            drawMatrixLED();
            return;
     }
     if(nowInfoDisplay>0){
           nowInfoDisplay++;
           if(nowInfoDisplay>=3000){
                 nowInfoDisplay=0;
                 Serial.println("endende");
                 eraseAll();
                 backVram();
           }
           drawMatrixLED();
           return;
     }
     
      drawMatrixLED();

      if(mode || analogcount<100){
            //入力の平均からサンプル化 ノイズ対策
            for(int i=n-1;i>0;i--) f[i] = f[i-1];//過去１０回分の値を記録
            f[0] = analogRead(A_CLOCK_INPUT);
            ave = 0;
            for(int i=0;i<n;i++) ave += f[i];        //総和を求める
            ave = (float)ave/n;   //標本数で割って平均を求める
            //Serial.println( analogRead(A_CLOCK_INPUT) ) ;
            //CLOCK入力がある程度なくなったら、スタート地点にもどる（リセット処理）
            if(nowPlay && ave==0){
                  zeroZcount++;
                  if(zeroZcount>8000){
                        Serial.println( String( "  ---- STOP ----- "));
                        zeroZcount=0;
                        nowPlay=false;
                  }
            }else{
                  zeroZcount=0;
            }
      }
      


      if(mode && f[0]>500){
            if(lastAnalog1<=500){
                  triggerStart();
        //           Serial.println("TRIIGER" );
            }
      }
     //Serial.println( analogRead(2));
       if(mode ){
           if(f[0]>1000){
                  countClock++;
                  if(countClock>300){
                        countClock=0;
                        mode=false;
                        Serial.println( "内部クロック" );
                        
                   }
           }else{
                 countClock=0;
           }
      }
      if(!mode && f[0]<500 && lastAnalog1<500){
            countClock++;
            if(countClock>30){
                  countClock=0;
                  mode=true;
                  Serial.println( "外部クロック" );
            }
      }

      lastAnalog1=f[0];//ave;


      //--------------------------------------------
      button1 = analogRead(A_BUTTON1);
      //Serial.println( button1);
      clickTackNoAA=-1;
      if(button1>=-10 && button1<350){
            button1Count++;
            clickTackNoAA=0;
            longpressB++;
      }else if(button1>=350 && button1<900){
            button1Count++;
            clickTackNoAA=1;
            longpressB++;
      }
      if(lastClickA[0] >=0 && lastClickA[0]==lastClickA[1] && lastClickA[0]==lastClickA[2] && clickTackNoAA==-1 ){//ONpressUP
           
             if(longpressB>1200){
                   if(!selectMode){
                        if(lastClickA[0]==0){
                              setInfo(1001) ;
                        // saveData(0);
                              Serial.println( "LONG  ボタン  青 ");
                        }
                        if(lastClickA[0]==1){
                              setInfo(1002) ;
                              
                        // loadData(0);
                              Serial.println( "LONG ボタン黄色");
                        }
                   }
             }else{
                   if(selectMode){
                         if(lastClickA[0]==0){//青 CANCEL
                              setLED();
                              eraseAll();
                              setDataToVRAM();
                         }
                         if(lastClickA[0]==1){//黄色　 RESET
                               resetData();
                         }
                         selectMode=0;
                   }else{
                        if(lastClickA[0]==0){
                              channel++;
                              page=0;
                              if(channel>2)channel=0;
                              setLED();
                              setInfo(100+5-(channel*2+page));
                              Serial.println( "ボタン  青 " +String(channel));
                        }
                        if(lastClickA[0]==1){
                              page++;
                              if(page>1)page=0;
                              setLED();
                              setInfo(100+5-(channel*2+page));
                              Serial.println( "ボタン黄色" +page);
                        }
                   }
             }
             longpressB=0;
              button1Count=0;
      }
      for(int i=0;i<4;i++){
            lastClickA[i+1]=lastClickA[i];
      }
      lastClickA[0] = clickTackNoAA;


      //--------------------------------------------1~8のボタン
      taktSW1to4= analogRead(A_TAKTSWITCH1);
      taktSW5to8= analogRead(A_TAKTSWITCH2);
      //Serial.println( taktSW5to8 );
      clickTackNo=-1;
      if(taktSW1to4>=-10 && taktSW1to4<350){
            clickTackNo=0;longpressA++;
      }else if(taktSW1to4>=350 && taktSW1to4<590){
            clickTackNo=1;longpressA++;
      }else if(taktSW1to4>=590 && taktSW1to4<720){
            clickTackNo=2;longpressA++;
      }else if(taktSW1to4>=720 && taktSW1to4<790){
            clickTackNo=3;longpressA++;
      }
      if(taktSW5to8>=-10 && taktSW5to8<350){
            clickTackNo=4;longpressA++;
      }else if(taktSW5to8>=350 && taktSW5to8<590){
            clickTackNo=5;longpressA++;
      }else if(taktSW5to8>=590 && taktSW5to8<720){
            clickTackNo=6;longpressA++;
      }else if(taktSW5to8>=720 && taktSW5to8<790){
            clickTackNo=7;longpressA++;
            
      }
      if(lastClick[0] >=0 && lastClick[0]==lastClick[1] && lastClick[0]==lastClick[2] && clickTackNo==-1 ){//ONpressUP
            if(selectMode==1){//save
                 saveData(lastClick[0]);
                 setLED();
                 eraseAll();
                 setDataToVRAM();
                 setInfo(19);
                 selectMode=0;
            }else if(selectMode==2){//load
                  loadData(lastClick[0]);
                  selectMode=0;
                  setInfo(20);
            }else{
                  if(longpressA>1200){
                        maxstep=lastClick[0]+1 + page*8;
                        setInfo(maxstep);
                        Serial.println("long "+String(maxstep));
                  }else{
                        tiggerStep[channel] ^= (1 <<(lastClick[0]+page*8)) ;
                        reversePixel(2+channel*2+page,lastClick[0]);
                        setLED();
                        if(lastClick[0]==6){
                        }
                        if(lastClick[0]==7){
                        }
                  }
           }
            longpressA=0;
      }
      for(int i=0;i<4;i++){
            lastClick[i+1]=lastClick[i];
      }
      lastClick[0] = clickTackNo;
      


      if(!mode &&  analogcount>300 && analogcount<400){
            vol= analogRead(A_VOLUME);
            
      }
      if(analogcount>500){
             analogcount=0;
      }

}

int time_data[3]={0,0,0};
void time_count(void) {
      
       if(!mode){//内部クロックモードのときの、つまみ値からテンポ設定
             innerCLock++;
           //  vol = (int)(  (float)((float)vol/(float)1024)*(float)  );//400-1000
             if(innerCLock > ((1050-vol)+20)/12 ){
                  triggerStart();
                  innerCLock=0;
             }
       }
       //------------------Gate を少し長くいれるためのもの
       for(int i=0;i<3;i++){
            if(nowON[i]){
                  time_data[i]++;
                  if(time_data[i]>5){
                        nowON[i]=false;
                        switch(i){
                              case 0:analogWrite( A_PIN_OUT1, 0 ); break;
                              case 1:analogWrite( A_PIN_OUT2, 0 ); break;
                              case 2:analogWrite( A_PIN_OUT3, 0 ); break;
                        }
                        time_data[i]=0;
                        if(nowInfoDisplay==0 && selectMode==0){
                              setLED();
                        }
                  }
            }
       }
}
void triggerStart(){
      //digitalWrite(rclkPin, LOW);                //送信中のRCLKをLowにする
      if(!nowPlay){
            nowPoint=-1;
            nowPlay=true;
            zeroZcount=0;
      }
      leds=0;
      if(nowInfoDisplay==0 && selectMode==0){
            offPixel(nowPoint/8,nowPoint%8);
      }
      nowPoint++;
      if(nowPoint>=maxstep){
            nowPoint=0;
      }
      //Serial.println( nowPoint);
      for(int i=0;i<3;i++){
            if( (tiggerStep[i] & (1<<nowPoint) )>0){    
            //Serial.println( String( "  ---- PLAY ----- ")+ String(test));
            //test++;
              playGate(i);
            }
      }
      /* digitalWrite(rclkPin, LOW);
      shiftOut(dsPin, srclkPin, LSBFIRST, (tiggerStep[channel] | (1<<nowPoint) )); //シフト演算を使って点灯するLEDを選択
      digitalWrite(rclkPin, HIGH);               //送信終了後RCLKをHighにする
      */
     if(nowInfoDisplay==0 && selectMode==0){
            onPixel(nowPoint/8,nowPoint%8);
     }            
}
void setLED(){
      digitalWrite(rclkPin, LOW);
      
      shiftOut(dsPin, srclkPin, LSBFIRST, (tiggerStep[channel]>>page*8) & 0x00FF ); //シフト演算を使って点灯するLEDを選択
      digitalWrite(rclkPin, HIGH);               //送信終了後RCLKをHighにする

}
void selectModeLED(){
      digitalWrite(rclkPin, LOW);
      
      shiftOut(dsPin, srclkPin, LSBFIRST, 0xFF ); //シフト演算を使って点灯するLEDを選択
      digitalWrite(rclkPin, HIGH);               //送信終了後RCLKをHighにする

}

void playGate(int ch){
      int v = (int) D_V_OUT * 51;
      switch(ch){
             case 0:analogWrite( A_PIN_OUT1, v ); break;
             case 1:analogWrite( A_PIN_OUT2, v ); break;
             case 2:analogWrite( A_PIN_OUT3, v ); break;
      }
      
      nowON[ch]=true;
}
void saveData(int id){
      EEPROM.write(0+id*10,tiggerStep[0] & 0x00FF);
      EEPROM.write(1+id*10,(tiggerStep[0]>>8) & 0x00FF);
      EEPROM.write(2+id*10,tiggerStep[1] & 0x00FF);
      EEPROM.write(3+id*10,(tiggerStep[1]>>8) & 0x00FF);
      EEPROM.write(4+id*10,tiggerStep[2] & 0x00FF);
      EEPROM.write(5+id*10,(tiggerStep[2]>>8) & 0x00FF);
      EEPROM.write(6+id*10,(maxstep & 0x00FF)) ;
      Serial.println("SAVE "+String(id));
}
void loadData(int id){
      tiggerStep[0] = EEPROM.read(0+id*10) | (EEPROM.read(1+id*10)<<8)  ;
      tiggerStep[1] = EEPROM.read(2+id*10) | (EEPROM.read(3+id*10)<<8)  ;
      tiggerStep[2] = EEPROM.read(4+id*10) | (EEPROM.read(5+id*10)<<8)  ;
      maxstep= EEPROM.read(6+id*10);
      Serial.println("LOAD "+String(id));

      Serial.println(tiggerStep[0]);

      channel=0;
      page=0;
      setLED();

      eraseAll();

      setDataToVRAM();
}
void resetData(){
      tiggerStep[0] = 0  ;
      tiggerStep[1] = 0 ;
      tiggerStep[2] = 0;
      maxstep=8;
      channel=0;
      page=0;
      setLED();

      eraseAll();

      setDataToVRAM();
}
void setDataToVRAM(){
      
      for(int y=0;y<8;y++){
           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[0]>>y) & B00000001 ) << 5);
           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[0]>>y+8) & B00000001 ) << 4);

           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[1]>>y) & B00000001 ) << 3);
           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[1]>>y+8) & B00000001 ) << 2);

           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[2]>>y) & B00000001 ) << 1);
           VRAM[y]=VRAM[y] &  (0xFF ^ ( (tiggerStep[2]>>y+8) & B00000001 ) );
      }
      
}