//#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
LiquidCrystal_I2C disp(0x27,16,2);
//keypad
const byte ROWS = 4; 
const byte COLS = 4; 
const int stepMotor[4] = {10,11,12,13};//step motor

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {':', '0', ';', 'D'}
};
//*=10, #=11, A=17,B=18,C=19,D=20
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//global variables


byte Arrow[8] ={
0b01000, 0b01100, 0b11010, 0b00001, 0b11010, 0b01100, 0b01000, 0b00000};

bool stringComplete = false;

static const int maxSec = 3000;
int loc = 0, locIn=0, mode = 0;

const unsigned int analogPin[16] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};//analog pin

String strDisplay = "";
String inputString = "";
String nameMode[5] = {"Count          ","Check            ","Data            ","Modify          ","Setting         "};
String changeAble[3] = {"ID      ","MaxCount","Pin     "};


//utlity functions
int check_digit (char c) {
    return (c>='0') && (c<='9');
}
int readKeypad(){
  int ret = (int)(customKeypad.getKey()-'0');
  if(ret!=-48)Serial.println(String(String(ret)+String(loc)));
  return ret;
}
void setTopBar(String s){
  while(s.length() < 16){
    s += " ";
  }
  disp.clear();
  disp.setCursor(0,0);
  disp.print(s);
  disp.setCursor(0,1);
}
String fillStr(String s,const char fi,int sn){
  while(s.length()<sn){s+=fi;}
  return s;
}
void printLn(String s,const int r){
  disp.setCursor(0,r);
  disp.print(fillStr(s,' ' ,16));
}
//takes an integer and output the LED by "0000" per bit, the nth bit is the nth item in array
//Input : int:state, pointer to int:arr, int size of arr
void setPin(int state,int* arr,int s){
  for(int i=0;i<s;i++){
    digitalWrite(*(arr+i),(state>>i)&1);
  }
}
void ccw(int d){
  setPin(1,stepMotor,4);
  delay(d);
  setPin(2,stepMotor,4);
  delay(d);
  setPin(4,stepMotor,4);
  delay(d);
  setPin(8,stepMotor,4);
  delay(d);
}
void cw(int d){
  setPin(8,stepMotor,4);
  delay(d);
  setPin(4,stepMotor,4);
  delay(d);
  setPin(2,stepMotor,4);
  delay(d);
  setPin(1,stepMotor,4);
  delay(d);
}
//describes a scanning string
class ScanStr{
  public:
  ScanStr(int maxc,int subd,int dist){
    maxCount=maxc;
    firstPos=0;
    subDelta=subd;
    maxDist=dist;
  }
  ScanStr(int maxc,int stpos,int subd,int dist){
    maxCount=maxc;
    firstPos=stpos;
    subDelta=subd;
    maxDist=dist;
  }
  String getSettingStr(){
    return String("A"+String(maxCount)+"B"+String(firstPos)+"C"+String(subDelta)+"D"+String(maxDist));
  }
  int getFirstPosition(){
    return firstPos;
  }
  int getPositionDiff(){
    return subDelta;
  }
  int getMaxUltDistance(){
    return maxDist;
  }
  private:
    int maxCount,firstPos,subDelta,maxDist;
    void setA(int n){
      maxCount = n;
    }
    void setB(int n){
      firstPos = n;
    }
    void setC(int n){
      subDelta = n;
    }
    void setD(int n){
      maxDist = n;
    }
    
};
//descirbes a class of product
class Product{
  public:
  Product(int p,long int i,int m){
    this->state=0;
    this->maxcount=m;
    this->ccount=0;
    this->pin=p;
    this->id=i;
  }
  Product(int p,long int i){
    this->state=0;
    this->maxcount=0;
    this->ccount=0;
    this->pin=p;
    this->id=i;
  }
  int getProdPin(){
    return this->pin;
  }
  long int getID(){
    return this->id;
  }
  int getCount(){
    return this->ccount;
  }
  int getMCount(){
    return this->maxcount;
  }
  int getState(){
    return this->state;
  }
  void importStr(ScanStr s){
    int a,b,c,d;
  }
  int importRaw(String s){
    int checkcount=0;//ABCD, 4+=error
    int arr[4]={0,0,0,0};
    //interprets the string'
    for(int i=0;i<s.length();i++){
      //Serial.println(s);
      if(!isDigit(s[i])){
        s.remove(i,1);
        arr[checkcount]=s.toInt();
        //Serial.println(String(String(checkcount) + " : " + arr[checkcount]));
        while(isDigit(s[0])){
          s.remove(0,1);
        }
        checkcount++;
        if(checkcount>=4){
          break;
        }
        i=-1;
      }
    }
    printLn("Press any key...",1);
    if(checkcount<4){
      printLn("Failed...",0);
      //error handle, show on disp
      return 0;
    }else{
      //successful handle, show confirmaiton
      str=ScanStr(arr[0],arr[1],arr[2],arr[3]);
      maxcount = arr[0];
      printLn("Success!",0);
      return 1;
    }
    int rr=0;
    while(rr){
      rr=readKeypad();
    }
    return 1;
  }

  String getDisplayStr(){
    String ret = String(getID());
    while(ret.length()<6){
      ret += " ";
    }
    return String(ret+"|"+this->pin+"|"+this->state+"|"+this->ccount+"|"+this->maxcount+"  ");
  }
  void setMCount(int p){
    maxcount=p;
    if(ccount > maxcount){
      ccount=maxcount;
    }
  }
  ScanStr getScanningStr(){
    return this->str;
  }
  //setters
  void setpin(int p){
    this->pin=p;
  }
  void setID(long int p){
    this->id=p;
  }
  void setMC(int m){
    this->maxcount=m;
  }
  void setState(int s){
    this->state = s;
  }
  void resetCount(){this->ccount=0;}
  void validate(){
    float d=0;
    // generate 10-microsecond pulse to TRIG pin
    digitalWrite(22+(pin*2), HIGH);
    delayMicroseconds(10);
    digitalWrite(22+(pin*2), LOW);
  
    // measure duration of pulse from ECHO pin
    d = 0.17*pulseIn(23+(pin*2), HIGH);
    if((str.getMaxUltDistance())>=d){
      ccount++;
    }
    Serial.println(String(pin) + " : " + String(d) + " " + String(analogRead(analogPin[pin+1])));
  }
  private:
  long int id;//pin - sensor complex to use
  int pin,state,maxcount,ccount;
  //unsigned short maxcount,ccount;
  ScanStr str=ScanStr(0,0,0,0);
};

Product shelves[2] = {Product(0,46102,6),Product(1,73895,5)};

void setup() {
  for(int i=10;i<=13;i++){
    pinMode(i,OUTPUT);
  }
  for(int i=22;i<=40;i+=2){
    pinMode(i,OUTPUT);
    pinMode(i+1,INPUT);
    pinMode(analogPin[(i/2)-10],INPUT);
  }
  Serial.begin(9600);
  inputString.reserve(200);
  // put your setup code here, to run once:
  // liquid crystal : SDA, SCL (no need to declare the pins)
  // Reserved Pins : Uno=A4,A5  Mega=20,21
  disp.init();
  disp.clear();         
  disp.backlight();
  disp.noCursor();
  disp.createChar(0, Arrow);
  // sensor complex : 2xdigital = ult, 1xanalog=infrared
  // testing pins for input
  // max spots = 16001
  shelves[0].importRaw("A4B1C533D135");//predefined input string
  shelves[1].importRaw("A4B1C533D135");
  mode=-1;
}
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
void loop() {
  int tlr=readKeypad();
  if(stringComplete) {
    //fuck it just set the mode
    if(inputString=="count\n"){
      //fill this later
      mode=0;
      Serial.print("ok count\n");
    }else if(inputString=="check\n"){
      mode=1;
    }else if(inputString=="data\n"){
      mode=2;
    }else if(inputString=="edit\n"){
      mode=3;
    }else if(inputString=="set\n"){
      mode=4;
    }else if(inputString=="ask\n"){
      Serial.print("L ");
      for(int i=0;i<2;i++){
        Serial.print(String(shelves[i].getID()) + " ");
      }
    }else if(inputString[0]=='q' && inputString[1]=='u'){//query n locking pattern
      int i = (int)(inputString[6]-'0');
      Serial.print(String("M ") + shelves[i].getProdPin() + " " + shelves[i].getState() + " " + shelves[i].getCount() + " " + shelves[i].getMCount() + " ");
    }
    Serial.print(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  //int tlr=readKeypad();
  //Serial.println(readKeypad());
  switch(mode){
    case 0:{//counting mode
      disp.clear();
      setTopBar("Counting");
      int t=650,d=1,p=0;
      int tries[2]={0,0};
      shelves[0].resetCount();
      shelves[1].resetCount();
      char r;
      for(int i=0;i<=t;i++){
        //counting stuffs, B+nC and D
        for(int j=0;j<2;j++){
          if(((shelves[j].getCount())<(shelves[j].getMCount())) && i==((shelves[j].getScanningStr().getFirstPosition())+((shelves[j].getScanningStr().getPositionDiff())*tries[j]))){
            tries[j]++;
            //do the sensors
            shelves[j].validate();
          }
        }
        //keypad conditions
        r=customKeypad.getKey();
        if(i%256 == 0){disp.print("|");p++;}
        if(r=='A'){
          for(int j=0;j<(i+10);j++){
            ccw(3);
            if(j%256 == 0){disp.print("<");}
          }
          t=0;
          break;  
        }else if(r=='B'){
          setTopBar("Paused");
          disp.print("Press any key");
          delay(150);
          while(readKeypad()==-48){
          }
          i-=15;//reinforce
          setTopBar("Counting");
          disp.print(fillStr("|",'|',p));
        }
        cw(3);
      }
      //clockwise
      for(int i=0;i<=t;i++){
        //no scanning, just keypad conditions
        r=customKeypad.getKey();
        if(i%256 == 0){disp.print("|");p++;}
        if(r=='A'){
          continue;  
        }else if(r=='B'){
          setTopBar("Paused");
          disp.print("Press any key");
          delay(150);
          while(readKeypad()==-48){
          }
          i-=15;//reinforce
          setTopBar("Counting");
          disp.print(fillStr("|",'|',p));
        }
        ccw(3);
      }
      //counterclockwise
      disp.print("|");
      delay(500);
      mode=-1;
      Serial.print("S");
      break;
    }
    case 1:{//checking mode
      int error=0,clean=0;
      disp.clear();
      setTopBar("Checking...");
      for(int i=0;i<2;i++){
        disp.print("|");
        if(shelves[i].getCount() != shelves[i].getMCount()){
          shelves[i].setState(1);
          error++;
        }else{
          shelves[i].setState(2);
          clean++;
        }
        delay(150);//simulate time
      }
      disp.clear();
      printLn("Results:",0);
      printLn(String("E:"+String(error)+" C:"+String(clean)),1);
      Serial.print("R");
      int rr=-48;
      while(rr==-48){
        rr=readKeypad();
      }
      disp.clear();
      mode=-1;
      Serial.print("S");
      break;
    }
    case 4:{//string setting mode
      int cS[2] = {0,0};
      locIn=0;
      disp.setCursor(0,0);
      disp.print(String("1: "+String(shelves[0].getID())+"      "));
      disp.setCursor(0,1);
      disp.print(String("2: "+String(shelves[1].getID())+"      "));
      while(1){//select product
        cS[0]=readKeypad();
        if(cS[0]>0 && cS[0]<3)break;
      }
      cS[0]--;
      disp.clear();
      disp.setCursor(0,0);
      disp.print("1: Get String");
      disp.setCursor(0,1);
      disp.print("2: Set String");
      while(1){//select option
        cS[1]=readKeypad();
        if(cS[1]>0 && cS[1]<3)break;
      }
      cS[1]--;
      disp.clear();
      if(cS[1]==0){//do BOTH!
        disp.setCursor(0,0);
        disp.print("Setting Str:");
        disp.setCursor(0,1);
        disp.print((shelves[cS[0]].getScanningStr()).getSettingStr());
      }else{//import
        String input;char r;
        printLn("Input String",0);
        while(1){
          r=customKeypad.getKey();
          if(r==':'){
            if(input.length()!=0)input.remove(input.length()-1,1);//trim a character off
          }else if(r==';'){//transcribe
            shelves[cS[0]].importRaw(input);
            break;
          }else if(r!=0){
            input+=r;
          }
          printLn(input,1);
        }
      }
      Serial.print("R");
      while(readKeypad()==-48){}
      mode=-1;
      locIn=0;
      disp.clear();
      Serial.print("S");
      break;
    }
    case 2://data display mode
      if(tlr==0){
        Serial.print("S");
        mode=-1;
        disp.clear();
        disp.clear();
      }else if(tlr==2){
        locIn=max(0,locIn-1);
      }else if(tlr==8){
        locIn=min(1,locIn+1);
      }
      //display
      if(locIn==0){
          disp.setCursor(0,0);
          disp.print("  ID  |P|S|C|M    ");
          disp.setCursor(0,1);
          disp.print(shelves[0].getDisplayStr());
      }else{
        for(int i=0;i<2;i++){
          disp.setCursor(0,i);
          disp.print(shelves[i+locIn-1].getDisplayStr());
        }
      }
      break;
    case 3:{
      disp.clear();
      int cS[3]={0,0,0};
      locIn=0;
      disp.setCursor(0,0);
      disp.print(String("1: "+String(shelves[0].getID())));
      disp.setCursor(0,1);
      disp.print(String("2: "+String(shelves[1].getID())));
      while(1){//select product
        cS[0]=readKeypad();
        if(cS[0]>0 && cS[0]<3)break;
      }
      cS[0]--;
      disp.setCursor(0,0);
      disp.print("1: ID   2: MaxCount");
      disp.setCursor(0,1);
      disp.print("3: Sensor");
      while(1){//select field
        cS[1]=readKeypad();
        if(cS[1]>0 && cS[1]<4)break;
      }
      cS[1]--;
      while(!digitalRead(3)){}
      setTopBar("Value:");
      while(1){//change information
        tlr=readKeypad();
        if(tlr==2){
          locIn++;
        }else if(tlr==8){
          locIn--;
        }else if(tlr==0){
          break;
        }
        disp.setCursor(0,0);
        disp.print("Value:");
        disp.setCursor(0,1);
        switch(cS[1]){
          case 0:disp.print(fillStr(String(shelves[cS[0]].getID()+locIn),' ',16));break;
          case 1:disp.print(fillStr(String(shelves[cS[0]].getMCount()+locIn),' ',16));break;
          case 2:locIn=max(-1*shelves[cS[0]].getProdPin(),locIn);disp.print(fillStr(String(shelves[cS[0]].getProdPin()+locIn),' ',16));break;
        }
      }
      switch(cS[1]){
        case 0:shelves[cS[0]].setID(locIn+shelves[cS[0]].getID());break;
        case 1:shelves[cS[0]].setMC(locIn+shelves[cS[0]].getMCount());break;
        case 2:shelves[cS[0]].setpin(locIn+shelves[cS[0]].getProdPin());break;
      }
      //setting information
      Serial.print("S");
      locIn=0;
      mode=-1;
      break;
    }
    default:{
      // put your main code here, to run repeatedly:
      if(tlr==4){
        loc--;
        if(loc<0)loc=4;
        Serial.print("h ");
        //while(readKeypad()==4){}
      }
      if(tlr==6){
        loc++;
        if(loc>4)loc=0;
        Serial.print("hh ");
        //while(readKeypad()==6){}
      }
      if(tlr==0){
        mode=loc;
        disp.clear();
        Serial.print("sel ");
        //while(readKeypad()==0){}
      }
      disp.setCursor(0,0);
      disp.print("= Select  Mode =");
      disp.setCursor(0,1);
      disp.print("=");
      disp.setCursor(1,1);
      disp.write(0);
      disp.setCursor(2,1);
      disp.print(String(nameMode[loc]));
      break;
    }
  }
}
