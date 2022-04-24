#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <openssl/ssl.h>

#include "eventreceiver.h"
#include "polling.h"
#include "semaphore.h"
#include "socketinfo.h"
#include "threading.h"

namespace Core {

class DataBlockPool;
class Logger;
class WorkManager;
class TickPoke;
enum class ResolverResult;

/* The IOManager class handles all socket interaction in an application.
 * It listens on all managed sockets and dispatches related events to the
 * WorkManager.
 */
class IOManager : public EventReceiver {
public:
  IOManager(WorkManager& wm, TickPoke& tp);
  ~IOManager();

  /* The IOManager needs to be initialized by calling this function right before
   * the application enters its main loop.
   * @param prefix: prefix name of the thread for debugging purposes
   * @param id: id number of the thread for debugging purposes
   */
  void init(const std::string& prefix, int id = 0);

  /* begin stopping the IOManager event loop */
  void preStop();

  /* Stops the IOManager event loop and joins the thread. */
  void stop();

  /* Register an external file descriptor with the IOManager. An FDData(int)
   * event will occur when data is available for reading.
   * @param er: The event receiver
   * @param fd: the file descriptor to listen on
   * @return: an identifier
   */
  int registerExternalFD(EventReceiver* er, int fd);

  /* Create and listen on a TCP client socket that connects to the specified host/port.
   * @param er: The event receiver
   * @param addr: The address to connect to
   * @param port: The port to connect to
   * @param resolving: Whether the socket becomes busy with resolving a dns
   * @param addrfam: The address family (IPv4 or IPv6) to use
   * @param listenimmediately: whether to begin listening for data on the socket
   *                           immediately after connecting or not. Set to false
   *                           if SSL is to be negotiated immediately instead.
   * @return: a socket identifier
   *
   * An FDConnecting() event will be produced when the socket is connecting, if
   * the host name had to be remotely resolved.
   * When the socket has connected successfully, an FDConnected() event will be produced.
   * If the socket fails to connect, an FDFail() event will be produced.
   * When data is available on the socket, an FDData(int, char*, unsigned int) event
   * will be produced.
   * When the socket has been disconnected remotely, an FDDisconnected() event will be
   * produced.
   */
  int registerTCPClientSocket(EventReceiver* er,
                              const std::string& addr,
                              int port,
                              bool& resolving,
                              AddressFamily addrfam = AddressFamily::IPV4_IPV6,
                              bool listenimmediately = true);

  /* Same as above without last few args */
  int registerTCPClientSocket(EventReceiver* er, const std::string& addr, int port);

  /* Create and listen on a TCP server socket that binds to the specified port.
   * @param er: The event receiver
   * @param port: The port to bind to
   * @param addrfam: The address family (IPv4 or IPv6) to use
   * @param local: whether to listen for local connections only
   * @return: a socket identifier
   *
   * When a new socket has connected, an FDNew() event will be produced.
   * If the listener fails to bind to the specified port, an FDFail() event
   * will be produced.
   */
  int registerTCPServerSocket(EventReceiver* er,
                              int port,
                              AddressFamily addrfam = AddressFamily::IPV4,
                              bool local = false);

  /* Listen on an already bound TCP server socket.
   * @param er: The event reciver
   * @param sockfd: The file descriptor to listen on
   * @param addrfam: The address family (IPv4 or IPv6) of the socket
   * @return: a socket identifier
   */
  int registerTCPServerSocketExternalFD(EventReceiver* er, int sockfd, AddressFamily addrfam);

  /* Register for events on a socket that was created through a TCP server socket
   * owned by this IOManager instance.
   * @param er: The event receiver
   * @param sockid: The socket identifier to register
   * @param listenimmediately: whether to begin listening for data on the socket
   *                           immediately after connecting or not. Set to false
   *                           if SSL is to be negotiated immediately instead.
   *
   * Events are the same as for registerTCPClientSocket above.
   */
  void registerTCPServerClientSocket(EventReceiver* er, int sockid, bool listenimmediately = true);

  /* Create and listen on a Unix domain socket.
   * @param er: The event receiver
   * @param path: The socket path
   * @return: a socket identifier
   */
  int registerUnixServerSocket(EventReceiver* er, const std::string& path);

  /* Create and listen on an UDP server socket that binds to the specified port.
   * @param er: The event receiver
   * @param port: The port to bind to
   * @param addrfam: The address family (IPv4 or IPv6) of the socket.
   * @return: a socket identifier
   *
   * Events are the same as for registerTCPServerSocket above.
   */
  int registerUDPServerSocket(EventReceiver* er, int port, AddressFamily addrfam = AddressFamily::IPV4);

  /* Take over a previously registered socket with a new EventReceiver.
   * @param er: The new event receiver
   * @param sockid: The socket identifier to register
   */
  void adopt(EventReceiver* er, int sockid);

  /* Initiate a client-side SSL negotiation on a socket.
   * @param sockid: The socket identifier
   *
   * SSL negotiation will proceed in the background. When it has finished,
   * an FDSSLSuccess() or an FDDisconnected() event will be produced.
   */
  void negotiateSSLConnect(int sockid);

  /* Initiate a client-side SSL negotiation on a socket that should reuse
   * session data from a previously stored session.
   * @param sockid: The socket identifier
   * @param sessionkey: The key of the previously stored session.
   *
   * SSL negotiation will proceed in the background. When it has finished,
   * an FDSSLSuccess() or an FDDisconnected() event will be produced.
   */
  void negotiateSSLConnect(int sockid, int sessionkey);

  /* Initiate a client-side SSL negotiation on a socket that should inherit
   * session data from a parent socket.
   * @param sockid: The socket identifier
   * @param parentsockid: The socket identifier for the parent socket.
   *
   * SSL negotiation will proceed in the background. When it has finished,
   * an FDSSLSuccess() or an FDDisconnected() event will be produced.
   */
  void negotiateSSLConnectParent(int sockid, int parentsockid);

  /* Initiate a server-side SSL negotiation on a socket.
   * @param sockid: The socket identifier
   *
   * SSL negotiation will proceed in the background. When it has finished,
   * an FDSSLSuccess() or an FDDisconnected() event will be produced.
   */
  void negotiateSSLAccept(int sockid);

  /* Send data on a socket.
   * @param sockid: The socket identifier.
   * @param data: The data to send.
   * @return: Whether the socket can accept more data for sending immediately
   *
   * When the data has finished sending, an FDSendComplete() event will be produced.
   */
  bool sendData(int sockid, const std::string& data);

  /* Send data on a socket.
   * @param sockid: The socket identifier.
   * @param data: The data to send.
   * @return: Whether the socket can accept more data for sending immediately
   *
   * When the data has finished sending, an FDSendComplete() event will be produced.
   */
  bool sendData(int sockid, const std::vector<char>& data);

  /* Send data on a socket.
   * @param sockid: The socket identifier.
   * @param data: The data to send.
   * @param dataLen: The length of the data to send.
   * @return: Whether the socket can accept more data for sending immediately
   *
   * When the data has finished sending, an FDSendComplete() event will be produced.
   */
  bool sendData(int sockid, const char* data, unsigned int dataLen);

  /* Return the cipher of an SSL socket.
   * @param sockid: The socket identifier.
   * @return: The negotiated cipher
   */
  std::string getCipher(int sockid) const;

  /* Return whether reusing a previous SSL session will be/was attempted.
   * @param sockid: The socket identifier.
   * @return: true/false
   */
  bool getSSLSessionReuse(int sockid) const;

  /* Return whether a previous SSL session has been reused successfully.
   * @param sockid: The socket identifier.
   * @return: true/false
   */
  bool getSSLSessionReused(int sockid) const;

  /* Get the session key stored for session reuse for a specific socket.
   * @param sockid: The socket identifier.
   * @return: the session key, or -1 if none was found
   */
  int getReusedSessionKey(int sockid) const;

  /* Return the remote address of a socket.
   * @param sockid: The socket identifier.
   * @return: The remote address
   */
  std::string getSocketAddress(int sockid) const;

  /* Return the remote port of a socket.
   * @param sockid: The socket identifier.
   * @return: The remote port
   */
  int getSocketPort(int sockid) const;

  /* Return the file descriptor of a socket.
   * @param sockid: The socket identifier.
   * @return: The file descriptor
   */
  int getSocketFileDescriptor(int sockid) const;

  /* Return the local address of a socket, either IPv4 or IPv6.
   * @param sockid: The socket identifier.
   * @return: The local address
   */
  std::string getInterfaceAddress(int sockid) const;

  /* Return the IPv4 address of the interface that a socket is bound to.
   * @param sockid: The socket identifier.
   * @return: The local address
   */
  StringResult getInterfaceAddress4(int sockid) const;

  /* Return the IPv6 address of the interface that a socket is bound to.
   * @param sockid: The socket identifier.
   * @return: The local address
   */
  StringResult getInterfaceAddress6(int sockid) const;

  /* Return the address family of a socket.
   * @param sockid: The socket identifier.
   * @return: The address family
   */
  AddressFamily getAddressFamily(int sockid) const;

  /* Close a socket as soon as its send queue is emptied.
   * @param sockid: The socket identifier.
   */
  void closeSocket(int sockid);

  /* Close a socket immediately.
   * @param sockid: The socket identifier.
   */
  void closeSocketNow(int sockid);

  /* Pause the listening on a socket, for overload mitigation reasons
   * @param sockid: The socket identifier
   */
  void pause(int sockid);

  /* Resume the listening on a socket, for overload mitigation reasons
   * @param sockid: The socket identifier
   */
  void resume(int sockid);

  /* set SO_LINGER with timeout 0 on the socket, resulting in an
   * RST instead of FIN when the socket is closed. Used for avoiding
   * TIME_WAIT.
   */
  void setLinger(int sockid);

  /* Store the TLS session for this socket.
   * @param sockid: the socket identifier
   * @return: the stored session key
   */
  int storeSession(int sockid);

  /* Clear a previously stored TLS session.
   * @param sessionKey: the session key to clear
   */
  void clearSession(int sessionKey);

  /* Clear a previously stored TLS session.
   * @param sockid: the socket identifier holding the session key to clear
   */
  void clearReusedSession(int sockid);

  /* Whether a network interface to bind to has been specified
   * @return: true or false
   */
  bool hasBindInterface() const;

  /* Get the name of the network interface to bind to, if specified
   * @return: the name
   */
  std::string getBindInterface() const;

  /* Whether a local IP address to bind to has been specified
   * @return: true or false
   */
  bool hasBindAddress() const;

  /* Get the local IP address to bind to, if specified
   * @return: the IP address
   */
  std::string getBindAddress() const;

  /* List the available network interfaces
   * @param ipv4: whether to include ipv4 addresses
   * @param ipv6: whether to include ipv6 addresses
   * @return: list of interfaces
   */
  std::list<std::pair<std::string, std::string> > listInterfaces(bool ipv4 = true, bool ipv6 = false);

  /* Set the network interface to bind to
   * @param interface: the interface to bind to
   */
  void setBindInterface(const std::string& interface);

  /* Set the local IP address to bind to.
   * This setting will take precedence over setBindInterface
   * @param address: the local IP address to bind to
   */
  void setBindAddress(const std::string& address);

  /* Return the primary IPv4 address of a given interface
   * @param interface: The name of the interface
   * @return: the interface address
   */
  StringResult getInterfaceAddress(const std::string& interface) const;

  /* Return the primary IPv6 address of a given interface
   * @param interface: The name of the interface
   * @return: the interface address
   */
  StringResult getInterfaceAddress6(const std::string& interface) const;

  /* Return the same IPv6 address in a compact format.
   * @param address: The long address
   * @return: the shortened version
   */
  static std::string compactIPv6Address(const std::string& address);

  /* Used internally */
  void run();
  void tick(int message);
  void asyncTaskComplete(int type, int sockid);
  void workerReady();
  ResolverResult resolveHost(int sockid, bool mayblock);

private:
  bool handleError(EventReceiver* er, int sockid = -1);
  void closeSocketIntern(int sockid);
  void handleExternalIn(SocketInfo& socketinfo);
  void handleTCPConnectingOut(SocketInfo& socketinfo);
  void handleTCPPlainIn(SocketInfo& socketinfo);
  void handleTCPPlainOut(SocketInfo& socketinfo);
  void handleTCPSSLNegotiationIn(SocketInfo& socketinfo);
  void handleTCPSSLNegotiationOut(SocketInfo& socketinfo);
  void handleTCPSSLIn(SocketInfo& socketinfo);
  void handleTCPSSLOut(SocketInfo& socketinfo);
  void handleUDPIn(SocketInfo& socketinfo);
  void handleTCPServerIn(SocketInfo& socketinfo);
  void handleTCPNameResolution(SocketInfo& socketinfo);
  void logWrite(const std::string& text);
  void pollRead(SocketInfo& socketinfo);
  void pollWrite(SocketInfo& socketinfo);
  void setPollRead(SocketInfo& socketinfo);
  void setPollWrite(SocketInfo& socketinfo);
  void unsetPoll(SocketInfo& socketinfo);
  void autoPause(SocketInfo& socketinfo);
  StringResult getInterfaceName(const std::string& address) const;
  StringResult getAddressToBind(const AddressFamily addrfam, const SocketType socktype);
  Polling polling;
  Thread<IOManager> thread;
  mutable std::mutex socketinfomaplock;
  std::unordered_map<int, SocketInfo> socketinfomap;
  std::unordered_map<int, int> connecttimemap;
  std::unordered_set<int> autopaused;
  std::unordered_set<int> manuallypaused;
  WorkManager& workmanager;
  TickPoke& tickpoke;
  DataBlockPool& blockpool;
  std::shared_ptr<DataBlockPool> sendblockpool;
  int blocksize;
  int sockidcounter;
  std::string bindinterface;
  std::string bindaddress;
  bool hasbindinterface;
  bool hasbindaddress;
  std::unordered_map<int, SSL_SESSION*> sessions;
  int sessionkeycounter;
  Semaphore initialized;
};

} // namespace Core
