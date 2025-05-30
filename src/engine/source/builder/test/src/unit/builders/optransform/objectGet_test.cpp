#include "builders/baseBuilders_test.hpp"

#include "builders/opmap/opBuilderHelperMap.hpp"

using namespace builder::builders;

namespace
{
template<typename... Refs>
auto customRefExpected(base::Event value, Refs... refs)
{
    return [=](const BuildersMocks& mocks)
    {
        EXPECT_CALL(*mocks.ctx, validator()).Times(testing::AtLeast(1));
        for (auto ref : {refs...})
        {
            EXPECT_CALL(*mocks.validator, hasField(DotPath(ref))).WillOnce(testing::Return(false));
        }

        return value;
    };
}

template<typename... Refs>
auto customRefExpected(Refs... refs)
{
    return [=](const BuildersMocks& mocks)
    {
        EXPECT_CALL(*mocks.ctx, validator()).Times(testing::AtLeast(1));
        for (auto ref : {refs...})
        {
            EXPECT_CALL(*mocks.validator, hasField(DotPath(ref))).WillOnce(testing::Return(false));
        }

        return None {};
    };
}

} // namespace

namespace transformbuildtest
{
INSTANTIATE_TEST_SUITE_P(
    Builders,
    TransformBuilderTest,
    testing::Values(
        /*** Get Value ***/
        TransformT({}, opBuilderHelperGetValue, FAILURE()),
        TransformT({makeValue(R"({})")}, opBuilderHelperGetValue, FAILURE()),
        TransformT({makeRef("ref")}, opBuilderHelperGetValue, FAILURE()),
        TransformT({makeRef("ref"), makeValue(R"({})")}, opBuilderHelperGetValue, FAILURE()),
        TransformT({makeRef("obj"), makeRef("key")}, opBuilderHelperGetValue, SUCCESS(customRefExpected("obj", "key"))),
        TransformT({makeValue(R"({"key": "value"})"), makeRef("ref")},
                   opBuilderHelperGetValue,
                   SUCCESS(customRefExpected("ref"))),
        TransformT({makeRef("obj"), makeRef("key")},
                   opBuilderHelperGetValue,
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        /*** Merge Value ***/
        TransformT({}, opBuilderHelperMergeValue, FAILURE()),
        TransformT({makeValue(R"({})")}, opBuilderHelperMergeValue, FAILURE()),
        TransformT({makeRef("ref")}, opBuilderHelperMergeValue, FAILURE()),
        TransformT({makeRef("ref"), makeValue(R"({})")}, opBuilderHelperMergeValue, FAILURE()),
        TransformT({makeRef("obj"), makeRef("key")},
                   opBuilderHelperMergeValue,
                   SUCCESS(
                       [](const auto& mocks)
                       {
                           customRefExpected("obj", "key")(mocks);
                           EXPECT_CALL(*mocks.validator, hasField(DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT({makeValue(R"({"key": "value"})"), makeRef("ref")},
                   opBuilderHelperMergeValue,
                   SUCCESS(
                       [](const auto& mocks)
                       {
                           customRefExpected("ref")(mocks);
                           EXPECT_CALL(*mocks.validator, hasField(DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT({makeRef("obj"), makeRef("key")},
                   opBuilderHelperMergeValue,
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        /*** Merge Recursive Value ***/
        TransformT({}, opBuilderHelperMergeRecursiveValue, FAILURE()),
        TransformT({makeValue(R"({})")}, opBuilderHelperMergeRecursiveValue, FAILURE()),
        TransformT({makeRef("ref")}, opBuilderHelperMergeRecursiveValue, FAILURE()),
        TransformT({makeRef("ref"), makeValue(R"({})")}, opBuilderHelperMergeRecursiveValue, FAILURE()),
        TransformT({makeRef("obj"), makeRef("key")},
                   opBuilderHelperMergeRecursiveValue,
                   SUCCESS(
                       [](const auto& mocks)
                       {
                           customRefExpected("obj", "key")(mocks);
                           EXPECT_CALL(*mocks.validator, hasField(DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT({makeValue(R"({"key": "value"})"), makeRef("ref")},
                   opBuilderHelperMergeRecursiveValue,
                   SUCCESS(
                       [](const auto& mocks)
                       {
                           customRefExpected("ref")(mocks);
                           EXPECT_CALL(*mocks.validator, hasField(DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT({makeRef("obj"), makeRef("key")},
                   opBuilderHelperMergeRecursiveValue,
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("targetField")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       }))),
    testNameFormatter<TransformBuilderTest>("ObjectGet"));
} // namespace transformbuildtest

namespace transformoperatestest
{
INSTANTIATE_TEST_SUITE_P(
    Builders,
    TransformOperationTest,
    testing::Values(
        /*** Get Value ***/
        TransformT(R"({"ref": "key"})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeValue(R"({"key": "value"})"), makeRef("ref")},
                   SUCCESS(customRefExpected(makeEvent(R"({"ref": "key", "target": "value"})"), "ref"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": "value"}})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   SUCCESS(customRefExpected(
                       makeEvent(R"({"ref1": "key", "ref2": {"key": "value"}, "target": "value"})"), "ref1", "ref2"))),
        TransformT(R"({"ref": "key"})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeValue(R"({"key1": "value"})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref"))),
        TransformT(R"({"ref1": "key", "ref2": {"key1": "value"}})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2"))),
        TransformT(R"({"notRef": "key"})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeValue(R"({"key": "value"})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref"))),
        TransformT(R"({"notRef1": "key", "ref2": {"key": "value"}})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2"))),
        TransformT(R"({"ref1": "key", "notRef2": {"key": "value"}})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": {"a": "value"}}})",
                   opBuilderHelperGetValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target")))
                               .WillOnce(testing::Return(true));
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target.a")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        /*** Merge Value ***/
        TransformT(R"({"ref": "key", "target": {"k0": "v0"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": {"k1": "v1"}})"), makeRef("ref")},
                   SUCCESS(customRefExpected(
                       makeEvent(R"({"ref": "key", "target": {"k0": "v0", "k1": "v1"}})"), "ref", "target"))),
        TransformT(
            R"({"ref1": "key", "ref2": {"key": {"k0": "v0"}}, "target": {"k1": "v1"}})",
            opBuilderHelperMergeValue,
            "target",
            {makeRef("ref2"), makeRef("ref1")},
            SUCCESS(customRefExpected(
                makeEvent(R"({"ref1": "key", "ref2": {"key": {"k0": "v0"}}, "target": {"k1": "v1", "k0": "v0"}})"),
                "ref1",
                "ref2",
                "target"))),
        TransformT(R"({"ref": "key", "target": ["v0"]})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": ["v1"]})"), makeRef("ref")},
                   SUCCESS(customRefExpected(makeEvent(R"({"ref": "key", "target": ["v0", "v1"]})"), "ref", "target"))),
        TransformT(
            R"({"ref1": "key", "ref2": {"key": ["v0"]}, "target": ["v1"]})",
            opBuilderHelperMergeValue,
            "target",
            {makeRef("ref2"), makeRef("ref1")},
            SUCCESS(customRefExpected(makeEvent(R"({"ref1": "key", "ref2": {"key": ["v0"]}, "target": ["v1", "v0"]})"),
                                      "ref1",
                                      "ref2",
                                      "target"))),
        TransformT(R"({"ref": "key", "target": {"k0": "v0"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": ["v1"]})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref", "target"))),
        TransformT(R"({"ref": "key", "target": ["v0"]})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": {"k0": "v0"}})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref", "target"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": ["v1"]}, "target": {"k0": "v0"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2", "target"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": {"k0": "v0"}}, "target": ["v1"]})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2", "target"))),
        TransformT(R"({"notRef": "key", "target": {"k0": "v0"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": {"k1": "v1"}})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref", "target"))),
        TransformT(R"({"notRef": "key", "ref2": {"key": {"k0": "v0"}}, "target": {"k1": "v1"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2", "target"))),
        TransformT(R"({"ref1": "key", "notRef2": {"key": {"k0": "v0"}}, "target": {"k1": "v1"}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2", "target"))),
        TransformT(R"({"ref": "key"})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeValue(R"({"key": {"k1": "v1"}})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref", "target"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": {"k0": "v0"}}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(customRefExpected("ref1", "ref2", "target"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": {"a": "value"}}})",
                   opBuilderHelperMergeValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target")))
                               .WillOnce(testing::Return(true));
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target.a")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT(R"({"ref": "key"})",
                   opBuilderHelperMergeValue,
                   ".",
                   {makeValue(R"({"key": {"key1": "value"}})"), makeRef("ref")},
                   SUCCESS(
                       [](const BuildersMocks& mocks)
                       {
                           customRefExpected("ref", ".")(mocks);
                           return makeEvent(R"({"key1": "value", "ref": "key"})");
                       })),
        /*** Merge Recursive Value ***/
        TransformT(R"({"ref": "key", "target": {"k0": "v0", "nested": {"k1": "v1"}}})",
                   opBuilderHelperMergeRecursiveValue,
                   "target",
                   {makeValue(R"({"key": {"nested": {"k2": "v2"}}})"), makeRef("ref")},
                   SUCCESS(customRefExpected(
                       makeEvent(R"({"ref": "key", "target": {"k0": "v0", "nested": {"k1": "v1", "k2": "v2"}}})"),
                       "ref",
                       "target"))),
        TransformT(R"({"ref": "key-not-exits", "target": {"k0": "v0", "nested": {"k1": "v1"}}})",
                   opBuilderHelperMergeRecursiveValue,
                   "target",
                   {makeValue(R"({"key": {"nested": {"k2": "v2"}}})"), makeRef("ref")},
                   FAILURE(customRefExpected("ref", "target"))),
        TransformT(
            R"({"ref1": "key", "ref2": {"key": {"nested": {"k0": "v0"}}}, "target": {"nested": {"k1": "v1"}}})",
            opBuilderHelperMergeRecursiveValue,
            "target",
            {makeRef("ref2"), makeRef("ref1")},
            SUCCESS(customRefExpected(
                makeEvent(
                    R"({"ref1": "key", "ref2": {"key": {"nested": {"k0": "v0"}}}, "target": {"nested": {"k1": "v1", "k0": "v0"}}})"),
                "ref1",
                "ref2",
                "target"))),
        TransformT(R"({"ref1": "key", "ref2": {"key": {"a": "value"}}})",
                   opBuilderHelperMergeRecursiveValue,
                   "target",
                   {makeRef("ref2"), makeRef("ref1")},
                   FAILURE(
                       [](const auto& mocks)
                       {
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target")))
                               .WillOnce(testing::Return(true));
                           EXPECT_CALL(*mocks.allowedFields, check(testing::_, DotPath("target.a")))
                               .WillOnce(testing::Return(false));
                           return None {};
                       })),
        TransformT(R"({"key1": {"key2": "value"}, "ref": "key"})",
                   opBuilderHelperMergeRecursiveValue,
                   ".",
                   {makeValue(R"({"key": {"key1": {"key3": "value"}}})"), makeRef("ref")},
                   SUCCESS(
                       [](const BuildersMocks& mocks)
                       {
                           customRefExpected("ref", ".")(mocks);
                           return makeEvent(R"({"key1": {"key2": "value", "key3": "value"}, "ref": "key"})");
                       }))),
    testNameFormatter<TransformOperationTest>("ObjectGet"));
} // namespace transformoperatestest
