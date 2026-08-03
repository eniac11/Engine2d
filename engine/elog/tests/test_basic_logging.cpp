#include <gtest/gtest.h>
#include <engine/elog.h>

ELOG_DECLARE_LOGGING_CATEGORY(cEngine, "engine")
ELOG_DECLARE_LOGGING_CATEGORY(cSpecificLevel, "specificLevel", elog::CriticalMsg)


GTEST_TEST(BasicLogging, basic_debug) {
    std::println(elogDebug(), "Hello World, {}", 1);

}

GTEST_TEST(BasicLogging, basic_info) {
    std::println(elogInfo(), "Hello World, {}", 1);
}

TEST(BasicLogging, StreamOperators) {
    elogInfo() << "Hello World, stream operators" << std::endl;
}

GTEST_TEST(CategoryLogging, info) {
    std::println(elogCInfo(cEngine), "Hello World, {}", 1);
}

GTEST_TEST(CategoryLogging, critical) {
    // Usage: prefix logging line with elogC(Level)Enabled to optimise out the line if at the category compile time level, the log level is disabled.
    elogCCriticalEnabled(cSpecificLevel) std::println(elogCCritical(cSpecificLevel), "Hello World, Specific Critical {}", 1);
    // This is optimised out at higher optimisations. because constexpr
    elogCInfoEnabled(cSpecificLevel) std::println(elogCInfo(cSpecificLevel), "Hello World, Specific Info {}", 1);
}

GTEST_TEST(CategoryLogging, criticalEnvironDisable) {
    // std::string log_rules_before;
    // if (char const* e = std::getenv("ENGINE_LOG_RULES"); e != nullptr) {
    //     log_rules_before = e;
    // }
    ::setenv("ENGINE_LOG_RULES", "specificLevel{critical}=false", 1);
    elog::Elogger::global_logger().reset();
    // Usage: prefix logging line with elogC(Level)Enabled to optimise out the line if at the category compile time level, the log level is disabled.
    elogCCriticalEnabled(cSpecificLevel) std::println(elogCCritical(cSpecificLevel), "Hello World, Specific Critical Not Visiable {}", 1);
    // This is optimised out at higher optimisations. because constexpr
    elogCInfoEnabled(cSpecificLevel) std::println(elogCInfo(cSpecificLevel), "Hello World, Specific Info {}", 1);
    ::unsetenv("ENGINE_LOG_RULES");
    elog::Elogger::global_logger().reset();
}

