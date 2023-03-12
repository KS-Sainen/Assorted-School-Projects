import controlP5.*; //import ControlP5 library
import processing.serial.*;
Serial port;

ControlP5 cp5; //create ControlP5 object
PFont font;
PImage img;
Textlabel stateLabel,nerdLabel;
Textlabel[] prodLabel = {null,null,null,null};
ScrollableList productDrop;

int padding=25,xs=15,ys=15;
int numport=0,baud=9600,numprod=0,state=0;
String[] prodDisp = {"0","0","0","0"};
String[] portList = {};
String[] productList = {"None","None"};
String[] defaultProdDisp = {"Pin Complex : ","State : ","Count : ","Max Count : "};
String[] stateName = {"ready","doing","complete"};

void setup(){
  size(500,450);//window size, (width, height)
  background(255,255,255);//sets background color
  cp5 = new ControlP5(this);
  img = loadImage("geopadoru.png"); // Load the original image
  img.resize(70, 75);
  portList = Serial.list();
  printArray(portList);
  port = new Serial(this, portList[0], baud);
  //String[] fontList = PFont.list();
  //printArray(fontList);//get fonts list
  font = createFont("Segoe UI", 18);    // custom fonts for buttons and title
  cp5.setFont(font);
  nerdLabel = cp5.addTextlabel("label2").setText("port:con baud:9600").setPosition(0,439).setColorValue(0).setFont(createFont("Consolas",10));
  stateLabel = cp5.addTextlabel("label").setText("State : "+stateName[state]).setPosition(21,77).setColorValue(0).setFont(createFont("Consolas",15));
  cp5.addTextlabel("labelbig").setText("Shelf Control UI").setPosition(145,15).setColorValue(0).setFont(createFont("Segoe UI", 30));
  cp5.addButton("count")
     .setPosition(25,100)
     .setSize(210,50)
     .setCaptionLabel("count items")//sets the default state when turned on, also follow up with sending close commands to all valves (write a function!) 
     ;
  cp5.addButton("check")
     .setPosition(265,100)
     .setSize(210,50)
     .setCaptionLabel("check items")//sets the default state when turned on, also follow up with sending close commands to all valves (write a function!) 
     ;
   cp5.addButton("data")
     .setPosition(25,165)
     .setSize(210,50)
     .setCaptionLabel("view data")//sets the default state when turned on, also follow up with sending close commands to all valves (write a function!) 
     ;
  cp5.addButton("edit")
     .setPosition(265,165)
     .setSize(210,50)
     .setCaptionLabel("edit data")//sets the default state when turned on, also follow up with sending close commands to all valves (write a function!) 
     ;
  cp5.addButton("set")
     .setPosition(25,230)
     .setSize(210,50)
     .setCaptionLabel("motor setup")//sets the default state when turned on, also follow up with sending close commands to all valves (write a function!) 
     ;
  prodLabel[0] = cp5.addTextlabel("label3").setText("Pin Complex : ").setPosition(21,320).setColorValue(0).setFont(createFont("Consolas",15));
  prodLabel[1] = cp5.addTextlabel("label4").setText("State : ").setPosition(21,335).setColorValue(0).setFont(createFont("Consolas",15));
  prodLabel[2] = cp5.addTextlabel("label5").setText("Count : ").setPosition(21,350).setColorValue(0).setFont(createFont("Consolas",15));
  prodLabel[3] = cp5.addTextlabel("label6").setText("Max Count : ").setPosition(21,365).setColorValue(0).setFont(createFont("Consolas",15));
  productDrop = cp5.addScrollableList("select_prod").setPosition(265, 230).setSize(210, 100).setBarHeight(25).setItemHeight(25).setFont(createFont("Segoe UI", 14)).addItems(productList);
  cp5.addScrollableList("select_port").setPosition(25, 295).setSize(210, 100).setBarHeight(25).setItemHeight(25).setFont(createFont("Segoe UI", 14)).addItems(portList);
}

void draw(){
  nerdLabel.setText(String.format("port:%s baud:%d",portList[numport],baud));
  stateLabel.setText("State : " + stateName[state]);
  //update text - filled
  nerdLabel.draw(this);
  stateLabel.draw(this);
  //productDrop.draw(this);
  background(255,255,255);
  image(img, 430, 372); // Displays the image from point (0,0)
  img.loadPixels();
  //port event
  while (port.available() > 0) {
    String inStr = port.readString();
    print(inStr);
    if(inStr.charAt(0)=='L'){//ask command
      //begin read at position 2
      for(int i=0,r=2;i<2;i++){//hardcoded at 2
        productList[i]="";
        while(inStr.charAt(r)!=' '){
          productList[i]=productList[i]+inStr.charAt(r);
          r++;
        }
        r++;
      }
      productDrop.setItems(productList);
    }else if(inStr.charAt(0)=='M'){//query command
      for(int i=0,r=2;i<4;i++){
        prodDisp[i]="";
        while(inStr.charAt(r)!=' '){
          prodDisp[i]=prodDisp[i]+inStr.charAt(r);
          r++;
        }
        prodLabel[i].setText(defaultProdDisp[i] + prodDisp[i]);
        r++;
      }
    }else if(inStr.charAt(0)=='R'){//complete
      state=2;
    }else if(inStr.charAt(0)=='S'){//return
      state=0;
    }
  }
}
void select_prod(int n){
  numprod=n;
  port.write("query " + nf(n) + "\n");
}
void select_port(int n) {
  numport=n;
  try{

    // If you don't do this, you can't re-connect to the same port again.
    // Re-connecting to the same port gives you a new "myPort ID" and may
    // take a few seconds.
    if(port != null){
      port.clear();
      port.stop();
    }

    // Create a Serial Port connection.
    port = new Serial(this, portList[numport], baud);

    // User message.
    println("Connected to portName: " + portList[numport]);
    println("myPort = " + port);
    delay(1000);
    port.write("ask\n");

  }
  catch(Exception e){

    // User message.
    println("Error connecting to portName: " + portList[numport]);

    // Print detailed error information to the console.
    System.err.println(e);
    e.printStackTrace();
  }
}
public void count(){
  port.write("count\n");
  state=1;
}
public void check(){
  port.write("check\n");
  state=1;
}
public void data(){
  port.write("data\n");
  state=1;
}
public void edit(){
  port.write("edit\n");
  state=1;
}
public void set(){
  port.write("set\n");
  state=1;
}
