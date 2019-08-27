# WifiMeshFS 
For exmperimental purposes only!
Creates a wireless mesh network that will share and mirror text data with nearby nodes. Each node on the network's data is stored as a text file with extention .txt in the node's flashrom using SPIFFS.

All nodes are configured using a console terminal. (Tested and developed using putty, but any VT100 compatible terminal that supports a serial connection will do.) 

## How it works.
Each node is configured with a unique SSID but must share the same wifi password and wifi prefix.
### Example: 
NODE 1: SSID=ESP0001 WIFIPASSWORD=testpassword WIFIPREFIX=ESP
NODE 2: SSID=ESP0002 WIFIPASSWORD=testpassword WIFIPREFIX=ESP
NODE 2: SSID=ESP0003 WIFIPASSWORD=testpassword WIFIPREFIX=ESP

NODE 1 will store data in it's SPIFFS filesystem as a text file. I.E ESP0001.txt. The data contains 2 variables.
Serve count: Incremental value, when ever the data reads a sensor the value will be incremented.
Data: Text data to be transmitted across the mesh network.

Nodes will scan for nearby wireless networks, if the nearby network starts with 'ESP' as indicated by the Wifi prefix example.
It will connect and visit all APs starting with ESP and fetch a register containing a list of data files on the remote node.
Using the following URL 'http://192.168.4.1/getfile?file=list.txt' 'list.txt' contains a list of Node data files. Once the list has been retrieved the node will know which files to fetch next in a loop.
I.E http://192.168.4.1/getfile?file=ESP0001.txt , http://192.168.4.1/getfile?file=ESP0002.txt ect ect.

Each data file that has been retrieved will be disected and the node will use the 'Serve Count' value from the retrieved file to determine if the value is new. If the file does not exist in the local filesystem it will be created, if the serve count from the localfile is the same as the serve count from the retrieved file nothing will be done and if the local file's serve count is older it will overwrite the local file using the data from the retrieved file.

Each node will eventually mirror and share node data found in varius nearby nodes. If NODE 1 is too far away from NODE 3 but NODE 2 is in reach of NODE 1 and NODE 3, NODE 2 will ensure NODE 1 and NODE 3 is updated.

## Flash Rom Compiler Settings
Some of the flash rom will be used to store files that hold nearby node data. Therefore you need to set allocate some space.
From the Arduino IDE Menu --> Flash size: --> Select 4M (1M SPIFFS)

## Dependencies
When installing the ESP8266 libraries all dependencies are available by default except for EEPRM which can be found from here.
https://github.com/mailmartinviljoen/eeprm

## Initial Setup
Connect after flashing the ESP8266 with WifiMeshFS you can run the below commands to set it up.

setssid ESP0001
setappass YourPasswordHere
setwifiprefix ESP
setschedule 1
saveconfig
reboot

The name of the ESP you are flashing will be set to ESP0001, besure to change to I.E ESP0002 after setting up the next node.
Be sure to set the password and prefix the same for all other nodes.

after setup is comple and you rebooted the node, press the flash button on each node to generate some data. Note: with verose mode set to off you will not see anything when presssing the flash button.

## Console commands.
Type help to get a list of commands.
To get started you can type verbose on which will enable extra logging so you can see how the nodes are interacting.

## Custom Sensors.
In void CustomCode() you can provide your own code. By default the sketch comes with an example.
### Important: 
UpdateServCount(); Will increment the serve count so that the receiving nodes will know the data is new.
CreateFile(APPSSID+".txt",DataToWrite,"LOCAL");  Will write the new file.
UpDateFileListentry(); will update the loacal list.txt with a new list of files.
ServCount=Eeprm.readdata(4); is not necessary because its simply fetching the current serve count to be place into the data section of the data file. 

The file that will be written is in the following format, 
000000001This is the data portiosn.
Position 1 to 10 is where the serve count is stored and the rest is where the data from sensors are stored in text format.,




### Example
void CustomCode()
{
 String ServCount;
 int ESPFlashButtonState = digitalRead(0);

  if(ESPFlashButtonState==0)
  {
      String ServCount;
      String DataToWrite;
      String fl_serv_count;
      String Formatfl_serv_count;
      String fl_node_data;
      
      //Update serve count and then update the local file
      UpdateServCount();
      Eeprm Eeprm(0);
      ServCount=Eeprm.readdata(4);
  
      fl_serv_count=ServCount + "                        ";
      Formatfl_serv_count = fl_serv_count.substring(0,10);
      fl_node_data = "DATA VALUE IS:" + fl_serv_count;
      //Combine Serve Count with node data               
      DataToWrite = Formatfl_serv_count+fl_node_data;

      CreateFile(APPSSID+".txt",DataToWrite,"LOCAL");      
      //Serial.println("Data written:" + DataToWrite );
      UpDateFileListentry();
  }
}


## WARNING
When using the custom void, please be mindfull when writing data as each write has wear on the flashrom. Once worn out the device will missbehave or simply fail.















