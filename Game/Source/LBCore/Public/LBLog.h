#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

// As categorias vivem em LBCore e sao usadas por todos os modulos. Precisam ser
// exportadas da DLL, senao os outros modulos nao linkam (LNK2001).
LBCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLB, Log, All);
LBCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLBDamage, Log, All);
LBCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLBEvents, Log, All);
LBCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLBCombat, Log, All);
LBCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLBMatch, Log, All);
