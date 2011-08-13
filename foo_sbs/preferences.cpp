#include "preferences.h"
#include "foo_sbs.h"

cfg_int cfg_slim_port(guid_cfg_slim_port, default_cfg_slim_port);
cfg_int cfg_http_port(guid_cfg_http_port, default_cfg_http_port);
cfg_int cfg_bps(guid_cfg_bps, default_cfg_bps);
cfg_int cfg_enable(guid_cfg_enable, default_cfg_enable);

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM)
{
    ::SendMessage(GetDlgItem(IDC_SLIM_PORT), EM_SETLIMITTEXT, 5, 0 );
    SetDlgItemInt(IDC_SLIM_PORT, cfg_slim_port, FALSE);

    ::SendMessage(GetDlgItem(IDC_HTTP_PORT), EM_SETLIMITTEXT, 5, 0 );
    SetDlgItemInt(IDC_HTTP_PORT, cfg_http_port, FALSE);

    pfc::ptr_list_t<char> items;

    items.add_item((char *)"16");
    items.add_item((char *)"24");

    CWindow w = GetDlgItem(IDC_BPS);

    uSendMessage(w, CB_RESETCONTENT, 0, 0);

    t_size i;
    for (i = 0; i < items.get_count(); i++)
    {
        uSendMessageText(w, CB_ADDSTRING, 0, items[i]);
    }

    ::SendMessage(w, CB_SETCURSEL, cfg_bps, 0);

    CheckDlgButton(IDC_ENABLE, cfg_enable);

    return FALSE;
}

void CMyPreferences::OnFieldChange(UINT, int, CWindow)
{
    OnChanged();
}

void CMyPreferences::OnSelectionChange(UINT, int, CWindow)
{
    OnChanged();
}

void CMyPreferences::OnButtonClick(UINT, int, CWindow)
{
    OnChanged();
}

t_uint32 CMyPreferences::get_state()
{
    t_uint32 state = preferences_state::resettable;
    if (HasChanged()) state |= preferences_state::changed;
    return state;
}

void CMyPreferences::reset()
{
    SetDlgItemInt(IDC_SLIM_PORT, default_cfg_slim_port, FALSE);
    SetDlgItemInt(IDC_HTTP_PORT, default_cfg_http_port, FALSE);
    ::SendMessage(GetDlgItem(IDC_BPS), CB_SETCURSEL, default_cfg_bps, 0);
    CheckDlgButton(IDC_ENABLE, default_cfg_enable);

    OnChanged();
}

void CMyPreferences::apply()
{
    cfg_slim_port = GetDlgItemInt(IDC_SLIM_PORT, NULL, FALSE);
    cfg_http_port = GetDlgItemInt(IDC_HTTP_PORT, NULL, FALSE);
    cfg_bps = ::SendMessage(GetDlgItem(IDC_BPS), CB_GETCURSEL, 0, 0);
    cfg_enable = IsDlgButtonChecked(IDC_ENABLE);

    g_apply_preferences();

    OnChanged(); // our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged()
{
    return
        (GetDlgItemInt(IDC_SLIM_PORT, NULL, FALSE) != cfg_slim_port) ||
        (GetDlgItemInt(IDC_HTTP_PORT, NULL, FALSE) != cfg_http_port) ||
        (::SendMessage(GetDlgItem(IDC_BPS), CB_GETCURSEL, 0, 0) != cfg_bps) ||
        (IsDlgButtonChecked(IDC_ENABLE) != cfg_enable);
}

void CMyPreferences::OnChanged()
{
    // tell the host that our state has changed to enable/disable the apply button appropriately.
    m_callback->on_state_changed();
}
