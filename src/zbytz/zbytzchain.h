// Copyright (c) 2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_ZBYTZCHAIN_H
#define PIVX_ZBYTZCHAIN_H

#include "libzerocoin/Coin.h"
#include "libzerocoin/Denominations.h"
#include "libzerocoin/CoinSpend.h"
#include "primitives/transaction.h"
#include <list>
#include <string>

class CBlock;
class CBigNum;
class CBlockIndex;
struct CMintMeta;
class CValidationState;
class CZerocoinMint;
class uint256;

/*
bool BlockToMintValueVector(const CBlock& block, const libzerocoin::CoinDenomination denom, std::vector<CBigNum>& vValues);
*/
bool BlockToPubcoinList(const CBlock& block, std::list<libzerocoin::PublicCoin>& listPubcoins, bool fFilterInvalid);
/*
bool BlockToZerocoinMintList(const CBlock& block, std::list<CZerocoinMint>& vMints, bool fFilterInvalid);
void FindMints(std::vector<CMintMeta> vMintsToFind, std::vector<CMintMeta>& vMintsToUpdate, std::vector<CMintMeta>& vMissingMints);
int GetZerocoinStartHeight();
bool GetZerocoinMint(const CBigNum& bnPubcoin, uint256& txHash);
bool IsPubcoinInBlockchain(const uint256& hashPubcoin, uint256& txid);
bool IsSerialKnown(const CBigNum& bnSerial);
*/
bool IsSerialInBlockchain(const CBigNum& bnSerial, int& nHeightTx);
bool IsSerialInBlockchain(const uint256& hashSerial, int& nHeightTx, uint256& txidSpend);
bool IsSerialInBlockchain(const uint256& hashSerial, int& nHeightTx, uint256& txidSpend, CTransactionRef tx);
bool RemoveSerialFromDB(const CBigNum& bnSerial);
/*
std::string ReindexZerocoinDB();
*/
libzerocoin::CoinSpend TxInToZerocoinSpend(const CTxIn& txin);
bool TxOutToPublicCoin(const CTxOut& txout, libzerocoin::PublicCoin& pubCoin, CValidationState& state);
std::list<libzerocoin::CoinDenomination> ZerocoinSpendListFromBlock(const CBlock& block, bool fFilterInvalid);

bool UpdateZBYTZSupply(const CBlock& block, CBlockIndex* pindex, bool fJustCheck);

#endif //PIVX_ZBYTZCHAIN_H
