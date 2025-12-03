#pragma once

// GCC Headers
#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "cgraph.h"
#include "plugin.h"
#include "diagnostic.h"
#include "langhooks.h"
#include "context.h"
#include "gimple.h"
#include "stringpool.h"
#include "vec.h"
#include "hash-map.h"
#include "tree-iterator.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "tree-ssa.h"
#include "print-tree.h"

// Standard C++ Headers usually needed with GCC plugins
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
