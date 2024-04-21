WIFI控制-ESP8266_V2
===

## 硬體
- 開發板: WEMOS D1 R2
- MCU: ESP8266

## 零件
- 4pin 風扇
  1. pwm輸出: 接風扇的pin4
  2. 風扇的正極: 請接5V以上的穩定供電源
- onboard LED
  * 持續亮: WIFI正在連線
  * 反覆亮: 請求資料成功
  * 暗: 不請求/請求資料失敗

## 定義部分
1. 向後端請求開關狀態
   * 可請求部分
     * Fan1: 風扇1
     * Fan2: 風扇2
2. 收到後，再依開關狀態而決定
   * 1: 開
   * 0: 關
## 相關檔案
- Tips: 請在編譯前設置完連線設置
  * V2: 加入wifi連線
- 'src'
  * 'src/main.cpp': 主程式
  * 'src/espCN.h': 伺服器&WiFi連線設置