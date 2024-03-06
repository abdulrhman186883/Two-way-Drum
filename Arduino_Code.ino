#include <SoftwareSerial.h>

SoftwareSerial Arduino_SoftSerial(10, 11);
bool deviceChange = false;
bool globalPing = false;
bool directPing = false;
int globalMessage = 0;
int directMessage = 0;
int directMessageSender = 0;
bool state = false;

String node_recivedString = "";

// SET HERE YOUR SENDER ID!!!!
#define MY_SENDER_ID 5

// IDs for other devices
#define JAKUB_ID 0
#define SUSHMA_ID 1
#define VARVARA_ID 2
#define NASIM_ID 3
#define SUPRIYA_ID 4
#define RAZOUL_ID 6

int deviceIds[] = {RAZOUL_ID, JAKUB_ID, VARVARA_ID, SUPRIYA_ID, SUSHMA_ID, NASIM_ID};

unsigned long lastHitTime = 0;
int counter = 0;
unsigned long patternStartTime = 0;
int hitsWithinPattern = 0;
int solenoidPin = 6;
int currentState = 0;
unsigned long cooldownDuration = 100;

float *deltaArray = new float[10];

// Pattern-specific parameters
unsigned long pattern1Break = 1000;
unsigned long pattern2Break = 1000;
unsigned long pattern3Break = 4000;

bool patternPrinted = false;

// Arrays to store hit information
float hitData[30] = {0}; // Initialize all elements to 0
float timeData[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Initialize all elements to -1
float valueData[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Initialize all elements to -1
float counterData[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Initialize all elements to -1
unsigned long msArray[10] = {0};                               // Initialize all elements to 0

const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

enum Pattern
{
  PATTERN_1,
  PATTERN_2,
  PATTERN_3,
  PATTERN_4,
  PATTERN_5,
  PATTERN_6,
};

Pattern DecisionTreePredict(float *timeData, float *valueData, float *counterData)
{
  Serial.println("PRINTING DELTA");
  for (int i = 0; i < 10; ++i)
  {
    Serial.println(deltaArray[i]);
  }

if (counterData[0] >= 1){
  if (deltaArray[3] <= 104.5)
  {
    if (deltaArray[1] <= 576.0)
    {
      return PATTERN_4;
    }
    else
    { // Delta_2 > 576.0
      return PATTERN_5;
    }
  }
  else
  { // Delta_4 > 104.5
    if (deltaArray[4] <= 389.0)
    {
      if (deltaArray[4] <= 135.0)
      {
        return PATTERN_1;
      }
      else
      { // Delta_5 > 135.0
        return PATTERN_2;
      }
    }
    else
    { // Delta_5 > 389.0
      if (deltaArray[1] <= 489.0)
      {
        return PATTERN_6;
      }
      else
      { // Delta_2 > 489.0
        return PATTERN_3;
      }
    }
  }
}
}
void processHitData()
{
  float features[30]; // Assuming all elements are float

  // Assign values from hitData to features
  for (int i = 0; i < 30; i++)
  {
    features[i] = static_cast<float>(hitData[i]);
  }

  Pattern predictedPattern = DecisionTreePredict(deltaArray, valueData, counterData);

  // Print information about the predicted pattern
  Serial.print("Predicted Pattern: ");
  switch (predictedPattern)
  {
  case PATTERN_1:
    Serial.println("Pattern 1");
    sendNetworkMessage(JAKUB_ID, 1); // Send message to Jakub + Global
    sendGlobalMessage(1);
    break;
  case PATTERN_2:
    Serial.println("Pattern 2");
    sendNetworkMessage(SUSHMA_ID, 2); // Send message to Sushma
    break;
  case PATTERN_3:
    Serial.println("Pattern 3");
    sendNetworkMessage(VARVARA_ID, 3); // Send message to Varvara
    break;
  case PATTERN_4:
    Serial.println("Pattern 4");
    sendNetworkMessage(NASIM_ID, 4); // Send message to Nasim
    break;
  case PATTERN_5:
    Serial.println("Pattern 5");
    sendNetworkMessage(SUPRIYA_ID, 5); // Send message to Supriya
    break;
  case PATTERN_6:
    Serial.println("Pattern 6");
    sendNetworkMessage(RAZOUL_ID, 6); // Send message to Razoul
    break;
  default:
    Serial.println("Unknown Pattern");
    break;
  }

  // Print the resulted arrays
  Serial.println("Resulted Arrays:");
  Serial.print("Time Data: ");
  for (int i = 0; i < 10; i++)
  {
    Serial.print(timeData[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Value Data: ");
  for (int i = 0; i < 10; i++)
  {
    Serial.print(valueData[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Counter Data: ");
  for (int i = 0; i < 10; i++)
  {
    Serial.print(counterData[i]);
    Serial.print(" ");
  }

  Serial.println();

  Serial.print("Milliseconds Data: ");
  for (int i = 0; i < 10; i++)
  {
    Serial.print(msArray[i]);
    Serial.print(" ");
  }

  Serial.println();
}

void resetPatternFlags();

void setup()
{
  Serial.begin(9600);
  Arduino_SoftSerial.begin(9600);
  for (byte a = 2; a <= 6; a++)
  {
    pinMode(a, OUTPUT);
  }
  pinMode(solenoidPin, OUTPUT);

  // initialize the array to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
}

void loop()
{
  int value = analogRead(A1);

  // Subtract the last reading
  total = total - readings[readIndex];
  // Read from the sensor
  readings[readIndex] = value;
  // Add the reading to the total
  total = total + readings[readIndex];
  // Advance to the next position in the array
  readIndex = readIndex + 1;

  // If we're at the end of the array, wrap around to the beginning
  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }

  // Calculate the average
  average = total / numReadings;

  // Use the 'average' value in your hit detection logic
  if (average >= 10 && currentState == 0)
  {
    currentState = 1;
  }
  else if (average <= 5 && currentState == 1)
  {
    currentState = 0;
  }

  float currentTime = millis();

  if (currentState == 1 && (millis() - lastHitTime) > cooldownDuration)
  {
    // Store hit information in the arrays
    timeData[counter] = static_cast<double>(currentTime); // Delta (time of the hit), set to 0 if counter is 1
    valueData[counter] = static_cast<float>(average);      // Value
    counterData[counter] = static_cast<float>(counter + 1); // Counter as float
    msArray[counter] = millis();                           // Milliseconds array

    Serial.print("Hit ");
    Serial.print(counter + 1);
    Serial.print(": Counter = ");
    Serial.print(counterData[counter]);
    Serial.print(", Value = ");
    Serial.print(valueData[counter]);
    Serial.print(", Delta = ");
    Serial.print(timeData[counter]);
    Serial.print(", Milliseconds = ");
    Serial.println(msArray[counter]);

    counter++;

    if (hitsWithinPattern == 0)
    {
      patternStartTime = millis();
    }

    hitsWithinPattern++;

    lastHitTime = millis();
  }

  // After printing the pattern
  if ((millis() - lastHitTime) > 5000 && !patternPrinted)
  {
    Serial.print(counter);

    // Check if the array is not full
    if (counter < 10)
    {
      for (int i = counter; i < 10; i++)
      {
        // Fill the remaining elements with -1
        timeData[i] = -1;
        valueData[i] = -1;
        counterData[i] = -1;
        msArray[i] = 0;
      }
    }

    for (int i = 0; i < 9; ++i)
    { // Loop through time data
      if (timeData[i + 1] == -1)
      { // Check for missing value indicated by -1
        deltaArray[i] = 7; // Assign 7 if the next time data point is missing
      }
      else
      {
        deltaArray[i] = timeData[i + 1] - timeData[i]; // Calculate delta
      }
    }

    // Print predicted pattern after filling the remaining elements
    processHitData();

    // Reset pattern-related flags
    resetPatternFlags();

    patternPrinted = true; // Set the flag to true to indicate that the pattern has been printed
  }
  else if ((millis() - lastHitTime) <= 5000)
  {
    patternPrinted = false; // Reset the flag when a new pattern is started
  }

  // Network communication handling
  handleNetworkCommunication();
}

void resetPatternFlags()
{
  counter = 0;
  hitsWithinPattern = 0;
  patternPrinted = false;
}

void activateSolenoid(int playCount)
{
  int shortDelay = 50;
  int longDelay = 200;

  for (int i = 0; i < playCount; ++i)
  {
    digitalWrite(solenoidPin, HIGH);
    delay(random(shortDelay, longDelay)); // Random delay within the specified range
    digitalWrite(solenoidPin, LOW);
    delay(random(shortDelay, longDelay)); // Random delay within the specified range
  }
}

String createMessage(bool ping, bool global, int address, int value, int sender)
{
  String dataToESP;

  dataToESP += String(ping);
  dataToESP += String(global);

  // adding address if necessary
  if (global)
  {
    dataToESP += "-";
  }
  else
  {
    dataToESP += String(address);
  }

  // adding value if necessary
  if (ping)
  {
    dataToESP += "000";
  }
  else
  {
    dataToESP += String(value / 100 % 10);
    dataToESP += String(value / 10 % 10);
    dataToESP += String(value % 10);
  }

  // adding my sender id
  dataToESP += String(sender);

  // finishing message
  dataToESP += "\n";

  return dataToESP;
}

void sendNetworkMessage(int address, int value)
{
  String message = createMessage(false, false, address, value, MY_SENDER_ID);
  Serial.print("Sending Message: ");
  Serial.println(message);
  Arduino_SoftSerial.print(message);
}

void handleNetworkCommunication()
{
  // RECEIVING MESSAGES FROM ESP
  bool dataFlag = false;
  while (Arduino_SoftSerial.available() > 0)
  {
    char c = Arduino_SoftSerial.read();
    node_recivedString += c;
    if (c == '\n')
    {
      dataFlag = true;
      break;
    }
  }
  if (dataFlag)
  {
    Serial.print("Received Message: ");
    Serial.println(node_recivedString);
    handleMessageFromNode(node_recivedString);
    dataFlag = false;
    node_recivedString = "";
  }

  // SENDING MESSAGES TO ESP
  if (deviceChange)
  {
    // Sending global ping
    if (globalPing)
    {
      String message = createMessage(true, true, 0, 0, MY_SENDER_ID);
      Serial.print("Sending Global Ping: ");
      Serial.println(message);
      Arduino_SoftSerial.print(message);
      globalPing = false;
    }

    // Sending direct ping
    if (directPing)
    {
      String message = createMessage(true, false, 0, 0, MY_SENDER_ID);
      Serial.print("Sending Direct Ping: ");
      Serial.println(message);
      Arduino_SoftSerial.print(message);
      directPing = false;
    }

    // Sending global message
    if (globalMessage != 0)
    {
      String message = createMessage(false, true, 0, globalMessage, MY_SENDER_ID);
      Serial.print("Sending Global Message: ");
      Serial.println(message);
      Arduino_SoftSerial.print(message);
      globalMessage = 0;
    }

    // Sending direct message
    if (directMessage != 0)
    {
      String message = createMessage(false, false, directMessageSender, directMessage, MY_SENDER_ID);
      Serial.print("Sending Direct Message: ");
      Serial.println(message);
      Arduino_SoftSerial.print(message);
      directMessage = 0;
    }

    deviceChange = false;
  }
}

void handleMessageFromNode(String message)
{
  // Reading Message from network as int variables
  bool isPing = (message[0] == '0') ? false : true;
  bool isGlobal = (message[1] == '0') ? false : true;
  int address;

  if (isGlobal)
  {
    address = -1;
  }
  else
  {
    address = (message[2] - '0');
  }
  int value;

  if (isPing)
  {
    value = 0;
  }
  else
  {
    value = 100 * (message[3] - '0') + 10 * (message[4] - '0') + (message[5] - '0');
  }
  int sender = message[6] - '0';

  Serial.println();

  // Handling message
  if (isPing && isGlobal)
  {
    handleGlobalPing();
  }
  else if (isPing)
  {
    handleDirectPing(sender);
  }
  else if (isGlobal)
  {
    handleGlobalMessage(sender, value);
  }
  else
  {
    handleDirectMessage(sender, value);
  }
  deviceChange = true;
}

void handleGlobalPing()
{
  // Implement your global ping action here
  Serial.println("Received Global Ping");
}

void handleDirectPing(int sender)
{
  // Implement your direct ping action here
  Serial.print("Received Ping from Device ID ");
  Serial.println(sender);
}

void handleGlobalMessage(int sender, int value)
{
  // Implement your global message handling action here
  Serial.print("Received Global Message from Device ID ");
  Serial.print(sender);
  Serial.print(" with Value ");
  Serial.println(value);

  // Example action: Toggle an LED based on the received value
  if (value == 1)
  {
    digitalWrite(2, HIGH); // Assuming LED is connected to pin 2
  }
  else
  {
    digitalWrite(2, LOW);
  }
}

// Modify the handleDirectMessage function to pass the sender value to activateSolenoid
void handleDirectMessage(int sender, int value)
{
  // Implement your direct message handling action here
  Serial.print("Received Direct Message from Device ID ");
  Serial.print(sender);
  Serial.print(" with Value ");
  Serial.println(value);

  // Check if the sender is in the specified range (0 to 6)
  if (sender >= 0 && sender <= 6)
  {
    // Trigger the solenoid based on the sender value
    activateSolenoid(sender);
  }

  // Example action: Toggle an LED based on the received value
  if (value == 1)
  {
    digitalWrite(3, HIGH); // Assuming LED is connected to pin 3
  }
  else
  {
    digitalWrite(3, LOW);
  }
}

void sendGlobalMessage(int value)
{
  // You can customize the value and sender as needed
  String message = createMessage(false, true, 0, value, MY_SENDER_ID);
  Serial.print("Sending Global Message: ");
  Serial.println(message);
  Arduino_SoftSerial.print(message);
}
