EVRYTHNG API - mbed integration.
=============================================================
We have created a wrapper to connect ARM/mbed devices to the EVRYTHNG engine. This allows ARM/mbed to talk to our API and use our service to store various sensor data, and create rapidly Web applications that read the mbed data from the engine and display it. With the EVRYTHNG API wrapper for ARM/mbed you can easily integrate the EVRYTHNG platform in your mbed applications as well!

## Usage ##

The EVRYTHNG wrapper for ARM/mbed is based on a set of structures and functions that let you invoke the EVRYTHNG engine API easily from your code.

Currently we support the following functionality out of the box:

### Get Properties of Thngs ###
Retrieves the last update of a property value of a Thng.
```c
EvrythngApi api(API_KEY);

string value;

int responseCode = api.getThngPropertyValue(THNG_ID, THNG_PROPERTY_NAME, value);

if (responseCode == 0)
{
    //Property retrieved successfully
}
else {

    //Check error code as defined in evry_error.h
}
```
### Update Properties of Thngs ###
Updates a property value of a Thng.
```c
EvrythngApi api(API_KEY);

int64_t timestamp = YOUR_TIMESTAMP;
string updateValue = YOUR_VALUE;

int responseCode = api.setThngPropertyValue(THNG_ID, THNG_PROPERTY_NAME, updateValue, timestamp);

if (responseCode == 0)
{
    //Property updated successfully
}
else {
    //Check error code as defined in evry_error.h
}

```
----------
## Test application ##

If you already have an mbed, simply log in on mbed.org.

- Launch the mbed compiler.
- Import the example application found at evrythng-api-example (or import it directly from the mbed project at https://mbed.org/users/vladounet/code/EvrythngApiExample/) 
 - Import As: Program
 - Target name: EvrythngApiExample (or whatever you like)

- Paste your EVRYTHNG API Key in main.cpp (in the constant API_KEY).
- Create an active digital identity (a thng in our jargon) for your mbed (either using the API or the developer portal).
- Copy the ID of the thng you just created to your main.cpp file as well (in the constant THNG_ID).
- Click compile, and copy the file to the mbed.

Once the program is started, it will read and write properties at given intervals into the ADI (use the API to access them).

For additional information about the API please check our [API Documentation](https://developers.evrythng.com/docs/evrythng-developer-resources).
