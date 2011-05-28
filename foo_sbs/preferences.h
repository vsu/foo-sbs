#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <foobar2000.h>
#include "../ATLHelpers/ATLHelpers.h"
#include "resource.h"

#define default_cfg_slim_port		3483
#define	default_cfg_http_port		9000
#define default_cfg_bps				1
#define default_cfg_enable			1

// {4F566A04-D217-4F32-82BC-7888B0EDDEE0}
static const GUID guid_cfg_slim_port =
{ 0x4F566A04, 0xD217, 0x4F32, { 0x82, 0xBC, 0x78, 0x88, 0xB0, 0xED, 0xDE, 0xE0 } };

// {DED6DDA2-F537-45AC-A9F7-199CD2A50001}
static const GUID guid_cfg_http_port =
{ 0xDED6DDA2, 0xF537, 0x45AC, { 0xA9, 0xF7, 0x19, 0x9C, 0xD2, 0xA5, 0x00, 0x01 } };

// {263631EC-BA6C-4CBB-AE8B-AEB3391963DC}
static const GUID guid_cfg_bps =
{ 0x263631EC, 0xBA6C, 0x4CBB, { 0xAE, 0x8B, 0xAE, 0xB3, 0x39, 0x19, 0x63, 0xDC } };

// {B2B3871A-91EE-41CC-814C-70DF72871A71}
static const GUID guid_cfg_enable =
{ 0xB2B3871A, 0x91EE, 0x41CC, { 0x81, 0x4C, 0x70, 0xDF, 0x72, 0x87, 0x1A, 0x71 } };

extern cfg_int cfg_slim_port;
extern cfg_int cfg_http_port;
extern cfg_int cfg_bps;
extern cfg_int cfg_enable;

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance
{
public:
    //Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
    CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

    //Note that we don't bother doing anything regarding destruction of our class.
    //The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.

    //dialog resource ID
    enum { IDD = IDD_CONFIG };

    // preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
    t_uint32 get_state();
    void apply();
    void reset();

    //WTL message map
    BEGIN_MSG_MAP(CMyPreferences)
    MSG_WM_INITDIALOG(OnInitDialog)
    COMMAND_HANDLER_EX(IDC_SLIM_PORT, EN_CHANGE, OnFieldChange)
    COMMAND_HANDLER_EX(IDC_HTTP_PORT, EN_CHANGE, OnFieldChange)
    COMMAND_HANDLER_EX(IDC_BPS, CBN_SELCHANGE, OnSelectionChange)
    COMMAND_HANDLER_EX(IDC_ENABLE, BN_CLICKED, OnButtonClick)
    END_MSG_MAP()

private:
    BOOL OnInitDialog(CWindow, LPARAM);
    void OnFieldChange(UINT, int, CWindow);
    void OnSelectionChange(UINT, int, CWindow);
    void OnButtonClick(UINT, int, CWindow);
    bool HasChanged();
    void OnChanged();

    const preferences_page_callback::ptr m_callback;
};

#endif