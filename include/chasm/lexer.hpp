#ifndef CHASM_LEXER_HPP
#define CHASM_LEXER_HPP


#include <string_view>
#include <exception>
#include <variant>
#include <memory>
#include <vector>
#include <format>

#include <chasm/chasm_exception.hpp>
#include <chasm/source_location.hpp>
#include <chasm/stream.hpp>
#include <chasm/arch.hpp>


namespace chasm
{
    enum class token_type
    {
        eof,

		numerical,           // [0-9-a-f-A-F]
        byte_ascii,          // 'A' (quotes included)
        keyword_define,      // define x ...
		keyword_config,      // config x = numerical
		keyword_default,      // config x = default
		keyword_sprite,      // sprite name [.., .., ..]
		keyword_raw,         // raw(...)
		keyword_proc_start,  // proc name
		keyword_proc_end,    // endp name
        identifier,          // constants defined with the "define" keywords, label/proc names and config names
        instruction,         // call, ret, jmp, cls...
		register_name,       // special and general purpose registers
		bracket_open,
		bracket_close,
		parenthesis_open,
		parenthesis_close,
		colon,
		dot_label,
		at_label,
		dollar_proc,
		hash_sprite,
		comma,
		equal
    };

	struct token
    {
        token_type type;
		source_location source_location;
		std::variant<uint16_t, std::string> data;

		[[nodiscard]] std::string to_string() const
		{
			if (std::holds_alternative<uint16_t>(data))
				return std::to_string(std::get<uint16_t>(data));

			return std::get<std::string>(data);
		}

		[[nodiscard]] uint16_t to_integer() const
		{
			return std::get<uint16_t>(data);
		}
    };


    class lexer final
    {
    public:
        explicit lexer(std::string&& buff);

        ~lexer() = default;

        lexer(const lexer&)            = delete;
        lexer(lexer&&)                 = delete;
        lexer& operator=(const lexer&) = delete;
        lexer& operator=(lexer&&)      = delete;

		[[nodiscard]] std::vector<token> enumerate_tokens();

    private:
		[[nodiscard]] token next_token();

        [[nodiscard]]
        char peek_chr() const;
        char next_chr();

        void skip_comment();
        void skip_wspaces();

		[[nodiscard]] token make_token(token_type type, std::string lexeme = {}) const;
		[[nodiscard]] token make_numerical_token(arch::size_type numerical_value) const;

		[[nodiscard]] arch::size_type read_numeric_lexeme();
        [[nodiscard]] std::string     read_alpha_lexeme();

    private:
        chasm::stream istream;
        source_location cursor;
    };


    namespace lexer_exception
    {
        struct invalid_digit_for_base : chasm_exception
        {
            invalid_digit_for_base(char digit, int base, const source_location& source_loc)
                : chasm_exception(
						"Invalid digit \"{}\" for numeric base {} at {}.",
						digit,
						base,
						chasm::to_string(source_loc))
            {}
        };

		struct numeric_constant_too_large : chasm_exception
		{
			numeric_constant_too_large(std::string numeric_lexeme, const source_location& source_loc)
				: chasm_exception(
						"Numeric constant \"{}\" at {} is too large for a 16-bit value.",
						numeric_lexeme,
						chasm::to_string(source_loc))
			{}
		};

        struct undefined_character_token : chasm_exception
        {
            explicit undefined_character_token(char c, const source_location& source_loc)
                : chasm_exception(
						"Character \"{}\" cannot match any token at {}.",
						c,
						chasm::to_string(source_loc))
            {}
        };
    }

	[[nodiscard]]
	constexpr std::string_view to_string(token_type type)
	{
		switch (type)
		{
			case token_type::eof:
				return "eof";
			case token_type::numerical:
				return "numerical";
			case token_type::byte_ascii:
				return "ascii";
			case token_type::keyword_define:
				return "define";
			case token_type::keyword_raw:
				return "raw";
			case token_type::identifier:
				return "identifier";
			case token_type::instruction:
				return "instruction";
			case token_type::register_name:
				return "register name";
			case token_type::bracket_open:
				return "open bracket";
			case token_type::bracket_close:
				return "close bracket";
			case token_type::parenthesis_open:
				return "open parenthesis";
			case token_type::parenthesis_close:
				return "close parenthesis";
			case token_type::colon:
				return "colon";
			case token_type::comma:
				return "comma";
			case token_type::dot_label:
				return "dot";
			case token_type::at_label:
				return "@";
			case token_type::dollar_proc:
				return "$";

			default:
				return "undefined";
		}
	}

	[[nodiscard]]
	inline std::string to_string(std::initializer_list<token_type> types)
	{
		std::string joined;

		for (const auto type : types)
		{
			if (!joined.empty())
				joined += ", ";

			joined += chasm::to_string(type);
		}

		return '(' + joined + ')';
	}
}


#endif //CHASM_LEXER_HPP
