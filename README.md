WifiMeshFS 
creates a wireless mesh network that will share and mirror text data with nearby nodes. Each node on the network's data is stored as a text file with extention .txt in the node's flashrom using SPIFFS.

All nodes are configured using a console terminal. (Tested and developed using putty, but any VT100 compatible terminal that supports a serial connection will do.) 

How it works.
Each node is configured with a unique SSID but must share the same wifi password and wifi prefix.
Example: 
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


