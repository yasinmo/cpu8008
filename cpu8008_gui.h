/* CPU8008 - Intel 8008 Emulator, with emulated videocontroller
 * By Yasin Morsli
*/
#ifndef CPU8008_GUI_H
#define CPU8008_GUI_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollBar>
#include <QtConcurrent/QtConcurrent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <vector>
#include "ui_cpu8008.h"
#include "cpu8008.h"

#define CPU_SPEED           500000
#define INSTR_PER_FRAME     (CPU_SPEED / 60.0)
#define FRAME_IN_MS         ((1.0 / 60.0) * 1000.0)

#define SCREEN_W            128
#define SCREEN_H            128
#define SCREEN_SCALE        2

#define VRAM_ADDR           0x2000
#define get_red(x)          ((x >> 0) & 1)
#define get_green(x)        ((x >> 1) & 1)
#define get_blue(x)         ((x >> 2) & 1)

namespace Ui {
    class CPU8008GUI;
}

class CPU8008GUI : public QMainWindow{
    Q_OBJECT

    public:
        cpu8008 cpu;
        QTimer *cpu_run_timer;
        QTextCursor exec_cursor, select_cursor;
        uint32_t pc_current_line = 0;
        uint32_t disasm_max_lines = 0;
        uint32_t disasm_view_addr = 0;
        uint32_t mem_viewer_max_lines = 0;
        uint32_t mem_viewer_addr = 0;
        QGraphicsScene *screen_scene;
        QPixmap *screen_pixmap;
        
        explicit CPU8008GUI(QWidget *parent = 0);
        ~CPU8008GUI();
        
    public slots:
        void run_cpu();
        void run_cpu_frame();
        void stop_cpu();
        void step_cpu();
        void updateAll();
        void updateExecCursor();
        void updateSelectCursor();
        void createDisasm();
        void updateDisasm();
        void updateDisasmAddressLineEdit();
        void applyDisasmAddressLineEdit();
        void applyDisasmViewAddress();
        void updatePCLineEdit();
        void applyPCLineEdit();
        void updateRegisterInfo();
        void createMemoryViewer();
        void updateMemoryViewer();
        void updateMemoryViewerAddressLineEdit();
        void applyMemoryViewerAddressLineEdit();
        void adjustCursorMemoryViewer();
        void updateScreen();
        
        void openRom();
        void reassignPCDialog();
        
    private:
        Ui::CPU8008GUI *ui;
        
};

#endif // CPU8008_GUI_H
