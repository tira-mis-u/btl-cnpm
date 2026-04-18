#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include "studentcontroller.h"
#include "classcontroller.h"
#include "departmentcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief MainWindow - View chính của ứng dụng
 *
 * Kết nối với các Controller thông qua signal-slot
 * Hiển thị dữ liệu từ Model qua QTableWidget
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // ── Tab Sinh viên ────────────────────────────────────────────────────────
    void onAddStudent();
    void onEditStudent();
    void onDeleteStudent();
    void onSearchStudent();
    void onClearSearch();
    void onStudentTableDoubleClick(int row, int col);
    void onExportCSV();
    void refreshStudentTable();

    // ── Tab Lớp học ──────────────────────────────────────────────────────────
    void onAddClass();
    void onEditClass();
    void onDeleteClass();
    void refreshClassTable();

    // ── Tab Khoa ─────────────────────────────────────────────────────────────
    void onAddDepartment();
    void onEditDepartment();
    void onDeleteDepartment();
    void refreshDepartmentTable();

    // ── Chung ────────────────────────────────────────────────────────────────
    void updateStatusBar();
    void onTabChanged(int index);
    void onLogout();

private:
    Ui::MainWindow*        ui;
    StudentController*     m_studentCtrl;
    ClassController*       m_classCtrl;
    DepartmentController*  m_deptCtrl;

    void setupUI();
    void setupConnections();
    void setupStudentTable();
    void setupClassTable();
    void setupDepartmentTable();

    void populateStudentTable(const QList<Student>& students);
    void populateClassTable(const QList<ClassModel>& classes);
    void populateDeptTable(const QList<Department>& depts);

    QString getSelectedStudentId();
    int     getSelectedClassId();
    int     getSelectedDeptId();

    void showInfo(const QString& msg);
    void showError(const QString& msg);
    bool confirmDelete(const QString& item);
};

#endif // MAINWINDOW_H
