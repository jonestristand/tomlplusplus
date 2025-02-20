// This file is a part of toml++ and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/tomlplusplus/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT

// This example demonstrates a method of merging one TOML data tree into another.

#include "examples.h"
#include <toml++/toml.h>

using namespace std::string_view_literals;

namespace
{
	// recursively merge 'rhs' into 'lhs'.
	//
	// note that this implementation always prioritizes 'rhs' over 'lhs' in the event of a conflict;
	// extending it to handle conflicts in some other manner is left as an exercise to the reader :)

	static void merge(toml::table& lhs, toml::table&& rhs)
	{
		rhs.for_each(
			[&](const toml::key& rhs_key, auto&& rhs_val)
			{
				auto lhs_it = lhs.lower_bound(rhs_key);

				// not found in lhs - direct move
				if (lhs_it == lhs.cend() || lhs_it->first != rhs_key)
				{
					lhs.emplace_hint(lhs_it, rhs_key, std::move(rhs_val));
					return;
				}

				// both nodes were tables -  recurse into them
				if constexpr (toml::is_table<decltype(rhs_val)>)
				{
					if (auto lhs_child_table = lhs_it->second.as_table())
					{
						merge(*lhs_child_table, std::move(rhs_val));
						return;
					}
				}

				// one or both weren't tables - replace lhs with rhs
				lhs.insert_or_assign(rhs_key, std::move(rhs_val));
			});
	}
}

int main(int argc, char** argv)
{
	const auto base_path	  = argc > 1 ? std::string_view{ argv[1] } : "merge_base.toml"sv;
	const auto overrides_path = argc > 2 ? std::string_view{ argv[2] } : "merge_overrides.toml"sv;

	toml::table tbl;
	try
	{
		tbl = toml::parse_file(base_path);
		merge(tbl, toml::parse_file(overrides_path));
	}
	catch (const toml::parse_error& err)
	{
		std::cerr << err << "\n";
		return 1;
	}

	std::cout << tbl << "\n";
	return 0;
}
