#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <ostream>
#include <source_location>
#include <sstream>
#include <vector>
#include <version>

#include <engine/elogConfig.h>
#include "enum_utils.h"

namespace elog {
    enum MessageType : std::uint32_t {
        InfoMsg = 1 << 0,
        WarningMsg = 1 << 1,
        ErrorMsg = 1 << 2,
        DebugMsg = 1 << 3,
        CriticalMsg = 1 << 4,
        FatalMsg = 1 << 5,
    };

    constexpr std::string message_type_to_string(MessageType const type) {
        switch (type) {
            case DebugMsg:
                return "Debug";
                break;
            case WarningMsg:
                return "Warning";
                break;
            case ErrorMsg:
                return "Error";
                break;
            case CriticalMsg:
                return "Critical";
                break;
            case FatalMsg:
                return "Fatal";
                break;
            case InfoMsg:
                return "Info";
                break;
        }
        return "UnknownLogMsg";
    }


    class LogRule {
        public:
            enum class CheckStatus { NoPass, Ignore, Pass };

            LogRule(std::string const& rule, MessageType const type, bool const enabled);

            /**
             * Parse and check category against rule.
             *
             * Syntax:
             *
             *   rules can contain wildcards which will match in anything inbetween.
             *
             *   Grammar:
             *   @code
             *     ruledef -> <rule>:<messagetype> EQ <bool>
             *     rule -> category | category.<wildcard>
             *   @endcode
             *
             */
            CheckStatus pass(std::string const& category, MessageType type) const;
            CheckStatus pass(std::string_view const& category, MessageType type) const;

            bool enabled() const { return m_enabled; }
            MessageType type() const { return m_type; }
            std::string const& rule() const { return m_rule; }

        private:
            bool match(std::vector<std::string> const& rule, std::vector<std::string> const& category, std::size_t r,
                       std::size_t c) const;
            bool match(std::vector<std::string> const& rule, std::vector<std::string_view> const& category,
                       std::size_t r, std::size_t c) const;
            void parse();
            MessageType m_type;
            bool m_enabled;
            std::string m_rule;
            std::vector<std::string> m_parts;
    };

    class LogRuleRegistry {
        public:
            LogRuleRegistry();
            void installFilter(std::string const& filterRules);
            LogRule::CheckStatus pass(std::string const& category, MessageType type) const;
            LogRule::CheckStatus pass(std::string_view const& category, MessageType type) const;

            std::vector<LogRule> const& rules() const { return m_rules; }

        private:
            void parse(std::string const& filter);
            void parseFilter(std::string const& filter);
            std::vector<MessageType> expandLevels(std::string const& levels);
            bool parseBool(std::string const& value);
            MessageType parseLogType(std::string const& value);
            std::vector<LogRule> m_rules;
    };

    LogRuleRegistry& get_registry();


    // class ELogFilter {
    //     using vectype = std::vector<LogRule>;
    //
    //     public:
    //         using const_iterator = vectype::const_iterator;
    //         using iterator = vectype::iterator;
    //
    //         void parse(std::string const& filter);
    //
    //         const_iterator cbegin() const {
    //             return m_rules.cbegin();
    //         }
    //
    //         const_iterator cend() const { return m_rules.cend(); }
    //         iterator begin() { return m_rules.begin(); }
    //         iterator end() { return m_rules.end(); }
    //
    //     private:
    //         vectype m_rules;
    // };
    struct Colour {
        std::uint8_t r{};
        std::uint8_t g{};
        std::uint8_t b{};
        bool operator<=>(Colour const&) const = default;
    };

    inline Colour white{255, 255, 255};
    inline Colour green{0, 255, 0};

    class ELogCategory {
        public:
            constexpr ELogCategory(std::string category_name, Colour colour = white) :
                m_name(std::move(category_name)),
                m_type(static_cast<MessageType>(InfoMsg | CriticalMsg | WarningMsg | DebugMsg)), m_colour(colour) {
            }

            constexpr ELogCategory(std::string category_name, MessageType const type, Colour colour = white) :
                m_name(std::move(category_name)), m_type(type), m_colour(colour) {
            }


            constexpr bool is_debug_enabled() const { return has_flag(m_type, elog::MessageType::DebugMsg); }
            constexpr bool is_warning_enabled() const { return has_flag(m_type, WarningMsg); }
            constexpr bool is_error_enabled() const { return has_flag(m_type, WarningMsg); }
            constexpr bool is_critical_enabled() const { return has_flag(m_type, CriticalMsg); }
            constexpr bool is_info_enabled() const { return has_flag(m_type, InfoMsg); }

            constexpr std::string const& category_name() const { return m_name; }
            constexpr Colour const& colour() const { return m_colour; }

        private:
            std::string m_name;
            MessageType const m_type;
            Colour const m_colour;
    };


    struct MessageContext {
        // Qt deletes copy constructor because string types are pointers but using string-views here so should be
        // copyable.
        constexpr MessageContext() noexcept = default;

        constexpr MessageContext(int lineNumber, std::string const& fileName, std::string const& functionName,
                                 std::string categoryName, Colour const& colour) noexcept :
            line(lineNumber), file(fileName), function(functionName), category(std::move(categoryName)),
            colour(colour) {
        }

        int line{}; // Line number
        std::string file{}; // File name
        std::string function{}; // Function name
        std::string category{}; // Logging category
        Colour colour = white;
    };

    class EDebugBuffer : public std::stringbuf {
        friend class MessageLogger;

        public:
            explicit EDebugBuffer(MessageContext const& context) : m_context(context), m_type(MessageType::DebugMsg) {
            }


            MessageType type() const { return m_type; }
            MessageContext const& context() const { return m_context; }
            void set_type(const MessageType type) { m_type = type; }
            void set_message_output(const bool enabled) { m_message_output = enabled; }

        protected:
            int sync() override;

        private:
            bool m_message_output = true;
            MessageContext const& m_context;
            MessageType m_type;
    };


    class MessageLogger {
        public:
            MessageLogger() : m_context(), m_buffer(m_context), m_stream(&m_buffer) {
            }

            MessageLogger(std::string const& file, const int line, std::string const& function,
                          Colour const& colour = white) :
                m_context(line, file, function, "default", colour), m_buffer(m_context), m_stream(&m_buffer) {
            }

            MessageLogger(std::string const& file, const int line, std::string const& function, std::string category,
                          Colour const& colour = white) :
                m_context(line, file, function, std::move(category), colour), m_buffer(m_context), m_stream(&m_buffer) {
            }
            #if defined(ELOG_USE_SOURCE_LOCATION) && __cpp_lib_source_location >= 201907L

            explicit MessageLogger(std::source_location location, Colour const& colour = white) : m_context(location.line(),
                    location.file_name(), location.function_name(), "default", colour), m_buffer(m_context),
                m_stream(&m_buffer) {
            }
            explicit MessageLogger(std::source_location const& location, std::string category, Colour const& colour = white) : m_context(location.line(),
                    location.file_name(), location.function_name(), std::move(category), colour), m_buffer(m_context),
                m_stream(&m_buffer) {
            };
            #endif
            ~MessageLogger();

            std::ostream& debug();
            std::ostream& debug(ELogCategory const& category);
            std::ostream& info();
            std::ostream& info(ELogCategory const& category);
            std::ostream& warning();
            std::ostream& warning(ELogCategory const& category);
            std::ostream& error();
            std::ostream& error(ELogCategory const& category);
            std::ostream& critical();
            std::ostream& critical(ELogCategory const& category);

        private:
            MessageContext m_context;
            EDebugBuffer m_buffer;
            std::ostream m_stream;
    };

    class Elogger {
        class EloggerPrivate;
        mutable std::recursive_mutex lck_log;

        public:
            using loghandler = std::function<void(MessageType, MessageContext&, std::string)>;

            Elogger();
            void reset();
            void install_filter_rules(std::string const& rules) const;

            void set_output_stream(std::ostream* stream) const;
            LogRuleRegistry const& registry();


            void install_log_handler(loghandler const& loghandler);

            void write_to_log(MessageType msgtype, MessageContext& ctx, std::string msg) const;

            static Elogger& global_logger();

        protected:
            void default_log_handler(MessageType msgtype, MessageContext& ctx, std::string msg);

        private:
            std::vector<loghandler> m_handlers;
            EloggerPrivate* d;
            static Elogger g_logger;
    };

    namespace handlers {
        #ifdef HAVE_JOURNAL
        void sd_journal_handler(MessageType msgtype, MessageContext& ctx, std::string msg);
        #endif // HAVE_JOURNAL
    } // namespace handlers
} // namespace elog

using ElogMessageTypeFlags = Flags<elog::MessageType>;

template <>
struct FlagTraits<elog::MessageType> {
    static constexpr bool const isBitmask = true;
    static constexpr ElogMessageTypeFlags allFlags = ElogMessageTypeFlags(
        elog::MessageType::CriticalMsg | elog::MessageType::WarningMsg | elog::MessageType::InfoMsg);
};

#define ELOG_DECLARE_LOGGING_CATEGORY(category, name, args...)                                                         \
    class category##Class : public elog::ELogCategory {                                                                \
        public:                                                                                                        \
            constexpr category##Class() : ELogCategory(std::string(name), ##args) {}                                   \
    };                                                                                                                 \
    constexpr category##Class const& category() {                                                                      \
        static category##Class clz{};                                                                                  \
        return clz;                                                                                                    \
    }
#if !defined(ELOG_USE_SOURCE_LOCATION)
#define elogInfo() elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).info()
#define elogWarning() elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).warning()
#define elogError() elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).error()
#define elogDebug() elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).debug()
#define elogCritical() elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).critical()

#define elogCInfoEnabled(category)                                                                                     \
    for (bool category_enabled = category().is_info_enabled(); category_enabled; category_enabled = false)
#define elogCWarningEnabled(category)                                                                                  \
    for (bool category_enabled = category().is_warning_enabled(); category_enabled; category_enabled = false)
#define elogCErrorEnabled(category)                                                                                    \
    for (bool category_enabled = category().is_error_enabled(); category_enabled; category_enabled = false)
#define elogCCriticalEnabled(category)                                                                                 \
    for (bool category_enabled = category().is_critical_enabled(); category_enabled; category_enabled = false)
#if defined(DEBUG)
#define elogCDebugEnabled(category)                                                                                \
        for (bool category_enabled = category().is_debug_enabled(); category_enabled; category_enabled = false)
#else
#define elogCDebugEnabled(category) for (; false;)
#endif


#define elogCInfo(category) elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).info(category())
#define elogCWarning(category) elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).warning(category())
#define elogCError(category) elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).error(category())
#define elogCDebug(category) elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).debug(category())
#define elogCCritical(category) elog::MessageLogger(__FILE__, __LINE__, __FUNCTION__).critical(category())
#elif defined(ELOG_DECLARE_LOGGING_CATEGORY) && __cpp_lib_source_location >= 201907L
#define elogInfo() elog::MessageLogger(std::source_location::current()).info()
#define elogWarning() elog::MessageLogger(std::source_location::current()).warning()
#define elogError() elog::MessageLogger(std::source_location::current()).error()
#define elogDebug() elog::MessageLogger(std::source_location::current()).debug()
#define elogCritical() elog::MessageLogger(std::source_location::current()).critical()

#define elogCInfoEnabled(category)                                                                                     \
    for (bool category_enabled = category().is_info_enabled(); category_enabled; category_enabled = false)
#define elogCWarningEnabled(category)                                                                                  \
    for (bool category_enabled = category().is_warning_enabled(); category_enabled; category_enabled = false)
#define elogCErrorEnabled(category)                                                                                    \
    for (bool category_enabled = category().is_error_enabled(); category_enabled; category_enabled = false)
#define elogCCriticalEnabled(category)                                                                                 \
    for (bool category_enabled = category().is_critical_enabled(); category_enabled; category_enabled = false)
#if defined(DEBUG)
#define elogCDebugEnabled(category)                                                                                \
        for (bool category_enabled = category().is_debug_enabled(); category_enabled; category_enabled = false)
#else
#define elogCDebugEnabled(category) for (; false;)
#endif


#define elogCInfo(category) elog::MessageLogger(std::source_location::current()).info(category())
#define elogCWarning(category) elog::MessageLogger(std::source_location::current()).warning(category())
#define elogCError(category) elog::MessageLogger(std::source_location::current()).error(category())
#define elogCDebug(category) elog::MessageLogger(std::source_location::current()).debug(category())
#define elogCCritical(category) elog::MessageLogger(std::source_location::current()).critical(category())
#endif