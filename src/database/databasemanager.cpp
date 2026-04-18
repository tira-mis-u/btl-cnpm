#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>
#include <QDir>

// ─── Singleton & Lifecycle ───────────────────────────────────────────────────

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        qCritical() << "Lỗi mở DB:" << m_lastError;
        return false;
    }

    // Bật foreign key support
    QSqlQuery pragma;
    pragma.exec("PRAGMA foreign_keys = ON;");

    if (!createTables()) {
        return false;
    }

    insertDefaultData();
    m_connected = true;
    qDebug() << "Database khởi tạo thành công:" << dbPath;
    return true;
}

void DatabaseManager::close() {
    if (m_db.isOpen()) {
        m_db.close();
        m_connected = false;
    }
}

bool DatabaseManager::isConnected() const { return m_connected; }
QString DatabaseManager::lastError() const { return m_lastError; }

// ─── Tạo bảng ────────────────────────────────────────────────────────────────

bool DatabaseManager::createTables() {
    QSqlQuery q;

    // Bảng departments (Khoa)
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS departments (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT    NOT NULL UNIQUE,
            description TEXT    DEFAULT ''
        )
    )")) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Bảng classes (Lớp)
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS classes (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            name          TEXT    NOT NULL UNIQUE,
            department_id INTEGER NOT NULL,
            FOREIGN KEY (department_id) REFERENCES departments(id)
        )
    )")) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Bảng students (Sinh viên)
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS students (
            student_id   TEXT    PRIMARY KEY,
            full_name    TEXT    NOT NULL,
            date_of_birth TEXT   NOT NULL,
            gender       TEXT    NOT NULL DEFAULT 'Nam',
            email        TEXT    DEFAULT '',
            phone        TEXT    DEFAULT '',
            address      TEXT    DEFAULT '',
            class_id     INTEGER NOT NULL,
            created_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (class_id) REFERENCES classes(id)
        )
    )")) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Bảng users (đăng nhập)
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT    NOT NULL UNIQUE,
            password TEXT    NOT NULL,
            role     TEXT    DEFAULT 'admin'
        )
    )")) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Index tìm kiếm nhanh
    q.exec("CREATE INDEX IF NOT EXISTS idx_student_name ON students(full_name)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_student_class ON students(class_id)");

    return true;
}

bool DatabaseManager::insertDefaultData() {
    QSqlQuery check;
    check.exec("SELECT COUNT(*) FROM departments");
    check.next();
    if (check.value(0).toInt() > 0) return true; // Đã có dữ liệu

    // Dữ liệu mẫu - Khoa
    struct DeptData { QString name, desc; };
    QList<DeptData> depts = {
        {"Công nghệ thông tin", "Đào tạo kỹ sư lập trình, an ninh mạng và AI."},
        {"Kinh tế", "Nghiên cứu thị trường, tài chính và quản lý vĩ mô."},
        {"Kỹ thuật điện - Điện tử", "Tự động hóa, mạch điện và hạ tầng năng lượng."},
        {"Quản trị kinh doanh", "Kỹ năng điều hành doanh nghiệp và khởi nghiệp."}
    };
    for (const auto& d : depts) {
        QSqlQuery q;
        q.prepare("INSERT OR IGNORE INTO departments(name, description) VALUES(?, ?)");
        q.addBindValue(d.name);
        q.addBindValue(d.desc);
        q.exec();
    }

    // Dữ liệu mẫu - Lớp
    struct ClassData { QString name; int deptId; };
    QList<ClassData> classes = {
        {"75DCTT36", 1}, {"75DCTT18", 1}, {"75DCTT67", 1},
        {"75DCMM36", 2}, {"75DCMM18", 2},
        {"75DMX36", 3},
        {"120DOMIXI36", 4}
    };
    for (const auto& c : classes) {
        QSqlQuery q;
        q.prepare("INSERT OR IGNORE INTO classes(name, department_id) VALUES(?,?)");
        q.addBindValue(c.name);
        q.addBindValue(c.deptId);
        q.exec();
    }

    // Dữ liệu mẫu - Sinh viên
    struct StuData { QString id, name, dob, gender, email, phone, address; int classId; };
    QList<StuData> students = {
        {"SV001", "Lê Nguyễn Minh Trí", "2004-03-15", "Nam", "minhtri@email.com",    "0901234567", "120 Yên Lãng, Hà Nội", 1},
        {"SV002", "Trần Quốc Thuận",    "2004-07-22", "Nữ",  "qthuan@email.com",    "0912345678", "120 An Liễng, Hà Nội", 1},
        {"SV003", "Phạm Trung Hiếu",    "2003-11-08", "Nam", "hieu@email.com",      "0923456789", "Thanh Hoá", 2},
        {"SV004", "Nguyễn Quốc Trung",  "2004-05-30", "Nữ",  "trungquoc@email.com", "0934567890", "Thanh Hoá", 3},
        {"SV005", "Phạm Văn Đam",       "2003-09-12", "Nam", "photographerPhuTho@email.com", "0945678901", "Phú Thọ", 4},
    };
    for (const auto& s : students) {
        QSqlQuery q;
        q.prepare(R"(
            INSERT OR IGNORE INTO students
                (student_id, full_name, date_of_birth, gender, email, phone, address, class_id)
            VALUES (?,?,?,?,?,?,?,?)
        )");
        q.addBindValue(s.id);
        q.addBindValue(s.name);
        q.addBindValue(s.dob);
        q.addBindValue(s.gender);
        q.addBindValue(s.email);
        q.addBindValue(s.phone);
        q.addBindValue(s.address);
        q.addBindValue(s.classId);
        q.exec();
    }

    // Tài khoản admin mặc định (password: admin123)
    QSqlQuery qa;
    qa.prepare("INSERT OR IGNORE INTO users(username,password,role) VALUES(?,?,?)");
    qa.addBindValue("admin");
    qa.addBindValue(hashPassword("admin123"));
    qa.addBindValue("admin");
    qa.exec();

    return true;
}

// ─── Utility ─────────────────────────────────────────────────────────────────

QString DatabaseManager::hashPassword(const QString& password) {
    return QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ─── AUTH ─────────────────────────────────────────────────────────────────────

bool DatabaseManager::verifyLogin(const QString& username, const QString& password) {
    QSqlQuery q;
    q.prepare("SELECT password FROM users WHERE username = ?");
    q.addBindValue(username);
    q.exec();
    if (q.next()) {
        return q.value(0).toString() == hashPassword(password);
    }
    return false;
}

// ─── DEPARTMENT CRUD ─────────────────────────────────────────────────────────

bool DatabaseManager::addDepartment(const Department& dept) {
    QSqlQuery q;
    q.prepare("INSERT INTO departments(name, description) VALUES(?, ?)");
    q.addBindValue(dept.getName());
    q.addBindValue(dept.getDescription());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateDepartment(const Department& dept) {
    QSqlQuery q;
    q.prepare("UPDATE departments SET name=?, description=? WHERE id=?");
    q.addBindValue(dept.getName());
    q.addBindValue(dept.getDescription());
    q.addBindValue(dept.getId());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteDepartment(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM departments WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QList<Department> DatabaseManager::getAllDepartments() {
    QList<Department> list;
    QSqlQuery q("SELECT id, name, description FROM departments ORDER BY name");
    while (q.next()) {
        list.append(Department(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString()
        ));
    }
    return list;
}

Department DatabaseManager::getDepartmentById(int id) {
    QSqlQuery q;
    q.prepare("SELECT id, name, description FROM departments WHERE id=?");
    q.addBindValue(id);
    q.exec();
    if (q.next()) {
        return Department(q.value(0).toInt(), q.value(1).toString(), q.value(2).toString());
    }
    return Department();
}

bool DatabaseManager::departmentHasClasses(int deptId) {
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM classes WHERE department_id=?");
    q.addBindValue(deptId);
    q.exec();
    return q.next() && q.value(0).toInt() > 0;
}

// ─── CLASS CRUD ───────────────────────────────────────────────────────────────

bool DatabaseManager::addClass(const ClassModel& cls) {
    QSqlQuery q;
    q.prepare("INSERT INTO classes(name, department_id) VALUES(?, ?)");
    q.addBindValue(cls.getName());
    q.addBindValue(cls.getDepartmentId());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateClass(const ClassModel& cls) {
    QSqlQuery q;
    q.prepare("UPDATE classes SET name=?, department_id=? WHERE id=?");
    q.addBindValue(cls.getName());
    q.addBindValue(cls.getDepartmentId());
    q.addBindValue(cls.getId());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteClass(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM classes WHERE id=?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QList<ClassModel> DatabaseManager::getAllClasses() {
    QList<ClassModel> list;
    QSqlQuery q(R"(
        SELECT c.id, c.name, c.department_id, d.name
        FROM classes c
        LEFT JOIN departments d ON c.department_id = d.id
        ORDER BY c.name
    )");
    while (q.next()) {
        list.append(ClassModel(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toInt(),
            q.value(3).toString()
        ));
    }
    return list;
}

QList<ClassModel> DatabaseManager::getClassesByDepartment(int deptId) {
    QList<ClassModel> list;
    QSqlQuery q;
    q.prepare(R"(
        SELECT c.id, c.name, c.department_id, d.name
        FROM classes c LEFT JOIN departments d ON c.department_id=d.id
        WHERE c.department_id=? ORDER BY c.name
    )");
    q.addBindValue(deptId);
    q.exec();
    while (q.next()) {
        list.append(ClassModel(q.value(0).toInt(), q.value(1).toString(),
                               q.value(2).toInt(), q.value(3).toString()));
    }
    return list;
}

ClassModel DatabaseManager::getClassById(int id) {
    QSqlQuery q;
    q.prepare("SELECT c.id, c.name, c.department_id, d.name FROM classes c "
              "LEFT JOIN departments d ON c.department_id=d.id WHERE c.id=?");
    q.addBindValue(id);
    q.exec();
    if (q.next())
        return ClassModel(q.value(0).toInt(), q.value(1).toString(),
                          q.value(2).toInt(), q.value(3).toString());
    return ClassModel();
}

bool DatabaseManager::classHasStudents(int classId) {
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM students WHERE class_id=?");
    q.addBindValue(classId);
    q.exec();
    return q.next() && q.value(0).toInt() > 0;
}

// ─── STUDENT CRUD ─────────────────────────────────────────────────────────────

bool DatabaseManager::addStudent(const Student& s) {
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO students
            (student_id, full_name, date_of_birth, gender, email, phone, address, class_id)
        VALUES (?,?,?,?,?,?,?,?)
    )");
    q.addBindValue(s.getStudentId());
    q.addBindValue(s.getFullName());
    q.addBindValue(s.getDateOfBirth().toString("yyyy-MM-dd"));
    q.addBindValue(s.genderToString());
    q.addBindValue(s.getEmail());
    q.addBindValue(s.getPhone());
    q.addBindValue(s.getAddress());
    q.addBindValue(s.getClassId());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateStudent(const Student& s) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE students SET
            full_name=?, date_of_birth=?, gender=?,
            email=?, phone=?, address=?, class_id=?
        WHERE student_id=?
    )");
    q.addBindValue(s.getFullName());
    q.addBindValue(s.getDateOfBirth().toString("yyyy-MM-dd"));
    q.addBindValue(s.genderToString());
    q.addBindValue(s.getEmail());
    q.addBindValue(s.getPhone());
    q.addBindValue(s.getAddress());
    q.addBindValue(s.getClassId());
    q.addBindValue(s.getStudentId());
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteStudent(const QString& studentId) {
    QSqlQuery q;
    q.prepare("DELETE FROM students WHERE student_id=?");
    q.addBindValue(studentId);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

static Student rowToStudent(QSqlQuery& q) {
    return Student(
        q.value(0).toString(),  // student_id
        q.value(1).toString(),  // full_name
        QDate::fromString(q.value(2).toString(), "yyyy-MM-dd"), // dob
        Student::genderFromString(q.value(3).toString()), // gender
        q.value(4).toString(),  // email
        q.value(5).toString(),  // phone
        q.value(6).toString(),  // address
        q.value(7).toInt(),     // class_id
        q.value(8).toString(),  // class_name
        q.value(9).toString()   // dept_name
    );
}

static const char* STUDENT_SELECT = R"(
    SELECT s.student_id, s.full_name, s.date_of_birth, s.gender,
           s.email, s.phone, s.address, s.class_id,
           c.name AS class_name, d.name AS dept_name
    FROM students s
    LEFT JOIN classes    c ON s.class_id     = c.id
    LEFT JOIN departments d ON c.department_id = d.id
)";

QList<Student> DatabaseManager::getAllStudents() {
    QList<Student> list;
    QSqlQuery q(QString("%1 ORDER BY s.full_name").arg(STUDENT_SELECT));
    while (q.next()) list.append(rowToStudent(q));
    return list;
}

QList<Student> DatabaseManager::searchStudents(const QString& keyword) {
    QList<Student> list;
    QSqlQuery q;
    q.prepare(QString("%1 WHERE s.student_id LIKE ? OR s.full_name LIKE ? "
                      "OR s.phone LIKE ? OR s.email LIKE ? OR c.name LIKE ? "
                      "OR d.name LIKE ? OR s.address LIKE ? "
                      "ORDER BY s.full_name").arg(STUDENT_SELECT));
    QString like = "%" + keyword + "%";
    q.addBindValue(like); // student_id
    q.addBindValue(like); // full_name
    q.addBindValue(like); // phone
    q.addBindValue(like); // email
    q.addBindValue(like); // class name
    q.addBindValue(like); // department name
    q.addBindValue(like); // address
    q.exec();
    while (q.next()) list.append(rowToStudent(q));
    return list;
}

QList<Student> DatabaseManager::getStudentsByClass(int classId) {
    QList<Student> list;
    QSqlQuery q;
    q.prepare(QString("%1 WHERE s.class_id=? ORDER BY s.full_name").arg(STUDENT_SELECT));
    q.addBindValue(classId);
    q.exec();
    while (q.next()) list.append(rowToStudent(q));
    return list;
}

Student DatabaseManager::getStudentById(const QString& studentId) {
    QSqlQuery q;
    q.prepare(QString("%1 WHERE s.student_id=?").arg(STUDENT_SELECT));
    q.addBindValue(studentId);
    q.exec();
    if (q.next()) return rowToStudent(q);
    return Student();
}

bool DatabaseManager::studentIdExists(const QString& studentId) {
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM students WHERE student_id=?");
    q.addBindValue(studentId);
    q.exec();
    return q.next() && q.value(0).toInt() > 0;
}

// ─── STATS ────────────────────────────────────────────────────────────────────

int DatabaseManager::countStudents() {
    QSqlQuery q("SELECT COUNT(*) FROM students");
    return q.next() ? q.value(0).toInt() : 0;
}
int DatabaseManager::countClasses() {
    QSqlQuery q("SELECT COUNT(*) FROM classes");
    return q.next() ? q.value(0).toInt() : 0;
}
int DatabaseManager::countDepartments() {
    QSqlQuery q("SELECT COUNT(*) FROM departments");
    return q.next() ? q.value(0).toInt() : 0;
}
