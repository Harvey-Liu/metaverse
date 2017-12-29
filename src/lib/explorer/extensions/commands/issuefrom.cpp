/**
 * Copyright (c) 2016-2017 mvs developers 
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/issuefrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp> 
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result issuefrom::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if(argument_.fee < 1000000000)
        throw asset_issue_poundage_exception{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw asset_symbol_existed_exception{"asset symbol is already exist in blockchain"};

    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, argument_.symbol);
    if(!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " not found"};
    #if 0
    if(asset_detail::asset_detail_type::created != sh_asset->at(0).detail.get_asset_type())
        throw asset_symbol_duplicate_exception{argument_.symbol + " has been issued"};
    #endif

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}  
    };
    auto issue_helper = issuing_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee);
    
    issue_helper.exec();
    // json output
    auto tx = issue_helper.get_transaction();
#if 0
    auto issue_helper = issuing_locked_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
    
    issue_helper.exec();
    // json output
    auto tx = issue_helper.get_transaction();
#endif
    // change asset status
    #if 0
    sh_asset->at(0).detail.set_asset_type(asset_detail::asset_detail_type::issued_not_in_blockchain);
    auto detail = std::make_shared<asset_detail>(sh_asset->at(0).detail);
    blockchain.store_account_asset(detail);
    #endif

    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

