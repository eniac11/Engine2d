#include <chrono>
#include <cstdio>
#include <engine/elog.h>
#include <iostream>
#include <ostream>
#include <print>
#include <string>
#include <sys/syslog.h>

#ifdef HAVE_JOURNAL
    #include <syslog.h>
    #define SD_JOURNAL_SUPPRESS_LOCATION
    #include <systemd/sd-journal.h>
#endif // HAVE_JOURNAL


#include "engine/string_utils.h"


namespace elog {
    LogRule::LogRule(std::string const& rule, MessageType const type, bool const enabled) :
        m_type(type), m_enabled(enabled), m_rule(rule) {
        parse();
    }


    bool LogRule::match(std::vector<std::string> const& rule, std::vector<std::string> const& category, std::size_t r,
                        std::size_t c) const {
        // Both exhausted.
        if (r == rule.size() && c == category.size())
            return true;

        // Rule exhausted but category remains.
        if (r == rule.size())
            return false;

        if (rule[r] == "*") {
            // '*' matches zero components.
            if (match(rule, category, r + 1, c))
                return true;

            // '*' matches one or more components.
            if (c < category.size())
                return match(rule, category, r, c + 1);

            return false;
        }

        // No category left.
        if (c == category.size())
            return false;

        if (rule[r] != category[c])
            return false;

        return match(rule, category, r + 1, c + 1);
    }

    bool LogRule::match(std::vector<std::string> const& rule, std::vector<std::string_view> const& category,
                        std::size_t r, std::size_t c) const {
        // Both exhausted.
        if (r == rule.size() && c == category.size())
            return true;

        // Rule exhausted but category remains.
        if (r == rule.size())
            return false;

        if (rule[r] == "*") {
            // '*' matches zero components.
            if (match(rule, category, r + 1, c))
                return true;

            // '*' matches one or more components.
            if (c < category.size())
                return match(rule, category, r, c + 1);

            return false;
        }

        // No category left.
        if (c == category.size())
            return false;

        if (rule[r] != category[c])
            return false;

        return match(rule, category, r + 1, c + 1);
    }

    void LogRule::parse() {
        split(m_rule, '.', m_parts);

        if (m_parts.empty())
            throw std::invalid_argument("Rule is empty");

        for (auto const& part : m_parts) {
            if (part.empty())
                throw std::invalid_argument("Empty category component");

            // '*' is only valid as an entire component.
            // Reject "ab*", "*ab", "**", etc.
            if (part.find('*') != std::string::npos && part != "*")
                throw std::invalid_argument("'*' must be a complete component");
        }
    }

    LogRuleRegistry::LogRuleRegistry() {
        std::string rules;
        if (char const* e = std::getenv("ENGINE_LOG_RULES"); e != nullptr)
            rules = e;
        if (!rules.empty())
            installFilter(rules);
    }

    void LogRuleRegistry::installFilter(std::string const& filterRules) { parse(filterRules); }

    LogRule::CheckStatus LogRuleRegistry::pass(std::string const& category, MessageType type) const {
        for (auto& rule : m_rules) {
            LogRule::CheckStatus status = rule.pass(category, type);
            if (status == LogRule::CheckStatus::Ignore)
                continue;
            if (status == LogRule::CheckStatus::Pass || status == LogRule::CheckStatus::NoPass)
                return status;
        }
        return LogRule::CheckStatus::Ignore;
    }

    LogRule::CheckStatus LogRuleRegistry::pass(std::string_view const& category, MessageType type) const {
        for (auto& rule : m_rules) {
            LogRule::CheckStatus status = rule.pass(category, type);
            if (status == LogRule::CheckStatus::Ignore)
                continue;
            if (status == LogRule::CheckStatus::Pass || status == LogRule::CheckStatus::NoPass)
                return status;
        }
        return LogRule::CheckStatus::Ignore;
    }

    void LogRuleRegistry::parse(std::string const& filter) {
        std::vector<std::string> parts;
        split(filter, ':', parts);
        for (auto const& entry : parts) {
            if (!entry.empty())
                parseFilter(entry);
        }
    }

    // LogRuleRegistry& get_registry() {
    //     static LogRuleRegistry registry;
    //     return registry;
    // }

    void LogRuleRegistry::parseFilter(std::string const& filter) {
        auto open_brace = filter.find('{');
        auto close_brace = filter.find("}");
        auto equals = filter.find('=');

        if (open_brace == std::string_view::npos || close_brace == std::string_view::npos || open_brace > close_brace) {
            throw std::invalid_argument("Invalid log filter: " + std::string(filter));
        }

        std::string category(filter.substr(0, open_brace));
        std::string levels(filter.substr(open_brace + 1, close_brace - 1 - open_brace));
        bool enabled = parseBool(filter.substr(equals + 1));

        auto levelList = expandLevels(levels);

        for (auto level : levelList) {
            m_rules.emplace_back(category, level, enabled);
        }
    }

    std::vector<MessageType> LogRuleRegistry::expandLevels(std::string const& levels) {
        std::vector<MessageType> result;


        // auto inner = levels.substr(1, levels.size() - 2);

        std::vector<std::string> parts;
        split(levels, ',', parts);
        for (auto const& level : parts)
            result.push_back(parseLogType(level));


        return result;
    }

    bool LogRuleRegistry::parseBool(std::string const& value) {
        if (value == "true" || value == "1")
            return true;

        if (value == "false" || value == "0")
            return false;

        throw std::invalid_argument("Invalid boolean value: " + std::string(value));
    }

    MessageType LogRuleRegistry::parseLogType(std::string const& value) {
        if (value == "debug")
            return MessageType::DebugMsg;

        if (value == "error")
            return MessageType::ErrorMsg;

        if (value == "info")
            return MessageType::InfoMsg;

        if (value == "warning")
            return MessageType::WarningMsg;

        if (value == "critical")
            return MessageType::CriticalMsg;

        throw std::invalid_argument("Unknown log level: " + std::string(value));
    }

    LogRule::CheckStatus LogRule::pass(std::string const& category, MessageType type) const {
        if (!has_flag(type, m_type)) {
            return LogRule::CheckStatus::Ignore;
        }

        std::vector<std::string> parts;
        split(category, '.', parts);


        if (!match(m_parts, parts, 0, 0))
            return CheckStatus::Ignore;

        return m_enabled ? CheckStatus::Pass : CheckStatus::NoPass;
    }

    LogRule::CheckStatus LogRule::pass(std::string_view const& category, MessageType type) const {
        if (!has_flag(type, m_type)) {
            return LogRule::CheckStatus::Ignore;
        }

        std::vector<std::string_view> parts;
        split(category, '.', parts);


        if (!match(m_parts, parts, 0, 0))
            return CheckStatus::Ignore;

        return m_enabled ? CheckStatus::Pass : CheckStatus::NoPass;
    }


    int EDebugBuffer::sync() {
        // Elogger::global_logger().write_to_log(m_type, const_cast<MessageContext&>(m_context), str());
        return std::stringbuf::sync();
    }

    MessageLogger::~MessageLogger() {
        Elogger::global_logger().write_to_log(m_buffer.type(), m_context, m_buffer.str());
    }

    std::ostream& MessageLogger::debug() {
        m_buffer.set_type(MessageType::DebugMsg);
        return m_stream;
    }

    std::ostream& MessageLogger::debug(ELogCategory const& category) {
        m_buffer.set_type(MessageType::DebugMsg);
        if (!category.is_debug_enabled()) {
            m_buffer.m_message_output = false;
        }
        m_context.category = category.category_name();
        m_context.colour = category.colour();
        return m_stream;
    }

    std::ostream& MessageLogger::info() {
        m_buffer.set_type(MessageType::InfoMsg);
        return m_stream;
    }

    std::ostream& MessageLogger::info(ELogCategory const& category) {
        m_buffer.set_type(MessageType::InfoMsg);
        if (!category.is_info_enabled()) {
            m_buffer.m_message_output = false;
        }
        m_context.category = category.category_name();
        m_context.colour = category.colour();
        return m_stream;
    }

    std::ostream& MessageLogger::warning() {
        m_buffer.set_type(MessageType::WarningMsg);
        return m_stream;
    }

    std::ostream& MessageLogger::warning(ELogCategory const& category) {
        m_buffer.set_type(MessageType::WarningMsg);
        if (!category.is_warning_enabled()) {
            m_buffer.m_message_output = false;
        }
        m_context.category = category.category_name();
        m_context.colour = category.colour();
        return m_stream;
    }

    std::ostream& MessageLogger::error() {
        m_buffer.set_type(MessageType::ErrorMsg);
        return m_stream;
    }

    std::ostream& MessageLogger::error(ELogCategory const& category) {
        m_buffer.set_type(MessageType::ErrorMsg);
        if (!category.is_error_enabled()) {
            m_buffer.m_message_output = false;
        }
        m_context.category = category.category_name();
        m_context.colour = category.colour();
        return m_stream;
    }

    std::ostream& MessageLogger::critical() {
        m_buffer.set_type(MessageType::CriticalMsg);
        return m_stream;
    }

    std::ostream& MessageLogger::critical(ELogCategory const& category) {
        m_buffer.set_type(MessageType::CriticalMsg);
        if (!category.is_critical_enabled()) {
            m_buffer.m_message_output = false;
        }
        m_context.category = category.category_name();
        m_context.colour = category.colour();
        return m_stream;
    }

    Elogger Elogger::g_logger{};
    static std::mutex global_logger_mutex;

    class Elogger::EloggerPrivate {
            friend class Elogger;
            LogRuleRegistry m_registry;
            std::ostream* m_stdout_stream = &std::cout;
            std::ostream* m_stderr_stream = &std::cerr;
    };

    Elogger::Elogger() : d(new EloggerPrivate()) {
        using namespace std::placeholders;
        auto boundFunc = [this](MessageType t, MessageContext& c, std::string s) {
            this->default_log_handler(t, c, std::move(s));
        };
        m_handlers.push_back(boundFunc);
    }

    void Elogger::reset() {
        std::lock_guard<std::mutex> lck(global_logger_mutex);
        delete d;
        d = new EloggerPrivate();
    }

    void Elogger::install_filter_rules(std::string const& rules) const {
        std::lock_guard<std::mutex> lck(lck_log);
        d->m_registry.installFilter(rules);
    }

    void Elogger::install_log_handler(loghandler const& loghandler) { m_handlers.push_back(loghandler); }

    void Elogger::set_output_stream(std::ostream* stream) const { d->m_stderr_stream = stream; }

    LogRuleRegistry const& Elogger::registry() {
        std::lock_guard<std::mutex> lck(lck_log);
        return d->m_registry;
    }


    void Elogger::write_to_log(MessageType msgtype, MessageContext& ctx, std::string msg) const {
        std::lock_guard<std::mutex> guard(lck_log);
        std::lock_guard<std::mutex> gbl_guard(global_logger_mutex);
        auto flag = Flags(d->m_registry.pass(ctx.category, msgtype));
        if (has_flag(flag, LogRule::CheckStatus::Pass) || has_flag(flag, LogRule::CheckStatus::Ignore)) {
            for (auto const& handler : m_handlers) {
                std::invoke(handler, msgtype, ctx, msg);
            }
        }
    }

    Elogger& Elogger::global_logger() { return g_logger; }

    void Elogger::default_log_handler(MessageType msgtype, MessageContext& ctx, std::string msg) {
        std::string const stype = message_type_to_string(msgtype);
        std::ostream* strm = d->m_stdout_stream;
        if (msgtype == MessageType::DebugMsg || msgtype == MessageType::CriticalMsg ||
            msgtype == MessageType::FatalMsg) {
            strm = d->m_stderr_stream;
        }
        std::chrono::system_clock::time_point const time = std::chrono::system_clock::now();
        std::chrono::zoned_time t_with_tz{std::chrono::current_zone(), time};
        std::string colour_ansi_code;
        if (ctx.colour != white)

            colour_ansi_code = std::format("\e[38;2;{0};{1};{2}m", ctx.colour.r, ctx.colour.g, ctx.colour.b);

        std::string const formatted_log_output = std::format("{5}{0}: [{2}/{4}] {3}{6}", ctx.category, t_with_tz, stype,
                                                             msg, ctx.function, colour_ansi_code, "\e[0m");


        *strm << formatted_log_output;
    }

    namespace handlers {

#ifdef HAVE_JOURNAL
        int get_journal_priority(MessageType msgtype) {
            switch (msgtype) {
                case InfoMsg:
                    return LOG_INFO;
                case WarningMsg:
                    return LOG_WARNING;
                case ErrorMsg:
                    return LOG_ERR;
                case DebugMsg:
                    return LOG_DEBUG;
                case CriticalMsg:
                    return LOG_CRIT;
                case FatalMsg:
                    return LOG_CRIT;
            }
        }

        void sd_journal_handler(MessageType msgtype, MessageContext& ctx, std::string msg) {
            int priority = get_journal_priority(msgtype);
            std::string line = std::to_string(ctx.line);
            std::string code_file = std::format("CODE_FILE={}", ctx.file);
            std::string code_line = std::format("CODE_LINE={}", ctx.line);
            int r = sd_journal_send_with_location(code_file.c_str(), code_line.c_str(), ctx.function.c_str(),
                                                  "CATEGORY=%s", ctx.category.c_str(), "MESSAGE=%s", msg.c_str(),
                                                  "PRIORITY=%i", priority, nullptr);
            if (r < 0)
                std::cerr << "Systemd Log" << ::strerror(-r) << std::endl;
        }
#endif // HAVE_JOURNAL

    } // namespace handlers
} // namespace elog
