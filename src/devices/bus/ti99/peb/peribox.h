// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Peripheral expansion box
    See peribox.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_PERIBOX_H
#define MAME_BUS_TI99_PEB_PERIBOX_H

#pragma once

#include "bus/ti99/ti99defs.h"

namespace bus { namespace ti99 { namespace peb {

class ti_expansion_card_device;
class peribox_slot_device;

/*****************************************************************************
    The overall Peripheral Expansion Box.
    See ti99defs.h for bus8z_device
******************************************************************************/

class peribox_device : public bus8z_device
{
	friend class peribox_slot_device;
public:
	peribox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &static_set_inta_callback(device_t &device, Object &&cb)  { return downcast<peribox_device &>(device).m_console_inta.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &static_set_intb_callback(device_t &device, Object &&cb)  { return downcast<peribox_device &>(device).m_console_intb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &static_set_ready_callback(device_t &device, Object &&cb) { return downcast<peribox_device &>(device).m_datamux_ready.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &static_set_lcp_callback(device_t &device, Object &&cb)   { return downcast<peribox_device &>(device).m_sgcpu_lcp.set_callback(std::forward<Object>(cb)); }

	// Next eight methods are called from the console
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);
	DECLARE_WRITE_LINE_MEMBER(senila);
	DECLARE_WRITE_LINE_MEMBER(senilb);

	DECLARE_WRITE_LINE_MEMBER( memen_in );
	DECLARE_WRITE_LINE_MEMBER( msast_in );

	DECLARE_WRITE_LINE_MEMBER( clock_in );

	// Part of configuration
	void set_prefix(int prefix) { m_address_prefix = prefix; }

	// Genmod support
	DECLARE_INPUT_CHANGED_MEMBER( genmod_changed );
	void set_genmod(bool set);

protected:
	peribox_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_config_complete() override;

	virtual machine_config_constructor device_mconfig_additions() const override;

	// Next three methods call back the console
	devcb_write_line m_console_inta;   // INTA line (Box to console)
	devcb_write_line m_console_intb;   // INTB line
	devcb_write_line m_sgcpu_lcp;       // For EVPC with SGCPU only
	devcb_write_line m_datamux_ready;  // READY line (to the datamux)

	void set_slot_loaded(int slot, peribox_slot_device* slotdev);
	peribox_slot_device *m_slot[9];     // for the sake of simplicity we donate the first two positions (0,1)

	// Propagators for the slot signals. All signals are active low, and
	// if any one slot asserts the line, the joint line is asserted.
	void inta_join(int slot, int state);
	void intb_join(int slot, int state);
	void lcp_join(int slot, int state);
	void ready_join(int slot, int state);

	int m_inta_flag;
	int m_intb_flag;
	int m_lcp_flag;
	int m_ready_flag;

	// The TI-99/4(A) Flex Cable Interface (slot 1) pulls up the AMA/AMB/AMC lines to 1/1/1.
	int m_address_prefix;

	// Most significant address byte strobe. Defined by TI-99/8.
	bool    m_msast;

	// Memory enable.
	bool    m_memen;
};

/************************************************************************
    Specific Box compositions
************************************************************************/
/*
    Variation for EVPC. We'd like to offer the EVPC slot device only if
    we started the ti99_4ev driver.
*/
class peribox_ev_device : public peribox_device
{
public:
	peribox_ev_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};

/*
    Variation for SGCPU (TI-99/4P). We put the EVPC and the HSGPL in slots 2 and 3.
*/
class peribox_sg_device : public peribox_device
{
public:
	peribox_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};

/*
    Variation for Geneve.
*/
class peribox_gen_device : public peribox_device
{
public:
	peribox_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};

/*
    Variation for TI-99/8
*/
class peribox_998_device : public peribox_device
{
public:
	peribox_998_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};

/*****************************************************************************
    A single slot in the box.
******************************************************************************/

class peribox_slot_device : public bus8z_device, public device_slot_interface
{
	friend class peribox_device;
public:
	peribox_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Called from the box (direction to card)
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_WRITE_LINE_MEMBER(senila);
	DECLARE_WRITE_LINE_MEMBER(senilb);
	DECLARE_WRITE_LINE_MEMBER(clock_in);

	// Called from the card (direction to box)
	DECLARE_WRITE_LINE_MEMBER( set_inta );
	DECLARE_WRITE_LINE_MEMBER( set_intb );
	DECLARE_WRITE_LINE_MEMBER( lcp_line );
	DECLARE_WRITE_LINE_MEMBER( set_ready );

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	// called from the box itself
	void set_genmod(bool set);

	device_t*   get_drive(const char* name);

protected:
	void device_start() override;
	void device_config_complete() override;

private:
	int get_index_from_tagname();
	ti_expansion_card_device *m_card;
	int m_slotnumber;
};


/*****************************************************************************
    The parent class for all expansion cards.
******************************************************************************/

class ti_expansion_card_device : public bus8z_device, public device_slot_card_interface
{
	friend class peribox_slot_device;

public:
	virtual DECLARE_READ8Z_MEMBER(crureadz) = 0;
	virtual DECLARE_WRITE8_MEMBER(cruwrite) = 0;

	void    set_senila(int state) { m_senila = state; }
	void    set_senilb(int state) { m_senilb = state; }

	virtual DECLARE_WRITE_LINE_MEMBER(clock_in) { }

protected:
	ti_expansion_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: bus8z_device(mconfig, type, tag, owner, clock)
		, device_slot_card_interface(mconfig, *this)
		, m_selected(false), m_cru_base(0), m_select_mask(0), m_select_value(0)
	{
		m_slot = static_cast<peribox_slot_device*>(owner);
		m_senila = CLEAR_LINE;
		m_senilb = CLEAR_LINE;
		m_genmod = false;
	}

	peribox_slot_device *m_slot;        // using a link to the slot for callbacks
	int m_senila;
	int m_senilb;

	// When true, card is accessible. Indicated by a LED.
	bool    m_selected;

	// When true, GenMod is selected. Modified by peribox_slot_device.
	bool    m_genmod;

	// CRU base. Used to configure the address by which a card is selected.
	int     m_cru_base;

	// Used to decide whether this card has been selected.
	int     m_select_mask;
	int     m_select_value;
};

#define MCFG_PERIBOX_SLOT_ADD(_tag, _slot_intf) \
	MCFG_DEVICE_ADD(_tag, PERIBOX_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, nullptr, false)

#define MCFG_PERIBOX_SLOT_ADD_DEF(_tag, _slot_intf, _default) \
	MCFG_DEVICE_ADD(_tag, PERIBOX_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _default, false)

#define MCFG_PERIBOX_INTA_HANDLER( _inta ) \
	devcb = &bus::ti99::peb::peribox_device::static_set_inta_callback( *device, DEVCB_##_inta );

#define MCFG_PERIBOX_INTB_HANDLER( _intb ) \
	devcb = &bus::ti99::peb::peribox_device::static_set_intb_callback( *device, DEVCB_##_intb );

#define MCFG_PERIBOX_READY_HANDLER( _ready ) \
	devcb = &bus::ti99::peb::peribox_device::static_set_ready_callback( *device, DEVCB_##_ready );

#define MCFG_PERIBOX_LCP_HANDLER( _lcp ) \
	devcb = &bus::ti99::peb::peribox_device::static_set_lcp_callback( *device, DEVCB_##_lcp );

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(PERIBOX,      bus::ti99::peb, peribox_device)
DECLARE_DEVICE_TYPE_NS(PERIBOX_SLOT, bus::ti99::peb, peribox_slot_device)

DECLARE_DEVICE_TYPE_NS(PERIBOX_EV,   bus::ti99::peb, peribox_ev_device)
DECLARE_DEVICE_TYPE_NS(PERIBOX_SG,   bus::ti99::peb, peribox_sg_device)
DECLARE_DEVICE_TYPE_NS(PERIBOX_GEN,  bus::ti99::peb, peribox_gen_device)
DECLARE_DEVICE_TYPE_NS(PERIBOX_998,  bus::ti99::peb, peribox_998_device)

#endif // MAME_BUS_TI99_PEB_PERIBOX_H
