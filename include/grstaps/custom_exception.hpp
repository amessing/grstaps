/*
 * Copyright (C) 2020 Andrew Messing
 *
 * GTAPS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * GTAPS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GTAPS_CUSTOM_EXCEPTION_HPP
#define GTAPS_CUSTOM_EXCEPTION_HPP

// global
#include <utility>

// external
#include <fmt/format.h>

// local
#include "gtaps/logger.hpp"

namespace gtaps
{
    /**
     * Base class for custom exceptions
     */
    class CustomException : std::exception
    {
    public:
        /**
         * Constructor
         *
         * \param name The name of the exception
         * \param function The function that this exception was thrown from
         * \param file The file that this exception was thrown from
         * \param line The line the exception was thrown on
         * \param message A message associated with the exception with more specific information
         *
         * \tparam Args The arguments to be formated into the message
         *
         * \note Forwards the message using fmt format
         */
        template <typename... Args>
        CustomException(const std::string& name,
                        const std::string& function,
                        const std::string& file,
                        int line,
                        const std::string& message,
                        Args&& ... args)
            : m_message(fmt::format(message, std::forward<Args>(args)...))
        {
            m_message = fmt::format("(<{1:s}> in {2:s} at {3:s}:{4:d}) {0:s} ", m_message, name, function, file, line);
        }

        CustomException(const std::string& message)
            : m_message(message)
        {}

        const char* what() const noexcept override
        {
            return m_message.c_str();
        }

    protected:
        std::string m_message;
    };
}  // namespace gtaps

/**
 * Macro for the information about where an exception occurred
 */
#define E_INFO __FUNCTION__, __FILE__, __LINE__

/**
 * Macro for creating a custom exception
 */
#define EXCEPTION(Err)                                                                                                 \
    class Err : public gtaps::CustomException                                                                          \
    {                                                                                                                  \
       public:                                                                                                         \
        template <typename... Args>                                                                                    \
        Err(const std::string& function = "?",                                                                         \
            const std::string& file     = "?",                                                                         \
            int line                    = -1,                                                                          \
            const std::string& message  = "",                                                                          \
            Args&&... args)                                                                                            \
            : gtaps::CustomException(#Err, function, file, line, message, std::forward<Args>(args)...)                 \
        {}                                                                                                             \
                                                                                                                       \
        Err(const std::string& message)                                                                                \
            : gtaps::CustomException(message)                                                                    \
        {}                                                                                                             \
    };

#ifdef NDEBUG
#    define ERROR(message, ...)                                                                                        \
        std::string formatted_message = fmt::format(message, ##__VA_ARGS__);                                           \
        gtaps::Logger::error("(<CustomException> in {0:s} at {1:s}:{2:d}) {3:s}", E_INFO, formatted_message);          \
        exit(1);
#    define CUSTOM_ERROR(Err, message, ...)
        std::string formatted_message = fmt::format(message, ##__VA_ARGS__);                                           \
        gtaps::Logger::error("(<#Err> in {0:s} at {1:s}:{2:d}) {3:s}", E_INFO, formatted_message);                     \
        exit(1);
#else
#    define ERROR(message, ...)                                                                                        \
        std::string formatted_message = fmt::format(message, ##__VA_ARGS__);                                           \
        std::string error_message = fmt::format("(<CustomException> in {0:s} at {1:s}:{2:d}) {3:s}", E_INFO, formatted_message); \
        gtaps::Logger::error(error_message);                                                                             \
        throw gtaps::CustomException(error_message);
#    define CUSTOM_ERROR(Err, message, ...)                                                                            \
        Err err_(E_INFO, message, ##__VA_ARGS__);                                                                      \
        gtaps::Logger::error(err_.what());                                                                             \
        throw err_;
#endif

#endif  // GTAPS_CUSTOM_EXCEPTION_HPP
