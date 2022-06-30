/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  History: Jan 10, 2015 : Initial
 */

#include <QString>
#include <QMenu>
#include <QStyle>
#include <QApplication>
#include <QActionGroup>

#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_flags.h"



void Ui_MainWindowBase::ConfigCpuSpeed(void)
{
	actionSpeed_x1 = new Action_Control(this, using_flags);
	actionSpeed_x1->setObjectName(QString::fromUtf8("actionSpeed_x1"));
	actionSpeed_x1->setCheckable(true);
	actionSpeed_x1->setChecked(true);
	actionSpeed_x1->setData(QVariant((int)0));
  
	actionSpeed_x2 = new Action_Control(this, using_flags);
	actionSpeed_x2->setObjectName(QString::fromUtf8("actionSpeed_x2"));
	actionSpeed_x2->setCheckable(true);
	actionSpeed_x2->setData(QVariant((int)1));
  
	actionSpeed_x4 = new Action_Control(this, using_flags);
	actionSpeed_x4->setObjectName(QString::fromUtf8("actionSpeed_x4"));
	actionSpeed_x4->setCheckable(true);
	actionSpeed_x4->setData(QVariant((int)2));
  
	actionSpeed_x8 = new Action_Control(this, using_flags);
	actionSpeed_x8->setObjectName(QString::fromUtf8("actionSpeed_x8"));
	actionSpeed_x8->setCheckable(true);
	actionSpeed_x8->setData(QVariant((int)3));
  
	actionSpeed_x16 = new Action_Control(this, using_flags);
	actionSpeed_x16->setObjectName(QString::fromUtf8("actionSpeed_x16"));
	actionSpeed_x16->setCheckable(true);
	actionSpeed_x16->setData(QVariant((int)4));
	
	connect(actionSpeed_x1 , SIGNAL(triggered()), this, SLOT(do_set_cpu_power())); // OK?  
	connect(actionSpeed_x2 , SIGNAL(triggered()), this, SLOT(do_set_cpu_power())); // OK?  
	connect(actionSpeed_x4 , SIGNAL(triggered()), this, SLOT(do_set_cpu_power())); // OK?  
	connect(actionSpeed_x8 , SIGNAL(triggered()), this, SLOT(do_set_cpu_power())); // OK?  
	connect(actionSpeed_x16, SIGNAL(triggered()), this, SLOT(do_set_cpu_power())); // OK?  

	actionGroup_CpuSpeed = new QActionGroup(this);
	actionGroup_CpuSpeed->setExclusive(true);
	actionGroup_CpuSpeed->addAction(actionSpeed_x1);
	actionGroup_CpuSpeed->addAction(actionSpeed_x2);
	actionGroup_CpuSpeed->addAction(actionSpeed_x4);
	actionGroup_CpuSpeed->addAction(actionSpeed_x8);
	actionGroup_CpuSpeed->addAction(actionSpeed_x16);
	//actionGroup_CpuSpeed->addAction(actionSpeed_FULL);

	switch(p_config->cpu_power) {
	case 0:
		actionSpeed_x1->setChecked(true);
		break;
	case 1:
		actionSpeed_x2->setChecked(true);
		break;
	case 2:
		actionSpeed_x4->setChecked(true);
		break;
	case 3:
		actionSpeed_x8->setChecked(true);
		break;
	case 4:
		actionSpeed_x16->setChecked(true);
		break;
	default:
		p_config->cpu_power = 0;
		actionSpeed_x1->setChecked(true);
		break;
	}
}
void Ui_MainWindowBase::do_change_boot_mode(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int mode = cp->data().value<int>();
	
	if((mode < 0) || (mode >= 8)) return;
	p_config->boot_mode = mode;
	emit sig_emu_update_config();
}



void Ui_MainWindowBase::ConfigCPUBootMode(int num)
{
	int i;
	QString tmps;
	
	if(num <= 0) return;
	if(num >= 8) num = 8;
  
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	actionGroup_BootMode = new QActionGroup(this);
	actionGroup_BootMode->setExclusive(true);
	for(i = 0; i < num; i++) {
		actionBootMode[i] = new Action_Control(this, using_flags);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionBootMode_") + tmps;
		actionBootMode[i]->setObjectName(tmps);
		actionBootMode[i]->setCheckable(true);
		if(i == p_config->boot_mode) actionBootMode[i]->setChecked(true);
		actionBootMode[i]->setData(QVariant(i));
		menuBootMode->addAction(actionBootMode[i]);
		actionGroup_BootMode->addAction(actionBootMode[i]);
		connect(actionBootMode[i], SIGNAL(triggered()), this, SLOT(do_change_boot_mode())); // OK?  
	}
	menuMachine->addAction(menuBootMode->menuAction());
}

void Ui_MainWindowBase::do_change_cpu_type(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int mode = cp->data().value<int>();
	
	if((mode < 0) || (mode >= 8)) return;
	p_config->cpu_type = mode;
	emit sig_emu_update_config();
}

void Ui_MainWindowBase::ConfigCPUTypes(int num)
{
	int i;
	QString tmps;
	if(num <= 0) return;
	if(num >= 8) num = 7;
	menuCpuType = new QMenu(menuMachine);
	menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
	
	actionGroup_CpuType = new QActionGroup(this);
	actionGroup_CpuType->setExclusive(true);
	for(i = 0; i < num; i++) {
		actionCpuType[i] = new Action_Control(this, using_flags);
		tmps = tmps.setNum(i);
		tmps = QString::fromUtf8("actionCpuType_") + tmps;
		actionCpuType[i]->setObjectName(tmps);
		actionCpuType[i]->setCheckable(true);
		if(i == p_config->cpu_type) actionCpuType[i]->setChecked(true);
		actionCpuType[i]->setData(QVariant(i));
		menuCpuType->addAction(actionCpuType[i]);
		actionGroup_CpuType->addAction(actionCpuType[i]);
		connect(actionCpuType[i], SIGNAL(triggered()), this, SLOT(do_change_cpu_type())); // OK?  
	}
	menuMachine->addAction(menuCpuType->menuAction());
}

void Ui_MainWindowBase::ConfigControlMenu(void)
{
	int i;
	actionReset = new Action_Control(this, using_flags);
	actionReset->setObjectName(QString::fromUtf8("actionReset"));
	connect(actionReset, SIGNAL(triggered()),
		this, SLOT(OnReset())); // OK?
	
	if(using_flags->is_use_special_reset()) {
		for(int i = 0; i < using_flags->get_use_special_reset_num(); i++) {
			QString tmps;
			tmps.setNum(i);
			tmps = QString::fromUtf8("actionSpecial_Reset") + tmps;
			actionSpecial_Reset[i] = new Action_Control(this, using_flags);
			actionSpecial_Reset[i]->setObjectName(tmps);
			actionSpecial_Reset[i]->setData(QVariant(i));
			if(i >= 15) break;
		}
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

			strncpy(tmpbuf, create_local_path(_T("%s.sta%d"), using_flags->get_config_name().toLocal8Bit().constData(), i), _MAX_PATH - 1);
				
			tmps = QString::fromUtf8("");
			tmpss = QString::fromUtf8("");
			tmpss.setNum(i);
			tmps = QString::fromUtf8("actionSave_State") + tmpss;
			actionSave_State[i]->setObjectName(tmps);
			actionLoad_State[i] = new Action_Control(this, using_flags);
			tmps = QString::fromUtf8("actionLoad_State") + tmpss;
			actionLoad_State[i]->setObjectName(tmps);
			tmps = QString::fromLocal8Bit(tmpbuf);
			actionSave_State[i]->setData(QVariant(tmps));
			actionLoad_State[i]->setData(QVariant(tmps));
		}
	}
	if(using_flags->is_use_debugger()) {
		for(i = 0; i < _MAX_DEBUGGER; i++) {
			QString tmps;
			tmps.setNum(i);
			actionDebugger[i] = new Action_Control(this, using_flags);
			actionDebugger[i]->setObjectName(QString::fromUtf8("actionDebugger") + tmps);
			actionDebugger[i]->setData(QVariant(i));
			connect(actionDebugger[i], SIGNAL(triggered()),
					this, SLOT(OnOpenDebugger())); // OK?
		}
	}
	ConfigCpuSpeed();
}

void Ui_MainWindowBase::connectActions_ControlMenu(void)
{
	menuControl->addAction(actionReset);
	if(using_flags->is_use_special_reset()) {
		for(int i = 0; i < using_flags->get_use_special_reset_num(); i++) {
			menuControl->addAction(actionSpecial_Reset[i]);
			if(i >= 15) break;
		}
	}
	menuControl->addSeparator();
	menuControl->addAction(menuCpu_Speed->menuAction());

	if(using_flags->is_use_auto_key()) {
		menuControl->addSeparator();
		menuControl->addAction(menuCopy_Paste->menuAction());
	}
	menuControl->addSeparator();
	if(using_flags->is_use_state()) {
		menuControl->addAction(menuState->menuAction());
	}
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

	for(int i = 0; i < using_flags->get_use_special_reset_num(); i++) {
		addAction(actionSpecial_Reset[i]);
		if(i >= 15) break;
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
	actionReset->setText(QApplication::translate("MenuControl", "Reset", 0));
	actionReset->setToolTip(QApplication::translate("MenuControl", "Reset virtual machine.", 0));
	actionReset->setIcon(ResetIcon);
	if(using_flags->is_use_special_reset()) {
		for(int i = 0; i < using_flags->get_use_special_reset_num(); i++) {
			actionSpecial_Reset[i]->setText(QApplication::translate("MenuControl", SpecialResetTitle, 0));
			actionSpecial_Reset[i]->setIcon(QIcon(":/icon_reset.png"));
			if(i >= 15) break;
		}
	}

	actionExit_Emulator->setText(QApplication::translate("MenuControl", "Exit Emulator", 0));
	actionExit_Emulator->setToolTip(QApplication::translate("MenuControl", "Exit emulator.", 0));
	actionExit_Emulator->setIcon(ExitIcon);

	actionSpeed_x1->setText(QApplication::translate("MenuControl", "Speed x1", 0));
	actionSpeed_x2->setText(QApplication::translate("MenuControl", "Speed x2", 0));
	actionSpeed_x4->setText(QApplication::translate("MenuControl", "Speed x4", 0));
	actionSpeed_x8->setText(QApplication::translate("MenuControl", "Speed x8", 0));
	actionSpeed_x16->setText(QApplication::translate("MenuControl", "Speed x16", 0));
	
	if(using_flags->is_use_auto_key()) {
		actionPaste_from_Clipboard->setText(QApplication::translate("MenuControl", "Paste from Clipboard", 0));
		actionPaste_from_Clipboard->setToolTip(QApplication::translate("MenuControl", "Paste ANK text to virtual machine from desktop's clop board.", 0));
		actionPaste_from_Clipboard->setIcon(QIcon(":/icon_paste.png"));
		actionStop_Pasting->setText(QApplication::translate("MenuControl", "Stop Pasting", 0));
		actionStop_Pasting->setToolTip(QApplication::translate("MenuControl", "Abort pasting ANK text to virtual machine.", 0));
		actionStop_Pasting->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
	}
	if(using_flags->is_use_state()) {
		menuSave_State->setTitle(QApplication::translate("MenuControl", "Save State", 0));
		menuSave_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
		menuSave_State->setToolTip(QApplication::translate("MenuControl", "Save snapshot to fixed bin file.", 0));
		menuLoad_State->setTitle(QApplication::translate("MenuControl", "Load State", 0));
		menuLoad_State->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
		menuLoad_State->setToolTip(QApplication::translate("MenuControl", "Load snapshot from fixed bin file.", 0));
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
		actionDebugger[0]->setText(QApplication::translate("MenuControl", "Main CPU", 0));
		actionDebugger[1]->setText(QApplication::translate("MenuControl", "Sub CPU", 0));
		actionDebugger[2]->setText(QApplication::translate("MenuControl", "Debugger 3", 0));
		actionDebugger[3]->setText(QApplication::translate("MenuControl", "Debugger 4", 0));
		menuDebugger->setTitle(QApplication::translate("MenuControl", "Debugger", 0));
	}
	menuControl->setTitle(QApplication::translate("MenuControl", "Control", 0));
	menuState->setTitle(QApplication::translate("MenuControl", "State", 0));
	
	if(using_flags->is_use_auto_key()) {
		menuCopy_Paste->setTitle(QApplication::translate("MenuControl", "Copy/Paste", 0));
	}
	menuCpu_Speed->setTitle(QApplication::translate("MenuControl", "CPU Speed", 0));
	if(using_flags->is_use_mouse()) {
		actionMouseEnable->setText(QApplication::translate("MenuControl", "Grab MOUSE", 0));
		actionMouseEnable->setToolTip(QApplication::translate("MenuControl", "Grabbing host's mouse.\nPress RIGHT Application key (or another) to toggle enable/disable.", 0));
	}
}

