import serial
import paho.mqtt.client as paho
import json

serial1=serial.Serial("COM11", 9600, timeout=3)

ACCESS_TOKEN = 'ac2oTqhShFfXPSVH0inF'  # Token of your device
broker = "demo.thingsboard.io"  # host name
port = 1883  # data listening port


def on_publish(client, userdata, result):  # create function for callback
    print("data published to thingsboard \n")
    pass


client1 = paho.Client("control1")  # create client object
client1.on_publish = on_publish  # assign function to callback
client1.username_pw_set(ACCESS_TOKEN)  # access token from thingsboard device
client1.connect(broker, port, keepalive=60)  # establish connection


while True:
    data=serial1.readline()
    print('ser:', data)
    dataString=data.decode("utf-8")
    print('str:', dataString)
    if len(dataString) > 10:
        splitted = dataString.split(":")
        humidity = splitted[0]
        temperature = splitted[1]
        print("Please check LATEST TELEMETRY field of your device")
        payload = {
            "Humidity": humidity,
            "Temperature": temperature
            }
        payload = json.dumps(payload)
        ret = client1.publish("v1/devices/me/telemetry", payload)  # topic- v1/devices/me/telemetry
        print(payload)
ser1.close()

