/*
 * Qt / Tape Menu, Utilities
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License : GPLv2
 *   History :
 *     Mar 20 2016 : Start
 */

#include <QApplication>
#include "commonclasses.h"
#include "mainwidget_base.h"
#include "menu_compactdisc.h"

#include "qt_dialogs.h"
#include "emu_thread_tmpl.h"

Menu_CompactDiscClass::Menu_CompactDiscClass(QMenuBar *root_entry, QString desc, std::shared_ptr<USING_FLAGS> p, QWidget *parent, int drv, int base_drv) : Menu_MetaClass(root_entry, desc, p, parent, drv, base_drv)
{
	use_write_protect = false;
	use_d88_menus = false;
}

Menu_CompactDiscClass::~Menu_CompactDiscClass()
{
}


void Menu_CompactDiscClass::create_pulldown_menu_device_sub(void)
{
	//
	action_swap_byteorder = new Action_Control(p_wid, using_flags);

	struct CSP_Ui_Menu::DriveIndexPair tmp;
	QVariant _tmp_ins;
	tmp.drive = media_drive;
	tmp.index = 0;
	_tmp_ins.setValue(tmp);
	action_swap_byteorder->setData(_tmp_ins);
	action_swap_byteorder->setVisible(true);
	action_swap_byteorder->setCheckable(true);

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() != nullptr) {
		action_swap_byteorder->setChecked(p->is_cdaudio_swap_byteorder(media_drive));
	} else {
		action_swap_byteorder->setChecked(false);
	}
	addAction(action_swap_byteorder);
}


void Menu_CompactDiscClass::connect_menu_device_sub(void)
{

	connect(this, SIGNAL(sig_open_media(int, QString)),	p_wid, SLOT(do_open_compact_disc_ui(int, QString)));
	connect(this, SIGNAL(sig_eject_media(int)),	p_wid, SLOT(do_eject_compact_disc(int)));
	connect(this, SIGNAL(sig_set_recent_media(int, int)), p_wid, SLOT(set_recent_compact_disc(int, int)));
	// Will move to EMUTHREADCLASS?
	connect(action_swap_byteorder, SIGNAL(toggled(bool)), p_wid, SLOT(do_swap_cdaudio_byteorder(bool)));
}

void Menu_CompactDiscClass::connect_via_emu_thread(EmuThreadClassBase *p)
{
	if(p == nullptr) return;
	connect(action_eject, SIGNAL(triggered()), p, SLOT(do_eject_compact_disc()), Qt::QueuedConnection);
}


void Menu_CompactDiscClass::retranslate_pulldown_menu_device_sub(void)
{
	action_insert->setText(QApplication::translate("MenuMedia", "Insert Compact Disc", 0));
	action_insert->setToolTip(QApplication::translate("MenuMedia", "Insert a image file; CD-ROM or CD audio.", 0));
	action_eject->setText(QApplication::translate("MenuMedia", "Eject Compact Disc", 0));
	action_eject->setToolTip(QApplication::translate("MenuMedia", "Eject a compact disc.", 0));

	action_swap_byteorder->setText(QApplication::translate("MenuMedia", "Swap byte order", 0));
	action_swap_byteorder->setToolTip(QApplication::translate("MenuMedia", "Swap audio track's byte order.\nThis is effects some ripped (not dedicated to AUDIO's endian) CD-ROMs.", 0));

	this->setTitle(QApplication::translate("MenuMedia", "CD ROM" , 0));
	action_insert->setIcon(QIcon(":/icon_cd.png"));
}
