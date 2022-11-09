/*
 * Created by switchblade on 11/5/22.
 */

#include "tests.hpp"

int main(int argc, char *argv[])
{
	if (argc > 1)
		for (int i = 0; i < argc; ++i)
		{
			for (auto [name, func]: tests)
				if (name == argv[i]) func();
		}
	else
		for (auto [_, func]: tests) func();
}