// Copyright (c) 2017 The Bitcoin developers
// Copyright (c) 2018 The ION Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_BYTZADDRENC_H
#define BITCOIN_BYTZADDRENC_H

#include "script/standard.h"
#include "consensus/tokengroups.h"

#include <string>
#include <vector>

class CChainParams;
//class CTokenGroupID;

enum BytzAddrType : uint8_t
{
    PUBKEY_TYPE = 0,
    SCRIPT_TYPE = 1,
    GROUP_TYPE = 2,
};

std::string EncodeBytzAddr(const CTxDestination &, const CChainParams &);
std::string EncodeBytzAddr(const std::vector<uint8_t> &id, const BytzAddrType addrtype, const CChainParams &params);

struct BytzAddrContent
{
    BytzAddrType type;
    std::vector<uint8_t> hash;
};

CTxDestination DecodeBytzAddr(const std::string &addr, const CChainParams &params);
BytzAddrContent DecodeBytzAddrContent(const std::string &addr, const CChainParams &params);
CTxDestination DecodeBytzAddrDestination(const BytzAddrContent &content);

std::vector<uint8_t> PackBytzAddrContent(const BytzAddrContent &content);
#endif
