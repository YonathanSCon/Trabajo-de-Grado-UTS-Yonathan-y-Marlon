#include <Wire.h> // Incluir la librería Wire para la comunicación I2C
#include <Adafruit_Sensor.h> // Incluir la librería Adafruit_Sensor para los sensores
#include <DHT.h> // Incluir la librería DHT para el sensor DHT11
#include <WiFi.h> // Incluir la librería WiFi para la conexión WiFi
#include <FirebaseESP32.h> // Incluir la librería FirebaseESP32 para la comunicación con Firebase
#include <Adafruit_GFX.h> // Incluir la librería Adafruit_GFX para gráficos en dispositivos Adafruit
#include <Adafruit_SSD1306.h> // Incluir la librería Adafruit_SSD1306 para la pantalla OLED
#include <RTClib.h> // Incluir la librería RTClib para el RTC

#define SCREEN_WIDTH 128   // Definir la anchura de la pantalla OLED en píxeles
#define SCREEN_HEIGHT 64   // Definir la altura de la pantalla OLED en píxeles

#define DHTPIN 14        // Pin al que está conectado el sensor DHT11
#define DHTTYPE DHT11    // Tipo de sensor DHT

DHT dht(DHTPIN, DHTTYPE);   // Crear un objeto DHT para el sensor DHT11 con el pin especificado y el tipo de sensor definido

const int MQ9Pin = 35;    // Pin al que está conectado el sensor MQ-9
const int MQ135Pin = 32;  // Pin al que está conectado el sensor MQ-135

int mq9Baseline = 350;   // Valor base para el sensor MQ-9 en un hogar típico
int mq135Baseline = 300; // Valor base para el sensor MQ-135 en un hogar típico

const char *ssid = "ssidwifi"; // Cambiar por el nombre de tu red WiFi
const char *password = "contraseñaWifi"; // Cambiar por la contraseña de tu red WiFi

const char *host = "urlBDFirebase"; // Cambiar por la URL de tu base de datos en Firebase sin el "https://" del inicio ni el "/" del final
const char *auth = "tokenAutenticacionFirebase"; // Cambiar por el token de autenticación secreto de tu base de datos de Firebase

FirebaseData firebaseData;   // Crear un objeto FirebaseData para manejar los datos de Firebase

// Configuración para la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pines para SDA y SCL del RTC (DS3231)
const int SDA_PIN = 21;
const int SCL_PIN = 22;

// Objeto para el RTC (DS3231)
RTC_DS3231 rtc;

// Función que se ejecuta una vez al inicio del programa para realizar todas las configuraciones necesarias
void setup() {
  Serial.begin(115200);   // Inicializar la comunicación serial a una velocidad de 115200 baudios
  delay(10);   // Esperar 10 milisegundos


  // Inicializar los sensores
  dht.begin(); // Inicializar el sensor DHT11
  pinMode(MQ9Pin, INPUT); // Configurar el pin del sensor MQ-9 como entrada
  pinMode(MQ135Pin, INPUT); // Configurar el pin del sensor MQ-135 como entrada

  // Iniciar la comunicación con el RTC
  Wire.begin(SDA_PIN, SCL_PIN); // Iniciar la comunicación I2C con los pines SDA y SCL
  if (!rtc.begin()) { // Comprobar si hay error al iniciar el RTC
    Serial.println(F("Error al iniciar el RTC"));
  }

  // Verificar si el RTC ha perdido la configuración
  if (!rtc.begin()) { // Si el RTC no está configurado
    Serial.println("RTC no está configurado. Configurando...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Configurar el RTC con la fecha y hora actual
  }

  // Ajustar la hora del RTC con una compensación de 56 segundos debido al restraso respecto al computador
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)).unixtime() + 56); // Ajustar la hora del RTC

  // Iniciar la comunicación con la pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Inicializar la pantalla OLED
    Serial.println(F("Error al iniciar la pantalla OLED"));
  }

  display.display(); // Limpiar la pantalla

  // Conectar a WiFi
  WiFi.begin(ssid, password); // Conectar a la red WiFi
  while (WiFi.status() != WL_CONNECTED) { // Esperar hasta que se establezca la conexión
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conexión exitosa a WiFi");

  // Inicializar la conexión a Firebase
  Firebase.begin(host, auth); // Inicializar la conexión a Firebase
  delay(1000);

  // Calibrar los sensores de gas
  calibrateGasSensors(); // Llamar a la función para calibrar los sensores de gas
}

// Función que se ejecuta de manera continua en un ciclo infinito mientras el programa esté en ejecución.
void loop() {
  // Leer datos del sensor DHT11
  float humidity = dht.readHumidity(); // Leer la humedad
  float temperature = dht.readTemperature(); // Leer la temperatura

  // Leer datos del sensor MQ-9 y MQ-135
  int mq9Value = analogRead(MQ9Pin); // Leer el valor del sensor MQ-9
  int mq135Value = analogRead(MQ135Pin); // Leer el valor del sensor MQ-135

  // Leer fecha y hora del RTC
  DateTime now = rtc.now(); // Leer la fecha y hora actual del RTC
  int day = now.day(); // Obtener el día
  int month = now.month(); // Obtener el mes
  int year = now.year(); // Obtener el año
  int hour = now.hour(); // Obtener la hora
  int minute = now.minute(); // Obtener el minuto
  int second = now.second(); // Obtener el segundo

  // Ajustar los valores de los sensores de gas
  mq9Value = map(mq9Value, 0, 4095, 0, 1000); // Ajustar el valor del sensor MQ-9
  mq135Value = map(mq135Value, 0, 4095, 0, 1000); // Ajustar el valor del sensor MQ-135

  // Imprimir valores en el puerto serie
  Serial.print("Fecha y Hora: "); // Imprimir la fecha y hora
  Serial.print(day); // Imprimir el día
  Serial.print("/");
  Serial.print(month); // Imprimir el mes
  Serial.print("/");
  Serial.print(year); // Imprimir el año
  Serial.print(" ");
  Serial.print(hour); // Imprimir la hora
  Serial.print(":");
  Serial.print(minute); // Imprimir el minuto
  Serial.print(":");
  Serial.print(second); // Imprimir el segundo
  Serial.print(", Temperatura: ");
  Serial.print(temperature); // Imprimir la temperatura
  Serial.print("°C, Humedad: ");
  Serial.print(humidity); // Imprimir la humedad
  Serial.print("%, Valor MQ-9: ");
  Serial.print(mq9Value); // Imprimir el valor del sensor MQ-9
  Serial.print("ppm, Valor MQ-135: ");
  Serial.print(mq135Value); // Imprimir el valor del sensor MQ-135
  Serial.println("ppm");

  // Mostrar valores en la pantalla OLED
  display.clearDisplay(); // Limpiar la pantalla
  display.setTextSize(1); // Establecer el tamaño del texto
  display.setTextColor(SSD1306_WHITE); // Establecer el color del texto
  display.setCursor(0,0); // Establecer la posición del cursor
  display.print("Fecha: "); // Imprimir la etiqueta "Fecha"
  display.print(day); // Imprimir el día
  display.print("/");
  display.print(month); // Imprimir el mes
  display.print("/");
  display.print(year); // Imprimir el año
  display.setCursor(0,10); // Establecer la posición del cursor
  display.print("Hora: "); // Imprimir la etiqueta "Hora"
  display.print(hour); // Imprimir la hora
  display.print(":");
  display.print(minute); // Imprimir el minuto
  display.print(":");
  display.println(second); // Imprimir el segundo
  display.print("Temp: "); // Imprimir la etiqueta "Temp"
  display.print(temperature); // Imprimir la temperatura
  display.println("C"); // Imprimir la unidad de temperatura
  display.print("Humedad: "); // Imprimir la etiqueta "Humedad"
  display.print(humidity); // Imprimir la humedad
  display.println("%"); // Imprimir el símbolo de porcentaje
  display.print("MQ-9: "); // Imprimir la etiqueta "MQ-9"
  display.print(mq9Value); // Imprimir el valor del sensor MQ-9
  display.println(" ppm"); // Imprimir la unidad de concentración de gases
  display.print("MQ-135: "); // Imprimir la etiqueta "MQ-135"
  display.print(mq135Value); // Imprimir el valor del sensor MQ-135
  display.println(" ppm"); // Imprimir la unidad de concentración de gases
  display.display(); // Mostrar los cambios en la pantalla


  // Enviar datos a Firebase
  sendToFirebase(day, month, year, hour, minute, second, temperature, humidity, mq9Value, mq135Value); // Llamar a la función para enviar datos a Firebase

  delay(1000);  // Esperar 1 segundo entre lecturas
}

// Funcion para calibrar los sensores de gas
void calibrateGasSensors() {
  Serial.println("Calibrando sensores de gas. Asegúrate de que el entorno esté limpio y sin gas durante la calibración."); // Imprimir mensaje de calibración de los sensores de gas
  delay(2000); // Esperar 2 segundos

  // Calibrar el sensor MQ-9
  Serial.println("Calibrando MQ-9..."); // Imprimir mensaje de calibración del sensor MQ-9
  for (int i = 0; i < 10; i++) { // Realizar 10 lecturas para calibrar
    mq9Baseline += analogRead(MQ9Pin); // Sumar el valor leído al valor base
    delay(500); // Esperar medio segundo entre lecturas
  }
  mq9Baseline /= 10; // Calcular el promedio de los valores leídos
  Serial.print("Valor base MQ-9: "); // Imprimir el valor base del sensor MQ-9
  Serial.println(mq9Baseline); 

  // Calibrar el sensor MQ-135
  Serial.println("Calibrando MQ-135..."); // Imprimir mensaje de calibración del sensor MQ-135
  for (int i = 0; i < 10; i++) { // Realizar 10 lecturas para calibrar
    mq135Baseline += analogRead(MQ135Pin); // Sumar el valor leído al valor base
    delay(500); // Esperar medio segundo entre lecturas
  }
  mq135Baseline /= 10; // Calcular el promedio de los valores leídos
  Serial.print("Valor base MQ-135: "); // Imprimir el valor base del sensor MQ-135
  Serial.println(mq135Baseline);

  Serial.println("Calibración completa."); // Imprimir mensaje de calibración completa
}
  
// Esta función se encarga de enviar los datos recopilados (fecha, hora, temperatura, humedad, valores de MQ-9 y MQ-135) a Firebase.
// Recibe como parámetros la fecha, hora, temperatura, humedad, y los valores de los sensores de gas (MQ-9 y MQ-135).
// Las lecturas de los datos en la base de datos de los sensores MQ9 y MQ135 se dan en la unidad "ppm", para la temperatura es en "°C" y la humedad es en "%"
void sendToFirebase(int day, int month, int year, int hour, int minute, int second, float temperature, float humidity, int mq9Value, int mq135Value) {
  // Enviar datos a Firebase
  String path = "/datos"; // Establecer la ruta en la base de datos de Firebase
  String json = "{\"fecha\":\"" + String(day) + "/" + String(month) + "/" + String(year) +
                "\",\"hora\":\"" + String(hour) + ":" + String(minute) + ":" + String(second) +
                "\",\"temperatura\":" + String(temperature) +
                ",\"humedad\":" + String(humidity) +
                ",\"mq9\":" + String(mq9Value) +
                ",\"mq135\":" + String(mq135Value) + "}"; // Crear el objeto JSON con los datos

  FirebaseJson jsonBuffer; // Crear un objeto de tipo FirebaseJson
  jsonBuffer.setJsonData(json); // Establecer los datos JSON

  if (Firebase.pushJSON(firebaseData, path, jsonBuffer)) { // Enviar los datos JSON a Firebase
    Serial.println("Datos enviados a Firebase correctamente"); // Imprimir mensaje de éxito
  } else {
    Serial.println("Error al enviar datos a Firebase"); // Imprimir mensaje de error
    Serial.println("Motivo: " + firebaseData.errorReason()); // Imprimir el motivo del error
  }
}
