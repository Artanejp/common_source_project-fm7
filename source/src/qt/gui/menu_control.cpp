/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */

#include <QString>
#include <QMenu>
#include <QStyle>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_flags.h"

//extern USING_FLAGS *using_flags;

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

void Action_Control::do_load_state(void)
{
	emit sig_load_state(binds->getStringValue());
}

void Action_Control::do_save_state(void)
{
	emit sig_save_state(binds->getStringValue());
}

void Ui_MainWindowBase::ConfigCpuSpeed(void)
{
	actionSpeed_x1 = new Action_Control(this, using_flags);
	actionSpeed_x1->setObjectName(QString::fromUtf8("actionSpeed_x1"));
	actionSpeed_x1->setCheckable(true);
	actionSpeed_x1->setChecked(true);
	actionSpeed_x1->binds->setValue1(0);
	connect(actionSpeed_x1, SIGNAL(triggered()), actionSpeed_x1->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x1->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x2 = new Action_Control(this, using_flags);
	actionSpeed_x2->setObjectName(QString::fromUtf8("actionSpeed_x2"));
	actionSpeed_x2->setCheckable(true);
	actionSpeed_x2->binds->setValue1(1);
	connect(actionSpeed_x2, SIGNAL(triggered()), actionSpeed_x2->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x2->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x4 = new Action_Control(this, using_flags);
	actionSpeed_x4->setObjectName(QString::fromUtf8("actionSpeed_x4"));
	actionSpeed_x4->setCheckable(true);
	actionSpeed_x4->binds->setValue1(2);
	connect(actionSpeed_x4, SIGNAL(triggered()), actionSpeed_x4->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x4->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x8 = new Action_Control(this, using_flags);
	actionSpeed_x8->setObjectName(QString::fromUtf8("actionSpeed_x8"));
	actionSpeed_x8->setCheckable(true);
	actionSpeed_x8->binds->setValue1(3);
	connect(actionSpeed_x8, SIGNAL(triggered()), actionSpeed_x8->binds, SLOT(set_cpupower())); // OK?  
	connect(actionSpeed_x8->binds, SIGNAL(on_cpu_power(int)), this, SLOT(set_cpu_power(int))); // OK?  
  
	actionSpeed_x16 = new Action_Control(this, using_flags);
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
	using_flags->get_config_ptr()->boot_mode = mode;
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
		actionBootMode[i] = new Action_Control(this, using_flags);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionBootMode_") + tmps;
		actionBootMode[i]->setObjectName(tmps);
		actionBootMode[i]->setCheckable(true);
		if(i == using_flags->get_config_ptr()->boot_mode) actionBootMode[i]->setChecked(true);
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
	using_flags->get_config_ptr()->cpu_type = mode;
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
		actionCpuType[i] = new Action_Control(this, using_flags);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionCpuType_") + tmps;
		actionCpuType[i]->setObjectName(tmps);
		actionCpuType[i]->setCheckable(true);
		if(i == using_flags->get_config_ptr()->cpu_type) actionCpuType[i]->setChecked(true);
		actionCpuType[i]->binds->setValue1(i);
		menuCpuType->addAction(actionCpuType[i]);
		actionGroup_CpuType->addAction(actionCpuType[i]);
		connect(actionCpuType[i], SIGNAL(triggered()), actionCpuType[i]->binds, SLOT(set_cpu_type())); // OK?  
		connect(actionCpuType[i]->binds, SIGNAL(on_cpu_type(int)), this, SLOT(do_change_cpu_type(int))); // OK?  
	}
}

void Ui_MainWindowBase::ConfigControlMenu(void)
{
	int i;
	actionReset = new Action_Control(this, using_flags);
	actionReset->setObjectName(QString::fromUtf8("actionReset"));
	connect(actionReset, SIGNAL(triggered()),
		this, SLOT(OnReset())); // OK?  
	if(using_flags->is_use_special_reset()) {
		actionSpecial_Reset = new Action_Control(this, using_flags);
		actionSpecial_Reset->setObjectName(QString::fromUtf8("actionSpecial_Reset"));
		connect(actionSpecial_Reset, SIGNAL(triggered()),
				this, SLOT(OnSpecialReset())); // OK?
	}

	actionExit_Emulator = new Action_Control(this, using_flags);
	actionExit_Emulator->setObjectName(QString::fromUtf8("actionExit_Emulator"));
	//connect(actionExit_Emulator, SIGNAL(triggered()),
	//	this, SLOT(on_actionExit_triggered())); // OnGuiExit()?  

	if(using_flags->is_use_auto_key()) {
		actionPaste_from_Clipboard = new Action_Control(this, using_flags);
		actionPaste_from_Clipboard->setObjectName(QString::fromUtf8("actionPaste_from_Clipboard"));
		connect(actionPaste_from_Clipboard, SIGNAL(triggered()),
				this, SLOT(OnStartAutoKey())); // OK?  
		actionStop_Pasting = new Action_Control(this, using_flags);
		actionStop_Pasting->setObjectName(QString::fromUtf8("actionStop_Pasting"));
		connect(actionStop_Pasting, SIGNAL(triggered()),
				this, SLOT(OnStopAutoKey())); // OK?
	}
	if(using_flags->is_use_state()) {
		for(i = 0; i < 10; i++) {
			QString tmps;
			QString tmpss;
			_TCHAR tmpbuf[_MAX_PATH];
			actionSave_State[i] = new Action_Control(this, using_flags);

			strncpy(tmpbuf, create_local_path(_T("%s.sta%d"), using_flags->get_config_name().toLocal8Bit().constData(), i), _MAX_PATH);
				
			tmps = QString::fromUtf8("");
			tmpss = QString::fromUtf8("");
			tmpss.setNum(i);
			tmps = QString::fromUtf8("actionSave_State") + tmpss;
			actionSave_State[i]->setObjectName(tmps);
			actionLoad_State[i] = new Action_Control(this, using_flags);
			tmps = QString::fromUtf8("actionLoad_State") + tmpss;
			actionLoad_State[i]->setObjectName(tmps);
			tmps = QString::fromLocal8Bit(tmpbuf);
			actionSave_State[i]->binds->setStringValue(tmps);
			actionLoad_State[i]->binds->setStringValue(tmps);
				
		
			connect(actionLoad_State[i], SIGNAL(triggered()),
					actionLoad_State[i], SLOT(do_load_state())); // OK?
			connect(actionSave_State[i], SIGNAL(triggered()),
					actionSave_State[i], SLOT(do_save_state())); // OK?
			
		}
	}
	if(using_flags->is_use_debugger()) {
		for(i = 0; i < _MAX_DEBUGGER; i++) {
			QString tmps;
			tmps.setNum(i);
			actionDebugger[i] = new Action_Control(this, using_flags);
			actionDebugger[i]->setObjectName(QString::fromUtf8("actionDebugger") + tmps);
			actionDebugger[i]->binds->setValue1(i);
			connect(actionDebugger[i], SIGNAL(triggered()),
					actionDebugger[i]->binds, SLOT(open_debugger())); // OK?  
			connect(actionDebugger[i]->binds, SIGNAL(on_open_debugger(int)),
					this, SLOT(OnOpenDebugger(int))); // OK?
		}
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
		for(int i = 0; i < 10; i++) {
			menuLoad_State->addAction(actionLoad_State[i]);
			menuSave_State->addAction(actionSave_State[i]);
		}
		menuState->addAction(menuSave_State->menuAction());
		menuState->addSeparator();
		menuState->addAction(menuLoad_State->menuAction());
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
		int i;
		for(i = 0; i < _MAX_DEBUGGER; i++) {
			menuDebugger->addAction(actionDebugger[i]);
			if(i > 0) actionDebugger[i]->setVisible(false);
		}
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
	actionReset->setToolTip(QApplication::translate("MainWindow", "Reset virtual machine.", 0));
	actionReset->setIcon(ResetIcon);
	if(using_flags->is_use_special_reset()) {
		actionSpecial_Reset->setText(QApplication::translate("MainWindow", SpecialResetTitle, 0));
		actionSpecial_Reset->setIcon(QIcon(":/icon_reset.png"));
	}

	actionExit_Emulator->setText(QApplication::translate("MainWindow", "Exit Emulator", 0));
	actionExit_Emulator->setToolTip(QApplication::translate("MainWindow", "Exit emulator.\n**WARN: WITHOUT confirming.**", 0));
	actionExit_Emulator->setIcon(ExitIcon);

	actionSpeed_x1->setText(QApplication::translate("MainWindow", "Speed x1", 0));
	actionSpeed_x2->setText(QApplication::translate("MainWindow", "Speed x2", 0));
	actionSpeed_x4->setText(QApplication::translate("MainWindow", "Speed x4", 0));
	actionSpeed_x8->setText(QApplication::translate("MainWindow", "Speed x8", 0));
	actionSpeed_x16->setText(QApplication::translate("MainWindow", "Speed x16", 0));
	
	if(using_flags->is_use_auto_key()) {
		actionPaste_from_Clipboard->setText(QApplication::translate("MainWindow", "Paste from Clipboard", 0));
		actionPaste_from_Clipboard->setToolTip(QApplication::translate("MainWindow", "Paste ANK text to virtual machine from desktop's clop board.", 0));
		actionPaste_from_Clipboard->setIcon(QIcon(":/icon_paste.png"));
		actionStop_Pasting->setText(QApplication::translate("MainWindow", "Stop Pasting", 0));
		actionStop_Pasting->setToolTip(QApplication::translate("MainWindow", "Abort pasting ANK text to virtual machine.", 0));
		actionStop_Pasting->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
	}
	if(using_flags->is_use_state()) {
		menuSave_State->setTitle(QApplication::translate("MainWindow", "Save State", 0));
		menuSave_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
		menuSave_State->setToolTip(QApplication::translate("MainWindow", "Save snapshot to fixed bin file.", 0));
		menuLoad_State->setTitle(QApplication::translate("MainWindow", "Load State", 0));
		menuLoad_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
		menuLoad_State->setToolTip(QApplication::translate("MainWindow", "Load snapshot from fixed bin file.", 0));
		for(int i = 0; i < 10; i++) {
			QString tmps;
			tmps.clear();
			tmps.setNum(i);
			tmps = QString::fromUtf8("Slot #") + tmps;
			actionSave_State[i]->setText(tmps);
			actionLoad_State[i]->setText(tmps);
		}
	}
	if(using_flags->is_use_debugger()) {
		actionDebugger[0]->setText(QApplication::translate("MainWindow", "Main CPU", 0));
		actionDebugger[1]->setText(QApplication::translate("MainWindow", "Sub CPU", 0));
		actionDebugger[2]->setText(QApplication::translate("MainWindow", "Debugger 3", 0));
		actionDebugger[3]->setText(QApplication::translate("MainWindow", "Debugger 4", 0));
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
		actionMouseEnable->setToolTip(QApplication::translate("MainWindow", "Grabbing host's mouse.\nPress RIGHT Application key (or another) to toggle enable/disable.", 0));
	}
}

void Ui_MainWindowBase::do_set_sound_device(int num)
{
	if((num < 0) || (num >= using_flags->get_use_sound_device_type())) return;
	using_flags->get_config_ptr()->sound_type = num;
	emit sig_emu_update_config();
}
