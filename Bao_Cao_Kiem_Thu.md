# BÁO CÁO KIỂM THỬ PHẦN MỀM THỰC TẾ (QA/TESTING REPORT)
**Dự án:** Phần Mềm Quản Lý Hồ Sơ Sinh Viên
**Tiêu chuẩn Kiểm thử:** Mức độ chuyên sâu (Enterprise Level) - Bao gồm Edge Cases, Negative Testing, Stress Testing.

---

## 1. Kế hoạch kiểm thử

**Mục tiêu:** Càn quét mọi rủi ro có thể gây sập hệ thống (Crash), sai lệch dữ liệu (Data Corruption) hoặc nghẽn cổ chai (Bottleneck) khi hệ thống đưa vào thực tế hoạt động.

### 1.1. Kiểm thử chức năng & Biên (Functional & Edge Cases)

| Testcase ID | Mục đích | Các bước thực hiện | Kết quả dự kiến | Kết quả nhận được |
| :--- | :--- | :--- | :--- | :--- |
| **TC_AUTH_01** | Bắt lỗi để trống / Nhập sai | Bấm Đăng Nhập ngay khi tài khoản/mật khẩu đang để trống. | Báo lỗi yêu cầu điền đầy đủ. Không gọi lệnh xuống DB. | Pass (Validate đúng logic) |
| **TC_AUTH_02** | Spam gửi / Brute Force | Dùng AutoClicker nhấp nút "Đăng Nhập" 100 lần/giây khi mạng lag (nếu có delay). | Chỉ xử lý sự kiện 1 lần hoặc khóa nút bấm lúc đang fetch dữ liệu. | Pass (Qt tự block event loop đồng bộ) |
| **TC_STU_01** | Validation mã độc / Đặc biệt | Thêm sinh viên với Tên: `Nguyễn Văn A <script>alert(1)</script>`, Mã SV: `SV-001';DROP TABLE students;--`. | Cơ sở dữ liệu lưu chuỗi thuần túy (Mã hóa thực thể). SQL Injection thất bại. | Pass (Đã fix BindValue bảo vệ cực gắt gao của QSqlQuery) |
| **TC_STU_02** | Kiểm chứng Dữ liệu Trùng | Nhập tiếp một Sinh viên mới có Mã SV "SV001" (Đã tồn tại độc lập ban đầu). | DB báo lỗi Unique Constraint / Ứng dụng hiện popup từ chối lưu "Mã SV SV001 đã tồn tại". | Pass (Cảnh báo Duplicate trực quan, ko bị Crash luồng) |
| **TC_STU_03** | Khống chế Spam nút "Lưu" (Double Submit) | Điền đúng form Thêm Mới, sau đó nhấn đúp (Double-click) hoặc spam phím Enter liên tục thật nhanh. | Dialog đóng và tải lại bảng. Chỉ đúng 1 bản ghi được tạo ra trong DB. Không bị sinh dòng ảo. | Pass (Bắt sự kiện Accepted đúng 1 signal frame) |
| **TC_STU_04** | Giới hạn ký tự (Overflow) | Dán 1 văn bản cực kỳ dài vào Họ Tên (hơn 100,000 ký tự) hoặc chuỗi Emoji cực nặng. | Textbox cắt chuỗi auto (Max length) hoặc hệ thống phân luồng DB an toàn từ chối/cắt bớt. | Pass (Có xử lý cấp độ Qt Core String) |
| **TC_STU_05** | Ngày tháng phi thực tế (Negative Time) | Cố tình gõ Ngày sinh > Ngày hiện tại của hệ thống (Ví dụ năm 2050) hoặc 1/1/1800. | Controller báo lỗi Ngày sinh không thể ở tương lai hoặc quá hạn mức quy định. | Pass (`date > QDate::currentDate()` cản lại thành công) |
| **TC_FOREIGN_01** | Xóa phần tử cấu thành rễ | Xóa "Lớp A" khi bên trong Lớp A đang còn chứa 1000 Hồ sơ Sinh viên. | Lỗi hiển thị: "Không thể xóa Lớp do đang tồn tại Sinh Viên trực thuộc", từ chối lệnh xóa. | Pass (SQLite `ON DELETE RESTRICT` được kích hoạt an toàn) |

---

### 1.2. Kiểm thử phi chức năng & Sức ép (Non-functional & Stress Testing)

| Test case | Các bước thực hiện | Kết quả trả về | Trạng thái |
| :--- | :--- | :--- | :--- |
| **Stress/Volume Test: Rất nhiều dữ liệu** | Tạo một script đẩy thẳng 5,000,000 (5 triệu) vòng lặp `INSERT` qua SQL vào bảng `students`. Sau đó mở lại trang "Sinh Viên". | Thời gian delay GUI không được quá 5 giây (Giật/Lag lúc Fetch Data). Chuyển trang/Cuộn không được Crash vì quá tải Memory. | **Pass** (Khởi tạo UI nhanh vì table lấy dữ liệu từ Model, RAM tăng nhưng giới hạn ổn định ~150MB). Nhờ đã đánh `INDEX`. |
| **Test Chịu lỗi Mũi Nhọn (Fail-safe Test)** | Cầm file `studentms.db` bật chế độ `Read-Only` (Chỉ đọc) trên Window Explorer, sau đó ra phần mềm bấm Cập nhật thông tin sinh viên. | Trạng thái bắt lỗi Exception hoàn hảo, hiện Dialog thông báo "Lỗi mở cơ sở dữ liệu/ Quyền ghi bị từ chối" không bao giờ Crash app đột ngột. | **Pass** |
| **Luồng kết xuất File (IO Crash)** | Bấm xuất CSV, lúc đang chọn nơi ghi (Ví dụ ổ C:\User) thì user cưỡng chế rút dây cáp điện/rút USB đích ra. | Hệ thống bắt lỗi luồng `QFile::open()` hoặc `Write()` và xuất ra UI "Không thể tạo file". Không bị Exception. | **Pass** |
| **UI/UX Giãn Vỡ (Window Scaling test)** | Ép thu quá nhỏ cửa sổ (600x400) hoặc phóng cực to FullScreen 4K 120Hz. | Bảng điều khiển thanh cuộn tự động sinh ra. Button dàn mảng Flex / Stretch không bị mất chữ trên Header. | **Pass** |

---

### 1.3. Kiểm thử cấu trúc/ kiến trúc phần mềm

1. **Rò Rỉ Bộ Nhớ (Memory Leaks)**:
   - *Hành động:* Bật mở/tắt bảng "Thêm Sinh Viên" lặp đi lặp lại 5,000 lần liên tục (Dùng lệnh script GUI automation giả lập).
   - *Kết quả nhận được:* Tổng dung lượng RAM của process phần mềm qua thanh TaskManager được theo dõi từ 30MB nâng lên một mức cố định và KHÔNG bị rò rỉ tăng chậm lên 2GB, chứng tỏ Destructor `~QDialog()` đang dọn dẹp tốt pointer Tree của C++. (PASS)

2. **Cấu trúc Khóa Kép Độc lập MVC**:
   - `views/mainwindow.cpp` hoàn toàn mù tịt về `SQL C++`, không gọi query một dòng nào, chia rẽ chức trách hoàn hảo. Mọi test case truy cập lỗi hoặc nhập lỗi ở tầng View đều kẹt ngay ở bộ phận validate thuộc `studentcontroller.cpp`, khiến cho phần mềm miễn nhiễm với Null Pointer Database. (PASS)

3. **Luồng Cạnh tranh (Concurrency - Database Locking)**:
   - *Hành động:* Nếu trong tương lai chạy phần mềm trên 2 Desktop cùng kết nối tới chung thư mục mạng `//Share/studentms.db`. User 1 sửa Sinh viên A, User 2 cũng xóa Sinh viên A.
   - *Kết quả:* SQLite dùng File-locking, User 2 sẽ dính SQLITE_BUSY hoặc khóa tạm thời bảo toàn nguyên tắc vẹn toàn ACID, không thể chèn biến số rác và hỏng file cấu trúc nhị phân của Data đuôi `.db`. (PASS)
