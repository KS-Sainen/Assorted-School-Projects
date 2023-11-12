//#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(14, 15);//put tx/rx here, hardcoded
LiquidCrystal_I2C disp(0x27,16,2);
//keypad
const byte ROWS = 4;
const byte COLS = 4;
//arranged in order : IN0-4
int sMY[4] = {10,11,12,13};//step motor y axis
int sMX[4] = {29,30,31,32};//step motor x axis
int pX=0,pY=0;//positioning
const int c1=18,c2=19;//channel pins for bluetooth

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


byte Arrow[8] ={0b01000, 0b01100, 0b11010, 0b00001, 0b11010, 0b01100, 0b01000, 0b00000};

bool stringComplete = false;

static const int maxSec = 3000;
int loc = 0, locIn=0, mode = 0;

int white[3]={255,255,255},black[3]={0,0,0};
int colPin[6]={24,25,26,27,28,29},ultPin=22;//colPin:LED,S0,S1(scale),S2,S3(reading),OUT, trig=ult, echo=ult+1
const byte colPattern[3]={0,3,2};
byte colScale=3;//by the S01234 schematics
const int colRange=5;//maximum frequency of each color R,G,B to be mapped to (255,255,255)
const unsigned int analogPin[16] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};//analog pin

String strDisplay = "";
String inputString = "";
String nameMode[5] = {"Count          ","Check            ","Data            ","Modify          ","Setting         "};
String changeAble[3] = {"ID      ","MaxCount","Pin     "};


//utlity functions
//not used anyemore, just use isDigit(c)
int check_digit (char c) {
    return (c>='0') && (c<='9');
}
//reads a value from keypad
int readKeypad(){
  int ret = (int)(customKeypad.getKey()-'0');
  if(ret!=-48)Serial.println(String(String(ret)+String(loc)));
  return ret;
}
//clears a display, leaving only the top row
void setTopBar(String s){
  s=fillStr(s,' ',16);
  disp.clear();
  disp.setCursor(0,0);
  disp.print(s);
  disp.setCursor(0,1);
}
//fills a string up to desired length using a character
String fillStr(String s,const char fi,int sn){
  while(s.length()<sn){s+=fi;}
  return s;
}
//prints an entire line of text
void printLn(String s,const int r){
  disp.setCursor(0,r);
  disp.print(fillStr(s,' ' ,16));
}
//Reads a specially formatted string and writes the INTEGER values to an array
//p = 0th index
//type : 0=Alphabet, 1=|
bool checkSep(int t,char c){
  switch(t){
    case 0:return isAlpha(c);
    case 1:return c=='|';
    default:return false;
  }
}
void readSeparator(String s,int* p,int type,int sz){
  int c=0;//counter
  for(int i=0;i<s.length();i++){
    if(checkSep(type,s[i])){//check if separator is reached
      s.remove(i,1);//delete that
      *(p+c)=s.toInt();//read the number
      while(!isDigit(s[i])){//delete the number
        s.remove(0,1);
      }
      c++;//go to next index
      if(c>=sz)return;//check if maximum size
      i--;//go back 1 character
    }
  }
}
//takes an number and outputs to the pin according to the base 2 digit, the nth bit is the nth item in such number
//Input : int:state, pointer to int:arr, int size of arr
void setPin(int state,int* arr,int s){
  for(int i=0;i<s;i++){
    digitalWrite(*(arr+i),(state>>i)&1);
  }
}
//the same as setPin, but with using INPUT/OUTPUT mode
//0 = OUTPUT, 1=INPUT
void modePin(int* arr,int m,int sz){
  for(int i=0;i<sz;i++){
    int s = ((m>>i)&1)?INPUT:OUTPUT;
    pinMode(*(arr+i),s);
  }
}
void ccw(int t,int d){
  int *p = (t==0)?sMY:sMX;//0 = y-axis, rest = x-axis
  setPin(1,p,4);
  delay(d);
  setPin(2,p,4);
  delay(d);
  setPin(4,p,4);
  delay(d);
  setPin(8,p,4);
  delay(d);
}
void cw(int t,int d){
  int *p = (t==0)?sMY:sMX;//0 = y-axis, rest = x-axis
  setPin(8,p,4);
  delay(d);
  setPin(4,p,4);
  delay(d);
  setPin(2,p,4);
  delay(d);
  setPin(1,p,4);
  delay(d);
}
//describes a scanning string
class newScanStr{
  public:
    newScanStr(int m,int a,int b,int c,int d){
      mode = m;
      this->a = a;
      this->b = b;
      this->c = c;
      this->d = d;
    }
    newScanStr(String s){//must be valid scanstr+
      importStr(s);
    }
    newScanStr(void){
      mode=0,a=mode,b=a,c=b,d=c;
    }
    void importStr(String s){
      int checkcount=0;//ABCD, 4+=error
      this->mode = s[0]-'0';
      s.remove(0,1);//trim out mode
      int arr[4]={0,0,0,0};//default value
      readSeparator(s,arr,0,4);
      this->a=arr[0];
      this->b=arr[1];
      this->c=arr[2];
      this->d=arr[3];
    }
    String getSettingStr(){
      return String("A"+String(a)+"B"+String(b)+"C"+String(c)+"D"+String(d));
    }
    String exportStr(){
      return String(mode) + " " + getSettingStr();
    }
    int a,b,c,d;//represents the 4 slots of information
    int mode;//represents the mode : 0-4
};
float getUltValue(){
    // generate 10-microsecond pulse to TRIG pin
    digitalWrite(ultPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultPin, LOW);
    // measure duration of pulse from ECHO pin
    return 0.17*pulseIn(ultPin+1, HIGH);
}
//descirbes a class of product
class newProduct : public newScanStr{
  public:
    newProduct(long int id,String s):newScanStr(s){
      this->id = id;
    }
    newProduct(long int id):newScanStr(){
      this->id = id;
    }
    void validate(){
      //is triggered only after C1 has been satisfied
      float H;
      switch(mode){
        case 0:
          //normal scan
          if(d>=getUltValue()){
            count++;
          }
          break;
        case 1:
          //also normal scan
          if(b>=getUltValue()){
            count++;
          }
          break;
        case 2:
          //go by height
          H = getUltValue();
          if(H>=c && H<=d){
            count++;
          }
          break;
        case 3:
          H = getUltValue();
          if(H>=b && H<=c){
            count++;
          }
          break;
        case 4:
          //read color sensor+check
          const int colRange = 5;
          int read=0,colCheck[3]={b,c,d};
          for(int i=0;i<3;i++){
            setPin(colPattern[i],colPin+3,2);//sets color to be read
            read = pulseIn(colPin[6],LOW);//add pin
            read = map(read,black[i],white[i],0,255);//map to RBG color space
            if((colCheck[i]+colRange)>=read && (colCheck[i]-colRange)<=read){//check
              continue;
            }else{
              return;
              break;//failsafe
            }
          }
          count++;
          break;
      }
    }
    String getDisplayStr(){
      String ret = fillStr(String(id),' ',6);
      return String(ret+"|"+this->s+"|"+this->count+"|"+this->mc+"  ");
    }
    long int id;
    int count,mc=0,s=0;
};

//Product shelves[2] = {Product(0,46102,6),Product(1,73895,5)};
int prodSize = 2,state=0;
newProduct shelves[4]={newProduct(93152,"0A4B1C533D135"),newProduct(84760,"4A533B0C128D0"),newProduct(42124,"4A533B0C128D0"),newProduct(54574,"4A533B0C128D0")};
//shelves[0]=newProduct(93152,"0A4B1C533D135");
//shelves[1]=newProduct(84760,"4A533B0C128D0");
String dummyScanStr = "0A4B1C533D135";
bool ifPaused(){
  char r;
  r=customKeypad.getKey();
  //if(i%256 == 0){disp.print("|");p++;}
  if(r=='A'){
    return false;
  }else if(r=='B'){
    setTopBar("Paused");
    disp.print("Press any key");
    delay(150);
    while(readKeypad()==-48){}
    setTopBar("Counting");
    //disp.print(fillStr("|",'|',p));
    return true;
  }else{
    return false;
  }
  return false;
}
int selectProduct(){
  int page=0,mp=max(0,prodSize-1)/4;
  //maps button press -> product index
  int ret[4]={0,1,2,3};
  //render menu, rendered as {A/B,C/D} in order {4n/4n+1,4n+2/4n+3} where n is zero-indexed and dentoes the pafe
  while(true){
    char in = readKeypad();
    bool change=false;
    if(in>='A' && in<='D'){
      return ret[page*4 + in-'A'];
    }else if(in=='2'){
      page=max(0,page-1);
      change=true;
    }else if(in=='8'){
      page=min(page+1,prodSize);
      change=true;
    }
    if(change){
      disp.clear();
      if(prodSize>=page*4){//filled page, simple all 4
        printLn(fillStr("A:"+String(shelves[page*4].id),' ',8)+fillStr("B:"+String(shelves[page*4 + 1].id),' ',8),0);
        printLn(fillStr("C:"+String(shelves[page*4 + 2].id),' ',8)+fillStr("D:"+String(shelves[page*4 + 3].id),' ',8),1);
      }else{//partially filled page, must render them individually
        for(int i=0;i<prodSize%4;i++){
          switch(i){
            case 0:disp.setCursor(0,0);break;
            case 1:disp.setCursor(0,8);break;
            case 2:disp.setCursor(8,0);break;
            case 3:disp.setCursor(8,8);break;
          }
          disp.print(fillStr(String('A'+i)+":"+String(shelves[page*4 + i].id),' ',8));
        }
      }
    }
  }
  return 0;
}

void setup() {
  modePin(sMY,0,4);
  modePin(sMX,0,4);
  modePin(colPin,6,32);
  setPin(colScale,colPin+1,2);//sets color scale
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
  // max spots = 160010
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
  inputString="";
  //read the string
  while(bluetooth.available()){
      inputString += (char)bluetooth.read();
  }
  if(inputString.length() >= 0) {
    //add specifications here
    //Receiving Packets : A,B,a,b,c,d,e,f,g,h,i
    //Sending Packet : A,B,C,D
    Serial.println("Packet Received : "+inputString);
    int r[50];//store input value here
    int index;//store index
    int* iptr;//store int pointer
    String strToSend = "";//store string to be printed here
    switch(inputString[0]){
      case 'A'://product info : index|id|mc|mode, implies setting information of a product
        //int r[4]={0,0,0,0};
        r[0]=0,r[1]=r[0],r[2]=r[1],r[3]=r[2];
        inputString.remove(0,1);
        readSeparator(inputString,r,1,4);
        if(r[0]<50 && r[0]<=0 && r[0]>=prodSize && prodSize<50){
          shelves[prodSize]=newProduct((long int)r[1],dummyScanStr);
          shelves[prodSize].mc = r[2];
          shelves[prodSize].mode = r[3];
          prodSize++;
        }
        break;
      case 'B'://scanning str : index|valid string anyhow, implies setting a scanning string
        index = inputString.toInt();
        if(index<prodSize){
          while(isDigit(inputString[0])){//delete the number
            inputString.remove(0,1);
          }
          inputString.remove(0,1);//and the |
          shelves[index].importStr(inputString);
        }
        break;
      case 'a'://counting command
        mode = 0;
        break;
      case 'b'://retrieve info analogus to A : index : sends a data packet detailing the product info to the source for display
        index = inputString.toInt();
        if(index<prodSize){
          bluetooth.print("A|"+String(index)+"|"+String(shelves[index].id)+"|"+String(shelves[index].mc)+"|"+String(shelves[index].mode));
        }
        break;
      case 'c'://counting command
        mode = 1;
        break;
      case 'd'://report state
        bluetooth.print("C|"+String(state));
        break;
      case 'e'://report list of products
        strToSend = "D|"+String(prodSize)+"|";
        for(int i=0;i<prodSize;i++){
          strToSend += String(shelves[i].id);
          if(i<prodSize-1){
            strToSend+="|";
          }
        }
        bluetooth.print(strToSend);
        break;
      case 'f'://set ult+co(1,2,3,4,5,6)l pin
        inputString.remove(0,1);
        readSeparator(inputString,r,1,7);
        ultPin=r[0];
        for(int i=0;i<6;i++)colPin[i]=r[i+1];
        break;
      case 'g'://set motor pin, 0=x and 1=y
        inputString.remove(0,1);
        readSeparator(inputString,r,1,5);
        iptr = (r[0]==0)?sMX:sMY;
        for(int i=0;i<4;i++)*(iptr+i) = *(r+1+i);
        break;
      case 'h'://set black color
        inputString.remove(0,1);
        readSeparator(inputString,r,1,3);
        for(int i=0;i<3;i++)black[i]=r[i];
        break;
      case 'i'://set white color
        inputString.remove(0,1);
        readSeparator(inputString,r,1,3);
        for(int i=0;i<3;i++)white[i]=r[i];
        break;
      case 'j'://report xy position
        bluetooth.print("E|"+String(pX)+"|"+String(pY));
        break;
      case 'k'://report color 0=raw, 1=mapped to 255,255,255
        strToSend="F|";int colRead;
        for(int i=0;i<3;i++){
          setPin(colPattern[i],colPin+3,2);//sets color to be read
          colRead = pulseIn(colPin[6],LOW);//add pin
          if(inputString[2]=='1')colRead = map(colRead,black[i],white[i],0,255);//map to RBG color space
          strToSend+=String(colRead)+"|";
        }
        bluetooth.print(strToSend);
        break;
      case 'l'://report ult.distance in 100um
        bluetooth.print("G|"+String(int(getUltValue()*10)));
        break;
      case 'm'://analogus to B
        inputString.remove(0,2);
        index = inputString.toInt();
        bluetooth.print("B|"+String(index)+"|"+shelves[index].exportStr());
        break;
      case 'n'://export all pin settings
        String strToSend = "H|";
        for(auto i  : sMX)strToSend+=String(i)+"|";
        for(auto i  : sMY)strToSend+=String(i)+"|";
        strToSend+=String(ultPin)+"|";
        for(auto i  : colPin)strToSend+=String(i)+"|";
        bluetooth.print(strToSend);
        break;
      case 'o'://export b+w value
        inputString.remove(0,2);
        strToSend ="F|";
        iptr = (inputString[0]=='1')?white:black;
        for(int i=0;i<3;i++)strToSend+=String(iptr[i])+"|";
        Serial.print(strToSend);
        break;
      case 'p'://freely spins a motor, 0=ccw and 1=cw
        inputString.remove(0,1);
        readSeparator(inputString,r,1,2);
        if(r[1]==0){
          ccw(r[0],3);
        }else{
          cw(r[0],3);
        }
        break;
      default:
        Serial.println("unknown data packet");
        break;
    }
    inputString = "";
    stringComplete = false;
  }
  //int tlr=readKeypad();
  //Serial.println(readKeypad());
  switch(mode){
    case 0:{//counting mode
      disp.clear();
      setTopBar("Counting");
      int t=1600,d=1,p=0,delta=200,tries=0;//delta is the length between lines
      for(int i=0;i<prodSize;i++){
        shelves[i].count = 0;//reset count
        //now scan
        for(int j=0;j<=t;j++){
          //check c1->c2
          switch(shelves[i].mode){
            case 0:
            if(shelves[i].c%j==shelves[i].b)shelves[i].validate();
              break;
            case 2:
            if(shelves[i].c%j==shelves[i].a)shelves[i].validate();
              break;
            case 4:
              if(shelves[i].a%j==0)shelves[i].validate();
              break;
            default:
              shelves[i].validate();
          }
          if(ifPaused())j-=15;
          ccw(0,3);//move
        }
        //go back
        for(int j=0;j<=t;j++){
          if(ifPaused())j-=15;
          cw(0,3);
        }
        //go left
        for(int j=0;j<=delta;j++){
          if(ifPaused())j-=15;
          cw(1,3);
        }
      }
      //then just go back
      for(int i=0;i<=(delta*(prodSize-1));i++){
        ccw(1,3);
      }
      //finish
      disp.clear();
      mode=-1;
      break;
    }
    case 1:{//checking mode
      int error=0,clean=0;
      disp.clear();
      setTopBar("Checking...");
      for(int i=0;i<prodSize;i++){
        disp.print("||");
        if(shelves[i].count != shelves[i].mc){
          error++;
        }else{
          clean++;
        }
        delay(150);//simulate time
      }
      disp.clear();
      printLn("Results:",0);
      printLn(String("E:"+String(error)+" C:"+String(clean)),1);
      int rr=-48;
      while(rr==-48){
        rr=readKeypad();
      }
      disp.clear();
      mode=-1;
      break;
    }
    case 4:{//string setting mode
      int cS[2] = {0,0};
      locIn=0;
      cS[0]=selectProduct();
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
        disp.print(shelves[cS[0]].exportStr());
      }else{//import
        String input;char r;
        printLn("Input String",0);
        while(1){
          r=customKeypad.getKey();
          if(r==':'){
            if(input.length()!=0)input.remove(input.length()-1,1);//trim a character off
          }else if(r==';'){//transcribe
            shelves[cS[0]].importStr(input);
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
    case 2:{//data display mode
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
      //display - 2 lines
      if(locIn==0){
          disp.setCursor(0,0);
          disp.print("  ID  |S|C|M    ");
          disp.setCursor(0,1);
          disp.print(shelves[0].getDisplayStr());
      }else{
        for(int i=0;i<2;i++){
          disp.setCursor(0,i);
          disp.print(shelves[i+locIn-1].getDisplayStr());
        }
      }
      break;
    }
    case 3:{//set info
      disp.clear();
      int cS[3]={0,0,0};
      locIn=0;
      cS[0]=selectProduct();
      disp.setCursor(0,0);
      disp.print("1: ID   2: MaxCount");
      while(1){//select field
        cS[1]=readKeypad();
        if(cS[1]>0 && cS[1]<3)break;
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
          case 0:disp.print(fillStr(String(shelves[cS[0]].id+locIn),' ',16));break;
          case 1:disp.print(fillStr(String(shelves[cS[0]].mc+locIn),' ',16));break;
          //case 2:locIn=max(-1*shelves[cS[0]].getProdPin(),locIn);disp.print(fillStr(String(shelves[cS[0]].getProdPin()+locIn),' ',16));break;
        }
      }
      switch(cS[1]){
        case 0:shelves[cS[0]].id=(locIn+shelves[cS[0]].id);break;
        case 1:shelves[cS[0]].mc=(locIn+shelves[cS[0]].mc);break;
        //case 2:shelves[cS[0]].setpin(locIn+shelves[cS[0]].getProdPin());break;
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
      }
      if(tlr==6){
        loc++;
        if(loc>4)loc=0;
      }
      if(tlr==0){
        mode=loc;
        disp.clear();
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
