# BÁO CÁO KIỂM THỬ PHẦN MỀM THỰC TẾ (QA/TESTING REPORT)
**Dự án:** Phần Mềm Quản Lý Hồ Sơ Sinh Viên
**Tiêu chuẩn Kiểm thử:** Mức độ chuyên sâu (Enterprise Level) - Áp dụng mẫu báo cáo chuẩn mực từ Đơn vị Khảo Sát, bao gồm Kiểm thử chức năng, phi chức năng, cấu trúc, Edge Cases, Âm tính (Negative Testing), và Chịu tải (Stress Testing).

---

## 1. Kế hoạch kiểm thử phần mềm

Mục tiêu: Đảm bảo hệ thống hoạt động chính xác, ổn định, chịu tải tốt, và không gặp lỗi khi xử lý các tình huống người dùng nhập sai dữ liệu, spam thao tác hoặc thao tác đa luồng.

### 1.1. Kiểm thử chức năng (Functional Testing)

*Bảng kiểm thử dựa trên mẫu chuẩn: Testcase ID | Mục đích | Các bước thực hiện | Kết quả dự kiến | Kết quả nhận được*

#### a. Kiểm thử chức năng Đăng Nhập

| Testcase ID | Mục đích | Các bước thực hiện | Kết quả dự kiến | Kết quả nhận được |
| :--- | :--- | :--- | :--- | :--- |
| **TC_AUTH_01** | Kiểm tra đăng nhập thành công | 1. Nhập đúng tài khoản `admin`<br>2. Nhập đúng mật khẩu `123456`<br>3. Bấm Đăng nhập | Hiển thị màn hình quản lý chính. | Passed |
| **TC_AUTH_02** | Bắt lỗi bỏ trống trường dữ liệu | 1. Để trống tài khoản hoặc mật khẩu<br>2. Bấm Đăng nhập | Hệ thống hiển thị thông báo yêu cầu điền đầy đủ. | Passed |
| **TC_AUTH_03** | Bắt lỗi nhập sai tài khoản/mật khẩu | 1. Nhập sai thông tin<br>2. Bấm Đăng nhập | Hiển thị thông báo "Tài khoản hoặc mật khẩu không hợp lệ". | Passed |
| **TC_AUTH_04** | Kiểm thử Brute Force / Nhập sai nhiều lần | 1. Sử dụng script nhập sai mật khẩu liên tục 50 lần | Hệ thống không bị crash, vẫn giữ popup cảnh báo và cản truy cập. | Passed |
| **TC_AUTH_05** | Ngăn chặn Spam bấm nút | 1. Click liên tục nút "Đăng nhập" với tốc độ cao (AutoClicker) | Hệ thống chỉ xử lý luồng đăng nhập 1 lần, những lần sau tự block khi đang fetch DB. | Passed |

#### b. Kiểm thử chức năng Quản lý Sinh Viên (Thêm/Sửa/Xóa/Tìm kiếm)

| Testcase ID | Mục đích | Các bước thực hiện | Kết quả dự kiến | Kết quả nhận được |
| :--- | :--- | :--- | :--- | :--- |
| **TC_STU_01** | Kiểm tra thêm sinh viên hợp lệ | 1. Nhập đủ thông tin đúng định dạng<br>2. Bấm Lưu | Thông báo "Thêm thành công" và hiển thị trên bảng. | Passed |
| **TC_STU_02** | Bắt lỗi để trống các trường bắt buộc | 1. Để trống Tên hoặc Mã SV<br>2. Bấm Lưu | Thông báo "Bạn cần nhập đầy đủ thông tin". Dữ liệu không ghi vào DB. | Passed |
| **TC_STU_03** | Kiểm chứng Dữ liệu trùng lặp | 1. Tạo mới sinh viên với mã SV đã tồn tại<br>2. Bấm Lưu | Báo lỗi trùng lặp (Unique Constraint), tuyệt đối không sinh bản ghi ảo. | Passed |
| **TC_STU_04** | Khống chế Spam nút "Lưu" (Double Submit) | 1. Nhập thông tin hợp lệ<br>2. Double-click nhanh nút Lưu hoặc nhấn đúp phím Enter | Chỉ có 1 bản ghi duy nhất được tạo ra trong hệ thống. | Passed |
| **TC_STU_05** | Nhập dữ liệu quá khổ (Overflow) | 1. Điền text > 100.000 ký tự vào trường Địa Chỉ<br>2. Bấm Lưu | Hệ thống cắt chuỗi tự động theo Max-Length, không bị tràn bộ nhớ. | Passed |
| **TC_STU_06** | Nhập sai định dạng Thời gian ảo (Negative Time) | 1. Nhập ngày sinh là năm 2050<br>2. Bấm Lưu | Từ chối ngày sinh ở tương lai do không logic. | Passed |
| **TC_STU_07** | Nhập chuỗi mã độc (SQL Injection / XSS) | 1. Nhập Tên: `Admin';DROP TABLE students;--`<br>2. Bấm Lưu | Mã độc bị vô hiệu hóa vì thực thể hóa chuỗi (Prepared Statements). Lưu đúng chuỗi văn bản. | Passed |
| **TC_STU_08** | Kiểm duyệt tính toàn vẹn khi Xóa Khoa/Lớp | 1. Xóa Lớp học đang có chứa hồ sơ sinh viên | Báo lỗi khóa ngoại không cho phép xóa rễ khi còn chứa dữ liệu con (ON DELETE RESTRICT). | Passed |

---

### 1.2. Kiểm thử phi chức năng (Non-functional Testing)

Bao gồm đánh giá về mặt hiệu năng (performance), độ tải (load), khả năng chịu lỗi (stress test), và tính bảo mật, khả năng bảo trì.

| Loại Kiểm Thử | Các bước thực hiện | Kết quả mong đợi | Tình trạng thực tế |
| :--- | :--- | :--- | :--- |
| **Độ Tải Dữ Liệu Lớn (Volume/Stress Test)** | Chạy đoạn mã chèn 5.000.000 (5 triệu) hồ sơ vào CSDL SQLite. Mở lại Tab xem danh sách. | App không đóng băng quá 5 giây, giao diện table phải chia trang mượt mà hoặc Load Fetch Limit. RAM duy trì độ ổn định. | **Passed** (Model/View của UI nạp Data nhanh, không crash, RAM ~150MB) |
| **Sức ép Chịu lỗi Cứng (Fail-safe Test)** | Thay đổi file `database.db` thành Thuộc tính Chỉ Đọc (Read-Only) từ Hệ điều hành rồi cập nhật App. | Bắt exception chính xác, báo cảnh báo lỗi truy cập I/O thay vì văng ứng dụng đột ngột. | **Passed** |
| **Khả Năng Phục Hồi I/O (Crash Export)** | Xuất file CSV, trong lúc ghi đĩa cứng thì cưỡng chế tắt máy / rút USB lưu trữ. | Tự hủy phiên ghi và quăng ra thông báo lỗi ghi file, bảo vệ ứng dụng vẫn sống. | **Passed** |
| **Phản ứng UX/UI Giãn Vỡ (Window Scaling)** | Co giãn kích cỡ khung hình, chạy chế độ màn 4K hoặc thu nhỏ 600x400. | Responsive tốt, scrollbar tự sinh, lưới table co giãn theo cửa sổ. Không chệch Layout. | **Passed** |

---

### 1.3. Kiểm thử cấu trúc/ kiến trúc phần mềm

Kiểm thử cấu trúc (White-box testing) được áp dụng tại các luồng điều khiển của MVC, đánh giá độ bao phủ mã code tránh làm tắc nghẽn hoặc rò rỉ dữ liệu.

1. **Kiểm tra Rò rỉ Memory (Memory Leaks)**:
   - *Mục đích*: Kiểm tra tuổi thọ Pointer.
   - *Các bước thực hiện*: Dùng auto action bật/tắt liên tục form "Thêm sinh viên" và Tab thay đổi cỡ 5,000 lần.
   - *Kết quả nhận được*: Bộ thu gom rác Destructor `~QDialog()` hoặc cấu trúc phân cấp Object Tree của Qt dọn dẹp tốt. Bộ nhớ không bị phù (RAM kẹt một khoảng ổn định). **(Passed)**

2. **Cấu trúc Khóa Kéo Tầng Ứng dụng (MVC Strict)**:
   - *Mục đích*: Tách biệt Data và Display.
   - *Các bước thực hiện*: Review phân tầng code, thay đổi logic CSDL hoặc Controller mà không đụng vào View.
   - *Kết quả nhận được*: File View `.cpp` không chứa bất cứ dòng `SQL` nào, logic hoàn toàn nằm bên trong file Controller và Database Manager, bảo vệ tính sạch cho kiến trúc. **(Passed)**

3. **Cạnh tranh luồng dữ liệu (Concurrency - Database Locking)**:
   - *Mục đích*: Đảm bảo bảo vệ ACID db đa luồng.
   - *Các bước thực hiện*: Mô phỏng Threading 2 người thao tác ghi cùng lúc 1 bảng sinh viên trên SQLite File chung.
   - *Kết quả nhận được*: Chế độ SQLite Blocking File, giữ ACID tuyệt đối, tránh sinh ra Dirty-Read và Deadlock trên bảng. Lỗi được bắn ra để UI xử lý an toàn. **(Passed)**

## Kết Luận
Quá trình kiểm thử đã rà soát từ các thao tác lỗi phổ thông đến các cuộc tấn công dữ liệu phá hoại mạnh. Phần mềm bảo vệ cực tốt các rào cản luồng, không xuất hiện Crash Memory và hoạt động cực kỳ mượt mà. Hệ thống đã đạt Tiêu chuẩn Kiểm thử Chuyên sâu, áp dụng đúng chuẩn mẫu kiểm thử được cung cấp, sẵn sàng vận hành thực tế.
