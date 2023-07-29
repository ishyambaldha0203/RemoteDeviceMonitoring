# RemoteDeviceMonitoring
Monitor and collect data from multiple remote devices.

## Brief of project
- System is having 2 major components, the Monitoring Server and the Device as a client.
- Device: This component sends the data to Monitoring Server.
- Monitor Server: The monitor has two jobs – receive the messages from Devices and keep a count of the total messages received from each Device.

## High-Level Design
**This is achieved using Socket Programming as a TCP server and client.**
```
                            ----------
                            |Device 1|
                           /----------
                          /              
                         /                 
                        /                  
                       /   
                      /                                         
   ----------------  /       ----------      
   |Monitoring Server| -------> |Device 2| 
   ----------------  \       ----------                     
                      \                    
                       \                   
                        \                  
                         \
                          \----------    
                           |Device N| 
                           ----------    
```
## Build the Application (monitor and device)
1. Using cmake version `3.19.0` to build the application.
2. Using C++ version `C++14` but it is compatible with C++17 as well.
3. Open the command prompt and navigate to the `RemoteDeviceMonitoring` directory.
4. Create a new directory named `build` by entering `mkdir build`, then navigate to the `build` directory ($cd build).
5. Run the `cmake` command by entering ($cmake ..).
6. Compile the application by entering the `make` command. To speed up compilation, you can use the `-j` option followed by the number of cores you want to use, for example, ($make -j4).
7. If compilation is successful, an executable file named `monitor` and 'device' will be created in the `./bin/exec/Debug` directory.

## Run the Application
1. Start Monitor Server($ ./monitor).
2. Start Devices as a client with Device ID as an argument in a separate terminal($ ./device 1, $ ./device 2, $ ./device 3).
3. Observe the log where Monitor Server is executing, It is print below details at run time,
   - Data and device name it is received from Device.
   - Number of total messages received from each device. 
   - Example logs,
     ```
        Message from device device_2, Data: 83
        Total message from device_1 is: 1
        Message from device device_1, Data: 86
        Total message from device_1 is: 2
        Message from device device_1, Data: 77
        Total message from device_2 is: 3
        Message from device device_2, Data: 15
     ```
## Limitation
**NOTE: For now I have kept the limit of 5 devices[`MaxDeviceSupported` default value in Common.hpp].**

## Future scope
1. Class declarations and definitions can be separated. Developed this project in no time so all source codes are written in the same file.
2. Common class can be created for all socket-related operations.
3. Monitor Server IP and Ports are hardcoded in source code but that can be made configurable by using some means of configuration file.
4. The number of Devices a Server can handle could also be made configurable but as its POC this is hardcoded.

## Known Issue/Bug
NOTE: None for now. 
