/* CPU8008 - Intel 8008 Emulator, with emulated videocontroller
 * By Yasin Morsli
*/
#include "cpu8008_gui.h"
#include "ui_cpu8008.h"
#include "cpu8008.h"

CPU8008GUI::CPU8008GUI(QWidget *parent) : QMainWindow(parent), ui(new Ui::CPU8008GUI){
    ui->setupUi(this);
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionRun, SIGNAL(triggered(bool)), this, SLOT(run_cpu()));
    connect(ui->actionStop, SIGNAL(triggered(bool)), this, SLOT(stop_cpu()));
    connect(ui->actionStep, SIGNAL(triggered(bool)), this, SLOT(step_cpu()));
    connect(ui->actionOpen_ROM, SIGNAL(triggered(bool)), this, SLOT(openRom()));
    connect(ui->actionGoto, SIGNAL(triggered(bool)), this, SLOT(reassignPCDialog()));
    
    connect(ui->text_view_disasm, SIGNAL(cursorPositionChanged()), this, SLOT(updateSelectCursor()));
    
    connect(ui->line_edit_disasm_addr, SIGNAL(returnPressed()), this, SLOT(applyDisasmAddressLineEdit()));
    connect(ui->line_edit_disasm_current_pc, SIGNAL(returnPressed()), this, SLOT(applyPCLineEdit()));
    connect(ui->line_edit_mem_viewer_addr, SIGNAL(returnPressed()), this, SLOT(applyMemoryViewerAddressLineEdit()));
    
    
//    ui->text_view_hex_data->setOverwriteMode(true);
    connect(ui->text_view_hex_data, SIGNAL(textChanged()), this, SLOT(adjustCursorMemoryViewer()));
    
    disasm_max_lines = (ui->text_view_disasm->height() / ui->text_view_disasm->fontMetrics().height()) - 1;
    mem_viewer_max_lines = (ui->text_view_hex_data->height() / ui->text_view_hex_data->fontMetrics().height()) - 1;
    
    ui->text_view_disasm->verticalScrollBar()->setMinimum(0);
    ui->text_view_disasm->verticalScrollBar()->setMaximum(sizeof(cpu.mem));
    
    cpu_run_timer = new QTimer(this);
    connect(cpu_run_timer, SIGNAL(timeout()), this, SLOT(run_cpu_frame()));
    
    screen_scene = new QGraphicsScene(0, 0, ui->screen_gfx_view->width(), ui->screen_gfx_view->height());
    ui->screen_gfx_view->setScene(screen_scene);
    screen_pixmap = new QPixmap(SCREEN_W, SCREEN_H);
    screen_scene->addPixmap(*screen_pixmap);
    screen_scene->update();
    
    cpu.reset();
    createDisasm();
    createMemoryViewer();
    updateAll();
}

CPU8008GUI::~CPU8008GUI(){
    delete ui;
}

void CPU8008GUI::run_cpu(){
    cpu_run_timer->start(FRAME_IN_MS);
}

void CPU8008GUI::run_cpu_frame(){
    for(uint32_t i = 0; (i < INSTR_PER_FRAME); i++){
        cpu.step();
    }
    updateScreen();
}

void CPU8008GUI::stop_cpu(){
    cpu_run_timer->stop();
    updateAll();
}

void CPU8008GUI::step_cpu(){
    cpu_run_timer->stop();
    cpu.step();
    updateAll();
}

void CPU8008GUI::updateAll(){
    updateDisasmAddressLineEdit();
    updatePCLineEdit();
    updateDisasm();
    updateRegisterInfo();
    updateSelectCursor();
    updateExecCursor();
    updateMemoryViewer();
    updateScreen();
}

void CPU8008GUI::updateExecCursor(){
    QList<QTextEdit::ExtraSelection> _selections;
    QTextEdit::ExtraSelection _selection;
    
    exec_cursor = ui->text_view_disasm->textCursor();
    exec_cursor.setPosition(0);
    exec_cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pc_current_line);
    
    ui->text_view_disasm->setTextCursor(exec_cursor);
    _selection.cursor = exec_cursor;
    _selection.cursor.clearSelection();
    _selection.format.setBackground(QColor(Qt::green).lighter(170));
    _selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    _selections.append(_selection);
    ui->text_view_disasm->setExtraSelections(_selections);
}

void CPU8008GUI::updateSelectCursor(){
    QList<QTextEdit::ExtraSelection> _selections;
    QTextEdit::ExtraSelection _selection;
    
    select_cursor = ui->text_view_disasm->textCursor();
    select_cursor.movePosition(QTextCursor::StartOfLine);
    
    _selection.cursor = exec_cursor;
    _selection.cursor.clearSelection();
    _selection.format.setBackground(QColor(Qt::green).lighter(170));
    _selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    _selections.append(_selection);
    
    ui->text_view_disasm->setTextCursor(select_cursor);
    _selection.cursor = select_cursor;
    _selection.cursor.clearSelection();
    _selection.format.setBackground(QColor(Qt::blue).lighter(170));
    _selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    _selections.append(_selection);
    ui->text_view_disasm->setExtraSelections(_selections);
}

void CPU8008GUI::createDisasm(){
    uint32_t _addr = 0;
    string _disasm_str = "";
    pc_current_line = 0;
    for(uint32_t i = 0; i < disasm_max_lines; i++){
        _disasm_str += cpu.debug_instr_info(_addr) + string("\n");
        _addr += cpu.get_instruction_length(_addr);
    }
    ui->text_view_disasm->setPlainText(QString(_disasm_str.c_str()));
}

void CPU8008GUI::updateDisasm(){
    uint32_t _line_c = 0;
    uint32_t _addr = cpu.pc[cpu.pc_p];
    string _disasm_str = "";
    pc_current_line = 0;
    for(uint32_t i = 0; (i < (disasm_max_lines / 2)) && (_addr > 0); i++){
        if(((cpu.get_instruction_length(_addr - 3)) == 3) && (_addr > 2)){
            _addr -= 3;
            _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        else if(((cpu.get_instruction_length(_addr - 2)) == 2) && (_addr > 1)){
            _addr -= 2;
            _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        else{
            _addr -= 1;
            if(cpu.get_instruction_length(_addr) > 1)
                _disasm_str = cpu.debug_unknown_info(_addr) + string("\n") + _disasm_str;
            else
                _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        pc_current_line++;
        _line_c++;
    }
    _addr = cpu.pc[cpu.pc_p];
    for(uint32_t i = _line_c; (i < disasm_max_lines) && (_addr < 0x4000); i++){
        _disasm_str += cpu.debug_instr_info(_addr);
        _addr += cpu.get_instruction_length(_addr);
        if((i < (disasm_max_lines - 1)) && (_addr < 0x4000)) _disasm_str += string("\n");
    }
    ui->text_view_disasm->setPlainText(QString(_disasm_str.c_str()));
}

void CPU8008GUI::updateDisasmAddressLineEdit(){
    int32_t _addr = disasm_view_addr;
    if(_addr < 0) _addr = 0x0000;
    char _char_buf[10];
    sprintf(_char_buf, "0x%04X", _addr);
    ui->line_edit_disasm_addr->setText(QString(_char_buf));
}

void CPU8008GUI::applyDisasmAddressLineEdit(){
    bool _conv_ok;
    QString _new_pc_str = ui->line_edit_disasm_addr->text();
    _new_pc_str.toInt(&_conv_ok, 0);
    if(_conv_ok) disasm_view_addr = (_new_pc_str.toInt(&_conv_ok, 0) % sizeof(cpu.mem));
    
    applyDisasmViewAddress();
}

void CPU8008GUI::updatePCLineEdit(){
    char _char_buf[10];
    sprintf(_char_buf, "0x%04X", cpu.pc[cpu.pc_p]);
    ui->line_edit_disasm_current_pc->setText(QString(_char_buf));
}

void CPU8008GUI::applyPCLineEdit(){
    bool _conv_ok;
    QString _new_pc_str = ui->line_edit_disasm_current_pc->text();
    _new_pc_str.toInt(&_conv_ok, 0);
    if(_conv_ok) cpu.pc[cpu.pc_p] = (_new_pc_str.toInt(&_conv_ok, 0) % sizeof(cpu.mem));
    
    updateAll();
}

void CPU8008GUI::applyDisasmViewAddress(){
    uint32_t _line_c = 0;
    uint32_t _new_addr, _addr;
    string _disasm_str = "";
    bool _conv_ok;
    
    QString _new_pc_str = ui->line_edit_disasm_addr->text();
    _new_pc_str.toInt(&_conv_ok, 0);
    if(_conv_ok) _new_addr = (_new_pc_str.toInt(&_conv_ok, 0) % sizeof(cpu.mem));
    else         _new_addr = cpu.pc[cpu.pc_p];
    
    _addr = _new_addr;
    for(uint32_t i = 0; (i < (disasm_max_lines / 2)) && (_addr > 0); i++){
        if(((cpu.get_instruction_length(_addr - 3)) == 3) && (_addr > 2)){
            _addr -= 3;
            _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        else if(((cpu.get_instruction_length(_addr - 2)) == 2) && (_addr > 1)){
            _addr -= 2;
            _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        else{
            _addr -= 1;
            if(cpu.get_instruction_length(_addr) > 1)
                _disasm_str = cpu.debug_unknown_info(_addr) + string("\n") + _disasm_str;
            else
                _disasm_str = cpu.debug_instr_info(_addr) + string("\n") + _disasm_str;
        }
        _line_c++;
    }
    _addr = _new_addr;
    for(uint32_t i = _line_c; (i < disasm_max_lines) && (_addr < 0x4000); i++){
        _disasm_str += cpu.debug_instr_info(_addr);
        _addr += cpu.get_instruction_length(_addr);
        if((i < (disasm_max_lines - 1)) && (_addr < 0x4000)) _disasm_str += string("\n");
    }
    ui->text_view_disasm->setPlainText(QString(_disasm_str.c_str()));
    
    updateDisasmAddressLineEdit();
    updateSelectCursor();
    updateExecCursor();
}

void CPU8008GUI::updateRegisterInfo(){
    ui->list_view_registers->clear();
    ui->list_view_stack->clear();
    char _char_buf[20];
    sprintf(_char_buf, "Flags       XXXX");
    _char_buf[12] = (cpu.carry)  ? 'C' : 'c';
    _char_buf[13] = (cpu.parity) ? 'P' : 'p';
    _char_buf[14] = (cpu.zero)   ? 'Z' : 'z';
    _char_buf[15] = (cpu.sign)   ? 'S' : 's';
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC        0x%04X", cpu.pc[cpu.pc_p]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "A           0x%02X", cpu.regs[cpu.REGS_A]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "B           0x%02X", cpu.regs[cpu.REGS_B]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "C           0x%02X", cpu.regs[cpu.REGS_C]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "D           0x%02X", cpu.regs[cpu.REGS_D]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "E           0x%02X", cpu.regs[cpu.REGS_E]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "H           0x%02X", cpu.regs[cpu.REGS_H]);
    ui->list_view_registers->addItem(QString(_char_buf));
    sprintf(_char_buf, "L           0x%02X", cpu.regs[cpu.REGS_L]);
    ui->list_view_registers->addItem(QString(_char_buf));
    
    sprintf(_char_buf, "StackP      0x%02X", cpu.pc_p);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC0       0x%04X", cpu.pc[0]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC1       0x%04X", cpu.pc[1]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC2       0x%04X", cpu.pc[2]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC3       0x%04X", cpu.pc[3]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC4       0x%04X", cpu.pc[4]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC5       0x%04X", cpu.pc[5]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC6       0x%04X", cpu.pc[6]);
    ui->list_view_stack->addItem(QString(_char_buf));
    sprintf(_char_buf, "PC7       0x%04X", cpu.pc[7]);
    ui->list_view_stack->addItem(QString(_char_buf));
}

void CPU8008GUI::createMemoryViewer(){
    string _mem_str = "";
    string _addr_str = "";
    char _char_buf[256];
    uint32_t _lines = 4;
    for(uint32_t i = 0; i < sizeof(cpu.mem); i++){
        if(((i % 16) == 0)){
            sprintf(_char_buf, "00:%04X\n", i);
            _addr_str += string(_char_buf);
            if(i != 0){
                _mem_str += string("\n");
                _lines++;
            }
        }
        else{
            _mem_str += string(" ");
        }
        sprintf(_char_buf, "%02X", cpu.mem[i]);
        _mem_str += string(_char_buf);
    }
    sprintf(_char_buf, "0x%04X", mem_viewer_addr);
    ui->line_edit_mem_viewer_addr->setText(QString(_char_buf));
    ui->text_view_hex_address->setPlainText(QString(_addr_str.c_str()));
    ui->text_view_hex_data->setPlainText(QString(_mem_str.c_str()));
    ui->scroll_widget_hex_viewer_layout->resize(ui->scroll_widget_hex_viewer_layout->contentsRect().width(), _lines * ui->text_view_hex_data->fontMetrics().height());
}

void CPU8008GUI::updateMemoryViewer(){
/*
    char _char_buf[10];
    for(uint32_t i = 0; i < cpu.last_write.size(); i++){
        sprintf(_char_buf, "%02X", cpu.mem[cpu.last_write.at(i)]);
        QTextCursor _tcur =  ui->text_view_hex_data->textCursor();
        _tcur.setPosition(3 * cpu.last_write.at(i));
        _tcur.setPosition((3 * cpu.last_write.at(i)) + 2, QTextCursor::KeepAnchor);
        ui->text_view_hex_data->setTextCursor(_tcur);
        ui->text_view_hex_data->textCursor().insertText(_char_buf);
    }
*/
    QString _addr_str = "";
    QString _data_str = "";
    char _char_buf[20];
    for(int i = 0; i < mem_viewer_max_lines; i++){
        sprintf(_char_buf, "00:%04X\n", mem_viewer_addr + (i * 16));
        _addr_str += QString(_char_buf);
        for(int j = 0; j < 16; j++){
            sprintf(_char_buf, "%02X", cpu.mem[mem_viewer_addr + (i * 16) + j]);
            _data_str += QString(_char_buf);
            if(j < 15) _data_str += " ";
        }
        if(i < (mem_viewer_max_lines - 1)) _data_str += "\n";
    }
    ui->text_view_hex_address->setPlainText(_addr_str);
    ui->text_view_hex_data->setPlainText(_data_str);
}

void CPU8008GUI::updateMemoryViewerAddressLineEdit(){
    
}

void CPU8008GUI::applyMemoryViewerAddressLineEdit(){
    bool _conv_ok;
    char _char_buf[10];
    
    QString _new_addr_str = ui->line_edit_mem_viewer_addr->text();
    _new_addr_str.toInt(&_conv_ok, 0);
    if(_conv_ok) mem_viewer_addr = ((_new_addr_str.toInt(&_conv_ok, 0) % sizeof(cpu.mem)) & 0xFFF0);
    
    sprintf(_char_buf, "0x%04X", mem_viewer_addr);
    ui->line_edit_mem_viewer_addr->setText(QString(_char_buf));
    
    updateMemoryViewer();
}

void CPU8008GUI::openRom(){
    QString _rom_filepath = QFileDialog::getOpenFileName(this, QString("Select ROM File"), "./", QString("ROM File (*.bin)"));
    QFile _rom_file(_rom_filepath);
    if(!_rom_file.open(QIODevice::ReadOnly)){
        QMessageBox _error_dialog;
        _error_dialog.setWindowTitle(QString("Error"));
        _error_dialog.setText("Could not open ROM File");
        _error_dialog.exec();
        return;
    }
    _rom_file.read((char*)cpu.mem, sizeof(cpu.mem));
    _rom_file.close();
    
    cpu.reset();
    createDisasm();
    createMemoryViewer();
    updateAll();
}

void CPU8008GUI::reassignPCDialog(){
    bool _conv_ok;
    QString _new_pc_str = QInputDialog::getText(this, QString("Goto"), QString(""));
    _new_pc_str.toInt(&_conv_ok, 0);
    if(_conv_ok) cpu.pc[cpu.pc_p] = (_new_pc_str.toInt(&_conv_ok, 0) % sizeof(cpu.mem));
    else{
        QMessageBox _error_dialog;
        _error_dialog.setWindowTitle(QString("Error"));
        _error_dialog.setText("Not a Number");
        _error_dialog.exec();
        return;
    }
    
    updateAll();
}

void CPU8008GUI::adjustCursorMemoryViewer(){
    QTextCursor _tcur = ui->text_view_hex_data->textCursor();
    if(((_tcur.position() % 3) == 2)) _tcur.movePosition(QTextCursor::Right);
    ui->text_view_hex_data->setTextCursor(_tcur);
}

void CPU8008GUI::updateScreen(){
    QPainter _painter(screen_pixmap);
    uint8_t _pixel, _r, _g, _b;
    for(uint32_t i = 0; i < SCREEN_H; i++){
        for(uint32_t j = 0; j < SCREEN_H; j++){
            _pixel = cpu.mem[VRAM_ADDR + (((i * SCREEN_W) + j) / 2)] >> (4 * (((i * SCREEN_W) + j) % 2));
            _r = get_red(_pixel)   ? 0xFF : 0x00;
            _g = get_green(_pixel) ? 0xFF : 0x00;
            _b = get_blue(_pixel)  ? 0xFF : 0x00;
            _painter.setPen(QColor(_r, _g, _b));
            _painter.drawPoint(j, i);
        }
    }
    _painter.end();
    screen_scene->clear();
    screen_scene->addPixmap(screen_pixmap->scaled(screen_scene->width(), screen_scene->height()));
    screen_scene->update();
}
