Tìm kiếm đường đi trên bản đồ hust
Đây là một ứng dụng server viết bằng C++ sử dụng thư viện `httplib` để triển khai API tìm đường đi ngắn nhất và tìm nhà xe gần nhất còn chỗ trống. Dữ liệu mô hình được xây dựng dựa trên cấu trúc đồ thị (graph) đại diện cho bản đồ khuôn viên có các điểm giao và nhà xe.

---

//Chức năng chính

- Tìm đường đi ngắn nhất giữa hai điểm trong đồ thị (Dijkstra).
- Tìm nhà xe gần nhất còn chỗ trống từ vị trí hiện tại (qua toạ độ).
- API cập nhật số lượng xe đang đỗ trong từng nhà xe.
- Server trả về đường đi dưới dạng danh sách toạ độ JSON.
- Phục vụ tĩnh file `index.html` như giao diện frontend.

---

//Cấu trúc dự án

```
project/
├── main.cpp              # File chính chạy server
├── index.html            # Giao diện web
├── README.md             # Tài liệu hướng dẫn
├── json.hpp              # Thư viện nlohmann/json
├── httplib.h             # Thư viện HTTP server
```

---

//Hướng dẫn chạy
1.Cài đặt thư viện

Không cần build thư viện ngoài. Chỉ cần có 2 file:
- [`json.hpp`](https://github.com/nlohmann/json/releases)
- [`httplib.h`](https://github.com/yhirose/cpp-httplib)

2.Biên dịch

```bash
g++ main.cpp -o parking_server -std=c++11
```
3.Chạy server

```bash
./parking_server
```

Sau khi chạy, server hoạt động tại `http://localhost:8080`

---

## 🌐 API Endpoint

### `GET /path?from={id1}&to={id2}`

Tìm đường đi ngắn nhất từ điểm `id1` đến `id2`.

- **Phản hồi:** Mảng JSON các toạ độ:
```json
[
  {"lat": 21.005, "lng": 105.845},
  {"lat": 21.004, "lng": 105.844}
]
```

---

### `POST /update-occupied`

Cập nhật số lượng xe đang đỗ ở một nhà xe.

- **Request body:**
```json
{
  "id": "Nha xe D4-6",
  "occupied": 100
}
```

- **Response:** `"Updated"` hoặc `"Point not found"`

---

### `POST /nearest`

Gửi vị trí hiện tại để tìm đường đến nhà xe gần nhất còn chỗ.

- **Request body:**
```json
{
  "lat": 21.003,
  "lng": 105.846
}
```

- **Response:**
```json
{
  "parkingID": "Nha xe B1",
  "status": "10/100",
  "path": [
    {"lat": 21.003, "lng": 105.846},
    {"lat": 21.004, "lng": 105.845}
  ]
}
```

---

//Kỹ thuật sử dụng

- **Đồ thị:** Mỗi nút là một địa điểm, mỗi cạnh có trọng số là số bước đi/thời gian ước lượng.
- **Dijkstra algorithm:** Tìm đường đi ngắn nhất giữa 2 nút.
- **Haversine Formula:** Tính khoảng cách địa lý giữa 2 toạ độ.
- **HTTP Server:** Thư viện `httplib.h` xử lý REST API.
- **JSON:** Sử dụng `nlohmann::json` để encode/decode dữ liệu.

---

//Bản đồ mô phỏng

![image](https://github.com/user-attachments/assets/5bc1ca91-e81f-4ac6-86dc-553ee389947f)

//Mục tiêu trong tương lai

- Thêm tính năng tìm nhiều nhà xe gần nhất.
- Tích hợp dữ liệu thực tế (API GPS).
- Giao diện hiển thị đường đi trực quan hơn.
- Thêm caching để giảm tính toán lặp.

---

//Thông tin liên hệ

Tác giả: Đỗ Thùy Linh
Hoàng Trọng Lâm
Nguyễn Trung Kiên
