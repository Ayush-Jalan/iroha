/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <boost/variant.hpp>
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/permissions.hpp"
#include "interfaces/query_responses/error_responses/stateful_failed_error_response.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

static constexpr iroha::StorageType storage_types[] = {
    iroha::StorageType::kPostgres, iroha::StorageType::kRocksDb};

/**
 * TODO mboldyrev 18.01.2019 IR-228 "Basic" tests should be replaced with a
 * common acceptance test
 *
 * C369 Get role permissions by user with allowed GetRoles permission
 * @given a user with kGetRoles permission
 * @when the user send query with getRolePermissions request
 * @then there is a valid RolePermissionsResponse
 */
TEST_F(AcceptanceFixture, CanGetRolePermissions) {
  for (auto const type : storage_types) {
    auto check_query = [](auto &query_response) {
      ASSERT_NO_THROW(
          boost::get<const shared_model::interface::RolePermissionsResponse &>(
              query_response.get()));
    };

    auto query = complete(baseQry().getRolePermissions(kRole));

    IntegrationTestFramework(1, type)
        .setInitialState(kAdminKeypair)
        .sendTxAwait(
            makeUserWithPerms(
                {shared_model::interface::permissions::Role::kGetRoles}),
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
        .sendQuery(query, check_query);
  }
}

/**
 * TODO mboldyrev 18.01.2019 IR-213 remove, covered by
 * postgres_query_executor_test GetRolePermsExecutorTest.Invalid
 *
 * C370 Get role permissions without allowed GetRoles permission
 * @given a user without kGetRoles permission
 * @when the user send query with getRolePermissions request
 * @then query should be recognized as stateful invalid
 */
TEST_F(AcceptanceFixture, CanNotGetRolePermissions) {
  for (auto const type : storage_types) {
    auto query = complete(baseQry().getRolePermissions(kRole));

    IntegrationTestFramework(1, type)
        .setInitialState(kAdminKeypair)
        .sendTxAwait(
            makeUserWithPerms({}),
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
        .sendQuery(query,
                   checkQueryErrorResponse<
                       shared_model::interface::StatefulFailedErrorResponse>());
  }
}
