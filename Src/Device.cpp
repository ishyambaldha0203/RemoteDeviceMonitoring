/*************************************************************************************************
 * @file Device.cpp
 *
 * @brief Implementation to simulate one instance of remote device.
 *
 *************************************************************************************************/
#include "Common.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

using namespace std;

namespace
{
    constexpr const char *DeviceNamePrefix = "device_";

    bool isExit = false;

    void PrintUsage()
    {
        cout << "Usage: ./device <device ID>" << endl;
    }

    bool IsNumber(const string &str)
    {
        for (char const &c : str)
        {
            if (isdigit(c) == 0)
            {
                return false;
            }
        }

        return true;
    }
} // Anonymous namespace

namespace DeviceImplementation
{
    /**
     * @class Device
     *
     * @brief A concrete implementation of device class.
     */
    class Device
    {
    public:
        /**
         * @brief Construct a new instance of Device.
         *
         * @param id A device id for unique identification.
         */
        Device(int32_t id) : _deviceId(id)
        {
            _isAlive = false;

            // Ignore SIGPIPE to keep client ALIVE if server went down.
            signal(SIGPIPE, SIG_IGN);

            // set unique device name using device ID.
            _deviceName = DeviceNamePrefix;
            _deviceName = _deviceName.append(to_string(_deviceId));

            if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                cerr << "socket: " << strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }

            int32_t on = 1;
            if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
            {
                cerr << "setsockopt: " << strerror(errno) << endl;
            }

            if (setsockopt(_socketFd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on)) < 0)
            {
                cerr << "setsockopt: " << strerror(errno) << endl;
            }
        }

        /**
         * @brief Destroy the Device object.
         */
        ~Device() = default;

        /**
         * @brief Get the Socket Fd object.
         *
         * @return int32_t socket file descriptor.
         */
        int32_t getSocketFd()
        {
            return _socketFd;
        }

        /**
         * @brief To connect the device to the monitoring server.
         *
         * @return int32_t status code.
         */
        int32_t connectServer()
        {
            struct sockaddr_in serverAddr;

            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(Default::ServerPort);
            if (inet_pton(AF_INET, Default::ServerIp, &serverAddr.sin_addr) <= 0)
            {
                cerr << "inet_pton: Invalid address/ Address not supported" << strerror(errno) << endl;
                return -1;
            }

            if (connect(_socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
            {
                cerr << "connect: " << strerror(errno) << endl;
                return -1;
            }

            _isAlive = true;

            return 0;
        }

        /**
         * @brief To generate some random data for demo purpose.
         *
         * @return int32_t Newly generated data.
         */
        int32_t generateData()
        {
            int32_t data;
            data = random() % 100;

            return data;
        }

        /**
         * @brief To send Message to monitoring server.
         *
         * @return int32_t A send status.
         */
        int32_t sendMessage()
        {
            if (_isAlive)
            {
                MessageFrame message;
                Response response;
                int32_t ret = 0;

                bzero((Response *)&response, sizeof(response));
                bzero((MessageFrame *)&message, sizeof(message));

                strcpy(message.deviceName, _deviceName.c_str());
                message.deviceId = _deviceId;
                message.data = generateData();

                cout << "Sending data to Server: " << message.data << endl;

                ret = write(_socketFd, &message, sizeof(message));
                if (ret < 0)
                {
                    cerr << "write: " << strerror(errno) << endl;
                    return -1;
                }

                ret = read(_socketFd, &response, sizeof(response));
                if (ret < 0)
                {
                    cerr << "read: " << strerror(errno) << endl;
                }

                /* Response as error code from server */
                cout << "Response code: " << response.errorCode << endl;
            }
            else
            {
                cerr << "Monitor Server is not alive, try to connect again." << endl;
                connectServer();
            }

            return 0;
        }

    private:
        int32_t _deviceId;
        string _deviceName;
        int32_t _socketFd;
        bool _isAlive;
    };
} // namespace DeviceImplementation

using namespace DeviceImplementation;

/**
 * @brief Main function to start the device simulation.
 *
 * @param argc A command line argument count.
 * @param argv An array of command line arguments.
 *
 * @return int32_t An application status code.
 */
int32_t main(int32_t argc, char *argv[])
{
    // Checks for Command line arguments.
    if (argc != 2)
    {
        cerr << "device must take only 1 argument, Its Device Id." << endl;
        PrintUsage();
        exit(EXIT_FAILURE);
    }

    int32_t deviceId = 0;
    int32_t ret = 0;

    // Validation for device ID
    if (IsNumber(argv[1]))
    {
        deviceId = stoi(argv[1]);
        if (deviceId < 1 || deviceId > Default::MaxDeviceSupported)
        {
            cerr << "Invalid Device Id, It must be b/w 1 and " << Default::MaxDeviceSupported << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Device " << deviceId << " is started." << endl;
    }
    else
    {
        cerr << "Invalid Device Id." << endl;
        exit(EXIT_FAILURE);
    }

    // Create instance for device.
    Device device(deviceId);

    // Connect device to server/
    device.connectServer();

    // Send data to monitoring server.
    do
    {
        ret = device.sendMessage();
        if (ret == -1)
        {
            isExit = true;
        }
        sleep(1);
    } while (!isExit);

    cerr << "If you are seeing this, something is fishy!" << endl;

    exit(EXIT_SUCCESS);
}
