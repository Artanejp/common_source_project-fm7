/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */

#include <QString>
//#include "menuclasses.h"
#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_flags.h"

extern USING_FLAGS *using_flags;

void Object_Menu_Control::set_boot_mode(void) {
	emit on_boot_mode(bindValue);
}
   
void Object_Menu_Control::set_cpu_type(void) {
	emit on_cpu_type(bindValue);
}
void Object_Menu_Control::set_cpupower(void) {
	emit on_cpu_power(bindValue);
}
void Object_Menu_Control::open_debugger(void) {
	emit on_open_debugger(bindValue);
}
void Object_Menu_Control::do_set_device_type(void){
	emit sig_device_type(this->getValue1());
}
void Object_Menu_Control::do_set_printer_device(void){
	emit sig_printer_device(this->getValue1());
}
void Object_Menu_Control::do_set_sound_device(void){
	emit sig_sound_device(this->getValue1());
}
void Object_Menu_Control::do_set_drive_type(void)
{
	emit sig_drive_type(getValue1());
}

void Ui_MainWindowBase::ConfigCpuSpeed(void)
{
	actionSpeed_x1 = new Action_Control(this);
	actionSpeed_x1->setObjectName(QString::fromUtf8("actionSpeed_x1"));
	actionSpeed_x1->setCheckable(true);
	actionSpeed_x1->setChecked(true);
	actionSpeed_x1->binds->setValue1(0);
	connect(actionSpeed_x1, SIGNAL(triggered()), actionSpeed_x1->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x1->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x2 = new Action_Control(this);
	actionSpeed_x2->setObjectName(QString::fromUtf8("actionSpeed_x2"));
	actionSpeed_x2->setCheckable(true);
	actionSpeed_x2->binds->setValue1(1);
	connect(actionSpeed_x2, SIGNAL(triggered()), actionSpeed_x2->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x2->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x4 = new Action_Control(this);
	actionSpeed_x4->setObjectName(QString::fromUtf8("actionSpeed_x4"));
	actionSpeed_x4->setCheckable(true);
	actionSpeed_x4->binds->setValue1(2);
	connect(actionSpeed_x4, SIGNAL(triggered()), actionSpeed_x4->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x4->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x8 = new Action_Control(this);
	actionSpeed_x8->setObjectName(QString::fromUtf8("actionSpeed_x8"));
	actionSpeed_x8->setCheckable(true);
	actionSpeed_x8->binds->setValue1(3);
	connect(actionSpeed_x8, SIGNAL(triggered()), actionSpeed_x8->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x8->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x16 = new Action_Control(this);
	actionSpeed_x16->setObjectName(QString::fromUtf8("actionSpeed_x16"));
	actionSpeed_x16->setCheckable(true);
	actionSpeed_x16->binds->setValue1(4);
	connect(actionSpeed_x16, SIGNAL(triggered()), actionSpeed_x16->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x16->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  

	actionGroup_CpuSpeed = new QActionGroup(this);
	actionGroup_CpuSpeed->setExclusive(true);
	actionGroup_CpuSpeed->addAction(actionSpeed_x1);
	actionGroup_CpuSpeed->addAction(actionSpeed_x2);
	actionGroup_CpuSpeed->addAction(actionSpeed_x4);
	actionGroup_CpuSpeed->addAction(actionSpeed_x8);
	actionGroup_CpuSpeed->addAction(actionSpeed_x16);
}
void Ui_MainWindowBase::do_change_boot_mode(int mode)
{
	if((mode < 0) || (mode >= 8)) return;
	config.boot_mode = mode;
	emit sig_emu_update_config();
}



void Ui_MainWindowBase::ConfigCPUBootMode(int num)
{
	int i;
	QString tmps;
	if(num <= 0) return;
	if(num >= 8) num = 7;
  
	actionGroup_BootMode = new QActionGroup(this);
	actionGroup_BootMode->setExclusive(true);
	for(i = 0; i < num; i++) {
		actionBootMode[i] = new Action_Control(this);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionBootMode_") + tmps;
		actionBootMode[i]->setObjectName(tmps);
		actionBootMode[i]->setCheckable(true);
		if(i == config.boot_mode) actionBootMode[i]->setChecked(true);
		actionBootMode[i]->binds->setValue1(i);
		menuBootMode->addAction(actionBootMode[i]);
		actionGroup_BootMode->addAction(actionBootMode[i]);
		connect(actionBootMode[i], SIGNAL(triggered()), actionBootMode[i]->binds, SLOT(set_boot_mode())); // OK?  
		connect(actionBootMode[i]->binds, SIGNAL(on_boot_mode(int)), this, SLOT(do_change_boot_mode(int))); // OK?  
	}
}

void Ui_MainWindowBase::do_change_cpu_type(int mode)
{
	if((mode < 0) || (mode >= 8)) return;
	config.cpu_type = mode;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::ConfigCPUTypes(int num)
{
	int i;
	QString tmps;
	if(num <= 0) return;
	if(num >= 8) num = 7;
   
	actionGroup_CpuType = new QActionGroup(this);
	actionGroup_CpuType->setExclusive(true);
	for(i = 0; i < num; i++) {
		actionCpuType[i] = new Action_Control(this);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionCpuType_") + tmps;
		actionCpuType[i]->setObjectName(tmps);
		actionCpuType[i]->setCheckable(true);
		if(i == config.cpu_type) actionCpuType[i]->setChecked(true);
		actionCpuType[i]->binds->setValue1(i);
		menuCpuType->addAction(actionCpuType[i]);
		actionGroup_CpuType->addAction(actionCpuType[i]);
		connect(actionCpuType[i], SIGNAL(triggered()), actionCpuType[i]->binds, SLOT(set_cpu_type())); // OK?  
		connect(actionCpuType[i]->binds, SIGNAL(on_cpu_type(int)), this, SLOT(do_change_cpu_type(int))); // OK?  
	}
}

void Ui_MainWindowBase::ConfigControlMenu(void)
{
	actionReset = new Action_Control(this);
	actionReset->setObjectName(QString::fromUtf8("actionReset"));
	connect(actionReset, SIGNAL(triggered()),
		this, SLOT(OnReset())); // OK?  
	if(using_flags->is_use_special_reset()) {
		actionSpecial_Reset = new Action_Control(this);
		actionSpecial_Reset->setObjectName(QString::fromUtf8("actionSpecial_Reset"));
		connect(actionSpecial_Reset, SIGNAL(triggered()),
				this, SLOT(OnSpecialReset())); // OK?
	}

	actionExit_Emulator = new Action_Control(this);
	actionExit_Emulator->setObjectName(QString::fromUtf8("actionExit_Emulator"));
	//connect(actionExit_Emulator, SIGNAL(triggered()),
	//	this, SLOT(on_actionExit_triggered())); // OnGuiExit()?  

	if(using_flags->is_use_auto_key()) {
		actionPaste_from_Clipboard = new Action_Control(this);
		actionPaste_from_Clipboard->setObjectName(QString::fromUtf8("actionPaste_from_Clipboard"));
		connect(actionPaste_from_Clipboard, SIGNAL(triggered()),
				this, SLOT(OnStartAutoKey())); // OK?  
		actionStop_Pasting = new Action_Control(this);
		actionStop_Pasting->setObjectName(QString::fromUtf8("actionStop_Pasting"));
		connect(actionStop_Pasting, SIGNAL(triggered()),
				this, SLOT(OnStopAutoKey())); // OK?
	}
	if(using_flags->is_use_state()) {
		actionSave_State = new Action_Control(this);
		actionSave_State->setObjectName(QString::fromUtf8("actionSave_State"));
		connect(actionSave_State, SIGNAL(triggered()),
				this, SLOT(OnSaveState())); // OK?  
		
		actionLoad_State = new Action_Control(this);
		actionLoad_State->setObjectName(QString::fromUtf8("actionLoad_State"));
		connect(actionLoad_State, SIGNAL(triggered()),
				this, SLOT(OnLoadState())); // OK?
	}
	if(using_flags->is_use_debugger()) {
		actionDebugger_1 = new Action_Control(this);
		actionDebugger_1->setObjectName(QString::fromUtf8("actionDebugger_1"));
		actionDebugger_1->binds->setValue1(0);
		connect(actionDebugger_1, SIGNAL(triggered()),
				actionDebugger_1->binds, SLOT(open_debugger())); // OK?  
		connect(actionDebugger_1->binds, SIGNAL(on_open_debugger(int)),
				this, SLOT(OnOpenDebugger(int))); // OK?  

		actionDebugger_2 = new Action_Control(this);
		actionDebugger_2->setObjectName(QString::fromUtf8("actionDebugger_2"));
		actionDebugger_2->binds->setValue1(1);
		connect(actionDebugger_2, SIGNAL(triggered()),
				actionDebugger_2->binds, SLOT(open_debugger())); // OK?  
		connect(actionDebugger_2->binds, SIGNAL(on_open_debugger(int)),
				this, SLOT(OnOpenDebugger(int))); // OK?  
		
		actionDebugger_3 = new Action_Control(this);
		actionDebugger_3->binds->setValue1(2);
		actionDebugger_3->setObjectName(QString::fromUtf8("actionDebugger_3"));
		connect(actionDebugger_3, SIGNAL(triggered()),
				actionDebugger_3->binds, SLOT(open_debugger())); // OK?  
		connect(actionDebugger_3->binds, SIGNAL(on_open_debugger(int)),
				this, SLOT(OnOpenDebugger(int))); // OK?  
	}
	ConfigCpuSpeed();
}

void Ui_MainWindowBase::connectActions_ControlMenu(void)
{
	menuControl->addAction(actionReset);
	if(using_flags->is_use_special_reset()) {
		menuControl->addAction(actionSpecial_Reset);
	}
	menuControl->addSeparator();
	menuControl->addAction(menuCpu_Speed->menuAction());

	if(using_flags->is_use_auto_key()) {
		menuControl->addSeparator();
		menuControl->addAction(menuCopy_Paste->menuAction());
	}
	menuControl->addSeparator();
	menuControl->addAction(menuState->menuAction());

	if(using_flags->is_use_debugger()) {
		menuControl->addAction(menuDebugger->menuAction());
	}
	menuControl->addSeparator();
	menuControl->addAction(actionExit_Emulator);

	if(using_flags->is_use_state()) {
		menuState->addAction(actionSave_State);
		menuState->addSeparator();
		menuState->addAction(actionLoad_State);
	}

	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste->addAction(actionPaste_from_Clipboard);
		menuCopy_Paste->addAction(actionStop_Pasting);
	}
	menuCpu_Speed->addAction(actionSpeed_x1);
	menuCpu_Speed->addAction(actionSpeed_x2);
	menuCpu_Speed->addAction(actionSpeed_x4);
	menuCpu_Speed->addAction(actionSpeed_x8);
	menuCpu_Speed->addAction(actionSpeed_x16);
	
	if(using_flags->is_use_debugger()) {
		menuDebugger->addAction(actionDebugger_1);
		menuDebugger->addAction(actionDebugger_2);
		menuDebugger->addAction(actionDebugger_3);
		menuDebugger->addSeparator();
	}
}

void Ui_MainWindowBase::createContextMenu(void)
{
	addAction(actionReset);

	if(using_flags->is_use_special_reset()) {
		addAction(actionSpecial_Reset);
	}
	addAction(menuCpu_Speed->menuAction());

	if(using_flags->is_use_auto_key()) {
		addAction(menuCopy_Paste->menuAction());
	}
	addAction(menuState->menuAction());

	if(using_flags->is_use_debugger()) {
		addAction(menuDebugger->menuAction());
	}
	addAction(actionExit_Emulator);
}


void Ui_MainWindowBase::retranslateControlMenu(const char *SpecialResetTitle,  bool WithSpecialReset)
{
	actionReset->setText(QApplication::translate("MainWindow", "Reset", 0));
	actionReset->setIcon(ResetIcon);
	if(using_flags->is_use_special_reset()) {
		actionSpecial_Reset->setText(QApplication::translate("MainWindow", SpecialResetTitle, 0));
		actionSpecial_Reset->setIcon(QIcon(":/icon_reset.png"));
	}

	actionExit_Emulator->setText(QApplication::translate("MainWindow", "Exit Emulator", 0));
	actionExit_Emulator->setIcon(ExitIcon);

	actionSpeed_x1->setText(QApplication::translate("MainWindow", "Speed x1", 0));
	actionSpeed_x2->setText(QApplication::translate("MainWindow", "Speed x2", 0));
	actionSpeed_x4->setText(QApplication::translate("MainWindow", "Speed x4", 0));
	actionSpeed_x8->setText(QApplication::translate("MainWindow", "Speed x8", 0));
	actionSpeed_x16->setText(QApplication::translate("MainWindow", "Speed x16", 0));
	
	if(using_flags->is_use_auto_key()) {
		actionPaste_from_Clipboard->setText(QApplication::translate("MainWindow", "Paste from Clipboard", 0));
		actionPaste_from_Clipboard->setIcon(QIcon(":/icon_paste.png"));
		actionStop_Pasting->setText(QApplication::translate("MainWindow", "Stop Pasting", 0));
		actionStop_Pasting->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
	}
	if(using_flags->is_use_state()) {
		actionSave_State->setText(QApplication::translate("MainWindow", "Save State", 0));
		actionSave_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
		actionLoad_State->setText(QApplication::translate("MainWindow", "Load State", 0));
		actionLoad_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
	}
	if(using_flags->is_use_debugger()) {
		actionDebugger_1->setText(QApplication::translate("MainWindow", "Debugger 1", 0));
		actionDebugger_2->setText(QApplication::translate("MainWindow", "Debugger 2", 0));
		actionDebugger_3->setText(QApplication::translate("MainWindow", "Debugger 3", 0));
		menuDebugger->setTitle(QApplication::translate("MainWindow", "Debugger", 0));
	}
	menuControl->setTitle(QApplication::translate("MainWindow", "Control", 0));
	menuState->setTitle(QApplication::translate("MainWindow", "State", 0));

	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste->setTitle(QApplication::translate("MainWindow", "Copy/Paste", 0));
	}
	menuCpu_Speed->setTitle(QApplication::translate("MainWindow", "CPU Speed", 0));
	if(using_flags->is_use_mouse()) {
		actionMouseEnable->setText(QApplication::translate("MainWindow", "Grab MOUSE", 0));
	}
}

void Ui_MainWindowBase::do_set_sound_device(int num)
{
	if((num < 0) || (num >= using_flags->get_use_sound_device_type())) return;
	config.sound_device_type = num;
	emit sig_emu_update_config();
}
