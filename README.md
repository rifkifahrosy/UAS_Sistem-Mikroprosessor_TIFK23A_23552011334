# ESP32 IoT Smart Controller (MQTT + FreeRTOS)

Proyek ini adalah implementasi sistem kendali perangkat keras berbasis IoT menggunakan **ESP32 (Wemos D1 R32)**. Sistem ini mendemonstrasikan manajemen mikroprosesor tingkat lanjut dengan menerapkan **Multitasking (FreeRTOS)**, **External Interrupts**, **PWM Control**, dan **Non-Volatile Storage (NVS)**, serta terintegrasi dengan Cloud Dashboard melalui protokol **MQTT**.

## Fitur Utama

Sistem ini dirancang untuk memenuhi kriteria manajemen proses yang efisien:

* **Dual-Core Multitasking (FreeRTOS):**
    * **Core 0 (TaskNetwork):** Menangani koneksi WiFi dan protokol MQTT (Publish/Subscribe) agar komunikasi data *real-time* tidak terputus.
    * **Core 1 (TaskControl):** Menangani logika input fisik, PWM LED, dan akses memori dengan prioritas tinggi.
* **Hardware Interrupt (ISR):** Input tombol menggunakan mekanisme interupsi (bukan *polling*) untuk respons instan dan efisiensi CPU.
* **Persistent Memory (NVS):** Status terakhir LED disimpan di Flash Memory (Library `Preferences`), sehingga sistem memulihkan kondisi terakhir saat *reboot* (mati listrik).
* **Cloud Connectivity:** Terhubung ke **HiveMQ Public Broker** untuk memantau status perangkat dari jarak jauh via internet.

## Perangkat Keras (Hardware)

* **Mikrokontroler:** ESP32 (Wemos D1 R32 / ESPDUINO-32)
* **Output:** LED (Pin IO12 - PWM Channel)
* **Input:** Push Button (Pin IO14 - Pull-up Internal)
* **Koneksi:** WiFi 2.4GHz
* **Komponen Lain:** Resistor 220Ω, Breadboard, Kabel Jumper.

### Skema Wiring
| Komponen | Pin ESP32 | Keterangan |
| :--- | :--- | :--- |
| **LED (+) Anoda** | GPIO 12 | PWM Output |
| **LED (-) Katoda** | GND | Via Resistor |
| **Tombol (Kaki 1)** | GPIO 14 | Interrupt Input (Pull-up) |
| **Tombol (Kaki 2)** | GND | Common Ground |

## Arsitektur Sistem (System Flow)

Sistem menggunakan arsitektur **Parallel Processing**. Diagram Swimlane di bawah ini menggambarkan pembagian tugas antara **Core 0** dan **Core 1**:

1.  **Inisialisasi:** Setup Hardware & Create Tasks.
2.  **Lane Control (Tengah):** Mengelola logika perubahan PWM dan penyimpanan memori saat interupsi terjadi.
3.  **Lane Network (Bawah):** Mengirimkan data ke MQTT Broker hanya saat ada perintah dari Lane Control.
