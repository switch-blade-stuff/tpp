/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

/* Define TPP_USE_IMPORT only if modules are enabled and supported by the compiler. */
#if defined(TPP_USE_MODULES) && defined(__cpp_modules)
#define TPP_USE_IMPORT
#endif