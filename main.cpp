/* CPU8008 - Intel 8008 Emulator, with emulated videocontroller
 * By Yasin Morsli
*/
using namespace std;

#include <stdio.h>
#include <iostream>
#include <QApplication>
#include "cpu8008_gui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CPU8008GUI w;
    w.show();
    
    return a.exec();
}
