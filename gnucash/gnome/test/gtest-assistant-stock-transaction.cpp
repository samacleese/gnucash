/********************************************************************
 * test-assistant-stock-transaction.cpp:                            *
 * Copyright 2022 Christopher Lam                                   *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, you can retrieve it from        *
 * https://www.gnu.org/licenses/old-licenses/gpl-2.0.html            *
 * or contact:                                                      *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 ********************************************************************/
#include "config.h"
#include <glib.h>
#include "../assistant-stock-transaction.cpp"
#include <memory>
#include <Account.h>
#include <Split.h>
#include <gnc-numeric.hpp>
#include <gnc-datetime.hpp>
#include <gtest/gtest.h>
extern "C"
{
#include <gnc-pricedb-p.h>
}

struct Tests
{
    unsigned int type_idx, dd, mm, yy;
    const char *desc;
    int stock_amt, stock_val, cash_val;
    bool capitalize;
    int fees_val, divi_val, capg_val, new_bal;
};

struct TestCase {
    std::vector<Tests> tests;
};

TestCase easyTestCase = {
    .tests =
    {
        //t, dd, mm, yyyy, desc               , stk.amt, stk.val , cash    , capitalize, fees, divi , capg  , new_bal

        // bal=0. next line is "open long".
        { 0, 1 , 7 , 2019, "Buy"              , 100    , 2000000 , 2000995 , true      , 995 , 0    , 0     , 100 },

        // bal>0. next lines are long_types
        { 0, 11, 12, 2019, "Buy"              , 50     , 1600000 , 1600995 , true      , 995 , 0    , 0     , 150 },
        { 1, 18, 3 , 2020, "Sell"             , 75     , 1200000 , 1199005 , false     , 995 , 0    ,-600995, 75 },
        { 0, 1 , 4 , 2020, "Buy"              , 250    , 4200000 , 4200995 , true      , 995 , 0    , 0     , 325 },
        { 3, 16, 4 , 2020, "ROC"              , 0      , 250000  , 250000  , true      , 0   , 0    , 0     , 325 },
        { 0, 2 , 5 , 2020, "Buy"              , 125    , 4750000 , 4750000 , true      , 0   , 0    , 0     , 450 },
        { 5, 11, 5 , 2020, "Split 2:1"        , 900    , 0       , 0       , true      , 0   , 0    , 0     , 900 },
        { 1, 21, 5 , 2020, "Sell"             , 135    , 2150000 , 2149005 , false     , 995 , 0    , 574702, 765 },
        { 0, 3 , 6 , 2020, "Buy"              , 150    , 2100000 , 2100000 , true      , 0   , 0    , 0     , 915 },
        { 1, 10, 6 , 2020, "Sell"             , 915    , 12810000, 12809005, false     , 995 , 0    , 1783309, 0 },

        // bal=0. next line is "open short".
        { 1, 10, 6 , 2020, "Short Sell"       , 85     , 1190000 , 1189005 , true      , 995 , 0    , 0     , -85 },

        // bal<0. next lines are short_types
        { 0, 15, 6 , 2020, "Short Sell"       , 65     , 1105000 , 1104005 , true      , 995 , 0    , 0     , -150 },
        { 1, 16, 6 , 2020, "Cover Buy"        , 50     , 500000  , 500995  , false     , 995 , 0    ,-264337, -100 },
        { 5, 17, 6 , 2020, "Split 2:1"        , -200   , 0       , 0       , false     , 0   , 0    , 0     , -200 },
        { 6, 18, 6 , 2020, "Reverse Split"    , -100   , 0       , 0       , false     , 0   , 0    , 0     , -100 },
        { 2, 19, 6 , 2020, "Comp Dividend"    , 0      , 0       , 50000   , false     , 0   , 50000, 0     , -100 },
        { 3, 19, 6 , 2020, "Comp ROC"         , 0      , 250000  , 250000  , false     , 0   , 0    , 0     , -100 },
        { 4, 19, 6 , 2020, "Comp ND"          , 0      , 20000   , 0       , false     , 0   , 20000, 0     , -100 },
        { 1, 20, 6 , 2020, "Cover Buy"        , 100    , 800000  , 800498  , false     , 498 , 0    ,-498673, 0 },

        // bal=0. next line is "open long".
        { 0, 20, 6 , 2020, "Buy"              , 100    , 800000  , 800498  , true      , 498 , 0    , 0     , 100 },

        // bal>0. next lines are long_types
        { 2, 21, 6 , 2020, "Dividend"         , 0      , 0       , 7000    , false     , 0   , 7000 , 0     , 100 },
        { 2, 25, 6 , 2020, "Dividend"         , 0      , 0       , 11000   , false     , 0   , 11000, 0     , 100 },
        { 0, 25, 6 , 2020, "+ Reinv"          , 1      , 10000   , 10000   , false     , 0   , 0    , 0     , 101 },
        { 1, 26, 6 , 2020, "Sell remainder"   , 1      , 10000   , 10000   , false     , 0   , 0    , 1975  , 100 },
        { 6, 26, 6 , 2020, "Reverse Split 1:2", 50     , 0       , 0       , false     , 0   , 0    , 0     , 50 },
        { 4, 27, 6 , 2020, "ND"               , 0      , 10000   , 0       , false     , 0   , 10000, 0     , 50 }
    }
};

class Stock_AssistantTest : public ::testing::TestWithParam<TestCase *> {
protected:
    std::shared_ptr<QofBook> m_book;
    gnc_commodity *stock_commodity, *USD;
    Account *broker_account, *stock_account, *cash_account, *dividend_account,
        *capgains_account, *fees_account;
    TestCase &m_testCase;

public:
    Stock_AssistantTest() :
        m_book (qof_book_new (), qof_book_destroy),
        broker_account (xaccMallocAccount (m_book.get ())),
        stock_account (xaccMallocAccount (m_book.get ())),
        cash_account (xaccMallocAccount (m_book.get ())),
        dividend_account (xaccMallocAccount (m_book.get ())),
        capgains_account (xaccMallocAccount (m_book.get ())),
        fees_account (xaccMallocAccount (m_book.get ())),
        m_testCase (*GetParam ())
    {
        qof_init();
        qof_book_register ();
        gnc_pricedb_register();
        gnc_commodity_table_register ();

        stock_commodity = gnc_commodity_new (m_book.get(), "SPY", "", "SPY", "", 100);
        USD = gnc_commodity_table_lookup (gnc_commodity_table_get_table (m_book.get()),
                                          "CURRENCY", "USD");

        xaccAccountBeginEdit (broker_account);
        xaccAccountSetName (broker_account, "Broker Account");
        xaccAccountSetType (broker_account, ACCT_TYPE_CASH);
        xaccAccountSetCommodity (broker_account, USD);

        xaccAccountBeginEdit (stock_account);
        xaccAccountSetName (stock_account, "Stock Account");
        xaccAccountSetType (stock_account, ACCT_TYPE_STOCK);
        xaccAccountSetCommodity (stock_account, stock_commodity);
        gnc_account_append_child (broker_account, stock_account);
        xaccAccountCommitEdit (broker_account);
        xaccAccountCommitEdit (stock_account);

        xaccAccountBeginEdit (cash_account);
        xaccAccountSetName (cash_account, "Cash Account");
        xaccAccountSetType (cash_account, ACCT_TYPE_BANK);
        xaccAccountSetCommodity (cash_account, USD);
        xaccAccountCommitEdit (cash_account);

        xaccAccountBeginEdit (dividend_account);
        xaccAccountSetName (dividend_account, "Dividend Account");
        xaccAccountSetType (dividend_account, ACCT_TYPE_INCOME);
        xaccAccountSetCommodity (dividend_account, USD);
        xaccAccountCommitEdit (dividend_account);

        xaccAccountBeginEdit (capgains_account);
        xaccAccountSetName (capgains_account, "Capgains Account");
        xaccAccountSetType (capgains_account, ACCT_TYPE_INCOME);
        xaccAccountSetCommodity (capgains_account, USD);
        xaccAccountCommitEdit (capgains_account);

        xaccAccountBeginEdit (fees_account);
        xaccAccountSetName (fees_account, "Fees Account");
        xaccAccountSetType (fees_account, ACCT_TYPE_EXPENSE);
        xaccAccountSetCommodity (fees_account, USD);
        xaccAccountCommitEdit (fees_account);
    }
};

static void test_failure_modes (Account *stock_account)
{
    StockAssistantModel model (stock_account);
    model.transaction_date = gnc_dmy2time64 (1, 1, 2022);

    // resetting txn_types will work the first time
    EXPECT_TRUE (model.maybe_reset_txn_types ());

    // trying to reset again shouldn't be necessary
    EXPECT_FALSE (model.maybe_reset_txn_types ());

    // set transaction-date to a different date.
    model.transaction_date = gnc_dmy2time64 (1, 2, 2022);

    // resetting txn_types will now work.
    EXPECT_TRUE (model.maybe_reset_txn_types ());

    // the Model is empty. generating list of splits should fail.
    auto [success_splits, summary, splitinfos] = model.generate_list_of_splits ();
    EXPECT_FALSE (success_splits); // no data!

    auto [success_create, txn] = model.create_transaction();
    EXPECT_FALSE (success_create); // no transaction created.
}

static void dump_acct (Account *acct)
{
    auto bal = GncNumeric(0);
    std::cout << '\n' << std::setw(20) << std::right << xaccAccountGetName (acct)
              << " Bal=" << std::setw(10) << std::right << GncNumeric (xaccAccountGetBalance (acct))
              << std::endl;
    for (auto n = xaccAccountGetSplitList (acct); n; n = n->next)
    {
        auto s = static_cast<Split*>(n->data);
        bal += xaccSplitGetAmount (s);
        std::cout << std::setw(20) << std::right << GncDateTime (xaccTransGetDate (xaccSplitGetParent (s))).format_iso8601()
                  << " amt=" << std::setw(10) << std::right << GncNumeric (xaccSplitGetAmount (s))
                  << " val=" << std::setw(10) << std::right << GncNumeric (xaccSplitGetValue (s))
                  << " bal=" << std::setw(10) << std::right << bal
                  << std::endl;
    }
}

TEST_P(Stock_AssistantTest, DoesStock_Assistant)
{
    test_failure_modes (stock_account);
    for (const auto& t : m_testCase.tests)
    {
        StockAssistantModel model (stock_account);
        model.transaction_date = gnc_dmy2time64 (t.dd, t.mm, t.yy);
        model.maybe_reset_txn_types ();

        model.set_txn_type (t.type_idx);
        model.transaction_description = t.desc;
        model.stock_amount = gnc_numeric_create (t.stock_amt * 100, 100);
        model.stock_value = gnc_numeric_create (t.stock_val, 100);
        model.cash_value = gnc_numeric_create (t.cash_val, 100);
        model.cash_account = cash_account;
        model.fees_account = fees_account;
        model.fees_capitalize = t.capitalize;
        model.fees_value = gnc_numeric_create (t.fees_val, 100);
        model.capgains_account = capgains_account;
        model.capgains_value = gnc_numeric_create (t.capg_val, 100);
        model.dividend_account = dividend_account;
        model.dividend_value = gnc_numeric_create (t.divi_val, 100);

        auto [success_splits, summary, splitinfos] = model.generate_list_of_splits ();
        EXPECT_TRUE (success_splits) << t.dd << '/' << t.mm << '/' << t.yy << ": "
                                     << t.desc << '='
                                     << GncNumeric(xaccAccountGetBalance (stock_account))
                                     << '\n' << summary;

        auto [success_txn, txn] = model.create_transaction ();
        EXPECT_TRUE (success_txn);

        EXPECT_EQ (xaccAccountGetBalance (stock_account).num, t.new_bal * 100);
    }

    dump_acct (stock_account);
    dump_acct (dividend_account);
    dump_acct (capgains_account);
    dump_acct (fees_account);
    dump_acct (cash_account);
    EXPECT_EQ (xaccAccountGetBalance (dividend_account).num, 42000);
    EXPECT_EQ (xaccAccountGetBalance (capgains_account).num, -995981);
    EXPECT_EQ (xaccAccountGetBalance (fees_account).num, 4478);
    EXPECT_EQ (xaccAccountGetBalance (cash_account).num, 1663049);
}

#ifndef INSTANTIATE_TEST_SUITE_P
// Silence "no previous declaration for" (treated as error due to -Werror) when building with GoogleTest < 1.8.1
static testing::internal::ParamGenerator<TestCase*>
gtest_InstantiationStock_AssistantTestStock_AssistantTest_EvalGenerator_();
static std::string gtest_InstantiationStock_AssistantTestStock_AssistantTest_EvalGenerateName_(const testing::TestParamInfo<TestCase*>&);

INSTANTIATE_TEST_CASE_P(
#else // INSTANTIATE_TEST_SUITE_P
INSTANTIATE_TEST_SUITE_P(
#endif // INSTANTIATE_TEST_SUITE_P
    InstantiationStock_AssistantTest,
    Stock_AssistantTest,
    ::testing::Values(
        &easyTestCase
    )
);
