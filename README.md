# Trabajo-de-Grado-UTS-Yonathan-y-Marlon
Este repositorio es presentado por los estudiantes Yonathan y Marlon siendo un prototipo para un Sistema de Monitoreo IoT para la calidad del aire en la UTS, para optar por el título de Tecnólogo en Desarrollo de Sistemas Informáticos
Para ejecutar el codigo, debe ingresarse a Arduino, escoger la tarjeta de desarrollo ESP32 Dev Module, se deben descargar las siguientes librerias:
- Adafruit BusIO versión 1.15.0 por Adafruit
- Adafruit GFX Library versión 1.11.9 por Adafruit
- Adafruit SSD1306 versión 2.5.9 por Adafruit
- Adafruit Unified Sensor versión 1.1.14 por Adafruit
- DHT sensor library versión 1.4.6 por Adafruit
- Firebase ESP32 Client versión 4.3.10 por Mobizt
- RTClib versión 2.1.3 por Adafruit

Una vez descargadas las librerias correspondientes hay que tener en cuenta 4 lineas de código importantes. Para la parte de la conexión a la red wifi a la que se desea conectar con la tarjeta de desarrollo, se deben tomar en cuenta las siguientes lineas de código, donde "ssid" es el nombre de la red wifi y "password" es la contraseña de ea red wifi. Estas lineas son las siguientes:

- const char *ssid = "ssidwifi"; // Cambiar por el nombre de tu red WiFi
- const char *password = "contraseñaWifi"; // Cambiar por la contraseña de tu red WiFi

Ahora bien, una vez hecho lo anterior, se procede a dar las credenciales para establecer la conexión con la base de datos de Firebase al que se desea conectar. Estas serían "host", donde es la url de la base de datos y "auth", que es el token de autenticación secreto de la base de datos al que se está conectado. Este último se encuentra dentro de Firebase en la Configuración de proyecto > Cuentas de servicio > Secretos de la base de datos > Secretos, y sería el conjunto de caracteres que está oculto y siplemente se le da clic en mostrar y esta listo para copiarse y pegarse dentro de la linea de código correspondiente. Las lineas de código mencionadas son las siguientes:

- const char *host = "urlBDFirebase"; // Cambiar por la URL de tu base de datos en Firebase sin el "https://" del inicio ni el "/" del final
- const char *auth = "tokenAutenticacionFirebase"; // Cambiar por el token de autenticación secreto de tu base de datos de Firebase

Antes de proceder con el último paso, debe mencionarse que en el caso que se quieran cambiar los pines del SDA y SCL del RTC dentro del código, estos pines deben ser los mismos a los pines SDA (Serial Data) y SCL (Serial Clock) a los que estara conectada la pantalla oled, lo mismo pasa al invertirse la acción, si se cambian los pines SDA y SCL de la pantalla, se cambian dentro del codigo los pines asignados al SDA y SCL del RTC. Esto es ALTAMENTE recomendable establecerlo antes de proceder con el montaje del PCB, ya que después no se deben realizar cambios dentro del codigo para estos los pines descritos.

Por último se ejecuta el código para que comience con su funcionalidad
