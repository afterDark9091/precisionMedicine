#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

void setup() {

  Serial.begin(115200);                                       //Serial connection
  WiFi.begin("ONO4B1E", "RaVjJv4H919S");   //WiFi connection

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(1000);
    Serial.println("Waiting for connection");
  }

}

void loop() {

	if (WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

		WiFiClient client;
		HTTPClient http;                    //Declare object of class HTTPClient

		http.begin(client,                  //Specify request destination
		           "http://192.168.1.40:8080/test.php"); 
		http.addHeader("Content-Type",      //Specify content-type header
		               "application/x-www-form-urlencoded");

		int httpCode = http.POST("uname=jimmy&password=1234");   //Send the request
		String payload = http.getString();  //Get the response payload

		Serial.print("Return: ");
		Serial.println(httpCode);           //Print HTTP return code
		Serial.println(payload);            //Print request response payload

		http.end();                         //Close connection

	} else {
    Serial.println("Error in WiFi connection");
    Serial.println("Done");
	}

  delay(30000);  //Send a request every 30 seconds

}
