#pragma once

// NetStartDlg (IDD 3009, Planet.exe 0x41dbc0-0x41e120): "Start Network Game", shown by
// ExplorerDlg::OnStartClicked when the network is active and the selected scenario is not a
// network reference.
//
//   Wait for clients         -> Network\Lobby              (engine argument /Lobby)
//   No runtime join          -> Network\NoRuntimeJoin
//   Deny game info requests  -> Network\NoReferenceRequest
//   Sign up at master server -> Network\MasterServerSignUp (enables the address)
//   Master server address    -> Network\MasterServerAddress
//   Control rate (1-10)      -> Network\ControlRate
//   Comment                  -> Network\Comment
// The engine reads everything but the lobby flag from the config (C4Config Network section).

#include "ClonkDialog.h"

#include <QStringList>

class NetStartDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit NetStartDlg(QWidget *parent = nullptr);

protected:
    void onOK() override;

private:
    void updateSignUp(); // OnSignUpAtServerClicked
};

// Shows the dialog. On OK the values are stored in the config, the config is saved (the engine
// reads it) and the extra engine arguments are appended to engine_args ("/Lobby" when waiting for
// clients). Returns false on cancel.
bool runNetStartDialog(QWidget *parent, QStringList &engine_args);
