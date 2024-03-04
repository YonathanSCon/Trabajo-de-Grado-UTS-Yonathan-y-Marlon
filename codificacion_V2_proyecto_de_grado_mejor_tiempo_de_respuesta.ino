#include <Wire.h> // Incluir la librería Wire para la comunicación I2C
#include <Adafruit_Sensor.h> // Incluir la librería Adafruit_Sensor para los sensores
#include <DHT.h> // Incluir la librería DHT para el sensor DHT11
#include <WiFi.h> // Incluir la librería WiFi para la conexión WiFi
#include <FirebaseESP32.h> // Incluir la librería FirebaseESP32 para la comunicación con Firebase
#include <Adafruit_GFX.h> // Incluir la librería Adafruit_GFX para gráficos en dispositivos Adafruit
#include <Adafruit_SSD1306.h> // Incluir la librería Adafruit_SSD1306 para la pantalla OLED
#include <RTClib.h> // Incluir la librería RTClib para el RTC

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DHTPIN 14        // Pin al que está conectado el sensor DHT11
#define DHTTYPE DHT11    // Tipo de sensor DHT

DHT dht(DHTPIN, DHTTYPE);

const int MQ9Pin = 35;    // Pin al que está conectado el sensor MQ-9
const int MQ135Pin = 32;  // Pin al que está conectado el sensor MQ-135

int mq9Baseline = 350;   // Valor base para el sensor MQ-9 en un hogar típico
int mq135Baseline = 300; // Valor base para el sensor MQ-135 en un hogar típico

const char *ssid = "ssidwifi"; // Cambiar por el nombre de tu red WiFi
const char *password = "contraseñaWifi"; // Cambiar por la contraseña de tu red WiFi

const char *host = "urlBDFirebase"; // Cambiar por la URL de tu base de datos en Firebase sin el "https://" del inicio ni el "/" del final
const char *auth = "tokenAutenticacionFirebase"; // Cambiar por el token de autenticación secreto de tu base de datos de Firebase

FirebaseData firebaseData;

// Configuración para la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pines para SDA y SCL del RTC (DS3231)
const int SDA_PIN = 21;
const int SCL_PIN = 22;

// Objeto para el RTC (DS3231)
RTC_DS3231 rtc;

unsigned long previousMillis = 0;  // Almacena el tiempo en milisegundos desde el inicio del programa en el que ocurrió la última ejecución de cierta acción.
const long interval = 1000;        // Define el intervalo de tiempo deseado entre las ejecuciones de ciertas acciones en el programa, en milisegundos.

void setup() {
  Serial.begin(115200);
  delay(10);

  // Inicializar los sensores
  dht.begin();
  pinMode(MQ9Pin, INPUT);
  pinMode(MQ135Pin, INPUT);

  // Iniciar la comunicación con el RTC
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!rtc.begin()) {
    Serial.println(F("Error al iniciar el RTC"));
  }

  // Verificar si el RTC ha perdido la configuración
  if (!rtc.begin()) {
    Serial.println("RTC no está configurado. Configurando...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Ajustar la hora del RTC con una compensación de 56 segundos
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)).unixtime() + 56);

  // Iniciar la comunicación con la pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error al iniciar la pantalla OLED"));
  }

  display.display(); // Limpiar la pantalla

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conexión exitosa a WiFi");

  // Inicializar la conexión a Firebase
  Firebase.begin(host, auth);
  delay(1000);

  // Calibrar los sensores de gas
  calibrateGasSensors();
}

void loop() {
  // Almacena el tiempo actual en milisegundos desde el inicio del programa.
  // Esta variable se utiliza para coordinar acciones basadas en el tiempo.
  unsigned long currentMillis = millis();

  // Verificar si ha pasado el intervalo de tiempo
  if (currentMillis - previousMillis >= interval) {
    // Actualizar el tiempo anterior
    previousMillis = currentMillis;

    // Leer datos del sensor DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Leer datos del sensor MQ-9 y MQ-135
    int mq9Value = analogRead(MQ9Pin);
    int mq135Value = analogRead(MQ135Pin);

    // Leer fecha y hora del RTC
    DateTime now = rtc.now();
    int day = now.day();
    int month = now.month();
    int year = now.year();
    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();

    // Ajustar los valores de los sensores de gas
    mq9Value = map(mq9Value, 0, 4095, 0, 1000);
    mq135Value = map(mq135Value, 0, 4095, 0, 1000);

    // Imprimir valores en el puerto serie
    Serial.print("Fecha y Hora: ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.print(year);
    Serial.print(" ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.print(second);
    Serial.print(", Temperatura: ");
    Serial.print(temperature);
    Serial.print("°C, Humedad: ");
    Serial.print(humidity);
    Serial.print("%, Valor MQ-9: ");
    Serial.print(mq9Value);
    Serial.print("ppm, Valor MQ-135: ");
    Serial.print(mq135Value);
    Serial.println("ppm");

    // Mostrar valores en la pantalla OLED
    display.clearDisplay(); // Limpiar la pantalla
    display.setTextSize(1); // Establecer el tamaño del texto
    display.setTextColor(SSD1306_WHITE); // Establecer el color del texto
    display.setCursor(0,0); // Establecer la posición del cursor
    display.print("Fecha: ");
    display.print(day); // Imprimir el día
    display.print("/");
    display.print(month); // Imprimir el mes
    display.print("/");
    display.print(year); // Imprimir el año
    display.setCursor(0,10); // Establecer la posición del cursor
    display.print("Hora: ");
    display.print(hour); // Imprimir la hora
    display.print(":");
    display.print(minute); // Imprimir el minuto
    display.print(":");
    display.println(second); // Imprimir el segundo
    display.print("Temp: ");
    display.print(temperature); // Imprimir la temperatura
    display.println("C");
    display.print("Humedad: ");
    display.print(humidity); // Imprimir la humedad
    display.println("%");
    display.print("MQ-9: ");
    display.print(mq9Value); // Imprimir el valor del sensor MQ-9
    display.println(" ppm");
    display.print("MQ-135: ");
    display.print(mq135Value); // Imprimir el valor del sensor MQ-135
    display.println(" ppm");
    display.display(); // Mostrar los cambios en la pantalla

    // Enviar datos a Firebase
    sendToFirebase(day, month, year, hour, minute, second, temperature, humidity, mq9Value, mq135Value);
  }
}

void calibrateGasSensors() {
  Serial.println("Calibrando sensores de gas. Asegúrate de que el entorno esté limpio y sin gas durante la calibración.");
  delay(2000);

  // Calibrar el sensor MQ-9
  Serial.println("Calibrando MQ-9...");
  for (int i = 0; i < 10; i++) {
    mq9Baseline += analogRead(MQ9Pin);
    delay(500);
  }
  mq9Baseline /= 10;
  Serial.print("Valor base MQ-9: ");
  Serial.println(mq9Baseline);

  // Calibrar el sensor MQ-135
  Serial.println("Calibrando MQ-135...");
  for (int i = 0; i < 10; i++) {
    mq135Baseline += analogRead(MQ135Pin);
    delay(500);
  }
  mq135Baseline /= 10;
  Serial.print("Valor base MQ-135: ");
  Serial.println(mq135Baseline);

  Serial.println("Calibración completa.");
}

void sendToFirebase(int day, int month, int year, int hour, int minute, int second, float temperature, float humidity, int mq9Value, int mq135Value) {
  // Enviar datos a Firebase
  String path = "/datos";
  String json = "{\"fecha\":\"" + String(day) + "/" + String(month) + "/" + String(year) +
                "\",\"hora\":\"" + String(hour) + ":" + String(minute) + ":" + String(second) +
                "\",\"temperatura\":" + String(temperature) +
                ",\"humedad\":" + String(humidity) +
                ",\"mq9\":" + String(mq9Value) +
                ",\"mq135\":" + String(mq135Value) + "}";

  FirebaseJson jsonBuffer;
  jsonBuffer.setJsonData(json);

  if (Firebase.pushJSON(firebaseData, path, jsonBuffer)) {
    Serial.println("Datos enviados a Firebase correctamente");
  } else {
    Serial.println("Error al enviar datos a Firebase");
    Serial.println("Motivo: " + firebaseData.errorReason());
  }
}
