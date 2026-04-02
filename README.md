# 🚀 Giao tiếp với phần mềm PC Simulator KIT IoT (STM32F401RE)

## 📌 Tổng quan

Dự án này là phiên bản nâng cấp từ **Smart Touch Switch Simulator**, mở rộng thêm khả năng **giao tiếp hai chiều với phần mềm mô phỏng trên máy tính (PC Simulator)** thông qua giao thức **UART**.

Hệ thống cho phép:

* Giám sát dữ liệu môi trường theo thời gian thực
* Điều khiển thiết bị từ máy tính
  → Mô phỏng một hệ thống **IoT hoàn chỉnh (Device ↔ Host)** tương tự ứng dụng trên điện thoại.

---

## 🏗 Kiến trúc hệ thống

<p align="center">
  <img src="docs/images/system_architecture.png" width="600"/>
</p>

---

## 🔄 Luồng hoạt động hệ thống

<p align="center">
  <img src="docs/images/system_flow.png" width="600"/>
</p>

---

## 📡 Giao tiếp UART

<p align="center">
  <img src="docs/images/uart_protocol.png" width="600"/>
</p>

---

## 🎯 Mục tiêu dự án

* Xây dựng giao tiếp **UART giữa STM32 và PC**
* Thiết kế hệ thống **truyền thông theo cấu trúc bản tin**
* Áp dụng **function callback** để xử lý lệnh
* Mô phỏng hệ thống **IoT thực tế: giám sát & điều khiển từ xa**

---

## 🖥 Mô tả hệ thống

Hệ thống gồm:

* **STM32F401RE (Nucleo) + KIT IoT**
* **Phần mềm PC Simulator KIT IoT**

Phần mềm PC có chức năng:

* Hiển thị nhiệt độ, độ ẩm, ánh sáng
* Điều khiển LED, buzzer, LCD
* Vẽ đồ thị dữ liệu theo thời gian thực

---

## 🔄 Chức năng chính

### 1️⃣ Hiển thị thông tin thiết bị

* Gửi thông tin KIT IoT lên phần mềm PC qua UART

### 2️⃣ Điều khiển bằng nút nhấn & phản hồi trạng thái

* Nhấn B1, B2, B4, B5:

  * Bật/tắt các màu LED: **RED / GREEN / BLUE / WHITE**
  * Gửi bản tin cập nhật trạng thái lên PC
* Nhấn lần nữa → tắt LED tương ứng

### 3️⃣ Còi buzzer

* Mỗi lần nhấn nút:

  * Phát **1 tiếng bíp**
  * Gửi trạng thái buzzer lên PC

### 4️⃣ Điều khiển từ PC

Nhận lệnh từ phần mềm để:

* Bật/tắt LED RGB
* Điều khiển buzzer
* Hiển thị nội dung lên LCD

### 5️⃣ Đọc dữ liệu cảm biến

* Nhiệt độ & độ ẩm (**I2C**)
* Cường độ ánh sáng (**ADC + DMA**)
* Chu kỳ lấy mẫu: **1 giây**
* Gửi dữ liệu lên PC để hiển thị dạng **đồ thị realtime**

---

## 🏗 Kiến trúc phần mềm

* Mô hình **Event-driven**
* **State Machine** (Startup / Idle / Communication)
* Tầng giao tiếp UART:

  * Xử lý bản tin
  * Phân tích dữ liệu (parsing)
* Sử dụng **callback function** để xử lý lệnh

---

## 🛠 Công nghệ sử dụng

* MCU: **STM32F401RE**
* Ngôn ngữ: **Embedded C**
* UART – Giao tiếp với PC
* GPIO – Nút nhấn
* PWM – Điều khiển LED
* SPI – LCD
* I2C – Cảm biến nhiệt độ & độ ẩm
* ADC + DMA – Cảm biến ánh sáng
* Timer – Lập lịch tác vụ

---

## 📊 Luồng hoạt động

1. Khởi tạo hệ thống
2. Gửi thông tin thiết bị lên PC
3. Vòng lặp chính:

   * Xử lý nút nhấn
   * Nhận & xử lý UART
   * Đọc cảm biến định kỳ
   * Gửi dữ liệu realtime
4. Cập nhật LED, buzzer, LCD

---

## 🔗 Phần mềm PC Simulator

Tải và chạy phần mềm **PC Simulator KIT IoT** để:

* Giám sát dữ liệu
* Điều khiển thiết bị

→ Phần mềm đóng vai trò như **HOST (trung tâm điều khiển)**

---

## 📚 Kiến thức đạt được

* Hiểu và triển khai **giao thức UART**
* Thiết kế hệ thống **truyền thông theo bản tin**
* Sử dụng **con trỏ hàm (callback)** trong embedded
* Kỹ năng xây dựng hệ thống IoT:

  * Giao tiếp thiết bị - trung tâm
  * Điều khiển từ xa
  * Giám sát thời gian thực

---

## 🔄 Phiên bản trước

👉 Smart_Touch_Switch_Simulator (không có giao tiếp PC)

---

## 📷 Demo

(Thêm ảnh hoặc video demo tại đây)

---

