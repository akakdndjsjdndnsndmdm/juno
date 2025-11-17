#include "constants.hpp"
#include "system_util.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

#include <print>

using namespace lexer;
using namespace parser;

// @brief The entry point of the application.
std::int32_t main( )
{
    std::println( "{} {} ({} {} on {}) {}",
        constants::APP_NAME,
        constants::APP_VERSION,
        constants::APP_COMMIT,
        constants::COMPILER_INFO,
        constants::BUILD_ARCH,
        system_util::get_system_platform(  )
    );

    constexpr std::string_view test_source = "42.4562";

    Lexer lexer { test_source };
    Parser parser { lexer.tokenize(  ) };

    const auto ast { parser.parse( ) };
    for ( auto & stmt : ast )
    {
        if ( const auto * expr_stmt { dynamic_cast< ExpressionStatement* >( stmt.get( ) ) } )
        {
            Expression* expr = expr_stmt->get_expression(  );

            if ( const auto * n { dynamic_cast< Number* >( expr )})
            {
                std::println( "Expression::Number = {}", n->get_value(  ));
            }
        }
    }

    return EXIT_SUCCESS;
}