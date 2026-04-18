#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "studentdialog.h"
#include "classdialog.h"
#include "departmentdialog.h"
#include "databasemanager.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QStatusBar>
#include <QIcon>

// ─── Constructor / Destructor ─────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_studentCtrl(new StudentController(this))
    , m_classCtrl(new ClassController(this))
    , m_deptCtrl(new DepartmentController(this))
{
    ui->setupUi(this);
    setWindowTitle("Phần Mềm Quản Lý Hồ Sơ Sinh Viên - v1.0");


    setupUI();
    setupConnections();
    refreshStudentTable();
    refreshClassTable();
    refreshDepartmentTable();
    updateStatusBar();
}

MainWindow::~MainWindow() {
    delete ui;
}

// ─── Setup ────────────────────────────────────────────────────────────────────

void MainWindow::setupUI() {
    setupStudentTable();
    setupClassTable();
    setupDepartmentTable();

    // Style chung cho bảng
    QString tableStyle = R"(
        QTableWidget {
            border: 1px solid #dce3ec;
            border-radius: 6px;
            gridline-color: #edf0f4;
            font-size: 13px;
            outline: none;
            /* Đảm bảo mọi hàng đều có cùng màu nền mặc định để không bị lệch */
            background-color: white; 
            alternate-background-color: #f8fafc;
        }
        QTableWidget::item { 
            /* Xóa bỏ padding và border lẻ tẻ ở item để tránh hiện tượng "thụt lề" */
            padding: 5px; 
            color: #1e293b;
            border: none;
        }
        /* Ép màu xanh blue chuẩn cho TẤT CẢ các hàng khi được chọn */
        QTableWidget::item:selected {
            background-color: #3b82f6;
            color: white;
        }
        /* Fix lỗi cửa sổ không active vẫn phải nhìn rõ */
        QTableWidget::item:selected:!active {
            background-color: #3b82f6;
            color: white;
        }
        QHeaderView::section {
            background-color: #1e40af;
            color: white;
            font-weight: bold;
            padding: 8px;
            border: none;
            text-align: left;
        }
    )";
    ui->tableStudents->setStyleSheet(tableStyle);
    ui->tableClasses->setStyleSheet(tableStyle);
    ui->tableDepts->setStyleSheet(tableStyle);
}

void MainWindow::setupStudentTable() {
    QTableWidget* t = ui->tableStudents;
    t->setColumnCount(9);
    t->setHorizontalHeaderLabels({
        "Mã SV", "Họ và Tên", "Ngày Sinh", "Giới Tính",
        "Điện Thoại", "Email", "Lớp", "Khoa", "Địa Chỉ"
    });
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->verticalHeader()->setVisible(false);
    t->setSortingEnabled(true);
}

void MainWindow::setupClassTable() {
    QTableWidget* t = ui->tableClasses;
    t->setColumnCount(3);
    t->setHorizontalHeaderLabels({"ID", "Tên Lớp", "Thuộc Khoa"});
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->verticalHeader()->setVisible(false);
    t->setColumnHidden(0, true); // Ẩn cột ID
}

void MainWindow::setupDepartmentTable() {
    QTableWidget* t = ui->tableDepts;
    t->setColumnCount(3);
    t->setHorizontalHeaderLabels({"ID", "Tên Khoa", "Mô Tả"});
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->verticalHeader()->setVisible(false);
    t->setColumnHidden(0, true); // Ẩn cột ID
}

void MainWindow::setupConnections() {
    // ── Sinh viên ────────────────────────────────────────────────────────────
    connect(ui->btnAddStudent,    &QPushButton::clicked, this, &MainWindow::onAddStudent);
    connect(ui->btnEditStudent,   &QPushButton::clicked, this, &MainWindow::onEditStudent);
    connect(ui->btnDeleteStudent, &QPushButton::clicked, this, &MainWindow::onDeleteStudent);
    connect(ui->btnSearch,        &QPushButton::clicked, this, &MainWindow::onSearchStudent);
    connect(ui->btnClearSearch,   &QPushButton::clicked, this, &MainWindow::onClearSearch);
    connect(ui->btnExportCSV,     &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(ui->lineSearch, &QLineEdit::returnPressed,   this, &MainWindow::onSearchStudent);
    connect(ui->tableStudents, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onStudentTableDoubleClick);

    // ── Lớp học ──────────────────────────────────────────────────────────────
    connect(ui->btnAddClass,    &QPushButton::clicked, this, &MainWindow::onAddClass);
    connect(ui->btnEditClass,   &QPushButton::clicked, this, &MainWindow::onEditClass);
    connect(ui->btnDeleteClass, &QPushButton::clicked, this, &MainWindow::onDeleteClass);

    // ── Khoa ─────────────────────────────────────────────────────────────────
    connect(ui->btnAddDept,    &QPushButton::clicked, this, &MainWindow::onAddDepartment);
    connect(ui->btnEditDept,   &QPushButton::clicked, this, &MainWindow::onEditDepartment);
    connect(ui->btnDeleteDept, &QPushButton::clicked, this, &MainWindow::onDeleteDepartment);

    // ── Tab change & Logout ───────────────────────────────────────────────────
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(ui->btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);
}

// ─── Populate Tables ─────────────────────────────────────────────────────────

void MainWindow::populateStudentTable(const QList<Student>& students) {
    QTableWidget* t = ui->tableStudents;
    t->setSortingEnabled(false);
    t->setRowCount(students.size());

    for (int i = 0; i < students.size(); ++i) {
        const Student& s = students[i];
        t->setItem(i, 0, new QTableWidgetItem(s.getStudentId()));
        t->setItem(i, 1, new QTableWidgetItem(s.getFullName()));
        t->setItem(i, 2, new QTableWidgetItem(s.getDateOfBirth().toString("dd/MM/yyyy")));
        t->setItem(i, 3, new QTableWidgetItem(s.genderToString()));
        t->setItem(i, 4, new QTableWidgetItem(s.getPhone()));
        t->setItem(i, 5, new QTableWidgetItem(s.getEmail()));
        t->setItem(i, 6, new QTableWidgetItem(s.getClassName()));
        t->setItem(i, 7, new QTableWidgetItem(s.getDeptName()));
        t->setItem(i, 8, new QTableWidgetItem(s.getAddress()));
        t->setRowHeight(i, 36);
    }
    t->setSortingEnabled(true);
    ui->lblStudentCount->setText(QString("Tổng: %1 sinh viên").arg(students.size()));
}

void MainWindow::populateClassTable(const QList<ClassModel>& classes) {
    QTableWidget* t = ui->tableClasses;
    t->setRowCount(classes.size());
    for (int i = 0; i < classes.size(); ++i) {
        t->setItem(i, 0, new QTableWidgetItem(QString::number(classes[i].getId())));
        t->setItem(i, 1, new QTableWidgetItem(classes[i].getName()));
        t->setItem(i, 2, new QTableWidgetItem(classes[i].getDepartmentName()));
        t->setRowHeight(i, 36);
    }
}

void MainWindow::populateDeptTable(const QList<Department>& depts) {
    QTableWidget* t = ui->tableDepts;
    t->setRowCount(depts.size());
    for (int i = 0; i < depts.size(); ++i) {
        t->setItem(i, 0, new QTableWidgetItem(QString::number(depts[i].getId())));
        t->setItem(i, 1, new QTableWidgetItem(depts[i].getName()));
        t->setItem(i, 2, new QTableWidgetItem(depts[i].getDescription()));
        t->setRowHeight(i, 36);
    }
}

// ─── Refresh ─────────────────────────────────────────────────────────────────

void MainWindow::refreshStudentTable() {
    populateStudentTable(m_studentCtrl->getAllStudents());
    updateStatusBar();
}
void MainWindow::refreshClassTable() {
    populateClassTable(m_classCtrl->getAllClasses());
}
void MainWindow::refreshDepartmentTable() {
    populateDeptTable(m_deptCtrl->getAllDepartments());
}

// ─── Student Slots ────────────────────────────────────────────────────────────

void MainWindow::onAddStudent() {
    StudentDialog dlg(this, nullptr, m_classCtrl);
    if (dlg.exec() == QDialog::Accepted) {
        Student s = dlg.getStudent();
        if (m_studentCtrl->addStudent(s)) {
            showInfo("Thêm sinh viên thành công!");
            refreshStudentTable();
        } else {
            showError(m_studentCtrl->lastError());
        }
    }
}

void MainWindow::onEditStudent() {
    QString id = getSelectedStudentId();
    if (id.isEmpty()) { showError("Vui lòng chọn sinh viên cần sửa."); return; }

    Student s = m_studentCtrl->getStudentById(id);
    StudentDialog dlg(this, &s, m_classCtrl);
    if (dlg.exec() == QDialog::Accepted) {
        Student updated = dlg.getStudent();
        if (m_studentCtrl->updateStudent(updated)) {
            showInfo("Cập nhật thông tin thành công!");
            refreshStudentTable();
        } else {
            showError(m_studentCtrl->lastError());
        }
    }
}

void MainWindow::onDeleteStudent() {
    QString id = getSelectedStudentId();
    if (id.isEmpty()) { showError("Vui lòng chọn sinh viên cần xóa."); return; }

    Student s = m_studentCtrl->getStudentById(id);
    if (!confirmDelete(QString("sinh viên '%1' (Mã: %2)").arg(s.getFullName()).arg(id)))
        return;

    if (m_studentCtrl->deleteStudent(id)) {
        showInfo("Đã xóa sinh viên thành công.");
        refreshStudentTable();
    } else {
        showError(m_studentCtrl->lastError());
    }
}

void MainWindow::onSearchStudent() {
    QString keyword = ui->lineSearch->text().trimmed();
    if (keyword.isEmpty()) { refreshStudentTable(); return; }
    populateStudentTable(m_studentCtrl->searchStudents(keyword));
}

void MainWindow::onClearSearch() {
    ui->lineSearch->clear();
    refreshStudentTable();
}

void MainWindow::onStudentTableDoubleClick(int /*row*/, int /*col*/) {
    onEditStudent();
}

void MainWindow::onExportCSV() {
    QString path = QFileDialog::getSaveFileName(
        this, "Xuất danh sách sinh viên", "danh_sach_sinh_vien.csv",
        "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    QList<Student> students = m_studentCtrl->getAllStudents();
    if (m_studentCtrl->exportToCSV(path, students)) {
        showInfo(QString("Xuất file thành công!\n%1").arg(path));
    } else {
        showError(m_studentCtrl->lastError());
    }
}

// ─── Class Slots ──────────────────────────────────────────────────────────────

void MainWindow::onAddClass() {
    ClassDialog dlg(this, nullptr, m_deptCtrl);
    if (dlg.exec() == QDialog::Accepted) {
        ClassModel cls = dlg.getClassModel();
        if (m_classCtrl->addClass(cls)) {
            showInfo("Thêm lớp học thành công!");
            refreshClassTable();
        } else {
            showError(m_classCtrl->lastError());
        }
    }
}

void MainWindow::onEditClass() {
    int id = getSelectedClassId();
    if (id <= 0) { showError("Vui lòng chọn lớp cần sửa."); return; }

    auto classes = m_classCtrl->getAllClasses();
    ClassModel* cls = nullptr;
    for (auto& c : classes) { if (c.getId() == id) { cls = &c; break; } }

    ClassDialog dlg(this, cls, m_deptCtrl);
    if (dlg.exec() == QDialog::Accepted) {
        ClassModel updated = dlg.getClassModel();
        if (m_classCtrl->updateClass(updated)) {
            showInfo("Cập nhật lớp học thành công!");
            refreshClassTable();
            refreshStudentTable(); // Cập nhật tên lớp trong bảng SV
        } else {
            showError(m_classCtrl->lastError());
        }
    }
}

void MainWindow::onDeleteClass() {
    int id = getSelectedClassId();
    if (id <= 0) { showError("Vui lòng chọn lớp cần xóa."); return; }

    int row = ui->tableClasses->currentRow();
    QString name = ui->tableClasses->item(row, 1)->text();
    if (!confirmDelete(QString("lớp '%1'").arg(name))) return;

    if (m_classCtrl->deleteClass(id)) {
        showInfo("Đã xóa lớp học thành công.");
        refreshClassTable();
    } else {
        showError(m_classCtrl->lastError());
    }
}

// ─── Department Slots ─────────────────────────────────────────────────────────

void MainWindow::onAddDepartment() {
    DepartmentDialog dlg(this, nullptr);
    if (dlg.exec() == QDialog::Accepted) {
        Department dept = dlg.getDepartment();
        if (m_deptCtrl->addDepartment(dept)) {
            showInfo("Thêm khoa thành công!");
            refreshDepartmentTable();
        } else {
            showError(m_deptCtrl->lastError());
        }
    }
}

void MainWindow::onEditDepartment() {
    int id = getSelectedDeptId();
    if (id <= 0) { showError("Vui lòng chọn khoa cần sửa."); return; }

    Department dept = DatabaseManager::instance().getDepartmentById(id);
    DepartmentDialog dlg(this, &dept);
    if (dlg.exec() == QDialog::Accepted) {
        Department updated = dlg.getDepartment();
        updated.setId(id);
        if (m_deptCtrl->updateDepartment(updated)) {
            showInfo("Cập nhật khoa thành công!");
            refreshDepartmentTable();
            refreshClassTable();
        } else {
            showError(m_deptCtrl->lastError());
        }
    }
}

void MainWindow::onDeleteDepartment() {
    int id = getSelectedDeptId();
    if (id <= 0) { showError("Vui lòng chọn khoa cần xóa."); return; }

    int row = ui->tableDepts->currentRow();
    QString name = ui->tableDepts->item(row, 1)->text();
    if (!confirmDelete(QString("khoa '%1'").arg(name))) return;

    if (m_deptCtrl->deleteDepartment(id)) {
        showInfo("Đã xóa khoa thành công.");
        refreshDepartmentTable();
    } else {
        showError(m_deptCtrl->lastError());
    }
}

// ─── Helper Methods ───────────────────────────────────────────────────────────

QString MainWindow::getSelectedStudentId() {
    int row = ui->tableStudents->currentRow();
    if (row < 0) return "";
    auto item = ui->tableStudents->item(row, 0);
    return item ? item->text() : "";
}

int MainWindow::getSelectedClassId() {
    int row = ui->tableClasses->currentRow();
    if (row < 0) return -1;
    auto item = ui->tableClasses->item(row, 0);
    return item ? item->text().toInt() : -1;
}

int MainWindow::getSelectedDeptId() {
    int row = ui->tableDepts->currentRow();
    if (row < 0) return -1;
    auto item = ui->tableDepts->item(row, 0);
    return item ? item->text().toInt() : -1;
}

void MainWindow::showInfo(const QString& msg) {
    QMessageBox::information(this, "Thông báo", msg);
}

void MainWindow::showError(const QString& msg) {
    QMessageBox::critical(this, "Lỗi", msg);
}

bool MainWindow::confirmDelete(const QString& item) {
    return QMessageBox::question(
        this, "Xác nhận xóa",
        QString("Bạn có chắc muốn xóa %1?\nHành động này không thể hoàn tác.").arg(item),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    ) == QMessageBox::Yes;
}

void MainWindow::updateStatusBar() {
    auto& db = DatabaseManager::instance();
    statusBar()->showMessage(
        QString("  Sinh viên: %1  |  Lớp học: %2  |  Khoa: %3  |  Hệ thống sẵn sàng")
            .arg(db.countStudents())
            .arg(db.countClasses())
            .arg(db.countDepartments())
    );
}

void MainWindow::onTabChanged(int index) {
    switch (index) {
        case 0: refreshStudentTable(); break;
        case 1: refreshClassTable();   break;
        case 2: refreshDepartmentTable(); break;
    }
}

void MainWindow::onLogout() {
    if (QMessageBox::question(this, "Đăng xuất", "Bạn có chắc muốn đăng xuất khỏi hệ thống?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        qApp->exit(1000);
    }
}
