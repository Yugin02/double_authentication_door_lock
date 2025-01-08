/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <Wire.h>
#include <LCD-I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

LCD_I2C lcd(0x27, 16, 2);  // Default address of most PCF8574 modules, change according

// Initialize the Software Serial for the fingerprint sensor
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Array to store IDs
bool idExists = false;
int number = 0;
bool fingerprintsListed = false;
#define MAX_FINGERPRINTS 100
int fingerprintIDs[MAX_FINGERPRINTS];  // Array to store IDs
int totalStoredFingerprints = 0;
const int RELAY_PIN = A3;


void setup() {
  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();

  // Fingerprint sensor module setup
  Serial.begin(9600);  // Set the baud rate for the serial monitor

  pinMode(RELAY_PIN, OUTPUT);
  // Set the data rate for the fingerprint sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint OK");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    while (1) { delay(1); }  // Stop if sensor not found
  }

  delay(2000);
}

void loop() {
  // Check if fingerprints have been listed
  digitalWrite(RELAY_PIN, LOW);
  if (!fingerprintsListed) {
    finger.getTemplateCount();  // Get the number of stored fingerprints
    if (finger.templateCount == 0) {
      Serial.println("No fingerprint data found.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No finger print");
      lcd.setCursor(0, 1);
      lcd.print("data found..");
      delay(1000);
      enrollPagWara();
    } else {
      Serial.print("Fingerprints stored: ");
      Serial.println(finger.templateCount);
      listStoredFingerprints();
      fingerprintsListed = true;  // Set flag to true to stop further listing
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("waiting for");
      lcd.setCursor(0, 1);
      lcd.print("finger to scan");
      Serial.println("waiting to scan..");
    }
  }

  uint8_t fingerprintID = getFingerprintIDez();
  if (fingerprintID > 0 && fingerprintID != 255) {  // Check if fingerprint ID is valid (greater than 0)
    if (checkFingerprintExists(fingerprintID)) {
      // Access Granted
      digitalWrite(RELAY_PIN, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted!");
      lcd.setCursor(0, 1);
      lcd.print("User ID: ");
      lcd.print(fingerprintID);
      Serial.println("Access Granted!");
      delay(5000);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Denied!");
      Serial.println("Access Denied!");
    }

    // delay(5000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("waiting for");
    lcd.setCursor(0, 1);
    lcd.print("finger to scan");
  } else if (fingerprintID == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied!");
    Serial.println("Access Denied!");
  }
}


bool checkFingerprintExists(uint8_t id) {
  for (int i = 0; i < totalStoredFingerprints; i++) {
    if (fingerprintIDs[i] == id) {
      Serial.println(id);
      return true;  // ID found, return true
    }
  }
  Serial.println(id);
  return false;  // ID not found, return false
}


//list stored fingerprint IDs
void listStoredFingerprints() {
  totalStoredFingerprints = 0;  // Reset the count of stored fingerprints

  for (uint16_t id = 1; id <= finger.templateCount; id++) {
    uint8_t result = finger.loadModel(id);  // Try loading the fingerprint template by ID

    if (result == FINGERPRINT_OK) {
      Serial.print("Stored Fingerprint ID: ");
      Serial.println(id);

      if (totalStoredFingerprints < MAX_FINGERPRINTS) {
        fingerprintIDs[totalStoredFingerprints] = id;  // Store the ID in the array
        totalStoredFingerprints++;                     // Increment the count
      } else {
        Serial.println("Array limit reached. Cannot store more fingerprint IDs.");
        break;
      }

    } else if (result == FINGERPRINT_NOTFOUND) {
      Serial.print("No fingerprint found at ID: ");
      Serial.println(id);
    } else {
      Serial.print("Error reading fingerprint at ID: ");
      Serial.println(id);
    }
  }
}

// Returns -1 if failed, otherwise returns fingerprint ID #
uint8_t getFingerprintIDez() {
  uint8_t result = finger.getImage();

  if (result == FINGERPRINT_NOFINGER) {
    // Return -1 if no finger is placed on the sensor
    return -1;
  } else if (result == FINGERPRINT_OK) {
    result = finger.image2Tz();
    if (result == FINGERPRINT_OK) {
      result = finger.fingerFastSearch();
      if (result == FINGERPRINT_OK) {
        // Ensure the returned fingerprint ID is valid (within the stored range)
        if (finger.fingerID > 0 && finger.fingerID <= finger.templateCount) {
          return finger.fingerID;  // Return the valid fingerprint ID
        }
      }
    }
  }

  // Return 0 if no valid fingerprint is found or if an undefined fingerprint is scanned
  return 0;
}

// Function to enroll fingerprints pag wara pa
void enrollPagWara() {
  for (int i = 1; i <= 3; i++) {
    Serial.print("Starting enrollment for fingerprint ID: ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting for");
    lcd.setCursor(0, 1);
    lcd.print("fingerprint...");
    delay(1000);
    Serial.println(i);
    if (enrollFingerprint(i)) {
      Serial.print("Fingerprint with ID ");
      Serial.print(i);
      Serial.println(" enrolled successfully.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID: ");
      lcd.print(i);
      lcd.setCursor(0, 1);
      lcd.print("enrolled succesfully");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Failled to enroll");
      lcd.setCursor(0, 1);
      lcd.print("fingerprint ID: ");
      lcd.print(i);
      Serial.print("Failed to enroll fingerprint with ID ");
      Serial.println(i);
    }
  }
}

// Display the main screen on the LCD
void displayMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("fingerprint...");
  delay(2000);  // Short delay
}

// Function to enroll a fingerprint with a specific ID
bool enrollFingerprint(int id) {
  int result;

  Serial.print("Starting enrollment for fingerprint ID: ");
  Serial.println(id);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enroll Fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("ID: ");
  lcd.print(id);
  delay(2000);

  for (int attempt = 1; attempt <= 2; attempt++) {  // Enroll the same finger 2 times for accuracy
    Serial.print("Place your finger on the sensor for scan ");
    Serial.println(attempt);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("scan attemp:");
    lcd.print(attempt);
    delay(1000);

    while (finger.getImage() != FINGERPRINT_OK)
      ;  // Wait until a finger is placed on the sensor

    result = finger.image2Tz(attempt);  // Convert image to template
    if (result == FINGERPRINT_OK) {
      Serial.print("Fingerprint captured for scan ");
      Serial.println(attempt);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fingerprint captured for");
      lcd.setCursor(0, 1);
      lcd.print("scan: ");
      lcd.print(attempt);
      delay(2000);

    } else {
      Serial.println("Failed to capture fingerprint.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Failed");
      delay(2000);
      return false;  // Enrollment failed
    }

    if (attempt < 2) {
      Serial.println("Remove finger and place it again.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("remove and place");
      lcd.setCursor(0, 1);
      lcd.print("it again");
      delay(1000);  // Give time to remove the finger
      while (finger.getImage() == FINGERPRINT_NOFINGER)
        ;  // Wait until the finger is removed
    }
  }

  // Create the fingerprint model
  result = finger.createModel();
  if (result == FINGERPRINT_OK) {
    Serial.println("Fingerprint model created successfully!");
  } else {
    Serial.println("Error creating fingerprint model.");
    return false;  // Enrollment failed
  }

  // Store the fingerprint model in the database
  result = finger.storeModel(id);
  if (result == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("stored");
    lcd.setCursor(0, 1);
    lcd.print("successfully!");
    delay(2000);
    return true;  // Enrollment successful
  } else {
    Serial.println("Error storing fingerprint.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("error storing");
    lcd.setCursor(0, 1);
    lcd.print("fingerprint");
    delay(2000);
    return false;  // Enrollment failed
  }
}

uint8_t getFingerprintEnroll(int id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
