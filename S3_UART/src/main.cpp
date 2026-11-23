/**
 * S3 ↔ MSPM0G3507 UART0 通信测试
 * S3 GPIO16(RX) ← MSPM0 PA10(TX)
 * S3 GPIO17(TX) → MSPM0 PA11(RX)
 */
#include <Arduino.h>

#define S3_RX 4
#define S3_TX 5

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, S3_RX, S3_TX);
    delay(500);
    Serial.println("\n=== S3 UART Test ===");
    Serial.printf("RX=GPIO%d TX=GPIO%d\n", S3_RX, S3_TX);
}

void loop() {
    /* MSPM0 → S3 → PC: 转发MSPM0数据到串口监视器 */
    while(Serial2.available()) {
        char c = Serial2.read();
        Serial.write(c);
    }
    /* PC → S3 → MSPM0: 从串口监视器发送到MSPM0 */
    while(Serial.available()) {
        char c = Serial.read();
        Serial2.write(c);
    }
    /* 每2秒发测试消息到MSPM0 */
    static unsigned long last = 0;
    if(millis() - last > 2000) {
        last = millis();
        Serial2.print("hello from S3\r\n");
        Serial.println("[sent: hello]");
    }
}
