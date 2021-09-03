/*
  Repeating Web client

  This sketch connects to a a web server and makes a request
  using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
  the Adafruit Ethernet shield, either one will work, as long as it's got
  a Wiznet Ethernet module on board.

  This example uses DNS, by assigning the Ethernet client with a MAC address,
  IP address, and DNS address.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 19 Apr 2012
  by Tom Igoe
  modified 21 Jan 2014
  by Federico Vanzati

  http://www.arduino.cc/en/Tutorial/WebClientRepeating
  This code is in the public domain.

*/

#include <QNEthernet.h>

using namespace qindesign::network;

#define USING_DHCP    true

#if !USING_DHCP
// Set the static IP address to use if the DHCP fails to assign
IPAddress myIP(192, 168, 2, 222);
IPAddress myNetmask(255, 255, 255, 0);
IPAddress myGW(192, 168, 2, 1);
IPAddress mydnsServer(8, 8, 8, 8);
#endif

// initialize the library instance:
EthernetClient client;

char server[] = "arduino.cc";  // also change the Host line in httpRequest()
//IPAddress server(100,24,172,113);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10 * 1000; // delay between updates, in milliseconds

// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80))
  {
    Serial.print("Connected to ");
    Serial.println(client.remoteIP());

    // send the HTTP PUT request
    client.println("GET /asciilogo.txt HTTP/1.1");
    client.println("Host: arduino.cc");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else
  {
    // if you couldn't make a connection
    Serial.println(F("Connection failed"));
  }
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\nStarting WebClientRepeating with NativeEthernet using QNEthernet Library");

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");

#if USING_DHCP
  // Using DHCP
  Serial.print("Using DHCP => ");
  Ethernet.begin();
#else
  Ethernet.setDNSServerIP(mydnsServer);

  // Using static IP
  Serial.print("Using static IP => ");
  Ethernet.begin(myIP, myNetmask, myGW);
#endif

  if (!Ethernet.waitForLocalIP(5000))
  {
    Serial.println("Failed to configure Ethernet");

    if (!Ethernet.linkStatus())
    {
      Serial.println("Ethernet cable is not connected.");
    }

    // Stay here forever
    while (true)
    {
      delay(1);
    }
  }
  else
  {
    Serial.print("IP Address = ");
    Serial.println(Ethernet.localIP());
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available())
  {
    char c = client.read();
    Serial.write(c);
    Serial.flush();
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval)
  {
    httpRequest();
  }
}
