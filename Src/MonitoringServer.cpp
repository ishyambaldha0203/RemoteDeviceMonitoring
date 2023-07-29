/*************************************************************************************************
 * @file MonitoringServer.cpp
 *
 * @brief Implementation of monitoring server.
 *
 *************************************************************************************************/
#include "Common.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>

#include <unordered_map>

using namespace std;

namespace
{
    constexpr const uint32_t MaxDeviceQueue = 32;
    constexpr const uint32_t PollTimeoutMilliSeconds = 3 * 60 * 1000;

    struct pollfd pollFd[Default::MaxDeviceSupported + 1]; // Added +1 for socket fd
    int32_t pollIndex = 0;
} // Anonymous namespace

namespace ServerImplementation
{
    /**
     * @class MonitoringServer
     *
     * @brief A concrete definition of MonitoringServer.
     */
    class MonitoringServer
    {
    public:
        /**
         * @brief Construct a new Monitoring Server object
         */
        MonitoringServer()
        {
            int32_t setSockOptvalOn = 1;

            if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                cerr << "socket: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }

            if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&setSockOptvalOn, sizeof(setSockOptvalOn)) < 0)
            {
                cerr << "setsockopt: " << strerror(errno) << std::endl;
            }

            if (fcntl(_socketFd, F_SETFL, fcntl(_socketFd, F_GETFL, 0) | O_NONBLOCK | FD_CLOEXEC) == -1)
            {
                cerr << "fcntl: " << strerror(errno) << std::endl;
            }
        }

        /**
         * @brief To bind the server IP and port.
         *
         * @return int32_t Status code.
         */
        int32_t Bind()
        {
            struct sockaddr_in serverAddr;

            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(Default::ServerPort);
            if (inet_pton(AF_INET, Default::ServerIp, &serverAddr.sin_addr) <= 0)
            {
                cerr << "Invalid address:" << Default::ServerIp << std::endl;
                exit(EXIT_FAILURE);
            }

            if ((::bind(_socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) != 0)
            {
                cerr << "bind: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
            else
            {
                cout << "Socket binding successfully." << endl;
            }

            return 0;
        }

        /**
         * @brief To listen the input connection from devices.
         *
         * @return int32_t Status code.
         */
        int32_t Listen()
        {
            if ((::listen(_socketFd, MaxDeviceQueue)) != 0)
            {
                cerr << "listen: " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
            else
            {
                cout << "Monitor Server is listening." << endl;
            }

            pollFd[pollIndex].fd = _socketFd;
            pollFd[pollIndex].events = POLLIN;
            pollIndex++;

            return 0;
        }

        int32_t Accept()
        {
            struct sockaddr_in clientAddr;
            uint32_t addrLen = sizeof(clientAddr);
            int32_t connectionFd = 0;

            do
            {
                // Accept each incoming connection. If accept fails with EWOULDBLOCK, then already accepted all of them.
                // Any other failure on accept will cause us to end the server.
                connectionFd = ::accept(_socketFd, (struct sockaddr *)&clientAddr, &addrLen);
                if (connectionFd < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        cerr << "accept: " << strerror(errno) << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    break; // Accepted all incoming connections.
                }

                cout << "New device connection accepted - " << connectionFd << endl;

                if (fcntl(_socketFd, F_SETFL, fcntl(connectionFd, F_GETFL, 0) | O_NONBLOCK | FD_CLOEXEC) == -1)
                {
                    cerr << "fcntl: " << strerror(errno) << std::endl;
                }

                // Add connection fd for polling.
                pollFd[pollIndex].fd = connectionFd;
                pollFd[pollIndex].events = POLLIN;
                pollIndex++;
            } while (connectionFd != -1);

            return 0;
        }

        int32_t ReadData(int32_t inputDataFd)
        {
            Response response;
            MessageFrame message;
            int32_t ret = 0;

            ret = read(inputDataFd, (MessageFrame *)&message, sizeof(message));
            if (ret < 0)
            {
                cerr << "read: " << strerror(errno) << endl;
            }

            cout << "Message from device " << message.deviceName << ", Data: " << message.data << endl;
            _messageCounter[message.deviceId]++;
            cout << "Total message from " << message.deviceName << " is: " << _messageCounter[message.deviceId] << endl;

            response.errorCode = 200;
            ret = write(inputDataFd, &response, sizeof(response));
            if (ret < 0)
            {
                cerr << "write: " << strerror(errno) << endl;
            }

            return 0;
        }

        /**
         * @brief Get the server socket file descriptor.
         *
         * @return int32_t A server socket file descriptor.
         */
        int32_t GetSocketFd()
        {
            return _socketFd;
        }

    private:
        int32_t _socketFd = 0;
        unordered_map<int32_t, int32_t> _messageCounter;
    };

    /**
     * @brief Server will keep on running in this function until its got termination.
     *
     * @param server An instance of server.
     */
    void PollMonitor(MonitoringServer &server)
    {
        int32_t nfdReady = 0;
        int32_t activePollFd = 0;
        bool isExit = false;

        do
        {
            // Polling all input stream, That is from Devices.
            nfdReady = poll(pollFd, Default::MaxDeviceSupported + 1, PollTimeoutMilliSeconds);
            if (nfdReady < 0)
            {
                cerr << "poll: " << strerror(errno) << endl;
            }

            if (nfdReady == 0)
            {
                cout << "Poll timeout. Stop running Monitor Server." << endl;
                exit(EXIT_FAILURE);
            }

            activePollFd = pollIndex;

            // Check for any request from Device.
            // If yes, Send the request to worker for processing.
            for (int32_t i = 0; i < activePollFd; i++)
            {
                /*
                 * Loop through to find the descriptors that returned
                 * POLLIN and determine whether it's the listening or the active connection.
                 */
                if (pollFd[i].revents == 0)
                {
                    continue;
                }

                // If revents is not POLLIN and fd is valid, it's an unexpected result.
                // So just close the fd and set it to -1.
                // If the POLLNVAL is set, it means the fd is invalid and close will fail.
                if ((pollFd[i].revents != POLLIN) && !(pollFd[i].revents & POLLNVAL))
                {
                    if (close(pollFd[i].fd) == -1)
                    {
                        cerr << "close: " << strerror(errno) << endl;
                    }
                    pollFd[i].fd = -1;

                    // Reorganize poll fd array.
                    for (int32_t j = i; j < pollIndex - 1; j++)
                    {
                        pollFd[j].fd = pollFd[j + 1].fd;
                    }
                    pollIndex--;

                    continue;
                }

                // Handle connection accept request.
                if (pollFd[i].fd == server.GetSocketFd())
                {
                    // Server accepts the connection from device.
                    server.Accept();
                }
                else
                {
                    // Read data from connected devices.
                    server.ReadData(pollFd[i].fd);
                }
            }
        } while (!isExit);
    }
} // namespace ServerImplementation

using namespace ServerImplementation;

/**
 * @brief Main function to start monitoring server.
 *
 * @param argc A command line argument count.
 * @param argv An array of command line arguments.
 *
 * @return int32_t An application status code.
 */
int32_t main(int32_t argc, char *argv[])
{
    int32_t epollFd;

    if ((epollFd = epoll_create1(EPOLL_CLOEXEC)) < 0)
    {
        cerr << "epoll_create1: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    // Create instance for Monitor Server
    MonitoringServer server;

    // Bind socket parameters.
    server.Bind();

    // Start listening to the devices.
    server.Listen();

    // Its monitoring server process handler.
    PollMonitor(server);

    cerr << "If you are seeing this, something is fishy!!!" << endl;

    exit(EXIT_SUCCESS);
}
