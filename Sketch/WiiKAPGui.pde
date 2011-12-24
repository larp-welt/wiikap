import processing.serial.*;
import controlP5.*;
import processing.opengl.*;

Serial g_serial;
ControlP5 controlP5;
Textlabel txtlblFirmware, version; 
ListBox commListbox;
Slider servoSliderH1,servoSliderH2,servoSliderH3;
Button buttonSTART,buttonSTOP,buttonREAD,buttonWRITE;
Slider RCChannels[] = new Slider[8];

// size of data for M
final int frame_size = 43;

cGraph g_graph;
int windowsX    = 800; int windowsY    = 540;
int xGraph      = 10;  int yGraph      = 260;

color yellow_ = color(200, 200, 20), green_ = color(30, 120, 30), red_ = color(120, 30, 30), black = color(0, 0, 0);

PFont font8,font12,font15;

Numberbox confP, confI, confD;
float gx,gy,gz,ax,ay,az;
int init_com, graph_on;
float time1,time2;
boolean graphEnable = false, readEnable = false, writeEnable = false;
boolean axGraph=false, ayGraph=true, azGraph=true, gxGraph=false, gyGraph=true, gzGraph=true;

cDataArray accTILT   = new cDataArray(100), accROLL    = new cDataArray(100), accPAN     = new cDataArray(100);
cDataArray gyroTILT  = new cDataArray(100), gyroROLL   = new cDataArray(100), gyroPAN    = new cDataArray(100);

int byteP, byteI, byteD;
int[] servo = { 1500, 1500, 1500, 1500 }, angle = new int[3], rcdata = new int[8];
int fwversion = 0;
String fwstring = "";

String shortifyPortName(String portName, int maxlen)  {
    String shortName = portName;
    if(shortName.startsWith("/dev/"))
      shortName = shortName.substring(5);  
  
    if(shortName.startsWith("tty.")) // get rid off leading tty. part of device name
      shortName = shortName.substring(4); 
  
    if(portName.length()>maxlen) {
      shortName = shortName.substring(0,(maxlen-1)/2) + "~" +shortName.substring(shortName.length()-(maxlen-(maxlen-1)/2));
    }
    if(shortName.startsWith("cu.")) // only collect the corresponding tty. devices
      shortName = "";
    return shortName;
}

controlP5.Controller hideLabel(controlP5.Controller c) {
    c.setLabel("");
    c.setLabelVisible(false);
    return c;
}


void setup(){ 
	size(windowsX,windowsY,OPENGL);
        frameRate(20);
	
	font8 = createFont("Arial bold",8,false);
	font12 = createFont("Arial bold",12,false);
	font15 = createFont("Arial bold",15,false);
	
	controlP5 = new ControlP5(this);
	controlP5.setControlFont(font12);
	
	g_graph = new cGraph(xGraph+110,yGraph, windowsX-(2*xGraph+110), windowsY-(yGraph+10));

	commListbox = controlP5.addListBox("portComList",10,65,120,240);
	
	commListbox.captionLabel().set("PORT COM");
        commListbox.setBarHeight(15);
        commListbox.setItemHeight(15);
	commListbox.setColorBackground(red_);
        commListbox.actAsPulldownMenu(true);
	for(int i=0;i<Serial.list().length;i++) {
	  String pn = shortifyPortName(Serial.list()[i], 13);
	  if (pn.length() >0 ) commListbox.addItem(pn,i);
	}
	
        txtlblFirmware = controlP5.addTextlabel("txtlblFirmaware", "Firmware: -.-", 180, 155);

        buttonSTART = controlP5.addButton("bSTART",1,xGraph,75,55,20); buttonSTART.setLabel("START"); buttonSTART.setColorBackground(red_);
        buttonSTOP = controlP5.addButton("bSTOP",1,xGraph+65,75,55,20); buttonSTOP.setLabel("STOP"); buttonSTOP.setColorBackground(red_);
        buttonREAD = controlP5.addButton("READ",1,xGraph,100,55,20);buttonREAD.setColorBackground(red_);
        buttonWRITE = controlP5.addButton("WRITE",1,xGraph+65,100,55,20);buttonWRITE.setColorBackground(red_);

        controlP5.addTextlabel("acclabel","ACC",10,yGraph+2); 
        
        Toggle tACC_ROLL = controlP5.addToggle("ACC_ROLL",axGraph,85,yGraph+15,20,10); 
        tACC_ROLL.setColorActive(color(255, 0, 0));
        tACC_ROLL.setColorBackground(black);
        tACC_ROLL.setLabel(""); 
        controlP5.addTextlabel("accrolllabel","ROLL",25,yGraph+17);
        
        Toggle tACC_TILT = controlP5.addToggle("ACC_TILT",ayGraph,85,yGraph+30,20,10); 
        tACC_TILT.setColorActive(color(0, 255, 0));
        tACC_TILT.setColorBackground(black);
        tACC_TILT.setLabel(""); 
        controlP5.addTextlabel("acctiltlabel","TILT",25,yGraph+32);
        
        Toggle tACC_PAN = controlP5.addToggle("ACC_PAN",azGraph,85,yGraph+45,20,10); 
        tACC_PAN.setColorActive(color(0, 0, 255));
        tACC_PAN.setColorBackground(black);
        tACC_PAN.setLabel(""); 
        controlP5.addTextlabel("accpanlabel","PAN",25,yGraph+47);
       
        controlP5.addTextlabel("gyrolabel","GYRO",10,yGraph+67);
     
        Toggle tGYRO_ROLL = controlP5.addToggle("GYRO_ROLL",gxGraph,85,yGraph+80,20,10); 
        tGYRO_ROLL.setColorActive(color(200, 200, 0));
        tGYRO_ROLL.setColorBackground(black);
        tGYRO_ROLL.setLabel(""); 
        controlP5.addTextlabel("gyrorolllabel","ROLL",25,yGraph+82);

        Toggle tGYRO_TILT = controlP5.addToggle("GYRO_TILT",gyGraph,85,yGraph+95,20,10); 
        tGYRO_TILT.setColorActive(color(0, 255, 255));
        tGYRO_TILT.setColorBackground(black);
        tGYRO_TILT.setLabel(""); 
        controlP5.addTextlabel("gyrotiltlabel","TILT",25,yGraph+97);
        
        Toggle tGYRO_PAN = controlP5.addToggle("GYRO_PAN",gzGraph,85,yGraph+110,20,10); 
        tGYRO_PAN.setColorActive(color(255, 0, 255));
        tGYRO_PAN.setColorBackground(black);
        tGYRO_PAN.setLabel(""); 
        controlP5.addTextlabel("gyropanlabel","PAN",25,yGraph+112);
        
        servoSliderH1 = controlP5.addSlider("Servo0",1000,2000,1500,0,0,120,10);
        servoSliderH1.setDecimalPrecision(0);
        
        servoSliderH2 = controlP5.addSlider("Servo1",1000,2000,1500,0,0,120,10);
        servoSliderH2.setDecimalPrecision(0);
        
        servoSliderH3 = controlP5.addSlider("Servo2",1000,2000,1500,0,0,120,10);
        servoSliderH3.setDecimalPrecision(0);

        for (int i = 0; i < 8; i++)
        {
           RCChannels[i] = controlP5.addSlider("channel"+i,1000,2000,1500,0,0,120,10);
           RCChannels[i].setDecimalPrecision(0); 
        }
        
        // Pan
        controlP5.addTextlabel("panlabel","Pan",200,80);
        confP = (controlP5.Numberbox) hideLabel(controlP5.addNumberbox("confP",0,250,75,40,16));confP.setDecimalPrecision(1);confP.setMultiplier(0.1);confP.setMax(20);
        confI = (controlP5.Numberbox) hideLabel(controlP5.addNumberbox("confI",0,300,75,40,16));confI.setDecimalPrecision(3);confI.setMultiplier(0.001);confI.setMax(0.250);
        confD = (controlP5.Numberbox) hideLabel(controlP5.addNumberbox("confD",0,350,75,40,16));confD.setDecimalPrecision(0);confD.setMultiplier(1);confD.setMax(50);
        
        confP.setColorBackground(red_);confP.setMin(0);confP.setDirection(Controller.HORIZONTAL);
        confI.setColorBackground(red_);confI.setMin(0);confI.setDirection(Controller.HORIZONTAL);
        confD.setColorBackground(red_);confD.setMin(0);confD.setDirection(Controller.HORIZONTAL);
        
        time1 = 0;
        time2 = 0;
        	
} 
 
void draw(){ 
	background(80);
	textFont(font15);
	text("WiiKAP V0.1",10,26);

        strokeWeight(3);
        fill(0);
        stroke(0);
        rectMode(CORNERS);
        rect(180, 55, 410, 150);
        rect(420, 55, 600, 150);
        rect(610, 55, windowsX-10, 250);

        fill(255);
        textFont(font15);    
        text("P", 265, 70);
        text("I", 315, 70);
        text("D", 365, 70);
        
        time1=millis();
        if (init_com==1) {
            if ((time1-time2)>50 && graph_on==1) 
            {
                g_serial.write('M');
                time2=time1;
            }
        }        

        servoSliderH1.setValue(servo[0]);
        servoSliderH2.setValue(servo[1]);
        servoSliderH3.setValue(servo[2]);
        
        servoSliderH1.setPosition(425,70);
        servoSliderH1.setCaptionLabel("PAN");
        servoSliderH1.show();

        servoSliderH2.setPosition(425,90);
        servoSliderH2.setCaptionLabel("TILT");
        servoSliderH2.show();

        servoSliderH3.setPosition(425,110);
        servoSliderH3.setCaptionLabel("ROLL");
        servoSliderH3.show();
        
        for (int i = 0; i < 8; i++)
        {
           RCChannels[i].setValue(rcdata[i]);
           RCChannels[i].setPosition(615, 70+i*20);
           RCChannels[i].setCaptionLabel("Ch"+i);
           RCChannels[i].show(); 
        }

        strokeWeight(1);
        fill(255, 255, 255);
        g_graph.drawGraphBox();

        strokeWeight(1.5);
        if (axGraph) { stroke(255, 0, 0); g_graph.drawLine(accROLL, -1000, +1000); }
        if (ayGraph) { stroke(0, 255, 0); g_graph.drawLine(accTILT, -1000, +1000); }
        if (azGraph) { stroke(0, 0, 255); g_graph.drawLine(accPAN, -1000, +1500); }
        if (gxGraph) { stroke(200, 200, 0);  g_graph.drawLine(gyroROLL, -1500, +1500); }
        if (gyGraph) { stroke(0, 255, 255);  g_graph.drawLine(gyroTILT, -1500, +1500); }
        if (gzGraph) { stroke(255, 0, 255);  g_graph.drawLine(gyroPAN, -1500, +1500); }

} 


public void bSTART() {
  if(graphEnable == false) {return;}
  graph_on=1;
  readEnable = true;
  buttonREAD.setColorBackground(green_);
  g_serial.clear();
}

public void bSTOP() {
  graph_on=0;
}

public void READ() {
  if(readEnable == false) {return;}
  
  confP.setValue(byteP/10.0);
  confI.setValue(byteI/1000.0);
  confD.setValue(byteD);
 
  buttonWRITE.setColorBackground(green_);

  confP.setColorBackground(green_);
  confI.setColorBackground(green_);
  confD.setColorBackground(green_);
  
  txtlblFirmware.setValue("Firmware: " + fwstring);
  
  writeEnable = true;
}

public void WRITE() {
  if(writeEnable == false) {return;}

  byteP = (round(confP.value()*10));
  byteI = (round(confI.value()*1000));
  byteD = (round(confD.value()));

  int[] s = new int[4];
  int p = 0;
  s[p++] = 'W'; // write to Eeprom @ arduino
  s[p++] = byteP;  
  s[p++] = byteI;
  s[p++] = byteD;
  for(int i = 0; i<4; i++) g_serial.write(char(s[i]));
}

void ACC_ROLL(boolean theFlag) {axGraph = theFlag;}
void ACC_TILT(boolean theFlag) {ayGraph = theFlag;}
void ACC_PAN(boolean theFlag) {azGraph = theFlag;}
void GYRO_ROLL(boolean theFlag) {gxGraph = theFlag;}
void GYRO_TILT(boolean theFlag) {gyGraph = theFlag;}
void GYRO_PAN(boolean theFlag) {gzGraph = theFlag;}

public void controlEvent(ControlEvent theEvent) {
  if (theEvent.isGroup())
  {
    if (theEvent.name()=="portComList") 
    {
      InitSerial(theEvent.group().value()); // initialize the serial port selected
    }
  }
}

// initialize the serial port selected in the listBox
void InitSerial(float portValue) {
  String portPos = Serial.list()[int(portValue)];
  
  g_serial = new Serial(this, portPos, 115200);
  
  init_com=1;
  buttonSTART.setColorBackground(green_);
  buttonSTOP.setColorBackground(green_);
  commListbox.setColorBackground(green_);
  
  graphEnable = true;
  g_serial.buffer(frame_size+1);
}

int p;
byte[] inBuf = new byte[frame_size];

int read16() {return (inBuf[p++]&0xff) + (inBuf[p++]<<8);}
int read8()  {return inBuf[p++]&0xff;}

int command;

void serialEvent(Serial p) { 
  processSerialData(); 
}

void processSerialData() {

  command = g_serial.read();

  if (command == 'M') {
    g_serial.readBytes(inBuf);
    p = 0;
    if (inBuf[frame_size-1] == 'M') {  // MultiKAP @ arduino send all data to GUI
      fwversion = read8();
      fwstring = "WiiKAP " + (fwversion>>4) + "." + (fwversion&0x0f);

      ax = read16(); ay = read16(); az = read16();
      gx = read16(); gy = read16(); gz = read16();

      accROLL.addVal(ax); accTILT.addVal(ay); accPAN.addVal(az);
      gyroROLL.addVal(gx); gyroTILT.addVal(gy); gyroPAN.addVal(gz);

      for (int i=0; i < 3; i++) { servo[i] = read16(); }
      for (int i=0; i < 8; i++) { rcdata[i] = read16(); }
      for (int i=0; i < 2; i++) { angle[i] = read16(); }
      
      byteP = read8();
      byteI = read8();
      byteD = read8();
 
    }
  }
}

class cDataArray {
  float[] m_data;
  int m_maxSize;
  int m_startIndex = 0;
  int m_endIndex = 0;
  int m_curSize;
  
  cDataArray(int maxSize){
    m_maxSize = maxSize;
    m_data = new float[maxSize];
  }
  void addVal(float val) {
    m_data[m_endIndex] = val;
    m_endIndex = (m_endIndex+1)%m_maxSize;
    if (m_curSize == m_maxSize) {
      m_startIndex = (m_startIndex+1)%m_maxSize;
    } else {
      m_curSize++;
    }
  }
  float getVal(int index) {return m_data[(m_startIndex+index)%m_maxSize];}
  int getCurSize(){return m_curSize;}
  int getMaxSize() {return m_maxSize;}
  float getMaxVal() {
    float res = 0.0;
    for(int i=0; i<m_curSize-1; i++) 
      if ((m_data[i] > res) || (i==0)) res = m_data[i];
    return res;
  }
  float getMinVal() {
    float res = 0.0;
    for(int i=0; i<m_curSize-1; i++) 
      if ((m_data[i] < res) || (i==0)) res = m_data[i];
    return res;
  }
  float getRange() {return getMaxVal() - getMinVal();}
}

// This class takes the data and helps graph it
class cGraph {
  float m_gWidth, m_gHeight;
  float m_gLeft, m_gBottom, m_gRight, m_gTop;
  
  cGraph(float x, float y, float w, float h) {
    m_gWidth     = w;
    m_gHeight    = h;
    m_gLeft      = x;
    m_gBottom    = y;
    m_gRight     = x + w;
    m_gTop       = y + h;
  }
  
  void drawGraphBox() {
    stroke(0, 0, 0);
    rectMode(CORNERS);
    rect(m_gLeft, m_gBottom, m_gRight, m_gTop);
  }
  
  void drawLine(cDataArray data, float minRange, float maxRange) {
    float graphMultX = m_gWidth/data.getMaxSize();
    float graphMultY = m_gHeight/(maxRange-minRange);
    
    for(int i=0; i<data.getCurSize()-1; ++i) {
      float x0 = i*graphMultX+m_gLeft;
      float y0 = m_gTop-(((data.getVal(i)-(maxRange+minRange)/2)*5 + (maxRange-minRange)/2)*graphMultY); // scaleSlider.value()
      float x1 = (i+1)*graphMultX+m_gLeft;
      float y1 = m_gTop-(((data.getVal(i+1)-(maxRange+minRange)/2 )*5 + (maxRange-minRange)/2)*graphMultY);
      line(x0, y0, x1, y1);
    }
  }
}
