#include  <EEPROM.h>
#include <eeprm.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);

//Custom declares for reading sensors


//Time Declares
float Minutes;
int SchedulerOff=0;

//Read Serial Declares
int incomingByte;
char incomingChar;
int CommandValid;
String cmdBuild;
String StringValue;
String VerboseMode="off";
int Online=1;



String SharedKey="123456";

//Wifi Setup.
    String APPSSID;
    String APPASS;
    String WIFIPrefix;    


void setup() {
  
  pinMode(2, OUTPUT);//ESP9266 LED
  digitalWrite(2, HIGH);
  String WifiSetupStatus="SUCCESS";
  Serial.begin(250000);
  clearAndHome();
  Serial.println("\033[1;33mWifiMeshFS\033[0m Starting up...");
  Serial.println("Starting serial port baud 250000                               [ \033[1;32mOK\033[0m ]");delay(1);
  

  Serial.println("Starting EEPROM memory                                         [ \033[1;32mOK\033[0m ]");delay(1);
  EEPROM.begin(512);
  Serial.println("Loading configuration from EEPROM                              [ \033[1;32mOK\033[0m ]");delay(1);
  LoadConfig();
  //Setup Accesss Point
  WiFi.softAP(APPSSID.c_str(), APPASS.c_str());  
  Serial.println("Starting WIFI AP mode                                          [ \033[1;32mOK\033[0m ]");delay(1);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("SSID: " + APPSSID);
  Serial.print(" AP IP address: ");
  Serial.println(myIP);
  
  /*
  //BOF WIFI SETUP
      WiFi.mode(WIFI_STA);
      WiFi.begin(APPSSID.c_str(), APPASS.c_str());  
      int ConAttempt=0;
      Serial.println("Connecting to Wireless network AP:" + APPSSID + " PASS:" + APPASS );
      while (WiFi.status() != WL_CONNECTED) {
        ConAttempt++;
        delay(500);    
        if (ConAttempt>=10)
              {
                  WifiSetupStatus="FAIL";
                  Serial.println("\nWireless connection failed. Returning to console.");
                  Serial.println("WIFI setup complete                                          [ \033[1;31mFAIL\033[0m ]");delay(1); 
                  break;
              }else{WifiSetupStatus="SUCCESS";}
        
      Serial.print(".");
      }
      if(WifiSetupStatus=="SUCCESS"){Serial.println("");Serial.println("WIFI setup complete                                            [ \033[1;32mOK\033[0m ]");delay(1);}
      Serial.println("");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP()); 
  //EOF WIFI SETUP
  */
  
  //Setup SPIFFS
  SPIFFS.begin();
  Serial.println("Starting SPIFFS filesystem                                     [ \033[1;32mOK\033[0m ]");delay(1);
  server.begin();
  CreateWebServerRoot();
  Serial.println("Starting HTTP Web Server and setting up HTTP root path         [ \033[1;32mOK\033[0m ]");delay(1);
  //SPIFFS.format();

  //Custom Setup
  pinMode(0, INPUT);
  Serial.println("Setting up custom functions                                    [ \033[1;32mOK\033[0m ]");delay(1);

  //Start Console
  //dosworking();
  //clearAndHome();
  Printwelcome();
}

void loop() {
 const unsigned long fiveMinutes = Minutes * 60 * 1000UL;
 static unsigned long lastSampleTime = 0 - Minutes;  // initialize such that a reading is due the first time through loop()

 unsigned long now = millis();
 if (now - lastSampleTime >= fiveMinutes)
 {
    lastSampleTime += fiveMinutes;
    // add code to take temperature reading here
    if(SchedulerOff==0)
      {
        if(Online==1){FetchNearby();}
      }else{}    
 }

  
if(Online==1){server.handleClient();}
readincommingserial();
//Run Custom Code
CustomCode();
}
void readincommingserial()
{
   
if (Serial.available() > 0) 
  {
   incomingByte = Serial.read();
   incomingChar = char(incomingByte);
      if (incomingByte != 13)
                {
                 if(incomingByte != 127){cmdBuild = cmdBuild + incomingChar;}else{cmdBuild= cmdBuild.substring(0,cmdBuild.length()-1);}//Remove Backspaces and the last mistake chracter
                }
                else
                {
                  Serial.println("");
                  //Parameterless
                  if (cmdBuild=="help"){Printhelp();CommandValid=1;}
                  if (cmdBuild=="welcome"){Printwelcome();CommandValid=1;}
                  if (cmdBuild=="clear"){clearAndHome();CommandValid=1;}
                  if (cmdBuild=="reboot"){Reboot();CommandValid=1;}
                  if (cmdBuild=="formatfs"){FormatFS();CommandValid=1;}
                  if (cmdBuild=="ls"){ListFiles();CommandValid=1;}
                  if (cmdBuild=="show verbose"){ShowVerbose();CommandValid=1;}
                  if (cmdBuild=="scanwifi"){ScanWifi();CommandValid=1;}
                  if (cmdBuild=="unlinkwifi"){DisconWifi();CommandValid=1;}
                  if (cmdBuild=="saveconfig"){SaveConfig();CommandValid=1;}
                  if (cmdBuild=="dumpconfig"){DumpConfig();CommandValid=1;}
                  if (cmdBuild=="reloadconfig"){LoadConfig();CommandValid=1;}
                  if (cmdBuild=="updateservcount"){UpdateServCount();CommandValid=1;Serial.println("Serve count updated");}
                  if (cmdBuild=="printservcount"){PrintServCount();CommandValid=1;}                           
                  if (cmdBuild=="updatelist"){UpDateFileListentry();CommandValid=1;}  
                  if (cmdBuild=="1"){Online=1;Serial.println("Entered [\033[1;32mOnline\033[0m] mode. System fully accesible by remote hosts.");CommandValid=1;}
                  if (cmdBuild=="0"){Online=0;Serial.println("Entered [\033[1;31mOffline\033[0m] mode.System not accesible by remote hosts.");CommandValid=1;}
                  if (cmdBuild=="schedule off"){Serial.println("Scheduler turned [\033[1;31moff\033[0m] Scheduler will not run.");SchedulerOff=1;CommandValid=1;} 
                  if (cmdBuild=="schedule on"){Serial.println("Scheduler turned [\033[1;32mon\033[0m] Scheduler will run.");SchedulerOff=0;CommandValid=1;}                  
                  //With Parameters
                  if (cmdBuild.substring(0,8)=="verbose "){SetVerboseMode();CommandValid=1;} 
                  if (cmdBuild.substring(0,5)=="more "){ShowFileContent();CommandValid=1;} 
                  if (cmdBuild.substring(0,7)=="cltest "){ClTest();CommandValid=1;} 
                  if (cmdBuild.substring(0,8)=="setssid "){setAPSSID();CommandValid=1;}
                  if (cmdBuild.substring(0,10)=="setappass "){SetAPPass();CommandValid=1;}
                  if (cmdBuild.substring(0,14)=="setwifiprefix "){SetWifiPrefix();CommandValid=1;}
                  if (cmdBuild.substring(0,10)=="fetchtest "){FetchTest();CommandValid=1;}
                  if (cmdBuild.substring(0,11)=="fetchnearby"){FetchNearby();CommandValid=1;}
                  if (cmdBuild.substring(0,7)=="delete "){DeleteFile();CommandValid=1;}
                  if (cmdBuild.substring(0,13)=="setservcount "){setServCount();CommandValid=1;}
                  if (cmdBuild.substring(0,12)=="setschedule "){SetSchedule();CommandValid=1;}
                                           
                  //Check for invalid command.;
                  if(CommandValid==0){Serial.println("'"  + cmdBuild + "' is not recognized as an internal or external command.");}
                  //Clear Last Command
                  cmdBuild="";
                  CommandValid=0;
                }
     //Repeat the key strokes
     Serial.print(incomingChar);    
     //Serial.print(incomingByte);//Uncomment this is for debugging purposes.
  }
}
void LoadConfig()
{
 String TriggerCounter;
 Eeprm Eeprm(0);
  
 APPSSID=Eeprm.readdata(1);
 APPASS=Eeprm.readdata(2); 
 WIFIPrefix=Eeprm.readdata(3); 
 TriggerCounter=Eeprm.readdata(5);
 APPSSID.trim();
 APPASS.trim();
 WIFIPrefix.trim();
 TriggerCounter.trim();

 //set APPSSID to SSID
 APPSSID=APPSSID;
 APPASS=APPASS;
 WIFIPrefix=WIFIPrefix;
 Minutes=TriggerCounter.toFloat();
 //Check if trigger is set correctly else overide it with a default value
    if(Minutes>=0.1 && Minutes<=999999){}else
    {
      Minutes=1; 
      Serial.println("\033[1;31mSchedule invalid\033[0m setting default to 1 minutes.");
      Eeprm.writedata(5,String(Minutes));
      EEPROM.commit();
      
    }
  if(VerboseMode=="on")
  {
    Serial.println("\033[1;32mConfiguration loaded...\033[0m");
  }
 
}
void setAPSSID()
{
  StringValue = cmdBuild.substring(8,1000);
  StringValue.trim();  
  Eeprm Eeprm(0);
  Eeprm.writedata(1,StringValue); 
  Serial.println("AP SSID set to: " + StringValue);
  
}
void SetAPPass()
{
  StringValue = cmdBuild.substring(10,1000);
  StringValue.trim();  
  Eeprm Eeprm(0);
  Eeprm.writedata(2,StringValue);
  Serial.println("AP password set to: " + StringValue); 
}
void SetWifiPrefix()
{
 StringValue = cmdBuild.substring(14,1000); 
 StringValue.trim(); 
 Eeprm Eeprm(0);
 Eeprm.writedata(3,StringValue);
 Serial.println("Wifi prefix set to: " + StringValue);
}
void setServCount()
{
  StringValue = cmdBuild.substring(13,1000);
  StringValue.trim(); 
  Eeprm Eeprm(0);
  Eeprm.writedata(4,StringValue);  
  Serial.println("Serve Count set to:" + StringValue );
}
void SetSchedule()
{
  StringValue = cmdBuild.substring(12,1000);
  StringValue.trim(); 
  Eeprm Eeprm(0);
  
  //Check if trigger is valid only allow values between 0.1 and 999999)
  if(StringValue.toFloat()>=0.1 && StringValue.toFloat()<=999999 )
  {
    Minutes = StringValue.toFloat();
    Eeprm.writedata(5,StringValue);
    Serial.print("Schedule set to:"); Serial.print(Minutes); Serial.println(" minute(s).");
  }else
  {
    Eeprm.writedata(5,StringValue);
    Serial.println("\033[1;31mInvalid schedule\033[0m defaulting to 1 minute. Schedule value must be between 0.1 and 999999 minutes.");
    Minutes = StringValue.toInt();
  }
  
  
}

void UpdateServCount()
{
  String ServCount;
  int NewServCount;
  Eeprm Eeprm(0);
  ServCount=Eeprm.readdata(4);
  ServCount.trim();
  NewServCount = ServCount.toInt();
  NewServCount++;
  ServCount = String(NewServCount);
  //Write New Serve Count
  Eeprm.writedata(4,ServCount);
  EEPROM.commit();  
}
void PrintServCount()
{
  String ServCount;
  Eeprm Eeprm(0);
  ServCount=Eeprm.readdata(4);
  Serial.println("Current Serve Count:" + ServCount);   
}
void SaveConfig()
{
  Eeprm Eeprm(0);
  EEPROM.commit();
  Serial.println("\033[1;32mConfiguration saved\033[0m");
}
void DumpConfig()
{  
  String TriggerCounter;
  Eeprm Eeprm(0);
  APPSSID=Eeprm.readdata(1);
  APPASS=Eeprm.readdata(2);
  WIFIPrefix=Eeprm.readdata(3);
  TriggerCounter=Eeprm.readdata(5);
  
  
  Serial.println("\033[1;32mConfigutration Dump.\033[0m");
  Serial.println("Access Point Name:   " + Eeprm.readdata(1));
  Serial.println("Access Point Pass:   " + APPASS);
  Serial.println("Access Point Prefix: " + WIFIPrefix);
  Serial.println("Scheduler Interval:  " + TriggerCounter + " minutes");
  Serial.println("Scheduler Disabled:  " + String(SchedulerOff));
  Serial.print("IP Address:          ");Serial.println(WiFi.localIP());

  
  
  
}

void DisconWifi()
{
  WiFi.disconnect();
  Serial.println("Disconnected from wireless network.");
}
void Printhelp()
{
  Serial.println("\033[1;31mHelp\033[0m - Displays a list of valid commands.");
  Serial.println("-----------------------------------------------------------------------------------");
  Serial.println("\033[1;32mSystem\033[0m");
  Serial.println("help                     Shows this help screen.");
  Serial.println("welcome                  Reprint the welcome screen.");
  Serial.println("clear                    Clear the console screen.");
  Serial.println("cltest [n]               VT100 color test.");
  Serial.println("reboot                   Reboot this node.");
  Serial.println("schedule off             Scheduler will not run.");
  Serial.println("schedule on              Scheduler will run.");
  Serial.println("1                        Online Mode - Fully operational");
  Serial.println("0                        Offline Mode - No scheduler and no fetch.");
  
      
  Serial.println("\033[1;32mWiFi Setup\033[0m");
  Serial.println("setssid [apname]         Set this node's access point name.");
  Serial.println("setappass [password]     Set access point password.");
  Serial.println("setwifiprefix [prefix]   Set access point prefix.");  
  Serial.println("setschedule [n minutes]  How often this node will scan and fetch from nearby nodes."); 
  Serial.println("saveconfig               Commit configuration into flash memory.");
  Serial.println("reloadconfig             Reload configuration from flash memory.");
  Serial.println("dumpconfig               Show configuration in flash memory.");
  Serial.println("unlinkwifi               Disconnect from current access point.");
  
  
  Serial.println("\033[1;32mFilesystem\033[0m");
  Serial.println("formatfs                 Format filesystem. All files will be lost.");
  Serial.println("ls                       Show list of files in filesystem.");
  Serial.println("more [filename.ext]      Dump file content to console.");
  Serial.println("delete [filename.ext]    Delete a file from filesystem.");
  Serial.println("updatelist               Update file register.");
  
  Serial.println("\033[1;32mUtilities \033[0m");
  Serial.println("show verbose             Shows the current verbode mode.");
  Serial.println("verbose [on] or [off]    On or Off. Shows changes and updates in console.");
  Serial.println("scanwifi                 Scans for nearby wifi access points."); 
  Serial.println("fetchtest                Fetch a url and print data to console."); 
  Serial.println("fetchnearby              Scan nearby access points and fetch data."); 
  Serial.println("setservcount [n]         Set the servecount to a new number.");
  Serial.println("updateservcount          Increment the servecount by 1.");   
  Serial.println("printservcount           Print current serve count to console.");       
  Serial.println("---------------------------------------------------------------------");  
}

void GetListFromRemoteHost(String RemoteHost)
{
    String Payload;    
    String PostData;
    
    
    String ls_line;
    HTTPClient http;
    PostData = "shared_key=" + SharedKey + "&fromdev=" + APPSSID;
    http.begin(RemoteHost + "?file=list.txt"); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
    int httpCode = http.POST(PostData);   //Send the request
    Payload = http.getString();    //Get the response payload
    //Uncomment to debug: Returns the payload form list.txt
    //Serial.println("URL=" + RemoteHost + "?file=list.txt" + " Payload:\n\r" + Payload);
     
   int ls_StartChar=0;
       for (int i = 0; i <= Payload.length(); i++)
       { 
          //Extract Lines from Payload
          if(Payload.substring(i,i+1)=="\n")
          {
              ls_line = Payload.substring(ls_StartChar,i-1); 
              ls_line.replace("/","");             
              //Uncomment to debug: Prints the file that will be fetched.
              //Serial.println("Will Request: " + RemoteHost + "?file=" + ls_line );
              if(ls_line!="list.txt"){GetFileFromRemoteHost(RemoteHost ,ls_line);}
              ls_StartChar=i+1; 
          }
          
       }
   
    
}

void GetFileFromRemoteHost(String RemoteHost,String FileNametoGet)
{
    String Payload;    
    String PostData;
    RemoteHost.trim();
    FileNametoGet.trim();
    //Uncomment to debug: Prints the file that will be fetched.
    //Serial.println("Will Request Data File:" + RemoteHost + "?file=" + FileNametoGet);    
    HTTPClient http;
    PostData = "shared_key=" + SharedKey;
    http.begin(RemoteHost + "?file=" + FileNametoGet); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
    int httpCode = http.POST(PostData);   //Send the request
    Payload = http.getString();    //Get the response payload  
    //Serial.println("Data File Payload: " + Payload);
    //BOF Break up the strings
      String pl_serv_count=Payload.substring(0,9);
      String pl_node_data=Payload.substring(9,Payload.length());
      pl_serv_count.trim();
      pl_node_data.trim();
    //EOF Break up the strings

    //Uncomment to debug:Prints incomming payload data
    //Serial.println("Payload Node_Name:" + FileNametoGet + " serv_count:" + pl_serv_count + " node_data:" + pl_node_data);

    //Read the local version of the file, compare and overwite if newer else leave alone.
      String FileContent;
      File FileObjR = SPIFFS.open("/" + FileNametoGet , "r");
      while (FileObjR.available()) {
          FileContent=FileContent += char(FileObjR.read());      
      } 
      FileObjR.close(); 
      
      //If empty copy payload directly to file and rebuild file list.txt to list.txt or else compare serv_count and overwrite or leave alone.
      if(FileContent==""){
        CreateFile(FileNametoGet,Payload,"NEW");
        UpDateFileListentry();
        
      }else
      {
        String fl_serv_count=FileContent.substring(0,9);
        String fl_node_data=FileContent.substring(9,FileContent.length());
        fl_serv_count.trim();
        fl_node_data.trim();
            if(pl_serv_count.toInt()>fl_serv_count.toInt())
            {
              CreateFile(FileNametoGet,Payload,"UPDATE");
            }else
            {
              if(VerboseMode=="on"){Serial.println("\033[1;36mNo changes for:" + FileNametoGet  + " - left unchanged.\033[0m " + "ServCount [file/payload] [\033[1;32m" + fl_serv_count + "/" + pl_serv_count + "\033[0m]"  );}
            }
      }
      
    
}

void CreateFile(String FileName,String DataToWrite,String WriteType)
{
  File FileObjW = SPIFFS.open("/" + FileName, "w");
  FileObjW.print(DataToWrite);
  FileObjW.close(); 
  
  DataToWrite.trim();
    if(VerboseMode=="on")
    {
      if(WriteType=="NEW"){Serial.println("\033[1;33mNew file created:   \033[0m" + FileName + " with data:[" + DataToWrite + "]");}
      if(WriteType=="UPDATE"){Serial.println("\033[1;33mLocal File updated: \033[0m" + FileName + " with data:[" + DataToWrite  + "]");}
      if(WriteType=="LOCAL"){Serial.println("\033[1;33mSensor Updated:     \033[0m" + FileName + " with data:[" + DataToWrite + "]");}
    }
}
void UpDateFileListentry()
{
    //Delete file because a new list will be created
    SPIFFS.remove("/list.txt");
    
    String BuildList="";
    String FileName=""; 
    String FormattedFile="";   
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
       FileName=dir.fileName();
       FileName.trim();
       FileName.replace("/","");
       //Append to file
       //if(VerboseMode=="on"){Serial.println("Writing new file:" + FileName + " to list.txt (file register)");}
         if(FileName!="list.txt")
         {
          BuildList = BuildList + FileName + "\r\n";
         }
    }
           //Add Blank Line at end of list.txt
           BuildList.replace(" ","");
           File FileObjW = SPIFFS.open("/list.txt", "w");
           FileObjW.print(BuildList);
           FileObjW.close();  

}



//Commands
void Printwelcome()
{
  Serial.println("");
  Serial.println("*********************************************************************");
  Serial.println("** Welcome to \033[1;31mWiFiMeshFS\033[0m CLi V2.01                                 **");
  Serial.println("**                                                                 **");  
  Serial.println("** Description: WiFiMeshFS is a mesh network for sharing events    **");
  Serial.println("** By Martin Viljoen                                               **");
  Serial.println("*********************************************************************");
  Serial.println("Type help to get a list of commands.");
  Serial.println("");
}
void clearAndHome()
{
  Serial.write(27);
  Serial.print("[2J"); // clear screen
  Serial.write(27);
  Serial.print("[H"); // cursor to home
}
void dosworking()
{
  int MyDelay=50;
  int progress=0;
  for (int i=0; i <= 5; i++)
  {
 
  progress = i * 10;
   
  Serial.println("|");
  Serial.print(progress);Serial.print("% complete.");  
  delay(MyDelay);
  clearAndHome();
   
  Serial.println("/");
  Serial.print(progress);Serial.print("% complete."); 
  delay(MyDelay);
  clearAndHome();
   
  Serial.println("-");
  Serial.print(progress);Serial.print("% complete."); 
  delay(MyDelay);
  clearAndHome();
   
  Serial.println("\\");
  Serial.print(progress);Serial.print("% complete."); 
  delay(MyDelay);
  clearAndHome();
  
  }
  Serial.println("100% Done!");
}
void Reboot()
{
  //Reboot this node
  ESP.restart();
}
void ClTest()
{
  StringValue = cmdBuild.substring(7,1000);
  StringValue.trim();
  Serial.println("Printing Color:" + StringValue);
  Serial.println("\033[1;" + StringValue + "mThe quick brown fox jumps over the lazy dog\033[0m");
 
}
void FormatFS()
{
  Serial.println("\033[1;31mFormat started.\033[0m Please do not remove power...");
  SPIFFS.format(); 
  Serial.println("Format complete: \033[1;33mAll system files wiped out.\033[0m \n\rNote: New file register will be re-created from nearby nodes."); 
}
void ListFiles()
{
    Serial.println("Listing all files in filesystem.");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
       Serial.println(dir.fileName()); 
    }  
}
void DeleteFile()
{
  StringValue = cmdBuild.substring(7,1000);
  StringValue.trim();
  SPIFFS.remove("/" + StringValue);
  Serial.println("\033[1;31m" + StringValue + "\033[0m deleted.");
}
void ShowVerbose()
{
  Serial.println("verbose mode=" + VerboseMode);
}
void SetVerboseMode()
{
  StringValue = cmdBuild.substring(8,1000);
  StringValue.trim();
    if(StringValue=="on"){VerboseMode="on";Serial.println("Verbose mode set to: [\033[1;32m" +  StringValue + "\033[0m]");}
    if(StringValue=="off"){VerboseMode="off";Serial.println("Verbose mode set to: [\033[1;32m" +  StringValue + "\033[0m]");}
    if(StringValue!="off" && StringValue!="on" ){Serial.println("Invalid verbose switch! Either set to [on] or [off]");}
}
void ShowFileContent()
{
      String FileContent;
      StringValue = cmdBuild.substring(5,1000);
      StringValue.trim();
      Serial.println("\033[1;39mBOF: [" + StringValue + "]\033[0m");      
      File FileObjR = SPIFFS.open("/" + StringValue , "r");
      if (!FileObjR) {
          Serial.println("File not found.");
      }      
      while (FileObjR.available()) {
          FileContent=FileContent += char(FileObjR.read());      
      } 
      FileObjR.close(); 
      Serial.println(FileContent);
      Serial.println("\033[1;39mEOF: [" + StringValue + "]\033[0m");                
}
void ScanWifi()
{
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  
}
void CreateWebServerRoot()
{
  server.on("/getfile", GetFile);
}

void GetFile()
{
      String FileContent;
      String FileToLoad;
      String FromDevice;
      if(server.hasArg("file"))
      {
              FileToLoad = server.arg("file");
              File FileObjR = SPIFFS.open("/" + FileToLoad , "r");
              if (!FileObjR) {
                  
                  if(VerboseMode=="on")
                  {
                    FileContent="File not found whilst trying to serve " + FileToLoad;
                    Serial.println("File not found whilst trying to serve " + FileToLoad);                    
                   }
              }      
              while (FileObjR.available()) {
                  FileContent=FileContent += char(FileObjR.read());      
              } 
              FileObjR.close();        
      }
      server.send(200, "text/plain", FileContent);
      //Blink LED when remote host fetched from this device
      for (int i = 0; i <= 3; i++) {
        digitalWrite(2, HIGH);
        delay(50);
        digitalWrite(2, LOW);  
        delay(50);      
      }
      digitalWrite(2, HIGH);//Turn off LED

 
}

void FetchTest()
{
    StringValue = cmdBuild.substring(10,1000);
    StringValue.trim();    
    String Payload;    
    String PostData;
    //Uncomment to debug: Prints the file that will be fetched.
    //Serial.println("Will now open:" + RemoteHost + "/" + FileNametoGet);    
    HTTPClient http;
    PostData = "shared_key=" + SharedKey;
    http.begin(StringValue); 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
    int httpCode = http.POST(PostData);   //Send the request
    Payload = http.getString();    //Get the response payload  
    Serial.println("\033[1;33mBOF Fetching URL:" + StringValue + "\033[0m\n\r" +  Payload);
    Serial.println("\033[1;33mEOF Fetching URL:" + StringValue + "\033[0m");
   
}
void FetchNearby()
{
    /*
    StringValue = cmdBuild.substring(12,1000);
    StringValue.trim();  
    GetListFromRemoteHost(StringValue); 
    */
    //Default IP=http://192.168.4.1/getfile
    String WifiSetupStatus="SUCCESS";
    String ScannedAP;
    int ConAttempt=0;
    int SuccesFetchCount=0;
    int n = WiFi.scanNetworks();

    WiFi.disconnect();
    if(VerboseMode=="on"){Serial.println("Disconnecting from WIFI..");}
    
    if (n == 0) {
      if(VerboseMode=="on"){Serial.println("no wifi networks found");}
    } else {
        if(VerboseMode=="on")
        {
            Serial.print(n);
            Serial.println(" networks found");
            CustomCode();
            
        }
      for (int i = 0; i < n; ++i) {
        ScannedAP=WiFi.SSID(i);
        if(ScannedAP.substring(0,WIFIPrefix.length())==WIFIPrefix)
        {
          if(VerboseMode=="on"){Serial.println("Valid prefix found in AP:" + ScannedAP + " connection will be attempted.");}
        }else
        {
          //Serial.print("AP with invalid prefix found:["); 
          //Serial.println( ScannedAP.substring(0,WIFIPrefix.length()) + "]" + ScannedAP + " bailing out." );
          goto bailout;
        }
        
        // Print SSID and RSSI for each network found
        if(VerboseMode=="on"){Serial.print("Connecting to:");Serial.println(WiFi.SSID(i));}
        WiFi.mode(WIFI_STA);
        WiFi.begin(WiFi.SSID(i), APPASS.c_str());
        while (WiFi.status() != WL_CONNECTED)  {
          ConAttempt++;
          delay(500); 
          if(VerboseMode=="on"){Serial.print(".");}
              if (ConAttempt>=20)
              {
                  WifiSetupStatus="FAIL";
                  if(VerboseMode=="on"){Serial.println("\n\rWireless connection failed. Attempting to connect to next AP.");}
                  ConAttempt=0;
                  break;                
              }else{WifiSetupStatus="SUCCESS";}
          
        }
          if(WifiSetupStatus=="SUCCESS")
          {
            SuccesFetchCount++;
            if(VerboseMode=="on"){Serial.println();}
            if(VerboseMode=="on"){Serial.print("IP address: ");}
            if(VerboseMode=="on"){Serial.print(WiFi.localIP());}           
            if(VerboseMode=="on"){Serial.println(" Connect success, will now start to fetch data from: http://192.168.4.1/getfile ");}
            GetListFromRemoteHost("http://192.168.4.1/getfile");
            WiFi.disconnect();  
          }
        CustomCode();
        bailout:
        delay(10);
      }
    }  


  if(VerboseMode=="on"){Serial.println("Restarting AP Mode from config...");}
  WiFi.softAP(APPSSID.c_str(), APPASS.c_str()); 
  LoadConfig();
  if(VerboseMode=="on"){
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("SSID: " + APPSSID);
      Serial.print(" AP IP address: ");
      Serial.println(myIP);    
  }  
  if(VerboseMode=="on"){Serial.print("[\033[1;32mFetching complete\033[0m] Connected to ");Serial.print( SuccesFetchCount ); Serial.println(" nodes in total.");}
  
CustomCode();
}


//Sensor routine (This is were you put custom code)
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
