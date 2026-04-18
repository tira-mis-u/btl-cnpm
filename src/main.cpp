#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include "mainwindow.h"
#include "logindialog.h"
#include "databasemanager.h"

int main(int argc, char *argv[]) {
    // Ép fix kích thước chuẩn, không tự động co giãn theo tỉ lệ màn hình Windows
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif
    
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/pentagon_jar.png"));
    app.setApplicationName("StudentMS");
    app.setApplicationDisplayName("Quản Lý Sinh Viên");
    app.setOrganizationName("University");
    app.setStyle(QStyleFactory::create("Fusion"));

    // ── Bảng màu Fusion tùy chỉnh ────────────────────────────────────────────
    QPalette palette;
    palette.setColor(QPalette::Window,          QColor(245, 247, 250));
    palette.setColor(QPalette::WindowText,      QColor(30,  41,  59));
    palette.setColor(QPalette::Base,            QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase,   QColor(248, 250, 252));
    palette.setColor(QPalette::ToolTipBase,     QColor(255, 255, 220));
    palette.setColor(QPalette::ToolTipText,     QColor(30,  41,  59));
    palette.setColor(QPalette::Text,            QColor(30,  41,  59));
    palette.setColor(QPalette::Button,          QColor(226, 232, 240));
    palette.setColor(QPalette::ButtonText,      QColor(30,  41,  59));
    palette.setColor(QPalette::Highlight,       QColor(59,  130, 246));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link,            QColor(59,  130, 246));
    app.setPalette(palette);

    // Stylesheet chung
    app.setStyleSheet(R"(
        QToolTip {
            border: 1px solid #94a3b8;
            background: #fffbeb;
            color: #1e293b;
            padding: 4px 8px;
            border-radius: 4px;
        }
        QScrollBar:vertical {
            border: none;
            background: #f1f5f9;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #94a3b8;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #64748b; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        
        QComboBox QAbstractItemView {
            border: 1px solid #dce3ec;
            outline: none;
            selection-background-color: #1e40af;
            selection-color: white;
        }

        QComboBox QAbstractItemView::item {
            padding: 8px;
            color: #1e293b;
        }

        QComboBox QAbstractItemView::item:selected {
            background-color: #1e40af;
            color: white;
        }

        QComboBox QAbstractItemView::item:hover {
            background-color: #1e40af;
            color: white;
        }
    )");

    // ── Khởi tạo database SQLite ──────────────────────────────────────────────
    QString dbPath = qApp->applicationDirPath() + "/studentms.db";
    if (!DatabaseManager::instance().initialize(dbPath)) {
        QMessageBox::critical(nullptr, "Lỗi Khởi Tạo",
            "Không thể mở cơ sở dữ liệu!\n\n" +
            DatabaseManager::instance().lastError() +
            "\n\nKiểm tra quyền ghi thư mục.");
        return 1;
    }

    int ret = 1000;
    while (ret == 1000) {
        // ── Màn hình đăng nhập ────────────────────────────────────────────────────
        LoginDialog loginDlg;
        if (loginDlg.exec() != QDialog::Accepted) {
            ret = 0; // Thoát hẳn
            break;
        }

        // ── Cửa sổ chính ──────────────────────────────────────────────────────────
        MainWindow mainWin;
        mainWin.show();

        ret = app.exec();
    }

    // Dọn dẹp
    DatabaseManager::instance().close();
    return ret;
}
