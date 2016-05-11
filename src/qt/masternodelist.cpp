#include "masternodelist.h"
#include "ui_masternodelist.h"

#include "sync.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "activemasternode.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "wallet/wallet.h"
#include "init.h"

#include <QTimer>
#include <QMessageBox>

CCriticalSection cs_masternodes;

MasternodeList::MasternodeList(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodeList),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    ui->startButton->setEnabled(false);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMyNodeList()));
    timer->start(1000);

    updateNodeList();
}

MasternodeList::~MasternodeList()
{
    delete ui;
}

void MasternodeList::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // try to update list when masternode count changes
        connect(clientModel, SIGNAL(strMasternodesChanged()), this, SLOT(updateNodeList()));
    }
}

void MasternodeList::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void MasternodeList::StartAlias(std::string strAlias) {

    std::string statusObj;
    statusObj += "<center>Alias: " + strAlias;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        if(mne.getAlias() == strAlias) {
            std::string errorMessage;
            CMasternodeBroadcast mnb;

            bool result = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

            if(result) {
                statusObj += "<br>Successfully started masternode." ;
                mnodeman.UpdateMasternodeList(mnb);
                mnb.Relay();
            } else {
                statusObj += "<br>Failed to start masternode.<br>Error: " + errorMessage;
            }
            break;
        }
    }
    statusObj += "</center>";

    QMessageBox msg;
    msg.setText(QString::fromStdString(statusObj));

    msg.exec();
}

void MasternodeList::StartAll(std::string strCommand) {

    int total = 0;
    int successful = 0;
    int fail = 0;
    std::string statusObj;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        std::string errorMessage;
        CMasternodeBroadcast mnb;

        CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
        CMasternode *pmn = mnodeman.Find(vin);

        if(strCommand == "start-missing" && pmn) continue;

        bool result = activeMasternode.CreateBroadcast(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

        if(result) {
            successful++;
            mnodeman.UpdateMasternodeList(mnb);
            mnb.Relay();
        } else {
            fail++;
            statusObj += "\nFailed to start " + mne.getAlias() + ". Error: " + errorMessage;
        }
        total++;
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = strprintf("Successfully started %d masternodes, failed to start %d, total %d", successful, fail, total);
    if (fail > 0)
        returnObj += statusObj;

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();
}

void MasternodeList::updateMyMasternodeInfo(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, CMasternode *pmn)
{
    LOCK(cs_mnlistupdate);
    bool bFound = false;
    int nodeRow = 0;

    for(int i=0; i < ui->tableWidget_2->rowCount(); i++)
    {
        if(ui->tableWidget_2->item(i, 0)->text() == alias)
        {
            bFound = true;
            nodeRow = i;
            break;
        }
    }

    if(nodeRow == 0 && !bFound) {
        nodeRow = ui->tableWidget_2->rowCount();
        ui->tableWidget_2->insertRow(nodeRow);
    }

    QTableWidgetItem *aliasItem = new QTableWidgetItem(alias);
    QTableWidgetItem *addrItem = new QTableWidgetItem(addr);
    QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(pmn ? pmn->protocolVersion : -1));
    QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(pmn ? pmn->Status() : "MISSING"));
    QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(pmn ? (pmn->lastPing.sigTime - pmn->sigTime) : 0)));
    QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M:%S", pmn ? pmn->lastPing.sigTime : 0)));
    QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(pmn ? CBitcoinAddress(pmn->pubkey.GetID()).ToString() : ""));

    ui->tableWidget_2->setItem(nodeRow, 0, aliasItem);
    ui->tableWidget_2->setItem(nodeRow, 1, addrItem);
    ui->tableWidget_2->setItem(nodeRow, 2, protocolItem);
    ui->tableWidget_2->setItem(nodeRow, 3, statusItem);
    ui->tableWidget_2->setItem(nodeRow, 4, activeSecondsItem);
    ui->tableWidget_2->setItem(nodeRow, 5, lastSeenItem);
    ui->tableWidget_2->setItem(nodeRow, 6, pubkeyItem);
}

void MasternodeList::updateMyNodeList(bool reset) {
    static int64_t lastMyListUpdate = 0;

    // automatically update my masternode list only once in MY_MASTERNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t timeTillUpdate = lastMyListUpdate + MY_MASTERNODELIST_UPDATE_SECONDS - GetTime();
    ui->secondsLabel->setText(QString::number(timeTillUpdate));

    if(timeTillUpdate > 0 && !reset) return;
    lastMyListUpdate = GetTime();

    ui->tableWidget->setSortingEnabled(false);
    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
        CMasternode *pmn = mnodeman.Find(vin);

        updateMyMasternodeInfo(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), QString::fromStdString(mne.getPrivKey()), QString::fromStdString(mne.getTxHash()),
            QString::fromStdString(mne.getOutputIndex()), pmn);
    }
    ui->tableWidget->setSortingEnabled(true);

    // reset "timer"
    ui->secondsLabel->setText("0");
}

void MasternodeList::updateNodeList()
{
    static int64_t lastListUpdate = 0;

    // update only once in MASTERNODELIST_UPDATE_SECONDS seconds to prevent high cpu usage e.g. on filter change
    if(GetTime() - lastListUpdate < MASTERNODELIST_UPDATE_SECONDS) return;
    lastListUpdate = GetTime();

    TRY_LOCK(cs_masternodes, lockMasternodes);
    if(!lockMasternodes)
        return;

    QString strToFilter;
    ui->countLabel->setText("Updating...");
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    std::vector<CMasternode> vMasternodes = mnodeman.GetFullMasternodeVector();

    BOOST_FOREACH(CMasternode& mn, vMasternodes)
    {
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
        QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(mn.protocolVersion));
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(mn.Status()));
        QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(mn.lastPing.sigTime - mn.sigTime)));
        QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M:%S", mn.lastPing.sigTime)));
        QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(CBitcoinAddress(mn.pubkey.GetID()).ToString()));

        if (strCurrentFilter != "")
        {
            strToFilter =   addressItem->text() + " " +
                            protocolItem->text() + " " +
                            statusItem->text() + " " +
                            activeSecondsItem->text() + " " +
                            lastSeenItem->text() + " " +
                            pubkeyItem->text();
            if (!strToFilter.contains(strCurrentFilter)) continue;
        }

        ui->tableWidget->insertRow(0);
        ui->tableWidget->setItem(0, 0, addressItem);
        ui->tableWidget->setItem(0, 1, protocolItem);
        ui->tableWidget->setItem(0, 2, statusItem);
        ui->tableWidget->setItem(0, 3, activeSecondsItem);
        ui->tableWidget->setItem(0, 4, lastSeenItem);
        ui->tableWidget->setItem(0, 5, pubkeyItem);
    }

    ui->countLabel->setText(QString::number(ui->tableWidget->rowCount()));
    ui->tableWidget->setSortingEnabled(true);

}

void MasternodeList::on_filterLineEdit_textChanged(const QString &filterString) {
    strCurrentFilter = filterString;
    ui->countLabel->setText("Please wait...");
}

void MasternodeList::on_startButton_clicked()
{
    // Find selected node alias
    QItemSelectionModel* selectionModel = ui->tableWidget_2->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
        return;

    QModelIndex index = selected.at(0);
    int r = index.row();
    std::string strAlias = ui->tableWidget_2->item(r, 0)->text().toStdString();

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm masternode start"),
        tr("Are you sure you want to start masternode %1?").arg(QString::fromStdString(strAlias)),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAlias(strAlias);
        return;
    }

    StartAlias(strAlias);
}

void MasternodeList::on_startAllButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm all masternodes start"),
        tr("Are you sure you want to start ALL masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAll();
        return;
    }

    StartAll();
}

void MasternodeList::on_startMissingButton_clicked()
{

    if(masternodeSync.RequestedMasternodeAssets <= MASTERNODE_SYNC_LIST ||
      masternodeSync.RequestedMasternodeAssets == MASTERNODE_SYNC_FAILED) {
        QMessageBox::critical(this, tr("Command is not available right now"),
            tr("You can't use this command until masternode list is synced"));
        return;
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this,
        tr("Confirm missing masternodes start"),
        tr("Are you sure you want to start MISSING masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAll("start-missing");
        return;
    }

    StartAll("start-missing");
}

void MasternodeList::on_tableWidget_2_itemSelectionChanged()
{
    if(ui->tableWidget_2->selectedItems().count() > 0)
    {
        ui->startButton->setEnabled(true);
    }
}

void MasternodeList::on_UpdateButton_clicked()
{
    updateMyNodeList(true);
}
